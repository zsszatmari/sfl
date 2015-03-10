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
            color: "#f5f5f5"

            Image {
                objectName: "albumImageObject"
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height) * 9 / 10
                height: width
            }
        }
    }
}
