#include <QQmlContext>
#include <QDebug>

#include "PlaybackPanelController.h"
#include "IApp.h"
#include "IPlayer.h"

PlaybackPanelController::PlaybackPanelController(QQmlEngine *engine)
    : QmlController(engine)
{
    qmlEngine()->rootContext()->setContextProperty("playbackController", this);

    connect(this, SIGNAL(windowReady()), this, SLOT(setPlaybackConnection()));
}

PlaybackPanelController::~PlaybackPanelController()
{

}

void PlaybackPanelController::setPlaybackConnection()
{
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
        qDebug() << "Now the shuffle status: " << isShuffle;
    });

    _repeatConnection = Gear::IApp::instance()->player()->repeat().connector()
            .connect([this](const Gear::IPlayer::Repeat repeatMode)
    {
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
        qDebug() << "Now playing ratio is: " << ratio;
    });

    disconnect(this, SIGNAL(windowReady()), this, SLOT(setPlaybackConnection()));
}
