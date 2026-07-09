import QtQuick 2.15
import QtQuick.Window 2.15
import "../theme"

Rectangle {
    id: root
    property var model: []
    property int currentIndex: 0
    property alias font: label.font

    implicitWidth: 160
    implicitHeight: DeepinTheme.inputHeight
    color: DeepinTheme.inputBg
    radius: DeepinTheme.radiusSmall
    border.color: DeepinTheme.inputBorder
    border.width: 1

    Text {
        id: label
        anchors.left: parent.left; anchors.right: arrow.left
        anchors.leftMargin: 12; anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        text: model && currentIndex >= 0 && model.length > currentIndex ? String(model[currentIndex]) : ""
        font.family: DeepinTheme.fontFamily
        font.pixelSize: DeepinTheme.fontSizeNormal
        color: DeepinTheme.text
        elide: Text.ElideRight
    }

    Text {
        id: arrow
        anchors.right: parent.right; anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        text: "▾"; font.pixelSize: 12; color: DeepinTheme.subText
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            var wx = 0; var wy = 0
            if (root.Window && root.Window.window) {
                wx = root.Window.window.x; wy = root.Window.window.y
            }
            var localPos = root.mapToItem(null, 0, 0)
            var sx = localPos.x + wx
            var sy = localPos.y + root.height + 4 + wy
            var comp = Qt.createComponent("qrc:/qml/components/DropWindow.qml")
            if (comp.status === Component.Ready) {
                var win = comp.createObject(root)
                win.model = root.model
                win.currentIndex = root.currentIndex
                win.itemWidth = root.width + 60
                win.itemSelected.connect(function(idx) {
                    root.currentIndex = idx
                    win.close()
                })
                win.show()
                win.x = sx
                win.y = sy
            }
        }
    }
}
