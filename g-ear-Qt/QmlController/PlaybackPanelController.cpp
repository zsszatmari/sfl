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

PlaybackPanelController::PlaybackPanelController(QQmlEngine *engine)
    : QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("playbackController", this);

    connect(this, SIGNAL(windowReady()), this, SLOT(setPlaybackConnection()));
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

void PlaybackPanelController::setPlaybackConnection()
{
    QQuickItem *topBar = qmlWindow()->findChild<QQuickItem *>("topBarObjectName");
    auto paintable = Gear::IApp::instance()->themeManager()->style().get("general topbar").paintable();
    Painter painter(topBar);
    painter.setFillParent(true);
    painter.setZOrder(-1);
    paintable->paint(painter);

    _playingConnection = Gear::IApp::instance()->player()->playingConnector()
            .connect([this](const bool isPlaying)
    {
        QObject *playPauseButton = qmlWindow()->findChild<QObject *>("playPauseButtonObjectName");
        playPauseButton->setProperty("isPlaying", isPlaying);
        qDebug() << "Now the play status: " << isPlaying;
    });

    _shuffleConnection = Gear::IApp::instance()->player()->shuffle().connector()
            .connect([this](const bool isShuffle)
    {
        QObject *shuffleIconObject = qmlWindow()->findChild<QObject *>("shuffleIconObject");
        shuffleIconObject->setProperty("shuffleActive", isShuffle);
        qDebug() << "Now the shuffle status: " << isShuffle;
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
        qDebug() << "Now the repeat mode: " << (int)repeatMode;
    });

    _elapsedConnection = Gear::IApp::instance()->player()->elapsedTimeConnector()
            .connect([this](const std::string &str)
    {
        QObject *elapsedTime = qmlWindow()->findChild<QObject *>("elapsedTimeObjectName");
        elapsedTime->setProperty("elapsedTimeStr", QString::fromStdString(str));
        qDebug() << "Now the elapsed time: " << QString::fromStdString(str);
    });

    _remainingConnection = Gear::IApp::instance()->player()->remainingTimeConnector()
            .connect([this](const std::string &str)
    {
        QObject *remainingTime = qmlWindow()->findChild<QObject *>("remainingTimeObjectName");
        remainingTime->setProperty("remainingTimeStr", QString::fromStdString(str));
        qDebug() << "Now the remaining time: " << QString::fromStdString(str);
    });

    _ratioConnection = Gear::IApp::instance()->player()->songRatioConnector()
            .connect([this](const float ratio)
    {
        QObject *progressSlider = qmlWindow()->findChild<QObject *>("progressSliderObjectName");
        progressSlider->setProperty("progressRatio", ratio);
        qDebug() << "Now playing ratio is: " << ratio;
    });

    _SongInfoConnection = Gear::IApp::instance()->player()->songEntryConnector()
            .connect([this](const Gear::SongEntry songEntry)
    {
        QObject *songInfo = qmlWindow()->findChild<QObject *>("songInfoObjectName");
        songInfo->setProperty("title", QString::fromStdString(songEntry.song()->title()));
        songInfo->setProperty("artist", QString::fromStdString(songEntry.song()->artist()));
        songInfo->setProperty("album", QString::fromStdString(songEntry.song()->album()));
    });

    disconnect(this, SIGNAL(windowReady()), this, SLOT(setPlaybackConnection()));
}
