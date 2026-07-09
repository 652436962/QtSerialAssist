import QtQuick 2.15
import QtQuick.Window 2.15
import "../theme"

Window {
    id: root
    property var model: []
    property int currentIndex: 0
    property int itemWidth: 200
    signal itemSelected(int index)

    width: itemWidth
    height: Math.min(listView.contentHeight + 8, 300)
    flags: Qt.Popup | Qt.FramelessWindowHint
    color: DeepinTheme.card

    Rectangle {
        anchors.fill: parent; anchors.margins: 4; radius: DeepinTheme.radiusMedium
        color: DeepinTheme.card
        border.color: DeepinTheme.border; border.width: 1

        ListView {
            id: listView
            anchors.fill: parent; anchors.margins: 4
            clip: true; model: root.model
            delegate: Rectangle {
                width: listView.width; height: 34; radius: 4
                color: index === root.currentIndex ? Qt.rgba(DeepinTheme.blue.r, DeepinTheme.blue.g, DeepinTheme.blue.b, 0.15) : "transparent"
                Text {
                    anchors.fill: parent; leftPadding: 12; rightPadding: 12
                    text: modelData; font.family: DeepinTheme.fontFamily
                    font.pixelSize: DeepinTheme.fontSizeNormal
                    color: index === root.currentIndex ? DeepinTheme.blue : DeepinTheme.text
                    elide: Text.ElideRight; verticalAlignment: Text.AlignVCenter
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.itemSelected(index)
                }
            }
        }
    }
}
