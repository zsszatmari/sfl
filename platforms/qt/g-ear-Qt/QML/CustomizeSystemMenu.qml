import QtQuick 2.4

Row {
    spacing: 0
    Item {
        width: 20
        height: 20
        id: minimizeButton
        Rectangle {
            anchors.fill: parent
            color: minimizeButtonMouseArea.pressed ? "#3A5FCD" : "#4876FF"
            visible: minimizeButtonMouseArea.containsMouse
            radius: 1
        }

        Image {
            z: 1
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/Image/Minimize.png"
            smooth: true
            mipmap: true
            scale: minimizeButtonMouseArea.pressed ? 1.2 : 1
        }

        MouseArea {
            id: minimizeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                mainWindowController.minimizeWindow()
            }
        }
    }

    Item {
        id: maximizeButton
        width: 20
        height: 20
        Rectangle {
            anchors.fill: parent
            color: maximizeButtonMouseArea.pressed ? "#3A5FCD" : "#4876FF"
            visible: maximizeButtonMouseArea.containsMouse
            radius: 1
        }

        Image {
            id: maximizeIcon
            z: 1
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/Image/Maximize.png"
            smooth: true
            mipmap: true
            scale: maximizeButtonMouseArea.pressed ? 1.2 : 1
        }

        MouseArea {
            id: maximizeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                if (mainWindowController.windowMaximized()) {
                    mainWindowController.showNormalWindow()
                    maximizeButton.state = "normal"
                } else {
                    mainWindowController.maximizeWindow()
                    maximizeButton.state = "maximized"
                }
            }
        }

        states: [
            State {
                name: "maximized"
                PropertyChanges { target: maximizeIcon; source: "qrc:/images/Image/ShowNormal.png" }
            }
        ]
    }

    Item {
        id: closeButton
        width: 20
        height: 20
        Rectangle {
            anchors.fill: parent
            color: closeButtonMouseArea.pressed ? "#FF8C00" : "orange"
            visible: closeButtonMouseArea.containsMouse
            radius: 1
        }

        Image {
            z: 1
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/Image/Close.png"
            smooth: true
            mipmap: true
            scale: closeButtonMouseArea.pressed ? 1.2 : 1
        }

        MouseArea {
            id: closeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                mainWindowController.closeWindow()
            }
        }
    }
}

