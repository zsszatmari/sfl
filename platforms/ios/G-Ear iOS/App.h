//
//  App.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 5/31/13.
//
//

#ifndef __G_Ear__App__
#define __G_Ear__App__

#include "AppBase.h"
#include "ApplePreferences.h"
#include "RemoteControl.h"

namespace Gear
{
    class App final : public AppBase, public RemoteControl::DiscoverDelegate
    {
    // temporarily singleton
    public:
        static shared_ptr<IApp> &instance();
        
        virtual void jumpToPlaylist(const shared_ptr<IPlaylist> &playlist, bool start);
        virtual void editPlaylistName(const shared_ptr<IPlaylist> &playlist);
        virtual IPreferences &preferences();
        virtual void reloadPlaylists();
        virtual shared_ptr<IWebWindow> createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        virtual string dataPath() const;
        void showUserMessage(const UserMessage message);
        
        virtual string imageCacheDirectory() const;
        virtual shared_ptr<IFileManager> fileManager() const;
        virtual shared_ptr<LocalSession> createLocalSession();
        virtual shared_ptr<IJavascriptEngine> createJavascriptEngine() const;
        virtual shared_ptr<IPlaybackWorker> playbackWorker();
        
        virtual void applyTheme();
        
        virtual string trackingId();
        virtual string trackingClientId();
        virtual int runCount();
        virtual string version();
        virtual string trackingAgent();
        virtual int trialRemaining();
        virtual string purchasedVersion();
        virtual int screenWidth();
        virtual int screenHeight();
        
        void showRemoteConnectPanel();
        virtual void foundRemoteControllable(const std::string &name, const std::string &ip, int port) override;
        
    private:
        ApplePreferences _preferences;
        
        App();
        App(const App &rhs); // delete
        App &operator=(const App &rhs); // delete
    };
}

#endif /* defined(__G_Ear__App__) */
