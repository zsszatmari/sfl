//
//  App.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#include <iostream>
#include "App.h"
#import "AppDelegate.h"
#include "CocoaTouchWebWindow.h"
#include "StringUtility.h"
#include "AppleFileManager.h"
#include "LocalSessionImpl.h"
#include "JavascriptEngineIos.h"
#include "PlaybackWorker.h"
#include "AudioDeviceManager.h"
#include "AudioDeviceUnionWorker.h"
#include "Payload.h"
#include "constr_TYPE.h"
#import "SongsViewController.h"
#include "AppleSilenceWorker.h"

namespace Gear
{
#define method App::
    
    method App()
    {
        [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent animated:NO];
    }
    
    shared_ptr<IApp> & method instance()
    {
        static shared_ptr<IApp> app(new App());
        return app;
    }
    
    void method jumpToPlaylist(const shared_ptr<IPlaylist> &playlist, bool start)
    {
        // TODO
        
        /*
        auto f = [playlist, start]{
            [[AppDelegate sharedDelegate] setSelectedPlaylist:[[PlaylistWrapper alloc] initWithPlaylist:playlist]];
            
            if (start) {
                [[AppDelegate sharedDelegate].mainWindowController playFirst];
            }
        };
        
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.1f * NSEC_PER_SEC);
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            f();
        });*/
    }
    
    void method editPlaylistName(const shared_ptr<IPlaylist> &playlist)
    {
        //[[[[AppDelegate sharedDelegate] mainWindowController] playlistManager] startEditing:playlist];
    }
    
    IPreferences & method preferences()
    {
        return _preferences;
    }
    
    void method reloadPlaylists()
    {
        // TODO
    }
    
    shared_ptr<IWebWindow> method createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate)
    {
        return shared_ptr<IWebWindow>(new CocoaTouchWebWindow(delegate));
    }
    
    void method showUserMessage(const UserMessage message)
    {
        // TODO
    }
    
    string method dataPath() const
    {
        NSError *err = nil;
        NSURL *appSupportDirectory = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&err];
        return convertString([appSupportDirectory path]);
    }
    
    string method imageCacheDirectory() const
    {
        NSError *err = nil;
        NSURL *cacheDirectory = [[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&err];
        NSURL *imageDir = [cacheDirectory URLByAppendingPathComponent:@"art" isDirectory:YES];
        [[NSFileManager defaultManager] createDirectoryAtPath:[imageDir path] withIntermediateDirectories:YES attributes:nil error:nil];
        
        return convertString([imageDir path]);
    }
    
    shared_ptr<IFileManager> method fileManager() const
    {
        return shared_ptr<AppleFileManager>(new AppleFileManager());
    }
    
    shared_ptr<LocalSession> method createLocalSession()
    {
        return shared_ptr<LocalSession>(/*new LocalSessionImpl(shared_from_this()*/);
    }
    
    shared_ptr<IJavascriptEngine> method createJavascriptEngine() const
    {
        return shared_ptr<IJavascriptEngine>(new JavascriptEngineIos());
    }
    
    shared_ptr<IPlaybackWorker> method playbackWorker()
    {
        static auto worker = PlaybackWorker::create();
        static auto silence = AppleSilenceWorker::create();
        static auto unionWorker = AudioDeviceUnionWorker::create({worker, AppBase::playbackWorker(),silence});
        return unionWorker;
        //return worker;
    }
    
    string method trackingId()
    {
        return "UA-34360192-3";
    }
    
    string method trackingClientId()
    {
        @autoreleasepool {
            NSUUID *uuid = [[UIDevice currentDevice] identifierForVendor];
            return convertString([uuid UUIDString]);
        }
    }
    
    int method runCount()
    {
        // RunCount increased in applicationDidFinishLaunching. beware that it wasn't increasing before on snow leopard
        static dispatch_once_t onceToken;
        __block int runCount;
        dispatch_once(&onceToken, ^{
            @autoreleasepool {
                runCount = (int)[[NSUserDefaults standardUserDefaults] integerForKey:@"RunCount"];
            }
        });
        return runCount;
    }
    
    static string getPurchasedVersion()
    {
        @autoreleasepool {
            NSURL *url;
            if ([AppDelegate runningPreIOS7]) {
                url = [[[NSBundle mainBundle] bundleURL] URLByAppendingPathComponent:@"/Contents/_MASReceipt/receipt"];
            } else {
                url = [[NSBundle mainBundle] appStoreReceiptURL];
            }
            if (url == 0) {
                exit(174);
            }
            
            NSData *data = [NSData dataWithContentsOfURL:url];
            if (data == nil) {
                return "Developer Version";
            }
            
            struct Payload *payload = NULL;
            asn_DEF_Payload.ber_decoder(NULL, &asn_DEF_Payload, (void **)&payload, [data bytes], [data length], 0);
            
            string ret = "Older Version";
            for (int i = 0; i < payload->list.count; i++) {
                ReceiptAttribute_t *entry;
                
                entry = payload->list.array[i];
                if (entry->type == 19) {
                    OCTET_STRING_t *value = &entry->value;
                    ret = string((char *)value->buf, value->size);
                    break;
                }
            }
            
            asn_DEF_Payload.free_struct(&asn_DEF_Payload, payload, false);
            return ret;
        }
    }
    
    int method trialRemaining()
    {
        return 0;
    }
    
    string method purchasedVersion()
    {
        static string v = getPurchasedVersion();
        //std::cout << "purchased version: " << v << std::endl;
        return v;
    }
    
    string method version()
    {
        @autoreleasepool {
            static string version = convertString((NSString *)[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"]);
            return version;
        }
    }
    
    static string getAgent()
    {
        @autoreleasepool {
            __block NSString *agent = nil;
            dispatch_sync(dispatch_get_main_queue(), ^{
                UIWebView* webView = [[UIWebView alloc] initWithFrame:CGRectZero];
                agent = [webView stringByEvaluatingJavaScriptFromString:@"navigator.userAgent"];
            });
            return convertString(agent);
        }
    }
    
    string method trackingAgent()
    {
        // should not initialize right away because it breaks snow leopard
        static string agent;
        if (agent.empty()) {
            agent = getAgent();
        }
        return agent;
    }
    
    int method screenWidth()
    {
        @autoreleasepool {
            return [UIScreen mainScreen].bounds.size.width;
        }
    }
    
    int method screenHeight()
    {
        @autoreleasepool {
            return [UIScreen mainScreen].bounds.size.height;
        }
    }
    
    void method showRemoteConnectPanel()
    {
        // TODO: display a nice selection window
        remoteControl()->discover(MEMORY_NS::static_pointer_cast<App>(shared_from_this()));
    }
    
    void method foundRemoteControllable(const std::string &name, const std::string &ip, int port)
    {
        std::cout << "found remote " << ip << " and connecting\n";
        remoteControl()->start(ip, port);
    }
    
    void method applyTheme()
    {
        [[SongsViewController sharedController] applyTheme];
    }
}
