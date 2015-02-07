#include <QApplication>
#include <QQmlContext>
#include <QMessageBox>
#include <QDebug>

#include "MainWindowController.h"
#include "IApp.h"
#include "IPreferences.h"

MainWindowController::MainWindowController(QQmlEngine *engine)
        : QmlController(engine)
        , _systemTrayIcon(std::make_shared<SystemTrayIcon>(this))
{
    connect(_systemTrayIcon->getIcon().get(),
                SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason)));

    qmlEngine()->rootContext()->setContextProperty("mainWindowController", this);
}

MainWindowController::~MainWindowController()
{

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
        _systemTrayIcon->getIcon()->hide();
        qmlWindow()->setWindowState(Qt::WindowMinimized);
        qmlWindow()->setVisible(true);
    }
}

