import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.1

ApplicationWindow {
    id: mainwindow

    width: 1000
    height: 600

    TopBar {
        id: topBar
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        height: 80
    }

    SplitView {
        anchors.left: parent.left
        anchors.top: topBar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        orientation: Qt.Horizontal

        LeftBar {
            id: leftBar
            width: 200
            Layout.minimumWidth: 100
            Layout.maximumWidth: 500
        }

        RightArea {
            Layout.minimumWidth: 500
        }
    }

    statusBar: StatusBar {
        anchors.right: mainwindow.right

        RowLayout {
            anchors.fill: parent

            Label {
                anchors.centerIn: parent
                text: "0 songs"
            }

            SettingButton {
                id: settingsbutton

                function updateSettingButtonState() {
                    if (settingsWindow.visible) {
                        settingsbutton.setStateToPressed()
                    } else {
                        settingsbutton.setStateToIdle()
                    }
                }

                text:"Settings"
                width: 80
                height: 22
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    settingsWindow.visible = !settingsWindow.visible
                    updateSettingButtonState()
                }
            }
        }
    }

    Item {
        anchors.centerIn: parent
        SettingsWindow {
            id: settingsWindow
            onVisibleChanged: {
                settingsbutton.updateSettingButtonState()
            }
        }
    }


    MessageDialog {
        id: messageDialog
        objectName: "messageDialogObjectName"
        property string menuId: ""
        title: "Information"
        visible: false
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            songListController.conductMenuOrder(menuId, false)
        }
    }
}
