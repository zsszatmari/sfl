#include <QQmlContext>
#include <QDebug>

#include "PlaybackPanelController.h"
#include "IApp.h"
#include "IPlayer.h"
#include "Gui/ThemeManager.h"
#include "Gui/Style.h"
#include "Gui/Rules.h"
#include "Gui/IPaintable.h"
#include "sfl/just_ptr.h"
#include "Theme/Painter.h"
#include "PlaybackController.h"
#include "IPreferences.h"
#include "Global/PreferenceKey.h"

PlaybackPanelController::PlaybackPanelController(QQmlEngine *engine)
    : QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("playbackController", this);
    qmlEngine()->addImageProvider("albumImageProvider", _albumImageProvider.get());
}

PlaybackPanelController::~PlaybackPanelController()
{

}

void PlaybackPanelController::previous()
{
    Gear::IApp::instance()->player()->prev();
}

void PlaybackPanelController::next()
{
    Gear::IApp::instance()->player()->next();
}

void PlaybackPanelController::play()
{
    Gear::IApp::instance()->player()->play();
}

void PlaybackPanelController::setProgressRatio(const float &ratio)
{
    Gear::IApp::instance()->player()->setRatio(ratio);
}

void PlaybackPanelController::setVolume(const float &volume)
{
    Gear::PlaybackController::instance()->setVolume(volume);
}

void PlaybackPanelController::saveRatio(const float &ratio)
{
    Gear::IApp::instance()->preferences().setFloatForKey(PROGRESS_RATION_KEY, ratio);
}

void PlaybackPanelController::saveVolume(const float &volume)
{
    Gear::IApp::instance()->preferences().setFloatForKey(VOLUME_KEY, volume);
}

void PlaybackPanelController::qmlWindowReady()
{
    QQuickItem *topBar = qmlWindow()->findChild<QQuickItem *>("topBarObjectName");
    auto paintable = Gear::IApp::instance()->themeManager()->style().get("general topbar").paintable();
    Painter painter(topBar);
    painter.setFillParent(true);
    painter.setZOrder(-1);
    paintable->paint(painter);

    restoreRatio();
    restoreVolume();

    _playingConnection = Gear::IApp::instance()->player()->playingConnector()
            .connect([this](const bool isPlaying)
    {
        QObject *playPauseButton = qmlWindow()->findChild<QObject *>("playPauseButtonObjectName");
        playPauseButton->setProperty("isPlaying", isPlaying);
    });

    _shuffleConnection = Gear::IApp::instance()->player()->shuffle().connector()
            .connect([this](const bool isShuffle)
    {
        QObject *shuffleIconObject = qmlWindow()->findChild<QObject *>("shuffleIconObject");
        shuffleIconObject->setProperty("shuffleActive", isShuffle);
    });

    _repeatConnection = Gear::IApp::instance()->player()->repeat().connector()
            .connect([this](const Gear::IPlayer::Repeat repeatMode)
    {
        QObject *repeatIconObject = qmlWindow()->findChild<QObject *>("repeatIconObject");
        switch (repeatMode)
        {
        case Gear::IPlayer::Repeat::Off:
            repeatIconObject->setProperty("repeatActive", false);
            break;
        case Gear::IPlayer::Repeat::On:
            repeatIconObject->setProperty("repeatActive", true);
            break;
        case Gear::IPlayer::Repeat::OneSong:
            repeatIconObject->setProperty("repeatActive", true);
            break;
        }
    });

    _elapsedConnection = Gear::IApp::instance()->player()->elapsedTimeConnector()
            .connect([this](const std::string &str)
    {
        QObject *elapsedTime = qmlWindow()->findChild<QObject *>("elapsedTimeObjectName");
        elapsedTime->setProperty("elapsedTimeStr", QString::fromStdString(str));
    });

    _remainingConnection = Gear::IApp::instance()->player()->remainingTimeConnector()
            .connect([this](const std::string &str)
    {
        QObject *remainingTime = qmlWindow()->findChild<QObject *>("remainingTimeObjectName");
        remainingTime->setProperty("remainingTimeStr", QString::fromStdString(str));
    });

    _ratioConnection = Gear::IApp::instance()->player()->songRatioConnector()
            .connect([this](const float ratio)
    {
        QObject *progressSlider = qmlWindow()->findChild<QObject *>("progressSliderObject");
        bool pressed = progressSlider->property("pressed").toBool();
        if (!pressed)
        {
            progressSlider->setProperty("value", ratio);
        }
    });

    _SongInfoConnection = Gear::IApp::instance()->player()->songEntryConnector()
            .connect([this](const Gear::SongEntry songEntry)
    {
        QObject *songInfo = qmlWindow()->findChild<QObject *>("songInfoObjectName");
        songInfo->setProperty("title", QString::fromStdString(songEntry.song()->title()));
        songInfo->setProperty("artist", QString::fromStdString(songEntry.song()->artist()));
        songInfo->setProperty("album", QString::fromStdString(songEntry.song()->album()));
        QObject *albumImage = qmlWindow()->findChild<QObject *>("albumImageObject");
        long imageSize = albumImage->property("width").toInt() * albumImage->property("height").toInt();
        _promisedImage = Gear::IApp::instance()->player()->albumArt(imageSize);
        _AlbumImageConnection = _promisedImage->connector()
                .connect([this](const shared_ptr<Gui::IPaintable> &image)
        {
            if (image) {
                QQuickItem *imageItem = qmlWindow()->findChild<QQuickItem *>("albumImageObject");
                Painter albumImagePainter(imageItem);
                albumImagePainter.setAction(Painter::SetImageAction);
                albumImagePainter.setImageProvider("albumImageProvider", _albumImageProvider);
                image->paint(albumImagePainter);
            }
        });
    });
}

void PlaybackPanelController::restoreRatio()
{
    QObject *progressSlider = qmlWindow()->findChild<QObject *>("progressSliderObject");
    float ratio = Gear::IApp::instance()->preferences().floatForKey(PROGRESS_RATION_KEY);
    progressSlider->setProperty("value", ratio);
    setProgressRatio(ratio);
}

void PlaybackPanelController::restoreVolume()
{
    QObject *volumeSlider = qmlWindow()->findChild<QObject *>("volumeSliderObject");
    volumeSlider->setProperty("value", Gear::IApp::instance()->preferences().floatForKey(VOLUME_KEY));
}
