import QtQuick 2.3
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.1

Item {
    property alias settingsButton: settingButton
    RowLayout {
        anchors.fill: parent

        Image {
            id: shuffleIcon
            objectName: "shuffleIconObject"
            property bool shuffleActive: false
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 40

            mipmap: true
            smooth: true
            source: shuffleActive ? "image://themeIconProvider/ShuffleActive" : "image://themeIconProvider/ShuffleNormal"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    shuffleIcon.shuffleActive = !shuffleIcon.shuffleActive
                }
            }
        }

        Image {
            id: repeatIcon
            objectName: "repeatIconObject"
            property bool repeatActive: false
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: shuffleIcon.left
            anchors.leftMargin: 50

            mipmap: true
            smooth: true
            source: repeatActive ? "image://themeIconProvider/RepeatActive" : "image://themeIconProvider/RepeatNormal"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    repeatIcon.repeatActive = !repeatIcon.repeatActive
                }
            }
        }

        Label {
            anchors.centerIn: parent
            text: "0 songs"
        }

        SettingButton {
            id: settingButton

            function updateSettingButtonState() {
                if (settingsWindow.visible) {
                    settingButton.setStateToPressed()
                } else {
                    settingButton.setStateToIdle()
                }
            }

            text: "Settings"
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

