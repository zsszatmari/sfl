import QtQuick 2.0

Canvas {
    property var colorStops
    anchors.fill: parent

    onPaint: {
        var ctx = getContext("2d")

        var gradient = ctx.createLinearGradient(0, 0, 0, height)
        for (var pos in colorStops) {
            gradient.addColorStop(parseFloat(pos), colorStops[pos])
        }

        ctx.fillStyle = gradient
        ctx.fillRect(0, 0, width, height)
    }
}
