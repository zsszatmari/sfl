//
//  App.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#ifndef __G_Ear__App__
#define __G_Ear__App__

#include "DragManager.h"
#include "AppBase.h"
#include "ApplePreferences.h"
#import <IOKit/pwr_mgt/IOPMLib.h>

namespace Gear
{
    class App final : public AppBase
    {
    // temporarily singleton
    public:
        static shared_ptr<IApp> &instance();
        
        virtual DragManager &dragManager();
        virtual void editPlaylistName(const shared_ptr<IPlaylist> &playlist);
        virtual IPreferences &preferences();
        virtual shared_ptr<IWebWindow> createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        virtual void showUserMessage(const UserMessage message);
        virtual void appStarted();
        virtual string dataPath() const;
        virtual bool trialMode() const;
        virtual shared_ptr<IPlaybackWorker> playbackWorker();
        virtual void disableAppNap();
        virtual void enableAppNap();
        
        virtual string trackingId();
        virtual string trackingClientId();
        virtual int runCount();
        virtual string version();
        virtual string trackingAgent();
        virtual int trialRemaining();
        virtual string purchasedVersion();
        virtual int screenWidth();
        virtual int screenHeight();
        
    private:
        DragManager _dragManager;
        ApplePreferences _preferences;
        
        virtual string imageCacheDirectory() const;
        virtual shared_ptr<IFileManager> fileManager() const;

        SignalConnection _playingConnection;
        IOPMAssertionID _preventSleep;
        
        App();
        App(const App &rhs); // delete
        App &operator=(const App &rhs); // delete
    };
}

#endif /* defined(__G_Ear__App__) */
