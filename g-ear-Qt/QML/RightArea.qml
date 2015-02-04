import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQml 2.2
import QtQuick.Layouts 1.1

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
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                color: styleData.textColor
                elide: Text.ElideRight
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


            function addSubMenu(menuId, title, anObject) {
                var mySubMenu = contextMenu.addMenu(title)
                mySubMenu.objectName = menuId
                for (var key in anObject) {
                    var subItem = mySubMenu.addItem(anObject[key])
                    subItem.objectName = key
                    subItem.myTriggered = function() {
                        songListController.conductMenuOrder(subItem.objectName)
                    }
                    subItem.triggered.connect(subItem.myTriggered)
                }
            }

            function addNormalMenu(menuId, text) {
                var myMenuItem = contextMenu.addItem(text)
                myMenuItem.objectName = menuId
                myMenuItem.myTriggered = function() {
                    songListController.conductMenuOrder(myMenuItem.objectName)
                }
                myMenuItem.triggered.connect(myMenuItem.myTriggered)
            }

            function removeMenu(menuId) {
                for (var i = 0; i < contextMenu.items.length; ++i) {
                    if (contextMenu.items[i].objectName === menuId.toString()) {
                        var toBeRemoved = contextMenu.items[i]
                        contextMenu.removeItem(toBeRemoved)
                        break
                    }
                }
            }
        }

        sortIndicatorVisible: true

//        headerDelegate: Component {
//            Text {
//                anchors.verticalCenter: parent.verticalCenter
//                width: parent.width
//                color: "black"
//                elide: Text.ElideRight
//                text: styleData.value
//            }
//        }

        TableViewColumn {
            objectName: "sourceTableViewColumn"
            role: "source"; title: "Source";
            width: 100
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 90) {
                    width = 90
                }
            }
        }
        TableViewColumn {
            role: "position"; title: "#";
            horizontalAlignment: Text.AlignLeft;
            width: 30
            onWidthChanged: {
                if (width < 15) {
                    width = 15
                }
            }
        }
        TableViewColumn {
            role: "artist"; title: "Artist";
            width: 120
            horizontalAlignment: Text.AlignLeft;
            onWidthChanged: {
                if (width < 90) {
                    width = 90
                }
            }
        }
        TableViewColumn {
            role: "track"; title: "No";
            width: 50
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 30) {
                    width = 30
                }
            }
        }
        TableViewColumn {
            role: "title"; title: "Title";
            width: 75
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "durationMillis"; title: "Time";
            width: 120
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "album"; title: "Album";
            width: 75
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "genre"; title: "Genre";
            width: 100
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "year"; title: "Year";
            width: 100
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "rating"; title: "Rating";
            width: 100
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 90) {
                    width = 90
                }
            }
        }
        TableViewColumn {
            role: "playCount"; title: "Plays";
            width: 80
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 75) {
                    width = 75
                }
            }
        }
        TableViewColumn {
            role: "disc"; title: "Disc";
            width: 70
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 60) {
                    width = 60
                }
            }
        }
        TableViewColumn {
            role: "lastPlayed"; title: "Last Played";
            width: 140
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 135) {
                    width = 135
                }
            }
        }
        TableViewColumn {
            role: "creationDate"; title: "Added On";
            width: 130
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 120) {
                    width = 120
                }
            }
        }
        TableViewColumn {
            role: "albumArtist"; title: "Album Artist";
            width: 200
            horizontalAlignment: Text.AlignLeft;

            onWidthChanged: {
                if (width < 180) {
                    width = 180
                }
            }
        }

        anchors.fill: parent
        model: songListModel
    }
}
