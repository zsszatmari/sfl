import QtQuick 2.0

ListView {
    id: playlistCategory
    property string selectedCategoryName: ""
    property int selectedIndex: -1
    model: playlistCategoryModel
    delegate: PlaylistDelegate {}
    focus: true
    spacing: 20

//    MouseArea {
//        propagateComposedEvents: true
//    }
}
