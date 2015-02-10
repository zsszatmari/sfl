#ifndef FAKEAPP_H
#define FAKEAPP_H

#include "gmock/gmock.h"
#include "IPreferences.h"
#include "IKeychain.h"
#include "AppBase.h"
#include "IFileManager.h"

using namespace Gear;

class FakeExecutor : public IExecutor
{
public:
    virtual void addTask(Task &task)
    {
        task();
        /*static dispatch_queue_t queue = dispatch_queue_create("testqueue", 0);
        auto localTask = task;
        dispatch_async(queue, ^{
            localTask();
        });*/
    }
    
    virtual void addTaskAndWait(Task &task)
    {
        task();
    }
};

template<typename T>
T &nullRef()
{
    T *value = nullptr;
    return *value;
}

class MockPreferences : public IPreferences
{
public:
    MOCK_CONST_METHOD1(boolForKey, bool(const string &));
    MOCK_CONST_METHOD1(uintForKey, unsigned long(const string &));
    MOCK_CONST_METHOD1(floatForKey, float(const string &));
    MOCK_CONST_METHOD1(stringForKey, string(const string &));

    virtual void setUintForKey(const string &key, const uint64_t value) {}
    virtual void setFloatForKey(const string &key, const float value) {}
    virtual void setStringForKey(const string &key, const string &value) {}
};

class FakeKeychain : public IKeychain
{
    virtual string username(const string &identifier) const {return "";}
    virtual string pass(const string &identifier) const {return "";}
    
    virtual void save(const string &identifier, const string &username, const string &data) {}
    virtual void forget(const string &identifier) {}
};

class FakeFileManager : public IFileManager
{
public:
    virtual void removeFilesIfExceedsCache(const string &dir, const long long maxSize) {}
    virtual vector<char> getFileWithBasename(const string &dir, const string &basename) const {return vector<char>();}
    virtual vector<string> listFiles(const string &dir) {return vector<string>();}
    virtual void deleteFileWithBasename(const string &dir, const string &basename) {}
    virtual void deleteFile(const string &file) {}
    virtual void putFile(const string &dir, const string &filename, const vector<char> &data) {}

    virtual void getImageDimensions(const vector<char> &data, long &width, long &height) {}
};

class FakeApp : public AppBase
{
public:
    static shared_ptr<FakeApp> start()
    {
        {
            auto oldApp = IApp::instance();
            if (oldApp) {
                oldApp->terminate();
            }
        }

        DefaultExecutor::registerInstance(shared_ptr<IExecutor>(new FakeExecutor()));
        shared_ptr<FakeApp> app(new FakeApp());
        IApp::registerInstance(app);

        ::remove((app->dataPath() + "/" + "gear.db").c_str());
        app->appStarted();
        return app;
    }

	virtual void editPlaylistName(const shared_ptr<IPlaylist> &playlist) {}
        
    virtual DragManager &dragManager() { return nullRef<DragManager>(); }
    virtual MockPreferences &preferences() { static testing::NiceMock<MockPreferences> prefs;return prefs;}
    virtual shared_ptr<IWebWindow> createWebWindow(const shared_ptr<IWebWindowDelegate> &delegate) { return nullptr; }
    virtual string dataPath() const { return "/Users/zsszatmari/tmp"; }
    virtual shared_ptr<IKeychain> keychain() { return shared_ptr<IKeychain>(new FakeKeychain());}
    virtual void showUserMessage(const UserMessage message) {}
    virtual shared_ptr<IPreferencesPanel> preferencesPanel() const {return nullptr;}
    
    virtual bool phoneInterface() {return false;}

    virtual void addLowPriorityTask(const function<void()> &task) {}
    
    virtual void showRelated(const SongEntry &entry, const string &baseField) {}
    virtual SortDescriptor defaultSongOrder() const {return SortDescriptor();}

    virtual bool trialMode() const {return false;}
    virtual void freeUpMemory() {}
    virtual shared_ptr<IPlaybackWorker> playbackWorker() {return AppBase::playbackWorker();}

    virtual shared_ptr<IFileManager> fileManager() const {static shared_ptr<IFileManager> ret(new FakeFileManager());return ret;}

    // necessary for tracking:
    virtual string trackingId() {return "";}
    virtual string trackingClientId() {return "";}
    virtual int runCount() {return 0;}
    virtual string version() {return "";}
    virtual string trackingAgent() {return "";}
    virtual int trialRemaining() {return 0;}
    virtual string purchasedVersion() {return "";}
    virtual int screenWidth() {return 0;}
    virtual int screenHeight() {return 0;}
    virtual string imageCacheDirectory() const {return "";}

};

#endif