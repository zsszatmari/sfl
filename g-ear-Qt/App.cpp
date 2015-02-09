//
//  App.cpp
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#include "App.h"
#include "QTKeychain.h"
#include "PlaybackWorker.h"
#include "qtwebwindow.h"
#include "BoostFileManager.h"

namespace Gear
{
#define method App::

    method App()
    {
        _keychain = shared_ptr<IKeychain>(new QTKeychain());
    }

    shared_ptr<IApp> & method instance()
    {
        static shared_ptr<IApp> app(new App());
        return app;
    }

    void method editPlaylistName(const shared_ptr<IPlaylist> &playlist)
    {
        //[[[[AppDelegate sharedDelegate] mainWindowController] playlistManager] startEditing:playlist];
    }

    IPreferences & method preferences()
    {
        return _preferences;
    }

    shared_ptr<IWebWindow> method createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate)
    {
        return shared_ptr<IWebWindow>(new QtWebWindow(delegate));
    }

    void method showUserMessage(const UserMessage message)
    {
        // TODO
    }

    string method dataPath() const
    {
        // TODO: return an application specific data storage path
        return "/tmp";
    }

    string method imageCacheDirectory() const
    {
        // TODO: find a suitable directory, example below is for ios. (this is inside a directory which we need
        // for long term but system might purge when low on disk, unsure there is a Qt equivalent)
        return "/tmp";

        /*
        NSError *err = nil;
        NSURL *cacheDirectory = [[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&err];
        NSURL *imageDir = [cacheDirectory URLByAppendingPathComponent:@"art" isDirectory:YES];
        [[NSFileManager defaultManager] createDirectoryAtPath:[imageDir path] withIntermediateDirectories:YES attributes:nil error:nil];

        return convertString([imageDir path]);*/
    }

    shared_ptr<IFileManager> method fileManager() const
    {
        //return nullptr;
        return shared_ptr<IFileManager>(new BoostFileManager());
    }

    shared_ptr<IPlaybackWorker> method playbackWorker()
    {
#pragma message("TODO: playbackworker")
        return AppBase::playbackWorker();
    }

    shared_ptr<IJavascriptEngine> method createJavascriptEngine() const
    {
        return nullptr;
    }


    string method trackingId()
    {
#pragma message ("TODO: tracking!")
        return "";
    }

    string method trackingClientId()
    {
        return "";
    }

    int method runCount()
    {
        return 0;
    }

    string method version()
    {
        return "";
    }

    string method trackingAgent()
    {
        return "";
    }

    int method trialRemaining()
    {
        return 0;
    }

    string method purchasedVersion()
    {
        return "";
    }

    int method screenWidth()
    {
        return 0;
    }

    int method screenHeight()
    {
        return 0;
    }
}
