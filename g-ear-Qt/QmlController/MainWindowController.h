#ifndef MAINWINDOWCONTROLLER_HPP
#define MAINWINDOWCONTROLLER_HPP

#include <memory>
#include "Widget/SystemTrayIcon.h"
#include "QmlController.h"

class MainWindowController : public QmlController
{
    Q_OBJECT
public:
    explicit MainWindowController(QQmlEngine *engine);
    ~MainWindowController();

    Q_INVOKABLE void hideMainWindowToSystemTray();
    Q_INVOKABLE void showRunInBackgroundDialog();
    Q_INVOKABLE bool runInBackground();
    Q_INVOKABLE bool runInBackgroundAlreadyAsked();

private slots:
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    std::shared_ptr<SystemTrayIcon> _systemTrayIcon;
};

#endif // MAINWINDOWCONTROLLER_HPP
