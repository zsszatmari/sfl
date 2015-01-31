#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <iostream>

#include "SongListController.h"

#include "SongSortOrder.h"
#include "IApp.h"
#include "IPlaylist.h"
#include "ISongIntent.h"

SongListController::SongListController(QQmlEngine *engine,
                                       QObject *parent) :
    QmlController(engine, parent)
{
    qmlEngine()->rootContext()->setContextProperty("songListController", this);

    _songListModel = std::make_shared<SongListModel>();
    qmlEngine()->rootContext()->setContextProperty("songListModel", _songListModel.get());
}

void SongListController::resortSongList(const QString &identifier,
                                        const bool &isAscending)
{
   Gear::IApp::instance()->selectedPlaylist()->setSortDescriptor(
               Gear::SongSortOrder::sortDescriptor(identifier.toStdString(),
                                                   isAscending));
}

void SongListController::clearSelectedSongs()
{
    _selectedSongs.clear();
}

void SongListController::addSelectedSong(int rowIndex)
{
    _selectedSongs.push_back(_songListModel->songs().at(rowIndex));
}

void SongListController::popupContextMenu(const QString &fieldHint)
{
    std::vector<std::shared_ptr<Gear::ISongIntent>> songIntents =
            _songListModel->songArray()->songIntents(_selectedSongs,
                                                     fieldHint.toStdString());

    for (int i = 0; i < songIntents.size(); ++i)
    {
        std::shared_ptr<Gear::ISongIntent> st = songIntents.at(i);
    }
}
