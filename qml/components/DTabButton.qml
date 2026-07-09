import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme"

Rectangle {
    id: root

    property string text: ""
    property bool selected: false

    signal clicked()

    color: "transparent"
    implicitWidth: label.implicitWidth + DeepinTheme.spacingXLarge * 2
    implicitHeight: 42

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        font.family: DeepinTheme.fontFamily
        font.pixelSize: DeepinTheme.fontSizeNormal
        font.weight: root.selected ? Font.DemiBold : Font.Normal
        color: root.selected ? DeepinTheme.blue : DeepinTheme.subText
        Behavior on color { ColorAnimation { duration: 150 } }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: root.selected ? parent.width * 0.5 : 0
        height: 2
        radius: 1
        color: DeepinTheme.blue
        Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.clicked()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        radius: DeepinTheme.radiusSmall
        color: DeepinTheme.cardHover
        opacity: mouseArea.containsMouse ? 1.0 : 0.0
        z: -1
        Behavior on opacity { NumberAnimation { duration: 120 } }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
        }
    }
}
