#ifndef PLAYBACKPANELCONTROLLER_H
#define PLAYBACKPANELCONTROLLER_H

#include "QmlController.h"
#include "PromisedImage.h"
#include "SignalConnection.h"
#include "Theme/ImageProvider.h"

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
    Q_INVOKABLE void setProgressRatio(const float &ratio);
    Q_INVOKABLE void setVolume(const float &volume);
    Q_INVOKABLE void saveRatio(const float &ratio);
    Q_INVOKABLE void saveVolume(const float &volume);

protected:
    void qmlWindowReady();

private slots:
    void restoreRatio();
    void restoreVolume();

private:
    Base::SignalConnection _playingConnection;
    Base::SignalConnection _shuffleConnection;
    Base::SignalConnection _repeatConnection;
    Base::SignalConnection _elapsedConnection;
    Base::SignalConnection _remainingConnection;
    Base::SignalConnection _ratioConnection;
    Base::SignalConnection _SongInfoConnection;
    std::shared_ptr<Gear::PromisedImage> _promisedImage;
    Base::SignalConnection _AlbumImageConnection;
    std::shared_ptr<ImageProvider> _albumImageProvider = std::make_shared<ImageProvider>();
};

#endif // PLAYBACKPANELCONTROLLER_H
