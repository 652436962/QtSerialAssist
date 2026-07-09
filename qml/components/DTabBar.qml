import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"

Rectangle {
    id: root

    default property alias children: container.data
    property int currentIndex: 0

    color: DeepinTheme.card
    height: 42

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: DeepinTheme.border
    }

    RowLayout {
        id: container
        anchors.fill: parent
        anchors.leftMargin: DeepinTheme.spacingSmall
        spacing: 0
    }
}
