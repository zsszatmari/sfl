#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QQuickItem>
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

    _songListModel = std::make_shared<SongListModel>(nullptr);
    qmlEngine()->rootContext()->setContextProperty("songListModel", _songListModel.get());
    _contextMenuModel = std::make_shared<SongListContextMenuModel>(nullptr);
    _contextSubMenuModel = std::make_shared<SongListContextMenuModel>(nullptr);
    qmlEngine()->rootContext()->setContextProperty("songListContextMenuModel",
                                                   _contextMenuModel.get());
    qmlEngine()->rootContext()->setContextProperty("songListContextSubMenuModel",
                                                   _contextSubMenuModel.get());
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
    // test
    _contextMenuModel->clear();
    _contextSubMenuModel->clear();

    std::unique_ptr<SongListContextMenuModelItem> item(new SongListContextMenuModelItem);
    item->setMenuId(10);
    item->setMenuText("test1");
    _contextMenuModel->appendRow(std::move(item));

    std::unique_ptr<SongListContextMenuModelItem> item0(new SongListContextMenuModelItem);
    item0->setMenuId(0);
    item0->setMenuText("test0");
    _contextSubMenuModel->appendRow(std::move(item0));

    QObject *menuItem = qmlWindow()->findChild<QObject *>("contextMenuObject");

    QMetaObject::invokeMethod(menuItem, "popup");

//    std::vector<std::shared_ptr<Gear::ISongIntent>> songIntents =
//            _songListModel->songArray()->songIntents(_selectedSongs,
//                                                     fieldHint.toStdString());

//    std::vector<std::pair<std::string, std::vector<shared_ptr<Gear::ISongIntent>>>> subMenuSongIntents = _songListModel->songArray()->songIntentSubMenus(_selectedSongs);

//    for (int i = 0; i < songIntents.size(); ++i)
//    {
//        std::shared_ptr<Gear::ISongIntent> st = songIntents.at(i);
    //    }
}

void SongListController::conductMenuOrder(const int &menuId)
{
    qDebug() << "Do nothing, not implemented yet";
}
