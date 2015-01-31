import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQml 2.2

Item {
    TableView {
        id: songlistView
        objectName: "songListTableView"
        selectionMode: SelectionMode.ExtendedSelection
        onSortIndicatorColumnChanged: {
            var sortColumn = getColumn(sortIndicatorColumn);
            if (sortIndicatorOrder == Qt.AscendingOrder) {
                songListController.resortSongList(sortColumn.role, true);
            } else {
                songListController.resortSongList(sortColumn.role, false);
            }
        }

        onSortIndicatorOrderChanged: {
            var sortColumn = getColumn(sortIndicatorColumn);
            if (sortIndicatorOrder == Qt.AscendingOrder) {
                songListController.resortSongList(sortColumn.role, true);
            } else {
                songListController.resortSongList(sortColumn.role, false);
            }
        }

        rowDelegate: Component {
            Rectangle{
                color: {
                    if (styleData.row % 2 == 0)
                        return "white"
                    else
                        return "#E0E0E0"
                }

                Rectangle {
                    anchors.fill: parent
                    visible: styleData.selected
                    color: "steelblue"
                }
            }
        }

        itemDelegate: Item {
            Text {
                anchors.centerIn: parent
                color: styleData.textColor
                elide: styleData.elideMode
                text: styleData.value
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        if (styleData.selected) {
                            songlistView.currentRow = styleData.row
                        } else {
                            songlistView.selection.clear()
                            songlistView.currentRow = styleData.row
                            songlistView.selection.select(songlistView.currentRow)
                        }
                    }
                    songlistView.selection.forEach( function(rowIndex) {
                        songListController.clearSelectedSongs()
                        songListController.addSelectedSong(rowIndex)
                    }
                    )
                    songListController.popupContextMenu(songlistView.getColumn(styleData.column).role)
                }
            }
        }

        Menu {
            id: contextMenu
            objectName: "contextMenuObject"

            Instantiator {
                id: subInstantiator
                model: songListContextSubMenuModel
                Menu {
                    id: subMenu
                    title: model.menuText
                }
                onObjectAdded: {
                    console.log(object + " is added")
                    object.addItem("subtest0")
                    object.addItem("subtest1")
                    contextMenu.insertItem(index, object)
                }
                onObjectRemoved: contextMenu.removeItem(object)
            }

            Instantiator {
                model: songListContextMenuModel
                MenuItem {
                    text: model.menuText
                    onTriggered: songListController.conductMenuOrder(model.menuId)
                }
                onObjectAdded: contextMenu.insertItem(index, object)
                onObjectRemoved: contextMenu.removeItem(object)
            }
        }

        sortIndicatorVisible: true

        TableViewColumn {
            role: "source"; title: "Source";
            width: 90;
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "position"; title: "#";
            width: 14
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "artist"; title: "Artist";
            width: 90
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "track"; title: "No";
            width: 30
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "title"; title: "Title";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "durationMillis"; title: "Time";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "album"; title: "Album";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "genre"; title: "Genre";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "year"; title: "Year";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "rating"; title: "Rating";
            width: 90
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "playCount"; title: "Plays";
            width: 75
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "disc"; title: "Disc";
            width: 60
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "lastPlayed"; title: "Last Played";
            width: 135
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "creationDate"; title: "Added On";
            width: 120
            horizontalAlignment: Text.AlignHCenter;
        }
        TableViewColumn {
            role: "albumArtist"; title: "Album Artist";
            width: 180
            horizontalAlignment: Text.AlignHCenter;
        }

        anchors.fill: parent
        model: songListModel
    }
}
