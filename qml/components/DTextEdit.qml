import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme"

TextArea {
    id: root

    property bool monospace: false
    property bool readOnly: false
    property color bgColor: DeepinTheme.inputBg
    property color borderColor: DeepinTheme.border
    property color textColor: DeepinTheme.text
    property int borderRadius: DeepinTheme.radiusSmall

    font.family: monospace ? DeepinTheme.monoFamily : DeepinTheme.fontFamily
    font.pixelSize: DeepinTheme.fontSizeNormal
    color: textColor
    readOnly: root.readOnly
    selectByMouse: true
    wrapMode: TextArea.Wrap

    background: Rectangle {
        radius: root.borderRadius
        color: root.bgColor
        border.color: root.borderColor
        border.width: 1
    }

    padding: DeepinTheme.spacingSmall

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (event.modifiers & Qt.ControlModifier) {
                root.accepted()
                event.accepted = true
            }
        }
    }

    signal accepted()
}
