#include <QApplication>
#include <QQmlContext>
#include <QMessageBox>

#include "SharedMemoryStatusThreadWorker.h"

#ifdef _WIN32
#include <QWinJumpList>
#include <QWinJumpListItem>
#include <QWinJumpListCategory>
#endif

#include <QDir>
#include <QDebug>

#include "MainWindowController.h"
#include "IApp.h"
#include "IPreferences.h"

MainWindowController::MainWindowController(QQmlEngine *engine)
    : QmlController(engine)
    , _systemTrayIcon(std::make_shared<SystemTrayIcon>(this))
{
#ifdef _WIN32
    QWinJumpListItem *quitTask = new QWinJumpListItem(QWinJumpListItem::Link);
    quitTask->setTitle(tr("Quit Gear"));
    quitTask->setFilePath(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    quitTask->setArguments(QStringList("--quit"));

    QWinJumpList jumplist;
    jumplist.tasks()->addItem(quitTask);
    jumplist.tasks()->setVisible(true);
#endif

    connect(_systemTrayIcon->getIcon().get(),
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    qmlEngine()->rootContext()->setContextProperty("mainWindowController", this);


    qDebug() << "Starting shared memory status thread";
    SharedMemoryStatusThreadWorker *threadWorker = new SharedMemoryStatusThreadWorker();
    threadWorker->moveToThread(&_sharedMemoryStatusThread);
    connect(threadWorker, SIGNAL(jobDone()), qApp, SLOT(quit()));
    connect(threadWorker, SIGNAL(bringToFrontSignal()),
                     this, SLOT(bringMainWindowToFront()));
    connect(this, SIGNAL(startWork()), threadWorker, SLOT(doWork()));
    connect(&_sharedMemoryStatusThread, SIGNAL(finished()), threadWorker, SLOT(deleteLater()));
    _sharedMemoryStatusThread.start();
    emit startWork();

    connect(&_sharedMemoryTimer, SIGNAL(timeout()), threadWorker, SLOT(doWork()));
    _sharedMemoryTimer.start(10);
}

MainWindowController::~MainWindowController()
{
    _sharedMemoryTimer.stop();
    _sharedMemoryStatusThread.quit();
    _sharedMemoryStatusThread.wait();
}

void MainWindowController::hideMainWindowToSystemTray()
{
    qmlWindow()->hide();
    _systemTrayIcon->getIcon()->show();
}

void MainWindowController::showRunInBackgroundDialog()
{
    QMessageBox questionDialog;
    questionDialog.setModal(true);
    questionDialog.setText("Do you wish Gear to run in the background when you close the window, or quit?\n"
                           "You can change this setting later in the Preferences");
    questionDialog.addButton("Run in background", QMessageBox::YesRole);
    questionDialog.addButton("Quit", QMessageBox::NoRole);

    questionDialog.exec();

    if (questionDialog.result() == QMessageBox::AcceptRole)
    {
        qDebug() << "Saving RunInBackgroundAlreadyAsked to 1";
        Gear::IApp::instance()->preferences().setUintForKey("RunInBackgroundAlreadyAsked", 1);
        Gear::IApp::instance()->preferences().setUintForKey("RunInBackground", 1);
        hideMainWindowToSystemTray();
    }
    else if (questionDialog.result() == QMessageBox::RejectRole)
    {
        qDebug() << "Saving RunInBackgroundAlreadyAsked to 1";
        Gear::IApp::instance()->preferences().setUintForKey("RunInBackgroundAlreadyAsked", 1);
        Gear::IApp::instance()->preferences().setUintForKey("RunInBackground", 0);

        qDebug() << "Exit the application";
        qApp->exit();
    }
}

bool MainWindowController::runInBackground()
{
    return Gear::IApp::instance()->preferences().boolForKey("RunInBackground");
}

bool MainWindowController::runInBackgroundAlreadyAsked()
{
    return Gear::IApp::instance()->preferences().boolForKey("RunInBackgroundAlreadyAsked");
}

void MainWindowController::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        bringMainWindowToFront();
    }
}

void MainWindowController::bringMainWindowToFront()
{
    _systemTrayIcon->getIcon()->hide();
    qmlWindow()->setWindowState(Qt::WindowNoState);
    qmlWindow()->setVisible(true);
    qmlWindow()->raise();

#ifdef _WIN32
    SetForegroundWindow(HWND(qmlWindow()->winId()));
#endif
}

