import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: switchItem
    property string description: ""
    property alias isChecked: switchControl.checked

    signal checkedChanged(bool checked)

    onIsCheckedChanged: {
        checkedChanged(isChecked)
    }

    width: 200
    height: 20

    Label {
        anchors.left: parent.left
        text: description
    }

    Switch {
        id: switchControl
        anchors.right: parent.right

        style: SwitchStyle {
            groove: BorderImage {
                width: 80
                height: 18
                source: "qrc:/images/Image/SwitchGroove.png"
                Item {
                    anchors.left: parent.left
                    width: parent.width /2; height: parent.height
                    Text {
                        anchors.centerIn: parent
                        text: "ON"
                        color: "white"
                        font.family: "Consolas"
                        font.pixelSize: 13
                    }
                    Rectangle {
                        anchors.fill: parent
                        color: "lightblue"
                        visible: enabled ? false : true
                    }
                }
                Item {
                    anchors.right: parent.right
                    width: parent.width /2; height: parent.height
                    Text {
                        anchors.centerIn: parent
                        text: "OFF"
                        color: "white"
                        font.family: "Consolas"
                        font.pixelSize: 13
                    }
                }
            }
        }
    }
}
