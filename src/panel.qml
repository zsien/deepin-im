import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml.Models 2.15
import Qt.labs.platform 1.1

Window {
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    x: kimpanel.pos.x
    y: kimpanel.pos.y
    width: container.width
    height: container.height
    visible: kimpanel.showAux || kimpanel.showPreedit || kimpanel.showLookupTable
    title: qsTr("Hello World")

    Column {
        id: container

        Row {
            Row {
                visible: kimpanel.showAux

                Text {
                    text: kimpanel.aux.text
                }
                Text {
                    text: kimpanel.aux.attr
                }
            }

            Row {
                visible: kimpanel.showPreedit

                Text {
                    text: kimpanel.preedit.text.slice(0, kimpanel.preeditCaretPos) + "|" + kimpanel.preedit.text.slice(kimpanel.preeditCaretPos)
                }
                Text {
                    text: kimpanel.preedit.attr
                }
            }
        }

        Row {
            Repeater{
                visible: kimpanel.showLookupTable

                model: kimpanel.lookupTable
                delegate: Row {
                    Text {
                        id: label
                        text: modelData.label
                    }
                    Text {
                        id: text
                        text: modelData.text
                    }
                    Text {
                        id: attr
                        text: modelData.attr
                    }
                }
            }
        }
    }

    SystemTrayIcon {
        visible: kimpanel.properties.length != 0
        icon.name: kimpanel.properties.length != 0 ? kimpanel.properties[0].iconName : ""

        menu: Menu {
            id: contextMenu

            Instantiator {
                model: kimpanel.properties

                delegate: MenuItem {
                    text: modelData.shortText
                    icon.name: modelData.iconName
                    onTriggered: kimpanel.menuTriggered(modelData.name)
                }

                onObjectAdded: function(index, object) {
                    contextMenu.insertItem(index, object)
                }
                onObjectRemoved: function(index, object) {
                    contextMenu.removeItem(object)
                }
            }
        }
    }
}
