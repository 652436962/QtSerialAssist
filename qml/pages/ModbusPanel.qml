import QtQuick 2.15
import QtQuick.Controls 2.15
import "../theme"
import "../components"

Rectangle {
    id: root
    color: "transparent"

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AsNeeded

        Column {
            width: root.width
            spacing: DeepinTheme.spacingNormal

            Item { width: 1; height: DeepinTheme.spacingNormal }

            // Header
            Row {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingSmall

                Rectangle { width: 3; height: 18; radius: 2; color: DeepinTheme.orange; anchors.verticalCenter: parent.verticalCenter }
                Text { text: "Modbus 帧生成"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeLarge; font.weight: Font.DemiBold; color: DeepinTheme.text; anchors.verticalCenter: parent.verticalCenter }
            }

            // Format
            Column {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingTiny
                Text { text: "格式"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                DComboBox { id: formatCombo; width: parent.width; model: ["RTU","ASCII","TCP"]; currentIndex: core.modbus.format; onCurrentIndexChanged: core.modbus.format = currentIndex }
            }

            // Slave
            Column {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingTiny
                Text { text: "从站地址 (1-247)"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                SpinBox {
                    id: slaveSpin
                    width: parent.width; from: 1; to: 247; value: core.modbus.slaveAddr; editable: true
                    font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeNormal
                    implicitHeight: DeepinTheme.inputHeight
                    background: Rectangle { radius: DeepinTheme.radiusSmall; color: DeepinTheme.inputBg; border.color: DeepinTheme.inputBorder; border.width: 1 }
                    contentItem: TextInput { text: slaveSpin.textFromValue(slaveSpin.value); font: slaveSpin.font; color: DeepinTheme.text; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; readOnly: !slaveSpin.editable; validator: slaveSpin.validator }
                    onValueChanged: core.modbus.slaveAddr = value
                }
            }

            // Function Code
            Column {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingTiny
                Text { text: "功能码"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                DComboBox { id: funcCombo; width: parent.width; model: ["01 读线圈","03 读保持寄存器","05 写单个线圈","06 写单个寄存器","10 批量写寄存器"]; currentIndex: core.modbus.functionCode; onCurrentIndexChanged: core.modbus.functionCode = currentIndex }
            }

            // Address + Count
            Row {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingSmall

                Column {
                    width: (parent.width - parent.spacing) / 2
                    spacing: DeepinTheme.spacingTiny
                    Text { text: "寄存器地址"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    SpinBox {
                        id: addrSpin
                        width: parent.width; from: 0; to: 65535; value: core.modbus.registerAddr; editable: true
                        font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeNormal
                        implicitHeight: DeepinTheme.inputHeight
                        background: Rectangle { radius: DeepinTheme.radiusSmall; color: DeepinTheme.inputBg; border.color: DeepinTheme.inputBorder; border.width: 1 }
                        contentItem: TextInput { text: addrSpin.textFromValue(addrSpin.value); font: addrSpin.font; color: DeepinTheme.text; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; readOnly: !addrSpin.editable; validator: addrSpin.validator }
                        onValueChanged: core.modbus.registerAddr = value
                    }
                }

                Column {
                    width: (parent.width - parent.spacing) / 2
                    spacing: DeepinTheme.spacingTiny
                    Text { text: "数量"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }
                    SpinBox {
                        id: countSpin
                        width: parent.width; from: 1; to: 125; value: core.modbus.registerCount; editable: true
                        font.family: DeepinTheme.monoFamily; font.pixelSize: DeepinTheme.fontSizeNormal
                        implicitHeight: DeepinTheme.inputHeight
                        background: Rectangle { radius: DeepinTheme.radiusSmall; color: DeepinTheme.inputBg; border.color: DeepinTheme.inputBorder; border.width: 1 }
                        contentItem: TextInput { text: countSpin.textFromValue(countSpin.value); font: countSpin.font; color: DeepinTheme.text; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; readOnly: !countSpin.editable; validator: countSpin.validator }
                        onValueChanged: core.modbus.registerCount = value
                    }
                }
            }

            Rectangle {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                height: 1; color: DeepinTheme.border
            }

            Item {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                height: DeepinTheme.buttonHeight

                DButton {
                    anchors.fill: parent
                    text: "生成帧"; accent: true
                    onClicked: core.modbus.generateFrame()
                }
            }

            // Result
            Column {
                x: DeepinTheme.spacingNormal
                width: parent.width - DeepinTheme.spacingNormal * 2
                spacing: DeepinTheme.spacingTiny
                height: 120

                Text { text: "结果"; font.family: DeepinTheme.fontFamily; font.pixelSize: DeepinTheme.fontSizeSmall; color: DeepinTheme.subText }

                Rectangle {
                    width: parent.width; height: 100
                    radius: DeepinTheme.radiusSmall
                    color: DeepinTheme.inputBg
                    border.color: DeepinTheme.inputBorder
                    border.width: 1

                    TextArea {
                        anchors.fill: parent
                        anchors.margins: DeepinTheme.spacingSmall
                        readOnly: true
                        text: core.modbus.resultFrame
                        font.family: DeepinTheme.monoFamily; font.pixelSize: 12; color: DeepinTheme.text
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        background: null
                    }
                }
            }

            Item { width: 1; height: DeepinTheme.spacingNormal }
        }
    }
}
