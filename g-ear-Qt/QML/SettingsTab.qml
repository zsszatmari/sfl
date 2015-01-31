import QtQuick 2.0

Item {
    id: tabWidget

    default property alias content: stack.children

    property int current: 0

    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function setOpacities() {
        for (var i = 0; i < stack.children.length; ++i) {
            stack.children[i].opacity = (i == current ? 1 : 0)
            stack.children[i].z = (i == current ? 1 : 0)
            stack.children[i].enabled = (i == current ? true : false)
        }
    }

    Row {
        id: header
        spacing: 2
        Repeater {
            model: stack.children.length
            delegate: Rectangle {
                width: 62; height: 65
                color: "transparent"

                BorderImage {
                    anchors { fill: parent; leftMargin: 0; topMargin: 1; rightMargin: 1 }
                    border { left: 7; right: 7 }
                    source: {
                        if (tabWidget.current === index)
                            return "qrc:/images/Image/TabGrey.png"
                        else
                            return ""
                    }
                }

                Item {
                    anchors.fill: parent
                    Image {
                        id: tabIcon
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        width: 50
                        anchors.horizontalCenter: parent.horizontalCenter
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/images/Image/test.png"
                    }

                    Text {
                        anchors.top: tabIcon.bottom
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: stack.children[index].title
                        elide: Text.ElideRight
                        font.pixelSize: 13
                        color: "black"
                        renderType: Text.NativeRendering
                        font.family: "Consolas"
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: tabWidget.current = index
                }
            }
        }
    }

    Rectangle {
        id: horizontalLine
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width; height: 1
        color: "lightgrey"
    }

    Item {
        id: stack
        objectName: "settingsTabStack"
        width: tabWidget.width
        anchors.top: horizontalLine.bottom;
        anchors.topMargin: -1
        anchors.bottom: tabWidget.bottom
    }
}
