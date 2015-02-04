#include <QQuickItem>
#include <QQmlContext>
#include <QStringListModel>
#include <QStringList>
#include <QDebug>
#include <iostream>

#include "PlaylistController.h"
#include "IApp.h"
#include "SessionManager.h"
#include "PlaylistCategory.h"
#include "ValidPtr.h"
#include "QmlModel/PlaylistModel.h"

PlaylistController::PlaylistController(QQmlEngine *engine)
    : QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("playlistController", this);

    _playlistCategoryModel = std::make_shared<PlaylistCategoryModel>(qmlEngine());
    qmlEngine()->rootContext()->setContextProperty("playlistCategoryModel", _playlistCategoryModel.get());
}

void PlaylistController::userSelectedPlaylist(const QString &category,
                                              const QString &id,
                                              bool start)
{
    auto playlist = findPlaylistById(category, id);
    if (playlist != nullptr)
    {
        Gear::IApp::instance()->userSelectedPlaylist(playlist, start);
    }
}

const std::shared_ptr<Gear::IPlaylist> PlaylistController::findPlaylistById(const QString &category,
                                                                               const QString &id)
{
    auto playlists = Gear::IApp::instance()->sessionManager()->categories();
    for (auto it = playlists->begin(); it != playlists->end(); ++it)
    {
        auto playlist = (*it).playlists();
        if (QString::fromStdString((*it).title()) == category)
        {
            for (auto i = playlist.begin(); i != playlist.end(); ++i)
            {
                auto p = (*i);
                if (QString::fromStdString(p->playlistId()) == id)
                {
                    return p;
                }
            }
        }
    }

    return nullptr;
}
