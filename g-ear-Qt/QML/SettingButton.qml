import QtQuick 2.3

Item {
    id: settingButton
    property bool checked: false
    property string text: ""

    signal clicked()

    function setStateToPressed() {
        settingButton.state = 'pressed'
    }

    function setStateToIdle() {
        settingButton.state = 'idle'
    }

    function setStateToHovering() {
        settingButton.state = 'hovering'
    }

    state: 'idle'

    Rectangle {
        id: buttonBackground
        anchors.fill: parent
        radius: 4
    }

    Text {
        id: buttonText
        anchors.centerIn: parent
        text: settingButton.text
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true


        onEntered: {
            settingButton.setStateToHovering()
        }

        onExited: {
            if (settingButton.checked)
                settingButton.setStateToPressed()
            else
                settingButton.setStateToIdle()
        }

        onClicked: {
            settingButton.clicked()
        }
    }

    states: [
        State {
            name: "hovering"
            PropertyChanges { target: buttonBackground; color: "lightgrey" }
            PropertyChanges { target: settingButton; checked: settingButton.checked }
        },

        State {
            name: "pressed"
            PropertyChanges { target: buttonBackground; color: "green" }
            PropertyChanges { target: settingButton; checked: true }
        },

        State {
            name: "idle"
            PropertyChanges { target: buttonBackground; color: "lightblue" }
            PropertyChanges { target: settingButton; checked: false }
        }
    ]
}
