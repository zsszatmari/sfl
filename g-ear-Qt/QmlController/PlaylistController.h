#ifndef PLAYLISTCONTROLLER_H
#define PLAYLISTCONTROLLER_H

#include <QObject>

#include "IPlaylist.h"
#include "QmlController.h"
#include "QmlModel/PlaylistCategoryModel.h"

class QQuickItem;

class PlaylistController : public QmlController
{
    Q_OBJECT
public:
    PlaylistController(QQmlEngine *engine);

    Q_INVOKABLE void userSelectedPlaylist(const QString &category,
                                          const QString &id,
                                          bool start);

    Q_INVOKABLE void doFilter(const QString &filter);

protected:
    void qmlWindowReady();

private:
    const std::shared_ptr<Gear::IPlaylist> findPlaylistById(const QString &category,
                                                               const QString &id);

private:
    std::shared_ptr<PlaylistCategoryModel> _playlistCategoryModel;
    Base::SignalConnection _playlistCategoryConnection ;
};

#endif // PLAYLISTCONTROLLER_H
