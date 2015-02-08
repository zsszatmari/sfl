#include	<QApplication>
#include	<QQuickView>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <iostream>
#include "qtquick2controlsapplicationviewer.h"
#include	"IApp.h"
#include	"App.h"
#include "stdplus.h"

#include "SharedMemoryStatusThread.h"

using namespace Gear;

class CustomEvent : public QEvent
{
public:
    CustomEvent(const std::function<void()> &f) :
        QEvent(QEvent::User),
        _f(f)
    {
    }

    void run()
    {
        _f();
    }

private:
    std::function<void()> _f;
};

class EventProcessor : public QObject
{
private:
    virtual bool event(QEvent* aEvent)
    {
        CustomEvent *event = static_cast<CustomEvent *>(aEvent);
        event->run();

        return true;
    }
};

class QtExecutor : public IExecutor
{
public:
    QtExecutor()
    {
    }

    virtual void addTask(Task &task)
    {
        QEvent *event = new CustomEvent(task);

        QCoreApplication::postEvent(&_object, event);
    }

    virtual void addTaskAndWait(Task &task)
    {
        if (_object.thread() == QThread::currentThread()) {
            task();
            return;
        }

        std::condition_variable variable;
        std::mutex m;
        bool finished = false;

        QEvent *event = new CustomEvent([&]{
            task();

            std::lock_guard<std::mutex> l(m);
            finished = true;
            variable.notify_all();
        });

        std::unique_lock<std::mutex> l(m);
        while (!finished) {
            variable.wait(l);
        }
    }

private:
    // object with affinity for main thread
    EventProcessor _object;
};

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    //printf("log\n"); fflush(NULL);

    QSystemSemaphore sema("GEAR_MUSIC_PLAYER_KEY", 1, QSystemSemaphore::Open);
    sema.acquire();
    QSharedMemory mem("GEAR_MUSIC_PLAYER_KEY_WIN");
    std::cout << "Creating shared memory" << std::endl;
    if (!mem.create(1))
    {
        std::cout << "Creating shared memory failed" << std::endl;
        if (argv[1] == QString("--quit"))
        {
            if (mem.attach())
            {
                mem.lock();
                void *dataArea = mem.data();
                bool quitApp = true;
                std::memcpy(dataArea, &quitApp, 1);
                mem.unlock();
                mem.detach();
            }
            sema.release();
            exit(0);
        }
        else
        {
            QMessageBox::information(0, "Gear Music Player Error","An instance has already been running.");
            sema.release();
            exit(0);
        }
    }
    std::cout << "Created shared memory" << std::endl;

    mem.lock();
    void *dataArea = mem.data();
    bool notQuitApp = false;
    std::memcpy(dataArea, &notQuitApp, 1);
    mem.unlock();
    sema.release();

    std::cout << "Starting shared memory status thread" << std::endl;
    std::shared_ptr<SharedMemoryStatusThread> sharedMemoryStatusThread
            = std::make_shared<SharedMemoryStatusThread>(nullptr);
    QObject::connect(sharedMemoryStatusThread.get(), SIGNAL(finished()), qApp, SLOT(quit()));
    sharedMemoryStatusThread->start();

    shared_ptr<IExecutor> executor(new QtExecutor());
    DefaultExecutor::registerInstance(executor);
    IApp::registerInstance(App::instance());
    App::instance()->appStarted();

    // this does not work
    //qmlRegisterType<QtPreferencesPanel>("Gear", 1, 0, "PreferencesPanel");


    QtQuick2ControlsApplicationViewer viewer;
    viewer.setMainQmlFile(QStringLiteral("qrc:/qml/QML/GEarGui.qml"));
    //viewer.setMainQmlFile(QStringLiteral("qml/qttest/main.qml"));
    viewer.show();

    return app.exec();
}
