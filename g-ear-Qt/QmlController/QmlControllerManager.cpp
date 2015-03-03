#include <QDebug>
#include "QmlControllerManager.h"

QmlControllerManager::QmlControllerManager(QObject *parent) : QObject(parent)
{

}

QmlControllerManager::~QmlControllerManager()
{

}

void QmlControllerManager::initializeController(QQmlEngine *engine)
{
    _settingPanelController = std::make_shared<SettingPanelController>(engine);
    _playlistController = std::make_shared<PlaylistController>(engine);
    _songListController = std::make_shared<SongListController>(engine);
    _mainWindowController = std::make_shared<MainWindowController>(engine);
    _playbackPanelController = std::make_shared<PlaybackPanelController>(engine);
    _statusBarController = std::make_shared<StatusBarController>(engine);
}

void QmlControllerManager::setWindow(QQuickWindow *window)
{
    _mainWindowController->setWindow(window);
    _playbackPanelController->setWindow(window);
    _songListController->setWindow(window);
    _settingPanelController->setWindow(window);
    _statusBarController->setWindow(window);
}

QmlController *QmlControllerManager::controller(const QmlControllerManager::ControllerType &type)
{
    switch (type)
    {
    case MainWindowType:
        return _mainWindowController.get();
    case PlaybackPanelType:
        return _playbackPanelController.get();
    case PlaylistType:
        return _playlistController.get();
    case SettingPanelType:
        return _settingPanelController.get();
    case SongListType:
        return _songListController.get();
    case StatusBarType:
        return _statusBarController.get();
    default:
        return nullptr;
    }
}

