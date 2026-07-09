import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "theme"
import "components"
import "pages"

ApplicationWindow {
    id: mainWindow

    visible: true
    width: 1400
    height: 900
    minimumWidth: 800
    minimumHeight: 500
    title: "QtSerialAssist 2.0"
    color: DeepinTheme.bg

    property int currentPanel: 0

    Component.onCompleted: {
        DeepinTheme.themeMode = core.settings.themeIndex
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Header Bar ----
        Rectangle {
            Layout.fillWidth: true
            height: DeepinTheme.headerHeight
            color: DeepinTheme.card

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: DeepinTheme.spacingLarge
                anchors.rightMargin: DeepinTheme.spacingNormal
                spacing: DeepinTheme.spacingSmall

                // Logo + Title
                Row {
                    spacing: DeepinTheme.spacingSmall

                    Rectangle {
                        width: 28; height: 28; radius: 6
                        color: DeepinTheme.blue

                        Text {
                            anchors.centerIn: parent
                            text: "Q"
                            font.family: DeepinTheme.monoFamily
                            font.pixelSize: 14
                            font.bold: true
                            color: "#ffffff"
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: -2

                        Text {
                            text: "QtSerialAssist"
                            font.family: DeepinTheme.fontFamily
                            font.pixelSize: DeepinTheme.fontSizeNormal
                            font.weight: Font.DemiBold
                            color: DeepinTheme.text
                        }

                        Text {
                            text: "v" + core.version
                            font.family: DeepinTheme.monoFamily
                            font.pixelSize: 9
                            color: DeepinTheme.subText
                        }
                    }
                }

                Rectangle { width: 1; height: 24; color: DeepinTheme.border }

                // Panel toggles
                DButton {
                    text: "Modbus"
                    small: true
                    outline: currentPanel !== 1
                    accent: currentPanel === 1
                    onClicked: currentPanel = currentPanel === 1 ? 0 : 1
                }

                DButton {
                    text: "快捷命令"
                    small: true
                    outline: currentPanel !== 2
                    accent: currentPanel === 2
                    onClicked: currentPanel = currentPanel === 2 ? 0 : 2
                }

                Rectangle { width: 1; height: 24; color: DeepinTheme.border }

                DButton {
                    text: rightPanel.isTerminalMode ? "普通模式" : "终端模式"
                    small: true
                    outline: !rightPanel.isTerminalMode
                    danger: rightPanel.isTerminalMode
                    onClicked: rightPanel.toggleTerminalMode()
                }

                Item { Layout.fillWidth: true }

                DComboBox {
                    id: themeHeaderCombo
                    implicitWidth: 100
                    implicitHeight: 28
                    model: ["深色", "浅色", "跟随系统"]
                    currentIndex: DeepinTheme.themeMode
                    onCurrentIndexChanged: DeepinTheme.themeMode = currentIndex
                    font.pixelSize: DeepinTheme.fontSizeSmall
                }

                DButton {
                    text: "设置"
                    small: true
                    outline: true
                    onClicked: settingsPopup.open()
                }
            }
        }

        // ---- Divider ----
        Rectangle { Layout.fillWidth: true; height: 1; color: DeepinTheme.border }

        // ---- Main Content ----
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Left Panel / Modbus / QuickCmd
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: DeepinTheme.leftPanelWidth
                color: DeepinTheme.panelBg

                Loader {
                    anchors.fill: parent
                    sourceComponent: {
                        switch (currentPanel) {
                            case 0: return leftPanelComponent
                            case 1: return modbusPanelComponent
                            case 2: return quickCmdPanelComponent
                            default: return leftPanelComponent
                        }
                    }
                }

                Component { id: leftPanelComponent; LeftPanel {} }
                Component { id: modbusPanelComponent; ModbusPanel {} }
                Component { id: quickCmdPanelComponent; QuickCmdPanel {} }
            }

            // Divider
            Rectangle { width: 1; Layout.fillHeight: true; color: DeepinTheme.border }

            // Right Panel
            RightPanel {
                id: rightPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // ---- Divider ----
        Rectangle { Layout.fillWidth: true; height: 1; color: DeepinTheme.border }

        // ---- Status Bar ----
        Rectangle {
            Layout.fillWidth: true
            height: DeepinTheme.statusBarHeight
            color: DeepinTheme.card

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: DeepinTheme.spacingNormal
                anchors.rightMargin: DeepinTheme.spacingNormal
                spacing: DeepinTheme.spacingNormal

                // Connection dot
                Rectangle {
                    width: 7; height: 7; radius: 4
                    color: (core.serialPort.connected || core.network.connected) ? DeepinTheme.green : DeepinTheme.red
                }

                Text {
                    text: core.statusMessage || "就绪"
                    font.family: DeepinTheme.fontFamily
                    font.pixelSize: DeepinTheme.fontSizeSmall
                    color: DeepinTheme.subText
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                // Stats
                Row { spacing: DeepinTheme.spacingLarge

                    Row { spacing: 4
                        Text { text: "RX"; font.family: DeepinTheme.monoFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.receiveText }
                        Text { text: core.dataProcessor.receiveBytes + " B"; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    }

                    Row { spacing: 4
                        Text { text: "TX"; font.family: DeepinTheme.monoFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.sendText }
                        Text { text: core.dataProcessor.sendBytes + " B"; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    }

                    Row { spacing: 4
                        Text { text: "帧"; font.family: DeepinTheme.monoFamily; font.pixelSize: 10; font.weight: Font.Bold; color: DeepinTheme.subText }
                        Text { text: core.dataProcessor.frameCount; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    }
                }
            }
        }
    }

    // ---- Settings Popup ----
    Popup {
        id: settingsPopup
        anchors.centerIn: parent
        width: 600
        height: 520
        modal: true
        dim: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0

        background: Rectangle { color: "transparent" }

        contentItem: SettingsPage {
            onClose: settingsPopup.close()
        }
    }
}
