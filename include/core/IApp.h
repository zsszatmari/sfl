//
//  IApp.h
//  G-Ear
//
//  Created by Zsolt Szatmári on 6/5/13.
//
//

#ifndef __G_Ear__IApp__
#define __G_Ear__IApp__

#include "stdplus.h"
#include "ManagedValue.h"
#include "PlaylistCategory.h"
#include "SortDescriptor.h"
#include MUTEX_H
#include SHAREDFROMTHIS_H

namespace Gui
{
    class ThemeManager;
}

namespace Gear
{
    using namespace Base;
    
    enum class UserMessage
    {
        NoSongs,
        ErrorRetrievingSongs
    };
    
    class ISession;
    class SessionManager;
    class IKeychain;
    class IPlayer;
    class IPlaylist;
    class ISongIntent;
    class IWebWindow;
    class IWebWindowDelegate;
    class DragManager;
    class ServiceManager;
    class AlbumArtStash;
    class IFileManager;
    class IPreferences;
    class IPreferencesPanel;
    class LocalSession;
    class IJavascriptEngine;
    class IPlaybackWorker;
    class SessionManager;
    class SongEntry;
    class QueueSongArray;
    class LastFmController;
    class Bridge;
    class PlayerSwitch;
    class Db;
    class ISongArray;
    class RemoteControl;
    class IEqualizer;
    
	class core_export IApp : public MEMORY_NS::enable_shared_from_this<IApp>
    {
    public:
        IApp();
        virtual ~IApp();
        virtual void editPlaylistName(const shared_ptr<IPlaylist> &playlist) = 0;
        
        virtual DragManager &dragManager() = 0;
        virtual IPreferences &preferences() = 0;
        virtual shared_ptr<IWebWindow> createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) = 0;
        virtual string dataPath() const = 0;
        shared_ptr<IPlaylist> selectedPlaylist() const;
        bool userSelectedCategory(int tag, const shared_ptr<IPlaylist> &playlist = shared_ptr<IPlaylist>(), bool start = false, bool implicit = false);
        void userSelectedPlaylist(const shared_ptr<IPlaylist> &playlist, bool start = false);
        
        virtual void showUserMessage(const UserMessage message) = 0;
        virtual shared_ptr<SessionManager> sessionManager() const = 0;
        virtual shared_ptr<ServiceManager> serviceManager() const = 0;
        virtual shared_ptr<Gui::ThemeManager> themeManager() const = 0;
        virtual shared_ptr<IPreferencesPanel> preferencesPanel() const = 0;
        
        virtual bool phoneInterface() = 0;

        virtual void appStarted() = 0;
        virtual shared_ptr<IPlayer> player() = 0;
        virtual shared_ptr<PlayerSwitch> playerSwitch() = 0;
        shared_ptr<ISongArray> queueSongArray() const;
        static shared_ptr<IApp> instance();
        virtual shared_ptr<IKeychain> keychain() = 0;
        virtual shared_ptr<LocalSession> createLocalSession();
        
        virtual shared_ptr<AlbumArtStash> albumArtStash();
        
        static void registerInstance(const shared_ptr<IApp> &app);
        
        ValueConnector<std::pair<PlaylistCategory,shared_ptr<IPlaylist>>>selectedPlaylistConnector();
        
        virtual void playFirstOfCurrentPlaylist() = 0;
        
        virtual void showRelated(const SongEntry &entry, const string &baseField) = 0;
        virtual SortDescriptor defaultSongOrder() const = 0;
        // only spotify uses this for now, so this is not mandatory yet
        virtual void askForCredentials(const string &title, string &user, string &pass) {}
        virtual bool trialMode() const;
        virtual void freeUpMemory() = 0;
        virtual shared_ptr<IPlaybackWorker> playbackWorker() = 0;
        shared_ptr<IEqualizer> equalizer();
        virtual void didBecomeActive() = 0;
        shared_ptr<Db> db() const;
                
        virtual void showRemoteConnectPanel();

        void terminate();
        void addTerminateCallback(const std::function<void()> &f);
        
        virtual shared_ptr<IFileManager> fileManager() const = 0;
        virtual shared_ptr<IJavascriptEngine> createJavascriptEngine() const = 0;
        void goBack();
        virtual void disableAppNap();
        virtual void enableAppNap();
        void recalculateQueue();
        SignalConnection preferenceConnect(const string &key, const std::function<void()> &f);
        virtual shared_ptr<RemoteControl> remoteControl() const = 0;

        virtual void applyTheme();

        // necessary for tracking:
        virtual string trackingId() = 0;
        virtual string trackingClientId() = 0;
        virtual int runCount() = 0;
        virtual string version() = 0;
        virtual string trackingAgent() = 0;
        virtual int trialRemaining() = 0;
        virtual string purchasedVersion() = 0;
        virtual int screenWidth() = 0;
        virtual int screenHeight() = 0;

        shared_ptr<LastFmController> lastFmController() const;
        virtual shared_ptr<Bridge> bridge() const;
        
    protected:
        void doSetSelectedPlaylist(const std::pair<PlaylistCategory,shared_ptr<IPlaylist>> &p, bool force = false);

        shared_ptr<AlbumArtStash> _albumArtStash;
        shared_ptr<IEqualizer> _equalizer;
        
        bool _userExplicitlySelected;
        shared_ptr<QueueSongArray> _queueSongArray;
        shared_ptr<LastFmController> _lastFmController;
        shared_ptr<Db> _db;
        
    private:
        ManagedValue<std::pair<PlaylistCategory,shared_ptr<IPlaylist>>> __selectedPlaylist;
        
        THREAD_NS::mutex _terminateMutex;
        std::vector<std::function<void()>> _terminateCallbacks;
        shared_ptr<Bridge> _bridge;
        virtual string imageCacheDirectory() const = 0;

        friend class SessionManager;
    };
}

#endif /* defined(__G_Ear__IApp__) */
