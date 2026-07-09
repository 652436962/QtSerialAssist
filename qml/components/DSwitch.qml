import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"

RowLayout {
    id: root

    property alias text: label.text
    property bool checked: false

    signal toggled(bool checked)

    spacing: DeepinTheme.spacingSmall

    Text {
        id: label
        font.family: DeepinTheme.fontFamily
        font.pixelSize: DeepinTheme.fontSizeSmall
        color: DeepinTheme.subText
        Layout.fillWidth: true
    }

    Rectangle {
        id: track
        width: 40
        height: 22
        radius: 11
        color: root.checked ? DeepinTheme.blue : (DeepinTheme.isDark ? "#30363d" : "#d0d7de")

        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            id: thumb
            width: 18
            height: 18
            radius: 9
            color: "#ffffff"
            x: root.checked ? track.width - width - 2 : 2
            y: 2

            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

            Rectangle {
                anchors.fill: parent
                radius: parent.radius
                color: "transparent"
                border.color: Qt.rgba(0, 0, 0, 0.08)
                border.width: 1
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.checked = !root.checked
                root.toggled(root.checked)
            }
        }
    }
}
