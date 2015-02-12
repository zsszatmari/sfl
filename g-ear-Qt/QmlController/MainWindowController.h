#ifndef MAINWINDOWCONTROLLER_HPP
#define MAINWINDOWCONTROLLER_HPP

#include <memory>
#include <QThread>
#include <QTimer>
#include "WinFeature/SystemTrayIcon.h"
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
    Q_INVOKABLE void closeWindow();
    Q_INVOKABLE void maximizeWindow();
    Q_INVOKABLE void showNormalWindow();
    Q_INVOKABLE void minimizeWindow();
    Q_INVOKABLE bool windowMaximized();

signals:
    void startWork();

private slots:
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void bringMainWindowToFront();
    void setWindowProperty();
    void makeWindowRunInBackground();
    void forceQuitApp();

    void pressMouse(int x, int y);
    void releaseMouse();
    void moveMouse(int x, int y);

private:
    std::shared_ptr<SystemTrayIcon> _systemTrayIcon;
    QThread _sharedMemoryStatusThread;
    QTimer _sharedMemoryTimer;

    bool _isDragging = false;
    bool _isResizing = false;

    QPoint _pressedPoint;
};

#endif // MAINWINDOWCONTROLLER_HPP
