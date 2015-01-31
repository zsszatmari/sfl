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
                anchors.centerIn: parent

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
        }
    }
}
