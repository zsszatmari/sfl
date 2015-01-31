#ifndef SONGLISTMODEL_H
#define SONGLISTMODEL_H

#include <QAbstractTableModel>
// workaround for https://bugreports.qt-project.org/browse/QTBUG-29331
#ifndef Q_MOC_RUN
#include "SignalConnection.h"
#include "IPlaylist.h"
#include "ISongArray.h"
#include "ValidPtr.h"
#include "Chain.h"
#include "SongEntry.h"
#endif

//class QQuickWindow;

class SongListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SongListModel(QObject *parent = 0);

    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QHash<int, QByteArray> roleNames() const;

    inline const std::shared_ptr<Gear::ISongArray> &songArray() const
    {
        return _songArray;
    }

    inline const Gear::SongView &songs() const
    {
        return _songs;
    }

private:
    void reloadTable();

    Base::SignalConnection _playlistConnection;
    Base::SignalConnection _updateConnection;
    std::shared_ptr<Gear::IPlaylist> _playlist;
    std::shared_ptr<Gear::ISongArray> _songArray;

    mutable Gear::SongView _songs;
};

#endif // SONGLISTMODEL_H
