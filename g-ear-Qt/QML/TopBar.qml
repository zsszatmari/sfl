import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

Item {
    objectName: "topBarObjectName"
    height: 62

    CustomizeSystemMenu {
        width: 60; height: 20
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 2
    }

    Item {
        id: controlPanel
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 120

        Image {
            id: preButton
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            source: preButtonMouseArea.containsMouse ? "image://themeIconProvider/PrePushed" : "image://themeIconProvider/PreNormal"

            MouseArea {
                id: preButtonMouseArea
                anchors.fill: parent

                onClicked: {
                    playbackController.previous()
                }
            }
        }

        Image {
            id: playPauseButton
            objectName: "playPauseButtonObjectName"
            property bool isPlaying: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            source: {
                if (isPlaying) {
                    return playPauseButtonMouseArea.containsMouse ? "image://themeIconProvider/PausePushed" : "image://themeIconProvider/PauseNormal"
                } else {
                    return playPauseButtonMouseArea.containsMouse ? "image://themeIconProvider/PlayPushed" : "image://themeIconProvider/PlayNormal"
                }
            }

            MouseArea {
                id: playPauseButtonMouseArea
                anchors.fill: parent

                onClicked: {
                    playbackController.play()
                }
            }
        }

        Image {
            id: nextButton
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10
            source: nextButtonMouseArea.containsMouse ? "image://themeIconProvider/NextPushed" : "image://themeIconProvider/NextNormal"

            MouseArea {
                id: nextButtonMouseArea
                anchors.fill: parent

                onClicked: {
                    playbackController.next()
                }
            }
        }
    }

    Item {
        id: songInfo
        objectName: "songInfoObjectName"
        property string title: ""
        property string artist: ""
        property string album: ""

        anchors.left: controlPanel.right

        anchors.top: parent.top
        anchors.topMargin: 15

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20

        anchors.right: progressSliderItem.left

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Text {
                id: songTitle

                text: songInfo.title
                font.bold: true
            }

            Text {
                id: artistAndAlbum

                text: {
                    if (songInfo.artist != "" && songInfo.album != "") {
                        return songInfo.artist + " -- " + songInfo.album
                    } else {
                        return ""
                    }
                }
            }
        }
    }


    Item {
        id: progressSliderItem

        anchors.right: volumeSliderItem.left
        anchors.rightMargin: 50

        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 5

        width: 280
        height: 20


        Text {
            id: elapsedTime
            property string elapsedTimeStr: "00:00"
            objectName: "elapsedTimeObjectName"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: progressSlider.left
            anchors.rightMargin: 5
            text: elapsedTimeStr
        }

        Slider {
            id: progressSlider
            objectName: "progressSliderObjectName"
            property real progressRatio: 0.0
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: 200
            value: progressRatio

            style: SliderStyle {
                groove: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 5
                    Rectangle {
                        width: parent.width * control.value
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: "#1E90FF"
                        radius: 5
                    }
                    color: "gray"

                    radius: 5
                }
                handle: Rectangle {
                    anchors.centerIn: parent
                    color: control.pressed ? "white" : "lightgray"
                    border.color: "gray"
                    border.width: 1
                    implicitWidth: 15
                    implicitHeight: 15
                    radius: 15
                }
            }
        }

        Text {
            id: remainingTime
            property string remainingTimeStr: "00:00"
            objectName: "remainingTimeObjectName"
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: progressSlider.right
            anchors.leftMargin: 5
            text: remainingTimeStr
        }
    }


    Item {
        id: volumeSliderItem

        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: 5
        width: 180
        height: 20

        Image {
            id: volumeIcon
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            width: 20
            height: 20
            smooth: true
            source: "qrc:/images/Image/player_volume.png"
        }

        Slider {
            id: volumeSlider
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: volumeIcon.right
            anchors.leftMargin: 5
            anchors.right: parent.right
            value: 0.5

            style: SliderStyle {
                groove: Rectangle {
                    implicitWidth: 80
                    implicitHeight: 5
                    Rectangle {
                        width: parent.width * control.value
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        color: "#1E90FF"
                        radius: 5
                    }
                    color: "gray"

                    radius: 5
                }
                handle: Rectangle {
                    anchors.centerIn: parent
                    color: control.pressed ? "white" : "lightgray"
                    border.color: "gray"
                    border.width: 1
                    implicitWidth: 15
                    implicitHeight: 15
                    radius: 15
                }

            }
        }
    }
}

