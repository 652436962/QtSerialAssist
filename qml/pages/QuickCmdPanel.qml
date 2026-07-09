import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"
import "../components"

Rectangle {
    id: root
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 42
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: DeepinTheme.spacingNormal
                anchors.rightMargin: DeepinTheme.spacingNormal
                spacing: DeepinTheme.spacingSmall

                Rectangle { width: 3; height: 18; radius: 2; color: DeepinTheme.purple; anchors.verticalCenter: parent.verticalCenter }

                Text { text: "快捷命令"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeLarge; font.weight: Font.DemiBold; color: DeepinTheme.text; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        // Group Selector
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: DeepinTheme.spacingNormal
            spacing: DeepinTheme.spacingSmall

            DComboBox {
                id: groupCombo
                Layout.fillWidth: true
                model: core.quickCmd.groups
                currentIndex: {
                    var idx = core.quickCmd.groups.indexOf(core.quickCmd.currentGroup)
                    return idx >= 0 ? idx : 0
                }
                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && currentIndex < model.length)
                        core.quickCmd.currentGroup = model[currentIndex]
                }
            }

            DButton {
                text: "+"
                accent: true
                small: true
                implicitWidth: 36
                onClicked: core.quickCmd.addGroup("NewGroup" + (core.quickCmd.groups.length + 1))
            }

            DButton {
                text: "-"
                danger: true
                small: true
                implicitWidth: 36
                onClicked: core.quickCmd.removeGroup(core.quickCmd.currentGroup)
            }
        }

        // Command List Header
        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: DeepinTheme.spacingNormal
            Layout.rightMargin: DeepinTheme.spacingNormal
            height: 30
            color: DeepinTheme.cardHover
            radius: DeepinTheme.radiusTiny

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: DeepinTheme.spacingSmall
                anchors.rightMargin: DeepinTheme.spacingSmall
                spacing: DeepinTheme.spacingSmall

                Text { text: "名称"; font.family: DeepinTheme.fontFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.subText; Layout.fillWidth: true }
                Text { text: "数据"; font.family: DeepinTheme.fontFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.subText; Layout.fillWidth: true }
                Text { text: "发送"; font.family: DeepinTheme.fontFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.subText; width: 40 }
            }
        }

        // Command List
        ListView {
            id: commandList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: DeepinTheme.spacingNormal
            Layout.rightMargin: DeepinTheme.spacingNormal
            layoutDirection: Qt.LeftToRight
            clip: true
            model: core.quickCmd.commands

            spacing: 2

            delegate: Rectangle {
                width: commandList.width
                height: 34
                radius: DeepinTheme.radiusTiny
                color: index % 2 === 0 ? DeepinTheme.cardHover : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: DeepinTheme.spacingSmall
                    anchors.rightMargin: DeepinTheme.spacingSmall
                    spacing: DeepinTheme.spacingSmall

                    Text {
                        text: modelData.name || ""
                        font.family: DeepinTheme.fontFamily
                        font.pixelSize: DeepinTheme.fontSizeNormal
                        color: DeepinTheme.text
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: modelData.data || ""
                        font.family: DeepinTheme.monoFamily
                        font.pixelSize: 11
                        color: DeepinTheme.subText
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    DButton {
                        text: "▸"
                        small: true
                        accent: true
                        implicitWidth: 36
                        height: 26
                        onClicked: core.sendData(modelData.data)
                    }
                }
            }
        }

        // Bottom Buttons
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: DeepinTheme.spacingNormal
            spacing: DeepinTheme.spacingSmall

            DButton {
                text: "+ 添加"
                small: true
                accent: true
                Layout.fillWidth: true
                onClicked: core.quickCmd.addCommand("NewCmd", "")
            }

            DButton {
                text: "- 删除"
                small: true
                danger: true
                Layout.fillWidth: true
                onClicked: {
                    if (commandList.currentIndex >= 0)
                        core.quickCmd.removeCommand(commandList.currentIndex)
                }
            }

            DButton {
                text: "全选"
                small: true
                outline: true
                Layout.fillWidth: true
                onClicked: {}
            }

            DButton {
                text: "发送全部"
                small: true
                accent: true
                Layout.fillWidth: true
                onClicked: {
                    for (var i = 0; i < commandList.count; i++) {
                        var cmd = commandList.model[i]
                        if (cmd && cmd.data) core.sendData(cmd.data)
                    }
                }
            }
        }
    }
}
