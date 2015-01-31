#include <QQuickItem>
#include <QDateTime>

#include "SongListModel.h"
#include "IApp.h"

using namespace Gear;

#define method SongListModel::

method SongListModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    // assume that this object lives forever. otherwise this would be outdated.
    _playlistConnection = Gear::IApp::instance()->selectedPlaylistConnector().connect([this](const std::pair<Gear::PlaylistCategory,
                                                                                      std::shared_ptr<Gear::IPlaylist>> &p){
        _playlist = p.second;
        if (_playlist) {
            qDebug() << "selected playlist: " << _playlist->playlistId().c_str() << " "
                     << _playlist->songArray()->predicateAsString().c_str();
            _songArray = _playlist->songArray();
            reloadTable();
        }
    });
}

void method reloadTable()
{
    if (_songArray) {
        _songs = _songArray->songs([this](const SongView &view,SongView::Event event, size_t offset, size_t size){

            qDebug() << "song list updated: " << view.size();
            if (!(view == this->_songs)) {

                qDebug() << "song list not this";
                return;
            }
            this->endResetModel();

            //[self showCount:_visibleSongs.size()];
        });
        //[self showCount:_visibleSongs.size()];
    }

/*    if (deselectOnNextReload) {
        [musicListView deselectAll:nil];
        deselectOnNextReload = NO;
    }
*/
    this->endResetModel();
    //[self.albumViewController reload];
}


QHash<int, QByteArray> method roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(0, "source");
    names.insert(1, "position");
    names.insert(2, "artist");
    names.insert(3, "track");
    names.insert(4, "title");
    names.insert(5, "durationMillis");
    names.insert(6, "album");
    names.insert(7, "genre");
    names.insert(8, "year");
    names.insert(9, "rating");
    names.insert(10, "playCount");
    names.insert(11, "disc");
    names.insert(12, "lastPlayed");
    names.insert(13, "creationDate");
    names.insert(14, "albumArtist");

    return names;
}

int method columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 15;
}

QVariant method data(const QModelIndex &index, int role) const
{
    _songs.setVisibleRange(std::max(0,index.row()-40), 80);
    QString roleStr = roleNames().value(role);
    QVariant retVal;

    switch (role)
    {
    case 0:
        // column title: 'Source' identifier: 'source' don't display any content for now,
        // there will be an icon displayed here later
        break;
    case 1:
        // column title: '#'    identifier: 'position'
        // but don't display any content. If the user clicks on column header,
        // order by 'identifier'
        break;
    case 2:
        // column title: 'Artist' identifier: 'artist' a text should be displayed,
        // similarly as now the title is
        retVal = QString::fromStdString(_songs.at(index.row()).song()->stringForKey(roleStr.toStdString()));
        break;
    case 3:
        // column title: 'No'  identifier 'track' an integer must be displayed,
        // get with intForKey(). but must be left empty if zero is returned.
        // In general, don't display zeros anywhere, neither at time, plays, disc, year, etc...
    {
        int64_t trackNumber = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (trackNumber > 0)
        {
            retVal = trackNumber;
        }
        break;
    }
    case 4:
        // column title: 'Title' identifier: 'tttle' this one is there already
        retVal = QString::fromStdString(_songs.at(index.row()).song()->stringForKey(roleStr.toStdString()));
        break;
    case 5:
        // column title: 'Time' identifier: 'durationMillis' time must be displayed in format 06:09.
        // It's in millseconds so divide by 1000 first.
    {
        int64_t durationMillis = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (durationMillis > 0)
        {
            int64_t seconds = durationMillis / 1000 % 60;
            int64_t minutes = durationMillis / 1000 / 60;
            QString secondsStr = seconds < 10 ? "0" + QString::number(seconds) : QString::number(seconds);
            QString minutesStr = minutes < 10 ? "0" + QString::number(minutes) : QString::number(minutes);
            retVal = minutesStr + ":" + secondsStr;
        }
        break;
    }
    case 6:
        // column title: 'Album' identifier: 'album'
        retVal = QString::fromStdString(_songs.at(index.row()).song()->stringForKey(roleStr.toStdString()));
        break;
    case 7:
        // column title: 'Genre' identifier: 'genre'
        retVal = QString::fromStdString(_songs.at(index.row()).song()->stringForKey(roleStr.toStdString()));
        break;
    case 8:
    {
        // column title: 'Year' identifier: 'year' (an integer)
        int64_t year = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (year != 0)
        {
            retVal = year;
        }
        break;
    }
    case 9:
        // column title: 'Rating' identifier: 'rating'   Display an integer for now,
        // but clickable icons (like, stars) will be here later
    {
        int64_t rating = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (rating != 0)
        {
            retVal = rating;
        }
        break;
    }
    case 10:
        // column title: 'Plays' identifier: 'playCount'  display an integer but nothing if zero
    {
        int64_t playCount = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (playCount > 0)
        {
            retVal = playCount;
        }
        break;
    }
    case 11:
        // column title: 'Disc' identifier: 'disc' integer or zero
    {
        int64_t disc = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (disc > 0)
        {
            retVal = disc;
        }
        break;
    }
    case 12:
        // column title: 'Last Played' identifier: 'lastPlayed' an integer is returned.
        // if zero, display nothing, if non-zero, treat it as nanoseconds since the UNIX epoch,
        // and display according to the current locales
    {
        int64_t lastPlayed = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (lastPlayed > 0)
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDateTime lastPlayedTime = currentDateTime.addMSecs(-lastPlayed / 1000000);
            retVal = lastPlayedTime.toString(Qt::SystemLocaleShortDate);
        }
        break;
    }
    case 13:
        // column title: 'Added On' identifier: 'creationDate' similar as previous
    {
        int64_t creationDate = _songs.at(index.row()).song()->intForKey(roleStr.toStdString());
        if (creationDate > 0)
        {
            QDateTime currentDateTime = QDateTime::currentDateTime();
            QDateTime creationDateTime = currentDateTime.addMSecs(-creationDate / 1000000);
            retVal = creationDateTime.toString(Qt::SystemLocaleShortDate);
        }
        break;
    }
    case 14:
        // column title: 'Album Artist' identifier: 'albumArtist'
    {
        retVal = QString::fromStdString(_songs.at(index.row()).song()->stringForKey(roleStr.toStdString()));
        break;
    }
    default:
        break;
    }

    return retVal;
}

int	method rowCount(const QModelIndex & parent) const
{
    // qDebug() << "row count: " << _songs.size();
    Q_UNUSED(parent);
    return _songs.size();
}
