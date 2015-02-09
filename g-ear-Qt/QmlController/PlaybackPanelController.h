#ifndef PLAYBACKPANELCONTROLLER_HPP
#define PLAYBACKPANELCONTROLLER_HPP

#include "QmlController.h"

#include "SignalConnection.h"

#include <boost/mpl/next_prior.hpp>

class PlaybackPanelController : public QmlController
{
    Q_OBJECT
public:
    explicit PlaybackPanelController(QQmlEngine *engine);
    ~PlaybackPanelController();

    Q_INVOKABLE void previous();
    Q_INVOKABLE void next();
    Q_INVOKABLE void play();

private slots:
    void setPlaybackConnection();

private:
    Base::SignalConnection _playingConnection;
    Base::SignalConnection _shuffleConnection;
    Base::SignalConnection _repeatConnection;
    Base::SignalConnection _elapsedConnection;
    Base::SignalConnection _remainingConnection;
    Base::SignalConnection _ratioConnection;
    Base::SignalConnection _SongInfoConnection;
};

#endif // PLAYBACKPANELCONTROLLER_HPP
