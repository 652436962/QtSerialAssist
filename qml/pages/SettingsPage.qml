import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"
import "../components"

Rectangle {
    id: root

    width: 600
    height: 520
    color: DeepinTheme.card
    radius: DeepinTheme.radiusLarge

    signal close()

    border.color: DeepinTheme.border
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        spacing: DeepinTheme.spacingNormal
        anchors.margins: 32

        Text {
            text: "设置"
            font.family: DeepinTheme.fontFamily
            font.pixelSize: DeepinTheme.fontSizeTitle
            font.weight: Font.DemiBold
            color: DeepinTheme.text
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: DeepinTheme.spacingTiny
            Text { text: "主题外观"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeNormal; color: DeepinTheme.text }
            DComboBox {
                Layout.fillWidth: true
                model: ["深色主题", "浅色主题", "跟随系统"]
                currentIndex: DeepinTheme.themeMode
                onCurrentIndexChanged: DeepinTheme.themeMode = currentIndex
            }
        }

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text { text: "自动滚动"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeNormal; color: DeepinTheme.text }
                Text { text: "收到数据时自动滚动到底部"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
            }
            DSwitch {
                checked: core.settings.autoScroll
                onToggled: core.settings.autoScroll = checked
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: DeepinTheme.spacingTiny
            Text { text: "界面语言"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeNormal; color: DeepinTheme.text }
            DComboBox {
                Layout.fillWidth: true
                model: ["简体中文", "English"]
                currentIndex: core.settings.language
                onCurrentIndexChanged: core.settings.language = currentIndex
            }
        }

        Item { Layout.fillHeight: true }

        Rectangle { Layout.fillWidth: true; height: 1; color: DeepinTheme.border }

        RowLayout {
            Layout.fillWidth: true
            spacing: DeepinTheme.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Text { text: "关于"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; font.weight: Font.Bold; color: DeepinTheme.subText }
                Text { text: "QtSerialAssist"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeNormal; font.weight: Font.Bold; color: DeepinTheme.text }
                Text { text: "跨平台串口/网络调试助手"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                Text { text: core.version; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                Text { text: "© 2024 QtSerialAssist"; font.family: DeepinTheme.fontFamily; font.pixelSize: 10; color: DeepinTheme.subText }
            }

            DButton {
                text: "检查更新"
                small: true
                outline: true
                onClicked: Qt.openUrlExternally("https://github.com/652436962/QtSerialAssist/releases")
            }
        }

        Text {
            Layout.fillWidth: true
            text: "github.com/652436962/QtSerialAssist"
            font.family: DeepinTheme.monoFamily
            font.pixelSize: DeepinTheme.fontSizeSmall
            color: DeepinTheme.blue
            horizontalAlignment: Text.AlignHCenter
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Qt.openUrlExternally("https://github.com/652436962/QtSerialAssist")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: DeepinTheme.spacingSmall
            Item { Layout.fillWidth: true }
            DButton { text: "取消"; outline: true; onClicked: root.close() }
            DButton {
                text: "保存"; accent: true
                onClicked: {
                    core.settings.themeIndex = DeepinTheme.themeMode
                    core.settings.save()
                    root.close()
                }
            }
        }
    }
}
