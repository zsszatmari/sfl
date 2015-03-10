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
#include "SongPredicate.h"

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

void PlaylistController::doFilter(const QString &filter)
{
    auto playlist = Gear::IApp::instance()->selectedPlaylist();
    if (playlist)
    {
        playlist->setFilterPredicate(Gear::SongPredicate("", filter.toStdString(),
                                                         Gear::SongPredicate::Contains()));
    }
}

void PlaylistController::qmlWindowReady()
{
    _playlistCategoryConnection = Gear::IApp::instance()->selectedPlaylistConnector()
            .connect([this](const std::pair<Gear::PlaylistCategory, shared_ptr<Gear::IPlaylist>> &p)
    {
        auto selected = Gear::IApp::instance()->selectedPlaylist();
        std::string value;
        if (selected)
        {
            value = selected->filterPredicate().value();
        }

        QObject *searchObject = qmlWindow()->findChild<QObject *>("searchItemObject");
        searchObject->setProperty("filter", QString::fromStdString(value));
    });
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
