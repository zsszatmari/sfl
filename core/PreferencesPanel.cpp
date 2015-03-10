//
//  PreferencesPanel.cpp
//  G-Ear core
//
//  Created by Zsolt Szatmári on 8/13/13.
//  Copyright (c) 2013 Treasure Box. All rights reserved.
//

#include "Environment.h"

#ifdef TARGET_OS_IPHONE
#ifdef PREFERENCESPANEL_MM
#define PREFERENCESPANEL_OK
#include "InAppStore.h"
#endif
#include "AppleSilenceWorker.h"
#endif

#ifndef PREFERENCESPANEL_MM
#ifndef TARGET_OS_IPHONE
#define PREFERENCESPANEL_OK
#endif
#endif

#ifdef PREFERENCESPANEL_OK

//#define DISABLE_INAPP

#include "PreferencesPanel.h"
#include "PreferenceGroup.h"
#include "Preference.h"
#include "GearUtility.h"
#include "ServiceManager.h"
#include "IApp.h"
#include "IService.h"
#include "LicenseManager.h"
#include "CoreDebug.h"
#include "LastFmController.h"
#include "RemoteControl.h"
#include "IPreferences.h"
#include "ThemeManager.h"
#include "sfl/Map.h"
#include "sfl/Vector.h"
#include "sfl/Maybe.h"
#include "IPlaybackWorker.h"

namespace Gear
{
#define method PreferencesPanel::
    
    using namespace sfl;
    
#ifdef OPTION_DISABLE_INAPP
    static bool inAppDisabled = false;
    using std::string;
    
    void IPreferencesPanel::disableInApp()
    {
        inAppDisabled = true;
    }
#else
#ifdef DEBUG_INAPP_ENABLE_EVERYTHING
#define inAppDisabled true
#else
#define inAppDisabled false
#endif
#endif
    
#if TARGET_IPHONE_SIMULATOR
#define inAppDisabled true
#endif
    
    static inline bool available(const IService &service)
    {
#if TARGET_OS_IPHONE && !defined(DISABLE_INAPP)
        if (inAppDisabled) {
            static bool alreadyIssued = false;
            if (!alreadyIssued) {
                NSLog(@"Warning!!!!?!!!");
                alreadyIssued = true;
            }
            return true;
        }
        return !service.inAppNecessary() || [[InAppStore sharedStore] isAvailable:convertString("com.treasurebox.gear.ios." + service.inAppIdentifier())];
#endif
        return true;
    }
    
    static inline bool purchaseInProgress(const IService &service)
    {
#if TARGET_OS_IPHONE
        return service.inAppNecessary() && [[InAppStore sharedStore] isInProgress:convertString("com.treasurebox.gear.ios." + service.inAppIdentifier())];
#endif
        return false;
    }
    
    static inline void tryPurchase(IService &service)
    {
#if TARGET_OS_IPHONE
        [[InAppStore sharedStore] tryPurchase:convertString("com.treasurebox.gear.ios." + service.inAppIdentifier()) success:^{
            // we need the original object here, and ARC just gives it us (counterintiuitively)
            service.connect();
         }];
#endif
    }
    
    void method checkService(const Gear::IService &service, const function<void()> &success , const function<void()> &fail)
    {
        if (inAppDisabled) {
            success();
            return;
        }
#if TARGET_OS_IPHONE
        checkLicense(service, success, fail);
#else
        success();
#endif
    }
    
    
    bool method checkImmediately(const Gear::IService &service)
    {
        return available(service);
        /*
        if (inAppDisabled) {
            return true;
        }
#if TARGET_OS_IPHONE
        return checkLicenseImmediately(service);
#else
        return true;
#endif*/
    }
    
    static inline void restorePurchases()
    {
#if TARGET_OS_IPHONE
        [[InAppStore sharedStore] restorePurchases];
#endif
    }
    
    vector<PreferenceGroup> method preferenceGroups() const
    {
    	vector<PreferenceGroup> ret;
    	{
    		vector<Preference> serviceSwitches;
    		auto services = IApp::instance()->serviceManager()->services();
    		for (auto it = services.begin() ; it != services.end() ; ++it) {
    			auto pService = *it;
    			Preference pref(Preference::Type::OnOff, pService->title(), [pService](int64_t value){
    				if (value == static_cast<int>(Preference::OnOffState::On)) {
                        
                        if (inAppDisabled) {
                            pService->connect();
                            return;
                        }
                        
#ifndef DISABLE_INAPP
                        checkLicense(*pService, [&]{
                            pService->connect();
                        },[&]{
                            tryPurchase(*pService);
                        });
#else
                        pService->connect();
#endif
    				} else {
    					pService->disconnect();
    				}
    			}, [pService]{
                    return static_cast<int64_t>([&]{
                        
                        if (pService->disabled()) {
                            return Preference::OnOffState::Disabled;
                        }
                        
                        if (purchaseInProgress(*pService)) {
                            return Preference::OnOffState::OnSpinning;
                        }
                        if (!available(*pService)) {
                            return Preference::OnOffState::Inactive;
                        } 
                        
                        auto state = pService->state();
                        switch(state) {
                            case IService::State::Offline:
                                return Preference::OnOffState::Off;
                            case IService::State::Online:
                                return Preference::OnOffState::On;
                            case IService::State::Connecting:
                                return Preference::OnOffState::OnSpinning;
                        }
                    }());
                });
    			serviceSwitches.push_back(pref);
    		}
            
            // serviceSwitches.push_back(Preference(Preference::Type::Text, "", [](int64_t value){}));

#if TARGET_OS_IPHONE
            serviceSwitches.push_back(Preference(Preference::Type::Special, "Last.fm", [](int64_t value){}, []()->int64_t{
                return IApp::instance()->lastFmController()->enabledConnector().value();
            }).setKey("lastfm"));
            serviceSwitches.push_back(Preference(Preference::Type::OnOff, "Remote Control", [](int64_t value){
                if (value) { 
                    IApp::instance()->showRemoteConnectPanel();
                }
            }, []()->int64_t{
                return IApp::instance()->remoteControl()->controlling();
            }));
#endif
            ret.push_back(PreferenceGroup("Services", serviceSwitches));
            
#if TARGET_OS_IPHONE
            {
                vector<Preference> extra;
                extra.push_back(Preference(Preference::Type::Button, "Restore Purchases", [](int64_t value){
                    if (value) {
                        restorePurchases();
                    }
                }));
                
                ret.push_back(PreferenceGroup("", extra));
            }
            {
                vector<Preference> extra;

                auto themes = Gui::ThemeManager::themes();
                auto themeNames = sfl::map(&fst<std::string,shared_ptr<Gui::ITheme>>, themes);
                extra.push_back(Preference(Preference::Type::Options, "Theme", [themeNames](int64_t value){
                    IApp::instance()->preferences().setStringForKey("Theme", maybe("",maybeAt(themeNames,value)));
                    IApp::instance()->themeManager()->refresh();
                }, [themeNames]{
                    return maybe(0,elemIndex(IApp::instance()->preferences().stringForKey("Theme"),themeNames));
                }, themeNames));
                
            /*    ret.push_back(PreferenceGroup("", extra));
            }
            {
                vector<Preference> extra;
            */
                auto playbackWorker = IApp::instance()->playbackWorker();
                auto availableAll = sfl::partition([](const AudioDevice &d)->bool{return d.deviceId() != AppleSilenceWorker::deviceId();}, playbackWorker->availableDevices());
                auto available = availableAll.first;
                auto silence = availableAll.second;

                auto isDefault = [](const AudioDevice &d){return d.deviceId() == 0;};
                extra.push_back(Preference(Preference::Type::Options, "ChromeCast", [available,playbackWorker,silence,isDefault](int64_t value){
                    auto device = available.at(value);
                    playbackWorker->setDevices(Vector::singleton(device) + filter([&](const AudioDevice &){return !isDefault(device);}, silence));
                }, [available,playbackWorker]{
                    auto current = playbackWorker->devices().at(0);
                    return maybe((size_t)0, elemIndex(current, available));
                }, sfl::map([&](const AudioDevice &d){if (isDefault(d)) return std::string("Off"); else return d.name();}, available)));
                
                ret.push_back(PreferenceGroup("", extra));
            }

            vector<Preference> preferences;
            preferences.push_back(Preference(Preference::Type::OnOff, "Enable 1-song looping", "SingleSongLoopPossible"));
            preferences.push_back(Preference(Preference::Type::OnOff, "Show Album Artists Tab", "ShowAlbumArtistsTab"));
            preferences.push_back(Preference(Preference::Type::OnOff, "Send anonymous usage", "SendUsageStatistics"));
            preferences.push_back(Preference(Preference::Type::OnOff, "Alternative album sort", "ForceSortByAlbumName"));
            preferences.push_back(Preference(Preference::Type::OnOff, "Disable screen lock", [](int64_t value){
                IApp::instance()->preferences().setUintForKey("DisableAutoLock", value);
                [UIApplication sharedApplication].idleTimerDisabled = value;
            }, []{
                bool value = IApp::instance()->preferences().boolForKey("DisableAutoLock");
                [UIApplication sharedApplication].idleTimerDisabled = value;
                return value;
            }));
            ret.push_back(PreferenceGroup("Preferences", preferences));
#endif
    	}
    	return ret;
    }
}

#endif
