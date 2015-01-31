#ifndef SONGLISTCONTROLLER_H
#define SONGLISTCONTROLLER_H

#include <QObject>
#include "QmlController.h"
#include "QmlModel/SongListModel.h"
#include "QmlModel/SongListContextMenuModel.h"
#include "SongEntry.h"

class QQmlEngine;
class QQuickWindow;

class SongListController : public QmlController
{
    Q_OBJECT
public:
    explicit SongListController(QQmlEngine *engine,
                                QObject *parent = 0);

    Q_INVOKABLE void resortSongList(const QString &identifier,
                                    const bool &isAscending);

    Q_INVOKABLE void clearSelectedSongs();

    Q_INVOKABLE void addSelectedSong(int rowIndex);

    Q_INVOKABLE void popupContextMenu(const QString &fieldHint);

    Q_INVOKABLE void conductMenuOrder(const int &menuId);

private:
    std::vector<Gear::SongEntry> _selectedSongs;
    std::shared_ptr<SongListModel> _songListModel;
    std::shared_ptr<SongListContextMenuModel> _contextMenuModel;
    std::shared_ptr<SongListContextMenuModel> _contextSubMenuModel;
};

#endif // SONGLISTCONTROLLER_H
