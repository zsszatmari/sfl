#ifndef QMLCONTROLLERMANAGER_HPP
#define QMLCONTROLLERMANAGER_HPP

#include <QObject>
#include <memory>

#include "MainWindowController.h"
#include "PlaybackPanelController.h"
#include "PlaylistController.h"
#include "SettingPanelController.h"
#include "SongListController.h"

class QmlControllerManager : public QObject
{
    Q_OBJECT
public:
    enum ControllerType
    {
        MainWindowType,
        PlaybackPanelType,
        PlaylistType,
        SettingPanelType,
        SongListType
    };

    explicit QmlControllerManager(QObject *parent = 0);
    ~QmlControllerManager();

    void initializeController(QQmlEngine *engine);
    void setWindow(QQuickWindow *window);

    QmlController *controller(const ControllerType &type);

private:
    std::shared_ptr<MainWindowController> _mainWindowController;
    std::shared_ptr<PlaybackPanelController> _playbackPanelController;
    std::shared_ptr<PlaylistController> _playlistController;
    std::shared_ptr<SettingPanelController> _settingPanelController;
    std::shared_ptr<SongListController> _songListController;
};

#endif // QMLCONTROLLERMANAGER_HPP
