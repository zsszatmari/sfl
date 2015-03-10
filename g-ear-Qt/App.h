#ifndef APP_H
#define APP_H

#include "AppBase.h"
#include "QTPreferences.h"

namespace Gear
{
    class App final : public AppBase
    {
    public:
        static shared_ptr<IApp> &instance();

        virtual void editPlaylistName(const shared_ptr<IPlaylist> &playlist);
        virtual IPreferences &preferences();
        virtual shared_ptr<IWebWindow> createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate);
        virtual string dataPath() const;
        void showUserMessage(const UserMessage message);

        virtual shared_ptr<IPlaybackWorker> playbackWorker();


        virtual shared_ptr<IFileManager> fileManager() const;

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
        QTPreferences _preferences;

        App();
        App(const App &rhs); // delete
        App &operator=(const App &rhs); // delete

        virtual string imageCacheDirectory() const;
    };
}

#endif // APP_H
