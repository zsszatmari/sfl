import QtQuick 2.3

Image {
    property bool fillParent: false
    property int margins: 0
    anchors.fill: fillParent ? parent : undefined
    anchors.margins: margins
}

