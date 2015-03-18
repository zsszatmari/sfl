import QtQuick 2.3
import QtQuick.Controls 1.2

Rectangle {
    id: leftBar


    ScrollView {
        id: topPart
        objectName: "leftBarTopPart"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: separatorLine.top
        frameVisible: false

        PlaylistView {
            anchors.fill: parent
        }
    }

    Rectangle {
        id: separatorLine
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomPart.top
        color: "gray"
    }

    Rectangle {
        id: bottomPart
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        color: "#f5f5f5"
        height: width

        Image {
            id: albumArt
            objectName: "albumImageObject"
            anchors.fill: parent
        }

        Item {
            id: nowPlayingTitle
            property int leftPadding: 0
            property int topPadding: 0
            property int bottomPadding: 0
            objectName: "nowPlayingTitleObject"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            height: topPadding + titleText.height + bottomPadding

            Text {
                id: titleText
                anchors.left: parent.left
                anchors.leftMargin: parent.leftPadding
                anchors.top: parent.top
                anchors.topMargin: parent.topPadding

                font.bold: true
                font.pixelSize: 18
                text: "NOW PLAYING"
            }
        }
    }
}
