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

        DTabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: 0

            DTabButton { text: "串口设置"; selected: tabBar.currentIndex === 0; onClicked: tabBar.currentIndex = 0 }
            DTabButton { text: "网络设置"; selected: tabBar.currentIndex === 1; onClicked: tabBar.currentIndex = 1 }
        }

        SwipeView {
            id: swipeView
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            interactive: false
            clip: true

            // ====== Serial Page ======
            ScrollView {
                id: serialScroll
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                Column {
                    width: swipeView.width
                    spacing: DeepinTheme.spacingNormal

                    Item { width: 1; height: DeepinTheme.spacingNormal }

                    // Port Selection
                    Column {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingTiny

                        Text { text: "串口号"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }

                        RowLayout {
                            width: parent.width
                            spacing: DeepinTheme.spacingSmall

                            DComboBox {
                                id: portCombo
                                Layout.fillWidth: true
                                model: core.serialPort.availablePorts
                                onModelChanged: {
                                    if (model && model.length > 0 && currentIndex >= 0 && currentIndex < model.length)
                                        core.serialPort.currentPort = model[currentIndex]
                                }
                                onCurrentIndexChanged: {
                                    if (currentIndex >= 0 && model && currentIndex < model.length)
                                        core.serialPort.currentPort = model[currentIndex]
                                }
                            }

                            DButton {
                                text: "刷新"; small: true; outline: true
                                implicitWidth: 56
                                onClicked: core.serialPort.refreshPorts()
                            }
                        }
                    }

                    // Baud + DataBits
                    Row {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingSmall

                        Column {
                            width: (parent.width - parent.spacing) / 2
                            spacing: DeepinTheme.spacingTiny
                            Text { text: "波特率"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                            DComboBox { id: baudCombo; width: parent.width; model: ["9600","19200","38400","57600","115200","230400","460800","921600"]; currentIndex: 4; onCurrentIndexChanged: core.serialPort.baudRate = parseInt(model[currentIndex]) }
                        }
                        Column {
                            width: (parent.width - parent.spacing) / 2
                            spacing: DeepinTheme.spacingTiny
                            Text { text: "数据位"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                            DComboBox { id: dataBitsCombo; width: parent.width; model: ["5","6","7","8"]; currentIndex: 3; onCurrentIndexChanged: core.serialPort.dataBits = parseInt(model[currentIndex]) }
                        }
                    }

                    // StopBits + Parity
                    Row {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingSmall

                        Column {
                            width: (parent.width - parent.spacing) / 2
                            spacing: DeepinTheme.spacingTiny
                            Text { text: "停止位"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                            DComboBox { id: stopBitsCombo; width: parent.width; model: ["1","1.5","2"]; currentIndex: 0; onCurrentIndexChanged: core.serialPort.stopBits = parseFloat(model[currentIndex]) }
                        }
                        Column {
                            width: (parent.width - parent.spacing) / 2
                            spacing: DeepinTheme.spacingTiny
                            Text { text: "校验位"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                            DComboBox { id: parityCombo; width: parent.width; model: ["None","Odd","Even","Mark","Space"]; currentIndex: 0; onCurrentIndexChanged: core.serialPort.parity = currentIndex }
                        }
                    }

                    // Flow Control
                    Column {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingTiny
                        Text { text: "流控"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                        DComboBox { id: flowCombo; width: parent.width; model: ["None","RTS/CTS","XON/XOFF"]; currentIndex: 0; onCurrentIndexChanged: core.serialPort.flowControl = currentIndex }
                    }

                    // Separator
                    Rectangle {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        height: 1; color: DeepinTheme.border
                    }

                    // Connect Button
                    Item {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        height: DeepinTheme.buttonHeight

                        DButton {
                            anchors.fill: parent
                            text: core.serialPort.connected ? "断开串口" : "打开串口"
                            accent: !core.serialPort.connected
                            danger: core.serialPort.connected
                            onClicked: {
                                if (core.serialPort.connected) core.serialPort.closePort()
                                else core.serialPort.openPort()
                            }
                        }
                    }

                    Item { width: 1; height: DeepinTheme.spacingNormal }
                }
            }

            // ====== Network Page ======
            ScrollView {
                id: netScroll
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                Column {
                    width: swipeView.width
                    spacing: DeepinTheme.spacingNormal

                    Item { width: 1; height: DeepinTheme.spacingNormal }

                    // Protocol
                    Column {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingTiny
                        Text { text: "协议类型"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                        DComboBox { id: protocolCombo; width: parent.width; model: ["TCP Server","TCP Client","UDP"]; currentIndex: core.network.protocolIndex; onCurrentIndexChanged: core.network.protocolIndex = currentIndex }
                    }

                    // IP
                    Column {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingTiny
                        Text {
                            text: protocolCombo.currentIndex === 0 ? "监听地址" : "目标 IP"
                            font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText
                        }

                        TextField {
                            id: ipField
                            width: parent.width
                            text: core.network.ipAddress
                            placeholderText: "192.168.1.100"
                            font.family: DeepinTheme.monoFamily
                            font.pixelSize: DeepinTheme.fontSizeNormal
                            color: DeepinTheme.text
                            placeholderTextColor: DeepinTheme.subText
                            leftPadding: DeepinTheme.spacingSmall; rightPadding: DeepinTheme.spacingSmall
                            verticalAlignment: TextInput.AlignVCenter
                            implicitHeight: DeepinTheme.inputHeight
                            background: Rectangle {
                                radius: DeepinTheme.radiusSmall
                                color: DeepinTheme.inputBg
                                border.color: ipField.activeFocus ? DeepinTheme.blue : DeepinTheme.inputBorder
                                border.width: 1
                            }
                            onTextChanged: core.network.ipAddress = text
                        }
                    }

                    // Port
                    Column {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        spacing: DeepinTheme.spacingTiny
                        Text { text: "端口"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }

                        SpinBox {
                            id: portSpin
                            width: parent.width
                            from: 1; to: 65535; value: core.network.port; editable: true
                            font.family: DeepinTheme.monoFamily
                            font.pixelSize: DeepinTheme.fontSizeNormal
                            implicitHeight: DeepinTheme.inputHeight

                            background: Rectangle {
                                radius: DeepinTheme.radiusSmall
                                color: DeepinTheme.inputBg
                                border.color: portSpin.activeFocus ? DeepinTheme.blue : DeepinTheme.inputBorder
                                border.width: 1
                            }

                            contentItem: TextInput {
                                text: portSpin.textFromValue(portSpin.value)
                                font: portSpin.font
                                color: DeepinTheme.text
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                readOnly: !portSpin.editable
                                validator: portSpin.validator
                            }

                            onValueChanged: core.network.port = value
                        }
                    }

                    Rectangle {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        height: 1; color: DeepinTheme.border
                    }

                    Item {
                        width: parent.width - DeepinTheme.spacingNormal * 2
                        x: DeepinTheme.spacingNormal
                        height: DeepinTheme.buttonHeight

                        DButton {
                            anchors.fill: parent
                            text: core.network.connected ? "断开连接" : "建立连接"
                            accent: !core.network.connected
                            danger: core.network.connected
                            onClicked: {
                                if (core.network.connected) core.network.closeConnection()
                                else core.network.openConnection()
                            }
                        }
                    }

                    Item { width: 1; height: DeepinTheme.spacingNormal }
                }
            }
        }
    }
}
