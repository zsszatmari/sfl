#include	<QApplication>
#include	<QQuickView>
#include "qtquick2controlsapplicationviewer.h"
#include	"IApp.h"
#include	"App.h"
#include <QThread>
#include "stdplus.h"

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
