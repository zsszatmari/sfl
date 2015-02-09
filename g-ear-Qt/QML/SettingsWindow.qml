import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.2

Window
{
    id: settingswindow
    title: "Preferences"
    visible: false
    flags: "Tool"
    width: 400; height: 200;

    SettingsTab {
        id: settingsTab
        anchors.fill: parent
        anchors.margins: 2
    }
}
