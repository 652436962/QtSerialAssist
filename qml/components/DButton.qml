import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme"

Button {
    id: root

    property bool accent: false
    property bool danger: false
    property bool outline: false
    property bool small: false

    font.family: DeepinTheme.fontFamily
    font.pixelSize: small ? DeepinTheme.fontSizeSmall : DeepinTheme.fontSizeNormal
    font.weight: Font.Medium

    implicitWidth: Math.max(small ? 60 : 80, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: small ? 28 : DeepinTheme.buttonHeight

    leftPadding: DeepinTheme.spacingNormal
    rightPadding: DeepinTheme.spacingNormal

    hoverEnabled: true

    contentItem: Text {
        text: root.text
        font: root.font
        color: {
            if (root.outline) {
                if (root.danger) return DeepinTheme.red
                if (root.accent) return DeepinTheme.blue
                return DeepinTheme.text
            }
            return "#ffffff"
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        id: bgRect
        radius: DeepinTheme.radiusSmall
        color: {
            if (root.outline) return "transparent"
            if (root.down) {
                if (root.danger) return Qt.darker(DeepinTheme.red, 1.15)
                if (root.accent) return Qt.darker(DeepinTheme.blue, 1.15)
                return Qt.darker(DeepinTheme.card, 1.08)
            }
            if (root.hovered) {
                if (root.danger) return Qt.lighter(DeepinTheme.red, 1.1)
                if (root.accent) return Qt.lighter(DeepinTheme.blue, 1.1)
                return DeepinTheme.cardHover
            }
            if (root.danger) return DeepinTheme.red
            if (root.accent) return DeepinTheme.blue
            return DeepinTheme.card
        }

        border.width: root.outline ? 1 : 0
        border.color: {
            if (!root.outline) return "transparent"
            if (root.down) {
                if (root.danger) return DeepinTheme.red
                if (root.accent) return DeepinTheme.blue
                return DeepinTheme.subText
            }
            if (root.hovered) {
                if (root.danger) return DeepinTheme.red
                if (root.accent) return DeepinTheme.blue
                return DeepinTheme.subText
            }
            if (root.danger) return DeepinTheme.red
            if (root.accent) return DeepinTheme.blue
            return DeepinTheme.border
        }

        Behavior on color { ColorAnimation { duration: 150 } }
    }

    scale: root.down ? 0.97 : 1.0
    Behavior on scale { NumberAnimation { duration: 100 } }
}
