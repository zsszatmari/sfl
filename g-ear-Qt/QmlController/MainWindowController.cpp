#include <QApplication>
#include <QQmlContext>
#include <QMessageBox>
#include <QPushButton>
#include <QQuickItem>

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
    connect(this, SIGNAL(windowReady()), this, SLOT(setWindowProperty()));

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
    QPushButton *runInBgButton = questionDialog.addButton("Run in background", QMessageBox::YesRole);
    connect(runInBgButton, SIGNAL(clicked()), this, SLOT(makeWindowRunInBackground()));
    QPushButton *quitButton = questionDialog.addButton("Quit", QMessageBox::NoRole);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(forceQuitApp()));

    questionDialog.exec();
}

bool MainWindowController::runInBackground()
{
    return Gear::IApp::instance()->preferences().boolForKey("RunInBackground");
}

bool MainWindowController::runInBackgroundAlreadyAsked()
{
    return Gear::IApp::instance()->preferences().boolForKey("RunInBackgroundAlreadyAsked");
}

void MainWindowController::closeWindow()
{
    qmlWindow()->close();
}

void MainWindowController::maximizeWindow()
{
    qmlWindow()->showMaximized();
}

void MainWindowController::showNormalWindow()
{
    qmlWindow()->showNormal();
}

void MainWindowController::minimizeWindow()
{
    qmlWindow()->showMinimized();
}

bool MainWindowController::windowMaximized()
{
    if (qmlWindow())
    {
        if (qmlWindow()->windowState() == Qt::WindowMaximized)
        {
            return true;
        }
    }

    return false;
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

void MainWindowController::setWindowProperty()
{
    qmlWindow()->setFlags(qmlWindow()->flags()
                          | Qt::FramelessWindowHint);

    connect(qmlWindow(), SIGNAL(mouseMoved(int, int)), this, SLOT(moveMouse(int, int)));
    connect(qmlWindow(), SIGNAL(mousePressed(int, int)), this, SLOT(pressMouse(int, int)));
    connect(qmlWindow(), SIGNAL(mouseReleased()), this, SLOT(releaseMouse()));
}

void MainWindowController::makeWindowRunInBackground()
{
    Gear::IApp::instance()->preferences().setUintForKey("RunInBackgroundAlreadyAsked", 1);
    Gear::IApp::instance()->preferences().setUintForKey("RunInBackground", 1);
    hideMainWindowToSystemTray();
}

void MainWindowController::forceQuitApp()
{
    Gear::IApp::instance()->preferences().setUintForKey("RunInBackgroundAlreadyAsked", 1);
    Gear::IApp::instance()->preferences().setUintForKey("RunInBackground", 0);

    qApp->exit();
}

void MainWindowController::pressMouse(int x, int y)
{
    _isDragging = true;
    _pressedPoint = qmlWindow()->mapToGlobal(QPoint(x, y));
}

void MainWindowController::releaseMouse()
{
    _isDragging = false;
}

void MainWindowController::moveMouse(int x, int y)
{
    if (_isDragging)
    {
        QPoint currentPoint = qmlWindow()->mapToGlobal(QPoint(x, y));
        QPoint offsetPoint = currentPoint - _pressedPoint;
        qmlWindow()->setPosition(qmlWindow()->position() + offsetPoint);
        _pressedPoint = currentPoint;
    }
}

