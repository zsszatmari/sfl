import QtQuick 2.3
import QtQuick.Controls 1.2

Component {
    ListView {
        id: playlistView

        width: parent.width
        height: 28 * playlistCount + 30

        header: Text {
            id: playlistHeader
            text: categoryName
            color: "grey"
            font.bold: true
        }

//        highlight: Rectangle { color: "steelblue"; radius: 1 }
//        highlightFollowsCurrentItem: true
        focus: true
        currentIndex: categoryName === playlistCategory.selectedCategoryName ? playlistCategory.selectedIndex : -1

        model: playlistModel
        delegate: Component {
            Item {
                id: itemContainer
                property string playlistID: playlistId

                width: parent.width
                height: 28

                MouseArea {
                    id: itemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    preventStealing: true
                    propagateComposedEvents: true
                    onClicked: {
                        playlistCategory.selectedCategoryName = categoryName
                        playlistCategory.selectedIndex = index
                        playlistController.userSelectedPlaylist(categoryName,
                                                                itemContainer.playlistID,
                                                                false)
                    }
                    onDoubleClicked: {
                        playlistCategory.selectedCategoryName = categoryName
                        playlistCategory.selectedIndex = index
                        playlistController.userSelectedPlaylist(categoryName,
                                                                itemContainer.playlistID,
                                                                true)
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "lightsteelblue"
                    radius: 2
                    visible: {
                        if (itemMouseArea.containsMouse)
                            return true
                        else
                            return false
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "steelblue"
                    radius: 2
                    visible: {
                        if (playlistView.currentIndex === index)
                            return true
                        else
                            return false
                    }
                }

                Text {
                    id: playlistText
                    x: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: playlistName
                }
            }
        }
    }
}

