//
//  App.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#ifdef ENABLE_PADDLE
#import <Paddle/Paddle.h>
#else
#include "Payload.h"
#include "constr_TYPE.h"
#endif
#include "App.h"
#import "PlaylistManager.h"
#import "AppDelegate.h"
#import "MainWindowController.h"
#import "CocoaWebWindow.h"
#include "StringUtility.h"
#include "AppleFileManager.h"
#include "PlaybackWorker.h"
#include "AudioDeviceUnionWorker.h"
#include "IEqualizer.h"

namespace Gear
{
#define method App::
    
    method App() :
        _preventSleep(0)
    {
    }
    
    shared_ptr<IApp> & method instance()
    {
        static shared_ptr<IApp> app(new App());
        return app;
    }
    
    DragManager & method dragManager()
    {
        return _dragManager;
    }
    
    void method editPlaylistName(const shared_ptr<IPlaylist> &playlist)
    {
        [[[[AppDelegate sharedDelegate] mainWindowController] playlistManager] startEditing:playlist];
    }
    
    IPreferences & method preferences()
    {
        return _preferences;
    }
    
    shared_ptr<IWebWindow> method createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate)
    {
        return shared_ptr<IWebWindow>(new CocoaWebWindow(delegate));
    }
    
    static NSAttributedString *googleLink()
    {
        NSAttributedString *googleURL = [AppDelegate hyperlinkFromString:@"music.google.com" withURL:[NSURL URLWithString:@"https://music.google.com"]];
        return googleURL;
    }
    
    void method showUserMessage(const UserMessage message)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            
            NSMutableAttributedString *string;
            switch(message) {
                case UserMessage::NoSongs: {
                    string = [[NSMutableAttributedString alloc] initWithString:@"Successful login, but it seems that you don't have any songs uploaded yet. Please log in at "];
                    [string appendAttributedString:googleLink()];
                    [string appendAttributedString:[[NSAttributedString alloc] initWithString:@", then download and use Google Music Manager to do so."]];
                    }
                    break;
                case UserMessage::ErrorRetrievingSongs: {
                    NSString *description = @"There was an error while receiving song data. If you are persistently unable to login, please send feedback";
                    string = [[NSMutableAttributedString alloc] initWithString:description];
                    }
                    break;
            }
            [[AppDelegate sharedDelegate] showErrorMessage:string];
        });

    }
    
    void method appStarted()
    {
        AppBase::appStarted();
        // we mustn't reference sharedDelegate here because it messes up with the needed principal class defined in plist...
        //[[[[AppDelegate sharedDelegate] mainWindowController] playlistManager] setupCategories];
        _playingConnection = player()->playingConnector().connect([this](bool playing){
            if (playing && _preventSleep == 0) {
                CFStringRef reasonForActivity = CFSTR("Playing song");
                if (lionOrBetter()) {
                    IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleSystemSleep,
                                               kIOPMAssertionLevelOn, reasonForActivity, &_preventSleep);
                } else {
                    IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep,
                                                               kIOPMAssertionLevelOn, reasonForActivity, &_preventSleep);
                }
            } else if (!playing && _preventSleep != 0) {
                if (IOPMAssertionRelease(_preventSleep)) {
                    _preventSleep = 0;
                }
            }
        });
        
        [[NSNotificationCenter defaultCenter] addObserverForName:NSUserDefaultsDidChangeNotification object:nil queue:nil usingBlock:^(NSNotification *note) {
            
            equalizer()->notifyChange(true);
        }];
    }
    
    string method dataPath() const
    {
        NSError *err = nil;
        @autoreleasepool {
#ifdef ENABLE_PADDLE
            NSFileManager *fileManager = [NSFileManager defaultManager];
            // we don't have sandboxing (that's because of software updates!) but still want the same directories for seamless interoperability
            NSURL *appSupportDirectory = [[[[[[fileManager URLForDirectory:NSLibraryDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:@"Containers"] URLByAppendingPathComponent:@"com.treasurebox.gear"] URLByAppendingPathComponent:@"Data"] URLByAppendingPathComponent:@"Library"] URLByAppendingPathComponent:@"Application Support"];
            [[NSFileManager defaultManager] createDirectoryAtPath:[appSupportDirectory path] withIntermediateDirectories:YES attributes:nil error:nil];
#else
            NSURL *appSupportDirectory = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&err];
#endif
            return convertString([appSupportDirectory path]);
        }
    }
    
    string method imageCacheDirectory() const
    {
        NSError *err = nil;
        NSFileManager *fileManager = [NSFileManager defaultManager];
#ifdef ENABLE_PADDLE
        NSURL *cacheDirectory = [[[[[[fileManager URLForDirectory:NSLibraryDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil] URLByAppendingPathComponent:@"Containers"] URLByAppendingPathComponent:@"com.treasurebox.gear"] URLByAppendingPathComponent:@"Data"] URLByAppendingPathComponent:@"Library"] URLByAppendingPathComponent:@"Caches"];
#else
        NSURL *cacheDirectory = [[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&err];
#endif
        NSURL *imageDir = [cacheDirectory URLByAppendingPathComponent:@"art" isDirectory:YES];
        [fileManager createDirectoryAtPath:[imageDir path] withIntermediateDirectories:YES attributes:nil error:nil];
        
        return convertString([imageDir path]);
    }
    
    shared_ptr<IFileManager> method fileManager() const
    {
        return shared_ptr<AppleFileManager>(new AppleFileManager());
    }
    
    bool method trialMode() const
    {
#ifdef ENABLE_PADDLE
        if (![[Paddle sharedInstance] productActivated]) {
            return true;
        }
#endif
        return false;
    }
    
    shared_ptr<IPlaybackWorker> method playbackWorker()
    {
        static auto worker = PlaybackWorker::create();
        static auto unionWorker = AudioDeviceUnionWorker::create(worker, AppBase::playbackWorker());
        return unionWorker;
    }
    
    static id activity = nil;
    static int appNapCount = 0;
    using std::mutex;
    using std::lock_guard;
    static mutex m;
    
    void method disableAppNap()
    {
        lock_guard<mutex> l(m);
        if (appNapCount > 0) {
            ++appNapCount;
            return;
        }
        if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
            activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityUserInitiated reason:@"need to have precise timing"];
        }
        appNapCount = 1;
    }
    
    void method enableAppNap()
    {
        lock_guard<mutex> l(m);
        if (activity) {
            --appNapCount;
            if (appNapCount == 0) {
                [[NSProcessInfo processInfo] endActivity:activity];
                activity = nil;
            }
        }
    }
    
    string method trackingId()
    {
        return "UA-34360192-2";
    }
    
    string method trackingClientId()
    {
        @autoreleasepool {
            NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
            NSString *clientId = [defaults stringForKey:@"analyticsId"];
            if (clientId) {
                return convertString(clientId);
            }
            CFUUIDRef uuid = CFUUIDCreate(NULL);
            clientId = CFBridgingRelease(CFUUIDCreateString(NULL, uuid));
            CFRelease(uuid);
            
            [defaults setObject:clientId forKey:@"analyticsId"];
            return convertString(clientId);
        }
    }
    
    int method runCount()
    {
        // RunCount increased in applicationDidFinishLaunching. beware that it wasn't increasing before on snow leopard
        static dispatch_once_t onceToken;
        __block int runCount;
        dispatch_once(&onceToken, ^{
            @autoreleasepool {
                runCount = (int)[[NSUserDefaults standardUserDefaults] integerForKey:@"RunCount"] + 1;
            }
        });
        return runCount;
    }
    
#ifndef ENABLE_PADDLE
    static string getPurchasedVersion()
    {
        @autoreleasepool {
            NSURL *url;
            if (floor(NSFoundationVersionNumber) <= NSFoundationVersionNumber10_6_8) {
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
#endif
    
    int method trialRemaining()
    {
#ifdef ENABLE_PADDLE
        @autoreleasepool {
            Paddle *paddle = [Paddle sharedInstance];
            if ([paddle productActivated]) {
                return -1;
            } else {
                auto ret = -[[paddle daysRemainingOnTrial] intValue];
                return ret;
            }
        }
#endif
        return 0;
    }
    
    string method purchasedVersion()
    {
#ifdef ENABLE_PADDLE
        return "Paddle";
#else
        static string v = getPurchasedVersion();
        //std::cout << "purchased version: " << v << std::endl;
        return v;
#endif
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
                agent = [[[WebView alloc] init] userAgentForURL:[NSURL URLWithString:@"http://www.google.com"]];
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
            NSScreen *screen = [NSScreen mainScreen];
            int ret = [screen frame].size.width;
            if ([screen respondsToSelector:@selector(backingScaleFactor)]) {
                ret *= [screen backingScaleFactor];
            }
            return ret;
        }
    }
    
    int method screenHeight()
    {
        @autoreleasepool {
            NSScreen *screen = [NSScreen mainScreen];
            int ret = [screen frame].size.height;
            if ([screen respondsToSelector:@selector(backingScaleFactor)]) {
                ret *= [screen backingScaleFactor];
            }
            return ret;
        }
    }
}

