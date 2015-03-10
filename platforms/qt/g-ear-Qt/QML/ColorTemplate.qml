import QtQuick 2.0

Rectangle {
    property bool fillParent: false
    anchors.fill: fillParent ? parent : undefined
}

