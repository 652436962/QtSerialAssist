import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../theme"
import "../components"

Rectangle {
    id: root
    color: "transparent"

    property bool isTerminalMode: false
    property bool isDisplayHex: false
    property bool isSendHex: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ---- Toolbar ----
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: DeepinTheme.card

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: DeepinTheme.spacingSmall
                anchors.rightMargin: DeepinTheme.spacingSmall
                spacing: DeepinTheme.spacingTiny

                DButton { text: "清空接收"; small: true; outline: true; onClicked: receiveArea.text = "" }
                DButton { text: "清空发送"; small: true; outline: true; onClicked: sendArea.text = "" }

                Rectangle { width: 1; height: 20; color: DeepinTheme.border }

                Row { spacing: 2
                    DButton { text: "HEX"; small: true; outline: !root.isDisplayHex; accent: root.isDisplayHex; onClicked: { root.isDisplayHex = true; core.dataProcessor.receiveMode = 0 } }
                    DButton { text: "ASCII"; small: true; outline: root.isDisplayHex; accent: !root.isDisplayHex; onClicked: { root.isDisplayHex = false; core.dataProcessor.receiveMode = 1 } }
                }

                Rectangle { width: 1; height: 20; color: DeepinTheme.border }

                DSwitch {
                    text: "定时发送"
                    checked: core.autoSendEnabled
                    onToggled: core.setAutoSend(checked, autoSendInterval.value)
                }

                SpinBox {
                    id: autoSendInterval
                    width: 110; from: 100; to: 60000; value: 1000; editable: true
                    font.family: DeepinTheme.monoFamily
                    font.pixelSize: DeepinTheme.fontSizeSmall
                    background: Rectangle {
                        radius: DeepinTheme.radiusTiny; color: DeepinTheme.inputBg
                        border.color: DeepinTheme.inputBorder; border.width: 1
                    }
                    contentItem: TextInput {
                        text: autoSendInterval.value; font: autoSendInterval.font; color: DeepinTheme.text
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                        readOnly: !autoSendInterval.editable; inputMethodHints: Qt.ImhDigitsOnly
                    }
                    onValueChanged: { if (core.autoSendEnabled) core.setAutoSend(true, value) }
                }

                Text { text: "ms"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }

                Item { Layout.fillWidth: true }

                Row { spacing: DeepinTheme.spacingNormal
                    Row { spacing: 3
                        Text { text: "RX"; font.family: DeepinTheme.monoFamily; font.pixelSize: 9; font.weight: Font.Bold; color: DeepinTheme.receiveText }
                        Text { text: core.dataProcessor.receiveBytes; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    }
                    Row { spacing: 3
                        Text { text: "TX"; font.family: DeepinTheme.monoFamily; font.pixelSize: 9; font.weight: Font.Bold; color: DeepinTheme.sendText }
                        Text { text: core.dataProcessor.sendBytes; font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: DeepinTheme.border }

        // ---- Receive Area ----
        ScrollView {
            id: receiveScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: receiveScroll.width
                spacing: 0

                Text {
                    text: root.isTerminalMode ? "终端模式 - 直接输入发送" : "接收数据"
                    font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall
                    color: root.isTerminalMode ? DeepinTheme.red : DeepinTheme.subText
                    Layout.leftMargin: DeepinTheme.spacingNormal
                    Layout.preferredHeight: 20
                }

                TextArea {
                    id: receiveArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readOnly: !root.isTerminalMode
                    font.family: DeepinTheme.monoFamily; font.pixelSize: 13
                    color: root.isTerminalMode ? DeepinTheme.sendText : DeepinTheme.receiveText
                    wrapMode: TextArea.Wrap; selectByMouse: true
                    leftPadding: DeepinTheme.spacingNormal; rightPadding: DeepinTheme.spacingNormal
                    topPadding: DeepinTheme.spacingSmall; bottomPadding: DeepinTheme.spacingSmall

                    background: Rectangle {
                        color: DeepinTheme.inputBg
                        border.color: root.isTerminalMode ? DeepinTheme.red : "transparent"
                        border.width: root.isTerminalMode ? 2 : 0
                    }

                    Keys.onPressed: function(event) {
                        if (root.isTerminalMode) {
                            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                core.sendData("\r\n"); event.accepted = true
                            } else if (event.key === Qt.Key_Backspace) {
                                if (receiveArea.cursorPosition > 0)
                                    receiveArea.remove(receiveArea.cursorPosition - 1, receiveArea.cursorPosition)
                                event.accepted = true
                            } else if (event.text.length > 0 && event.text[0] >= ' ') {
                                core.sendData(event.text)
                            }
                        }
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: DeepinTheme.border }

        // ---- Send Area ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            Layout.minimumHeight: 160
            Layout.maximumHeight: 160
            spacing: 0
            visible: !root.isTerminalMode

            RowLayout {
                Layout.fillWidth: true
                height: 28

                Text { text: "发送数据"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText; Layout.leftMargin: DeepinTheme.spacingNormal }

                Row { spacing: 2
                    DButton { text: "HEX"; small: true; outline: !root.isSendHex; accent: root.isSendHex; onClicked: root.isSendHex = true }
                    DButton { text: "ASCII"; small: true; outline: root.isSendHex; accent: !root.isSendHex; onClicked: root.isSendHex = false }
                }

                Item { Layout.fillWidth: true }

                DButton { text: "发送"; small: true; accent: true; onClicked: { if (sendArea.text.length > 0) core.sendData(sendArea.text, root.isSendHex) } }
                DButton { text: "发送+换行"; small: true; accent: true; onClicked: { if (sendArea.text.length > 0) core.sendData(sendArea.text + "\r\n", root.isSendHex) } }
            }

            ScrollView {
                id: sendScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                TextArea {
                    id: sendArea
                    width: sendScroll.width
                    font.family: DeepinTheme.monoFamily; font.pixelSize: 13
                    color: DeepinTheme.sendText; wrapMode: TextArea.Wrap; selectByMouse: true
                    leftPadding: DeepinTheme.spacingNormal; rightPadding: DeepinTheme.spacingNormal
                    topPadding: DeepinTheme.spacingSmall; bottomPadding: DeepinTheme.spacingSmall
                    placeholderText: "在此输入发送内容..."; placeholderTextColor: DeepinTheme.subText
                    text: "hello world"
                    background: Rectangle { color: DeepinTheme.sendBg }

                    Keys.onPressed: function(event) {
                        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && (event.modifiers & Qt.ControlModifier)) {
                            if (sendArea.text.length > 0) core.sendData(sendArea.text, root.isSendHex); event.accepted = true
                        }
                    }
                }
            }
        }

        Connections {
            target: core
            function onDataReceived(formattedData) {
                receiveArea.append("<--- " + formattedData)
                if (core.settings.autoScroll && !root.isTerminalMode)
                    receiveArea.cursorPosition = receiveArea.length
            }
            function onDataSent(formattedData) {
                receiveArea.append("---> " + formattedData)
                if (core.settings.autoScroll && !root.isTerminalMode)
                    receiveArea.cursorPosition = receiveArea.length
            }
        }
    }

    function toggleTerminalMode() {
        root.isTerminalMode = !root.isTerminalMode
        if (root.isTerminalMode) receiveArea.forceActiveFocus()
    }
}
