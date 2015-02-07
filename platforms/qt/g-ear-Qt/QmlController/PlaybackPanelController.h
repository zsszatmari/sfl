#ifndef PLAYBACKPANELCONTROLLER_HPP
#define PLAYBACKPANELCONTROLLER_HPP

#include "QmlController.h"

#include "SignalConnection.h"

class PlaybackPanelController : public QmlController
{
    Q_OBJECT
public:
    explicit PlaybackPanelController(QQmlEngine *engine);
    ~PlaybackPanelController();

private slots:
    void setPlaybackConnection();

private:
    Base::SignalConnection _playingConnection;
    Base::SignalConnection _shuffleConnection;
    Base::SignalConnection _repeatConnection;
    Base::SignalConnection _elapsedConnection;
    Base::SignalConnection _remainingConnection;
    Base::SignalConnection _ratioConnection;
};

#endif // PLAYBACKPANELCONTROLLER_HPP
