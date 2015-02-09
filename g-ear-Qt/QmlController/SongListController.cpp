#include <QQmlEngine>
#include <QQuickWindow>
#include <QQmlContext>
#include <QQuickItem>
#include <iostream>
#include <QUuid>
#include <QApplication>

#include "SongListController.h"

#include "SongSortOrder.h"
#include "IApp.h"
#include "IPlaylist.h"
#include "ISongIntent.h"
#include "IPlayer.h"

SongListController::SongListController(QQmlEngine *engine) :
    QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("songListController", this);

    _songListModel = std::make_shared<SongListModel>(nullptr);
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
    QObject *menuItem = qmlWindow()->findChild<QObject *>("contextMenuObject");
    for (auto it = _menuItems.begin(); it != _menuItems.end(); ++it)
    {
        QMetaObject::invokeMethod(menuItem, "removeMenu",
                Q_ARG(QVariant, QVariant(*it)));
    }
    _menuItems.clear();
    _allMenuItems.clear();

    auto subMenuSongIntents = _songListModel->songArray()->songIntentSubMenus(_selectedSongs);

    for (auto it = subMenuSongIntents.begin(); it != subMenuSongIntents.end(); ++it)
    {
        QString title = QString::fromStdString((*it).first);
        auto subMenus = (*it).second;
        QVariantMap map;
        for (auto i = subMenus.begin(); i != subMenus.end(); ++i)
        {
            QString menuId = QUuid::createUuid().toString();
            map.insert(menuId, QString::fromStdString((*i)->menuText()));
            _allMenuItems.insert(menuId, (*i));
        }

        QString tempSubId = QUuid::createUuid().toString();
        QMetaObject::invokeMethod(menuItem, "addSubMenu",
                Q_ARG(QVariant, QVariant(tempSubId)),
                Q_ARG(QVariant, QVariant(title)),
                Q_ARG(QVariant, QVariant::fromValue(map)));
        _menuItems.push_back(tempSubId);
    }

    auto songIntents = _songListModel->songArray()->songIntents(_selectedSongs,
                                                     fieldHint.toStdString());

    for (auto it = songIntents.begin(); it != songIntents.end(); ++it)
    {
        QString tempId = QUuid::createUuid().toString();
        QMetaObject::invokeMethod(menuItem, "addNormalMenu",
                Q_ARG(QVariant, QVariant(tempId)),
                Q_ARG(QVariant, QVariant(QString::fromStdString((*it)->menuText()))));

        _menuItems.push_back(tempId);
        _allMenuItems.insert(tempId, (*it));
    }

    QMetaObject::invokeMethod(menuItem, "popup");
}

void SongListController::conductMenuOrder(const QString &menuId, const bool &needConfirm)
{
    auto activatedMenu = _allMenuItems[menuId];
    if (activatedMenu->confirmationNeeded(_selectedSongs) && needConfirm)
    {
        auto confirmDialog = qmlWindow()->findChild<QObject *>("messageDialogObjectName");
        confirmDialog->setProperty("text", QString::fromStdString(activatedMenu->confirmationText(_selectedSongs)));
        confirmDialog->setProperty("menuId", menuId);
        confirmDialog->setProperty("visible", true);
    }
    else
    {
        activatedMenu->apply(_selectedSongs);
    }
}

void SongListController::playSong(const int &rowIndex)
{
    Gear::IApp::instance()->player()->play(_songListModel->songs().at(rowIndex));
}
