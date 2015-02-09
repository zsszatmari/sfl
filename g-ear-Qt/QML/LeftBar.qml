import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    id: leftBar

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        ScrollView {
            id: topPart
            objectName: "leftBarTopPart"
            height: 300
            Layout.minimumHeight: 200
            frameVisible: false

            PlaylistView {
                anchors.fill: parent
            }
        }

        Rectangle {
            id: bottomPart
            Layout.minimumHeight: 100
            color: "lightblue"

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: playModeArea.top


                width: {
                    var min = parent.width < parent.height ? parent.width : parent.height;
                    return min * 4 / 5;
                }
                height: width
                radius: 2

                Image {
                    id: bottomImage
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: "qrc:/images/Image/test.png"
                }
            }

            Item {
                id: playModeArea
                objectName: "playModeAreaObjectName"
                property int selectedRepeatModeIndex: 0
                property bool isShuffle: false
                anchors.bottom: parent.bottom
                width: 100
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter

                Item {
                    id: shuffleIconContainer
                    x: parent.width / 4 * 0
                    width: parent.width / 4
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        id: shuffleIcon
                        width: shuffleIconMouseArea.containsMouse ? 21 : 20
                        height: shuffleIconMouseArea.containsMouse ? 21 : 20
                        anchors.centerIn: parent

                        source: "qrc:/images/Image/Shuffle.png"
                        smooth: true
                        mipmap: true

                        MouseArea {
                            id: shuffleIconMouseArea
                            anchors.fill: parent
                        }
                    }
                }

                Item {
                    id: repeatPlaylistIconContainer
                    x: parent.width / 4 * 1
                    width: parent.width / 4
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        id: repeatPlaylistIcon
                        width: repeatPlaylistIconMouseArea.containsMouse ? 21 : 20
                        height: repeatPlaylistIconMouseArea.containsMouse ? 21 : 20
                        anchors.centerIn: parent
                        source: "qrc:/images/Image/RepeatPlaylist.png"
                        smooth: true
                        mipmap: true

                        MouseArea {
                            id: repeatPlaylistIconMouseArea
                            anchors.fill: parent
                        }
                    }
                }

                Item {
                    id: repeatOneSongIconContainer
                    x: parent.width / 4 * 2
                    width: parent.width / 4
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        id: repeatOneSongIcon
                        width: repeatOneSongIconMouseArea.containsMouse ? 21 : 20
                        height: repeatOneSongIconMouseArea.containsMouse ? 21 : 20
                        anchors.centerIn: parent
                        source: "qrc:/images/Image/RepeatOneSong.png"
                        smooth: true
                        mipmap: true

                        MouseArea {
                            id: repeatOneSongIconMouseArea
                            anchors.fill: parent
                        }
                    }
                }

                Item {
                    id: noRepeatIconContainer
                    x: parent.width / 4 * 3
                    width: parent.width / 4
                    anchors.verticalCenter: parent.verticalCenter
                    Image {
                        id: noRepeatIcon
                        width: noRepeatIconMouseArea.containsMouse ? 21 : 20
                        height: noRepeatIconMouseArea.containsMouse ? 21 : 20
                        anchors.centerIn: parent
                        source: "qrc:/images/Image/NoRepeat.png"
                        smooth: true
                        mipmap: true

                        MouseArea {
                            id: noRepeatIconMouseArea
                            anchors.fill: parent
                        }
                    }
                }

                Rectangle {
                    id: shuffleModeCover
                    width: 20; height: 20
                    radius: 20
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: shuffleIconContainer.left
                    visible: playModeArea.isShuffle

                    color: "orange"
                    opacity: 0.5
                }

                Rectangle {
                    id: repeatModeCover
                    width: 20; height: 3
                    radius: 1
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 5
                    anchors.horizontalCenter: {
                        switch (playModeArea.selectedRepeatModeIndex) {
                        case 0:
                            noRepeatIconContainer.horizontalCenter
                            break;
                        case 1:
                            repeatPlaylistIconContainer.horizontalCenter
                            break;
                        case 2:
                            repeatOneSongIconContainer.horizontalCenter
                            break;
                        default:
                            noRepeatIconContainer.horizontalCenter
                            break;
                        }
                    }

                    color: "red"
                }
            }
        }
    }
}
