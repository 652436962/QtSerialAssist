#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QHeaderView>
#include <QCheckBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QCoreApplication>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 分割器初始比例: 左侧 200px，右侧占据剩余空间
    ui->mainSplitter->setStretchFactor(0, 0);
    ui->mainSplitter->setStretchFactor(1, 1);
    ui->mainSplitter->setSizes({200, 1000});
    ui->mainSplitter->setHandleWidth(2);

    ui->rightSplitter->setStretchFactor(0, 3);
    ui->rightSplitter->setStretchFactor(1, 1);
    ui->rightSplitter->setSizes({450, 250});
    ui->rightSplitter->setHandleWidth(2);
    ui->rightSplitter->setCollapsible(1, true);  // 允许底部面板折叠（终端模式用）

    // Modbus 面板默认隐藏
    ui->modbusPanelWidget->hide();

    // 快捷指令面板默认隐藏
    ui->quickCmdPanelWidget->hide();

    ui->modbusSplitter->setStretchFactor(0, 0);
    ui->modbusSplitter->setStretchFactor(1, 0);
    ui->modbusSplitter->setStretchFactor(2, 1);
    ui->modbusSplitter->setSizes({200, 300, 600});
    ui->modbusSplitter->setHandleWidth(2);

    applyTheme(0);  // 默认深色主题

    m_tcpServer = new QTcpServer(this);
    m_autoSendTimer = new QTimer(this);
    m_autoRefreshTimer = new QTimer(this);
    m_translator = new QTranslator(this);
    m_quickCmdTimer = new QTimer(this);
    m_quickCmdTimer->setSingleShot(false);

    initQuickCmdGroups();

#ifdef HAS_SERIAL_PORT
    refreshSerialPorts();
#else
    ui->tabWidget->setTabEnabled(0, false);
    ui->tabWidget->setTabToolTip(0, "Serial Port not available");
#endif

    initConnections();
    loadSettings();
    statusBar()->showMessage("就绪");
}

MainWindow::~MainWindow()
{
#ifdef HAS_SERIAL_PORT
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
#endif
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
    }
    if (m_tcpServer) {
        m_tcpServer->close();
    }
    if (m_udpSocket) {
        m_udpSocket->close();
    }
    delete ui;
}

void MainWindow::initConnections()
{
    // connectSlotsByName in setupUi() auto-connects all on_<widget>_<signal> slots.
    // Only manually connect signals whose slot names do NOT follow that convention.

    connect(ui->connectNetBtn, &QPushButton::clicked, this, &MainWindow::on_connectBtn_clicked);
    connect(ui->sendNewLineBtn, &QPushButton::clicked, this, [this]() { on_sendBtn_clicked(true); });
    connect(ui->modbusBtn, &QPushButton::clicked, this, [this]() {
        ui->modbusPanelWidget->setVisible(!ui->modbusPanelWidget->isVisible());
    });
    connect(ui->quickCmdBtn, &QPushButton::clicked, this, [this]() {
        ui->quickCmdPanelWidget->setVisible(!ui->quickCmdPanelWidget->isVisible());
    });
    connect(ui->quickCmdGroupCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            loadQuickCmdGroup(ui->quickCmdGroupCombo->currentText());
        }
    });
    connect(ui->quickCmdTable, &QTableWidget::cellClicked, this, [this](int row, int column) {
        if (column == 3) {
            loadQuickCmdToSendArea(row);
        }
    });
    connect(m_autoSendTimer, &QTimer::timeout, this, &MainWindow::on_autoSendTimer);

#ifdef HAS_SERIAL_PORT
    m_autoRefreshTimer->start(2000);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &MainWindow::refreshSerialPorts);
#endif
}

#ifdef HAS_SERIAL_PORT
void MainWindow::refreshSerialPorts()
{
    if (ui->tabWidget->currentIndex() != 0) return;

    QString currentPort = ui->serialPortCombo->currentText();
    ui->serialPortCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        QString portName = info.portName();
        QString description = info.description();
        if (!description.isEmpty()) {
            portName += " - " + description;
        }
        ui->serialPortCombo->addItem(portName, info.portName());
    }

    int index = ui->serialPortCombo->findText(currentPort);
    if (index >= 0) {
        ui->serialPortCombo->setCurrentIndex(index);
    }
}
#endif

void MainWindow::on_connectBtn_clicked()
{
    int tabIndex = ui->tabWidget->currentIndex();

#ifdef HAS_SERIAL_PORT
    if (tabIndex == 0) {
        if (m_serialConnected) {
            if (m_serialPort) {
                m_serialPort->close();
                disconnect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::on_serialReadyRead);
            }
            m_serialConnected = false;
            ui->connectBtn->setText("打开连接");
            statusBar()->showMessage("串口已关闭");
        } else {
            QString portName = ui->serialPortCombo->currentData().toString();
            if (portName.isEmpty()) {
                statusBar()->showMessage("请选择串口");
                return;
            }

            m_serialPort = new QSerialPort(this);
            m_serialPort->setPortName(portName);

            if (m_serialPort->open(QIODevice::ReadWrite)) {
                m_serialPort->setBaudRate(ui->baudRateCombo->currentText().toInt());

                QSerialPort::DataBits dataBits = QSerialPort::Data8;
                switch (ui->dataBitsCombo->currentIndex()) {
                    case 0: dataBits = QSerialPort::Data5; break;
                    case 1: dataBits = QSerialPort::Data6; break;
                    case 2: dataBits = QSerialPort::Data7; break;
                    case 3: dataBits = QSerialPort::Data8; break;
                }
                m_serialPort->setDataBits(dataBits);

                QSerialPort::StopBits stopBits = QSerialPort::OneStop;
                switch (ui->stopBitsCombo->currentIndex()) {
                    case 0: stopBits = QSerialPort::OneStop; break;
                    case 1: stopBits = QSerialPort::OneAndHalfStop; break;
                    case 2: stopBits = QSerialPort::TwoStop; break;
                }
                m_serialPort->setStopBits(stopBits);

                QSerialPort::Parity parity = QSerialPort::NoParity;
                switch (ui->parityCombo->currentIndex()) {
                    case 0: parity = QSerialPort::NoParity; break;
                    case 1: parity = QSerialPort::OddParity; break;
                    case 2: parity = QSerialPort::EvenParity; break;
                    case 3: parity = QSerialPort::MarkParity; break;
                    case 4: parity = QSerialPort::SpaceParity; break;
                }
                m_serialPort->setParity(parity);

                QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
                switch (ui->flowControlCombo->currentIndex()) {
                    case 0: flowControl = QSerialPort::NoFlowControl; break;
                    case 1: flowControl = QSerialPort::HardwareControl; break;
                    case 2: flowControl = QSerialPort::SoftwareControl; break;
                }
                m_serialPort->setFlowControl(flowControl);

                connect(m_serialPort, &QSerialPort::readyRead, this, &MainWindow::on_serialReadyRead);

                m_serialConnected = true;
                m_currentProtocol = "Serial";
                ui->connectBtn->setText("关闭连接");
                statusBar()->showMessage(QString("已连接 %1 @ %2").arg(portName).arg(ui->baudRateCombo->currentText()));
            } else {
                statusBar()->showMessage("打开串口失败: " + m_serialPort->errorString());
                delete m_serialPort;
                m_serialPort = nullptr;
            }
        }
        return;
    }
#endif

    int protocol = ui->protocolCombo->currentIndex();

    if (protocol == 0) {
        if (m_tcpServer->isListening()) {
            m_tcpServer->close();
            m_tcpConnected = false;
            ui->connectBtn->setText("打开连接");
            statusBar()->showMessage("TCP 服务器已关闭");
        } else {
            quint16 port = ui->portSpinBox->value();
            if (m_tcpServer->listen(QHostAddress::Any, port)) {
                m_currentProtocol = "TCP Server";
                m_tcpConnected = true;
                ui->connectBtn->setText("关闭连接");
                statusBar()->showMessage(QString("TCP 服务器已启动，监听端口 %1").arg(port));
                connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::on_tcpServerNewConnection);
            } else {
                statusBar()->showMessage("TCP 服务器启动失败");
            }
        }
    } else if (protocol == 1) {
        if (m_tcpConnected) {
            m_tcpSocket->disconnectFromHost();
            m_tcpConnected = false;
            ui->connectBtn->setText("打开连接");
            statusBar()->showMessage("TCP 客户端已断开");
        } else {
            QString ip = ui->ipLineEdit->text();
            quint16 port = ui->portSpinBox->value();

            m_tcpSocket = new QTcpSocket(this);
            connect(m_tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::on_tcpClientReadyRead);
            connect(m_tcpSocket, &QTcpSocket::disconnected, this, [this]() {
                m_tcpConnected = false;
                ui->connectBtn->setText("打开连接");
                statusBar()->showMessage("TCP 连接已断开");
            });

            m_tcpSocket->connectToHost(ip, port);
            if (m_tcpSocket->waitForConnected(3000)) {
                m_currentProtocol = "TCP Client";
                m_tcpConnected = true;
                ui->connectBtn->setText("关闭连接");
                statusBar()->showMessage(QString("已连接 %1:%2").arg(ip).arg(port));
            } else {
                statusBar()->showMessage("连接失败");
                delete m_tcpSocket;
                m_tcpSocket = nullptr;
            }
        }
    } else if (protocol == 2) {
        if (m_udpBound) {
            m_udpSocket->close();
            m_udpBound = false;
            ui->connectBtn->setText("打开连接");
            statusBar()->showMessage("UDP 已关闭");
        } else {
            quint16 port = ui->portSpinBox->value();
            m_udpSocket = new QUdpSocket(this);
            connect(m_udpSocket, &QUdpSocket::readyRead, this, &MainWindow::on_udpReadyRead);

            if (m_udpSocket->bind(port)) {
                m_currentProtocol = "UDP";
                m_udpBound = true;
                ui->connectBtn->setText("关闭连接");
                statusBar()->showMessage(QString("UDP 已绑定端口 %1").arg(port));
            } else {
                statusBar()->showMessage("UDP 绑定失败");
                delete m_udpSocket;
                m_udpSocket = nullptr;
            }
        }
    }
}

void MainWindow::on_sendBtn_clicked(bool appendNewline)
{
    QString data = ui->sendTextEdit->toPlainText();
    if (data.isEmpty()) return;

    QByteArray bytes;

    if (ui->sendModeCombo->currentIndex() == 1) {
        // ASCII 模式：根据编码设置转换文本
        QString encoding = ui->encodingCombo->currentText();
        if (encoding == "GBK") {
            bytes = data.toLocal8Bit();
        } else {
            bytes = data.toUtf8();
        }
    } else {
        // HEX 模式：解析十六进制输入
        bytes = parseHexInput(data);
    }

    if (bytes.isEmpty()) return;

    // 发送新行：追加换行符
    if (appendNewline) {
        QString newline = ui->newlineCombo->currentText();
        if (newline == "\\r\\n") {
            bytes.append("\r\n");
        } else if (newline == "\\n") {
            bytes.append("\n");
        } else if (newline == "\\r") {
            bytes.append("\r");
        }
    }

    bool sent = false;
    int tabIndex = ui->tabWidget->currentIndex();

#ifdef HAS_SERIAL_PORT
    if (tabIndex == 0 && m_serialConnected && m_serialPort) {
        qint64 written = m_serialPort->write(bytes);
        m_serialPort->flush();
        sent = (written != -1);
    } else
#endif
    {
        int protocol = ui->protocolCombo->currentIndex();
        if (protocol == 1 && m_tcpConnected && m_tcpSocket) {
            qint64 written = m_tcpSocket->write(bytes);
            m_tcpSocket->flush();
            sent = (written != -1);
        } else if (protocol == 2 && m_udpBound && m_udpSocket) {
            QString ip = ui->ipLineEdit->text();
            quint16 port = ui->portSpinBox->value();
            qint64 written = m_udpSocket->writeDatagram(bytes, QHostAddress(ip), port);
            sent = (written != -1);
        }
    }

    if (sent) {
        m_sendBytes += bytes.size();
        ui->sendBytesLabel->setText(QCoreApplication::translate("MainWindow", "发送: %1 字节").arg(m_sendBytes));

        processIncomingData(bytes, "#4ecca3");
    }
}

void MainWindow::on_clearReceiveBtn_clicked()
{
    ui->receiveTextEdit->clear();
    m_lineBuffer.clear();
    m_receiveBytes = 0;
    m_frameCount = 0;
    ui->receiveBytesLabel->setText(QCoreApplication::translate("MainWindow", "接收: 0 字节"));
    ui->frameCountLabel->setText(QCoreApplication::translate("MainWindow", "帧数: 0"));
}

void MainWindow::on_clearSendBtn_clicked()
{
    ui->sendTextEdit->clear();
    m_sendBytes = 0;
    ui->sendBytesLabel->setText(QCoreApplication::translate("MainWindow", "发送: 0 字节"));
}

void MainWindow::on_autoSendCheckBox_toggled(bool checked)
{
    if (checked) {
        int interval = ui->intervalSpinBox->value();
        m_autoSendTimer->start(interval);
        statusBar()->showMessage(QString("自动发送已开启，间隔 %1 ms").arg(interval));
    } else {
        m_autoSendTimer->stop();
        statusBar()->showMessage("自动发送已关闭");
    }
}

void MainWindow::on_intervalSpinBox_valueChanged(int value)
{
    if (m_autoSendTimer->isActive()) {
        m_autoSendTimer->setInterval(value);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
#ifdef HAS_SERIAL_PORT
    Q_UNUSED(index);
    if (index == 0) {
        refreshSerialPorts();
    }
#else
    Q_UNUSED(index);
#endif
}

void MainWindow::on_receiveModeCombo_currentIndexChanged(int index)
{
    m_receiveMode = index;
}

void MainWindow::on_autoSendTimer()
{
    on_sendBtn_clicked();
}

#ifdef HAS_SERIAL_PORT
void MainWindow::on_serialReadyRead()
{
    if (!m_serialPort) return;

    QByteArray data = m_serialPort->readAll();
    m_receiveBytes += data.size();
    m_frameCount++;

    ui->receiveBytesLabel->setText(QCoreApplication::translate("MainWindow", "接收: %1 字节").arg(m_receiveBytes));
    ui->frameCountLabel->setText(QCoreApplication::translate("MainWindow", "帧数: %1").arg(m_frameCount));

    processIncomingData(data, "#6eb5ff");
}
#endif

void MainWindow::on_tcpClientReadyRead()
{
    if (!m_tcpSocket) return;

    QByteArray data = m_tcpSocket->readAll();
    m_receiveBytes += data.size();
    m_frameCount++;

    ui->receiveBytesLabel->setText(QCoreApplication::translate("MainWindow", "接收: %1 字节").arg(m_receiveBytes));
    ui->frameCountLabel->setText(QCoreApplication::translate("MainWindow", "帧数: %1").arg(m_frameCount));

    processIncomingData(data, "#6eb5ff");
}

void MainWindow::on_tcpServerNewConnection()
{
    QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
        QByteArray data = clientSocket->readAll();
        m_receiveBytes += data.size();
        m_frameCount++;

        ui->receiveBytesLabel->setText(QCoreApplication::translate("MainWindow", "接收: %1 字节").arg(m_receiveBytes));
        ui->frameCountLabel->setText(QCoreApplication::translate("MainWindow", "帧数: %1").arg(m_frameCount));

        processIncomingData(data, "#6eb5ff");
    });
}

void MainWindow::on_udpReadyRead()
{
    if (!m_udpSocket) return;

    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_udpSocket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        m_receiveBytes += data.size();
        m_frameCount++;

        ui->receiveBytesLabel->setText(QCoreApplication::translate("MainWindow", "接收: %1 字节").arg(m_receiveBytes));
        ui->frameCountLabel->setText(QCoreApplication::translate("MainWindow", "帧数: %1").arg(m_frameCount));

        processIncomingData(data, "#6eb5ff");
    }
}

QByteArray MainWindow::parseHexInput(const QString &input)
{
    QByteArray result;
    QString clean = input.simplified();

    QStringList hexPairs = clean.split(' ', Qt::SkipEmptyParts);
    for (const QString &pair : hexPairs) {
        bool ok;
        int value = pair.toInt(&ok, 16);
        if (ok) {
            result.append(static_cast<char>(value));
        }
    }

    return result;
}

QString MainWindow::formatReceiveData(const QByteArray &data)
{
    if (m_receiveMode == 0) {
        return data.toHex(' ').toUpper();
    }

    QString text = QString::fromUtf8(data);
    qsizetype replacementCount = 0;
    for (int i = 0; i < text.size(); i++) {
        if (text[i].unicode() == 0xFFFD) replacementCount++;
    }

    if (replacementCount > data.size() / 4) {
        text = QString::fromLatin1(data);
    }

    QString result;
    result.reserve(text.size());
    for (int i = 0; i < text.size(); i++) {
        QChar ch = text[i];
        ushort uc = ch.unicode();
        if (ch.isPrint() || ch == '\n' || ch == '\r' || ch == '\t') {
            result.append(ch);
        } else if (uc < 0x20) {
            result.append(QString("<%1>").arg(uc, 2, 16, QChar('0')).toUpper());
        } else {
            result.append('.');
        }
    }
    return result.toHtmlEscaped();
}

void MainWindow::processIncomingData(const QByteArray &data, const QString &color)
{
    m_lineBuffer.append(data);
    m_lineBuffer.replace("\r\n", "\n");
    m_lineBuffer.replace('\r', '\n');
    QByteArray line;
    int pos;
    while ((pos = m_lineBuffer.indexOf('\n')) >= 0) {
        line = m_lineBuffer.left(pos);
        m_lineBuffer.remove(0, pos + 1);
        if (!line.isEmpty()) {
            QByteArray cleanLine = line;
            int escIdx;
            while ((escIdx = cleanLine.indexOf('\x1B')) >= 0) {
                int end = escIdx + 1;
                if (end < cleanLine.size() && cleanLine[end] == '[') {
                    end++;
                    while (end < cleanLine.size() &&
                           ((cleanLine[end] >= '0' && cleanLine[end] <= '9') ||
                            cleanLine[end] == ';')) {
                        end++;
                    }
                    if (end < cleanLine.size()) end++;
                }
                cleanLine.remove(escIdx, end - escIdx);
            }
            QString displayData = formatReceiveData(cleanLine);
            ui->receiveTextEdit->append(
                QString("<span style='color:%1'>%2</span>").arg(color, displayData));
        }
    }
}

quint16 MainWindow::modbusCRC16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); i++) {
        crc ^= static_cast<quint8>(data[i]);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void MainWindow::on_modbusUpdateBtn_clicked()
{
    int slave = ui->modbusSlaveSpinBox->value();
    int funcIndex = ui->modbusFuncCombo->currentIndex();
    int addr = ui->modbusAddrSpinBox->value();
    int count = ui->modbusCountSpinBox->value();
    int format = ui->modbusFormatCombo->currentIndex();

    // 功能码: 01, 03, 05, 06, 10
    const quint8 funcCodes[] = {0x01, 0x03, 0x05, 0x06, 0x10};
    quint8 func = funcCodes[funcIndex];

    QByteArray frame;
    frame.append(static_cast<char>(slave));
    frame.append(static_cast<char>(func));
    frame.append(static_cast<char>((addr >> 8) & 0xFF));
    frame.append(static_cast<char>(addr & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));

    QString text;
    if (format == 0) {
        // RTU: 追加 CRC16
        quint16 crc = modbusCRC16(frame);
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        text = frame.toHex(' ').toUpper();
    } else if (format == 1) {
        // ASCII: :开头，LRC校验，\r\n结尾
        quint8 lrc = 0;
        for (int i = 0; i < frame.size(); i++) {
            lrc += static_cast<quint8>(frame[i]);
        }
        lrc = static_cast<quint8>(-lrc);
        QByteArray asciiFrame;
        asciiFrame.append(':');
        for (int i = 0; i < frame.size(); i++) {
            asciiFrame.append(QByteArray::number(static_cast<quint8>(frame[i]), 16).rightJustified(2, '0').toUpper());
        }
        asciiFrame.append(QByteArray::number(lrc, 16).rightJustified(2, '0').toUpper());
        asciiFrame.append("\r\n");
        text = QString::fromLatin1(asciiFrame);
    } else {
        // TCP: 添加 MBAP 头
        QByteArray tcpFrame;
        tcpFrame.append(static_cast<char>(0x00)); // 事务ID高
        tcpFrame.append(static_cast<char>(0x01)); // 事务ID低
        tcpFrame.append(static_cast<char>(0x00)); // 协议ID高
        tcpFrame.append(static_cast<char>(0x00)); // 协议ID低
        quint16 len = frame.size();
        tcpFrame.append(static_cast<char>((len >> 8) & 0xFF));
        tcpFrame.append(static_cast<char>(len & 0xFF));
        tcpFrame.append(frame);
        text = tcpFrame.toHex(' ').toUpper();
    }

    ui->modbusResponseEdit->setPlainText(text);
}

void MainWindow::initQuickCmdGroups()
{
    // 设置表格列宽
    QHeaderView *header = ui->quickCmdTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->resizeSection(0, 30);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    header->resizeSection(3, 60);

    // 从文件加载分组配置
    QString cmdDir = findCommandsDir();
    if (!cmdDir.isEmpty()) {
        QDir dir(cmdDir);
        QStringList filters;
        filters << "*.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

        for (const QFileInfo &fileInfo : files) {
            QFile file(fileInfo.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) continue;

            QByteArray rawData = file.readAll();
            file.close();

            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
            if (error.error != QJsonParseError::NoError) continue;

            QJsonObject root = doc.object();
            QString groupName = root.value("group").toString();
            QJsonArray cmds = root.value("commands").toArray();

            QList<QuickCmdItem> items;
            for (const QJsonValue &val : cmds) {
                QJsonObject cmdObj = val.toObject();
                QuickCmdItem item;
                item.name = cmdObj.value("name").toString();
                item.data = cmdObj.value("data").toString();
                items.append(item);
            }

            if (!groupName.isEmpty() && !items.isEmpty()) {
                m_quickCmdGroups.insert(groupName, items);
            }
        }
    }

    // 如果没有加载到任何分组，使用默认配置
    if (m_quickCmdGroups.isEmpty()) {
        QList<QuickCmdItem> defaultCmds;
        defaultCmds.append(QuickCmdItem("自定义1", ""));
        defaultCmds.append(QuickCmdItem("自定义2", ""));
        m_quickCmdGroups.insert("默认", defaultCmds);
    }

    // 填充分组下拉框
    ui->quickCmdGroupCombo->addItems(m_quickCmdGroups.keys());

    // 加载第一个分组
    if (ui->quickCmdGroupCombo->count() > 0) {
        loadQuickCmdGroup(ui->quickCmdGroupCombo->currentText());
    }
}

void MainWindow::loadQuickCmdGroup(const QString &groupName)
{
    if (!m_quickCmdGroups.contains(groupName)) return;

    const QList<QuickCmdItem> &items = m_quickCmdGroups[groupName];
    ui->quickCmdTable->setRowCount(items.size());

    for (int i = 0; i < items.size(); i++) {
        const QuickCmdItem &item = items[i];

        // 勾选框 (column 0)
        QWidget *checkWidget = new QWidget();
        QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
        checkLayout->setContentsMargins(0, 0, 0, 0);
        checkLayout->setAlignment(Qt::AlignCenter);
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setChecked(true);
        checkLayout->addWidget(checkBox);
        ui->quickCmdTable->setCellWidget(i, 0, checkWidget);

        // 名称 (column 1)
        QTableWidgetItem *nameItem = new QTableWidgetItem(item.name);
        nameItem->setToolTip(item.name);
        ui->quickCmdTable->setItem(i, 1, nameItem);

        // 数据 (column 2)
        QTableWidgetItem *dataItem = new QTableWidgetItem(item.data);
        dataItem->setToolTip(item.data);
        ui->quickCmdTable->setItem(i, 2, dataItem);

        // 加载按钮 (column 3)
        QWidget *btnWidget = new QWidget();
        QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(2, 2, 2, 2);
        QPushButton *loadBtn = new QPushButton("加载");
        loadBtn->setFixedHeight(24);
        btnLayout->addWidget(loadBtn);
        ui->quickCmdTable->setCellWidget(i, 3, btnWidget);

        ui->quickCmdTable->setRowHeight(i, 32);
    }
}

void MainWindow::loadQuickCmdToSendArea(int row)
{
    QTableWidgetItem *dataItem = ui->quickCmdTable->item(row, 2);
    if (!dataItem) return;

    QString data = dataItem->text();
    ui->sendTextEdit->setPlainText(data);
}

void MainWindow::on_quickCmdSendBtn_clicked()
{
    // 停止之前的定时器
    m_quickCmdTimer->stop();

    // 收集所有勾选的指令
    QList<QPair<QString, QString>> checkedItems;
    for (int i = 0; i < ui->quickCmdTable->rowCount(); i++) {
        QWidget *w = ui->quickCmdTable->cellWidget(i, 0);
        if (!w) continue;
        QCheckBox *cb = w->findChild<QCheckBox *>();
        if (!cb || !cb->isChecked()) continue;

        QTableWidgetItem *nameItem = ui->quickCmdTable->item(i, 1);
        QTableWidgetItem *dataItem = ui->quickCmdTable->item(i, 2);
        if (nameItem && dataItem) {
            checkedItems.append({nameItem->text(), dataItem->text()});
        }
    }

    if (checkedItems.isEmpty()) {
        statusBar()->showMessage("未选中任何快捷指令");
        return;
    }

    int delay = ui->quickCmdDelaySpinBox->value();
    m_quickCmdIndex = 0;

    // 断开旧连接，重新连接
    m_quickCmdTimer->disconnect();
    connect(m_quickCmdTimer, &QTimer::timeout, this, [this, checkedItems, delay]() {
        if (m_quickCmdIndex >= checkedItems.size()) {
            m_quickCmdTimer->stop();
            statusBar()->showMessage("快捷指令发送完成");
            return;
        }

        const auto &item = checkedItems[m_quickCmdIndex];
        ui->sendTextEdit->setPlainText(item.second);
        on_sendBtn_clicked(false);

        m_quickCmdIndex++;

        // 如果还有下一条，重新设置间隔
        if (m_quickCmdIndex < checkedItems.size()) {
            m_quickCmdTimer->setInterval(delay);
        }
    });

    m_quickCmdTimer->setInterval(0);  // 立即发送第一条
    m_quickCmdTimer->start();
    statusBar()->showMessage(QString("开始发送 %1 条快捷指令...").arg(checkedItems.size()));
}

QString MainWindow::findCommandsDir()
{
    QString cmdDir = QCoreApplication::applicationDirPath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    cmdDir = QDir::currentPath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    cmdDir = dir.absolutePath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    return QString();
}

void MainWindow::saveQuickCmdGroup(const QString &groupName)
{
    if (!m_quickCmdGroups.contains(groupName)) return;

    QString cmdDir = findCommandsDir();
    if (cmdDir.isEmpty()) return;

    // 查找对应的 JSON 文件
    QDir dir(cmdDir);
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    QString filePath;
    for (const QFileInfo &fi : files) {
        QFile f(fi.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            f.close();
            if (doc.object().value("group").toString() == groupName) {
                filePath = fi.absoluteFilePath();
                break;
            }
        }
    }

    // 如果没找到，创建新文件
    if (filePath.isEmpty()) {
        filePath = cmdDir + "/" + groupName + ".json";
    }

    QJsonObject root;
    root["group"] = groupName;

    QJsonArray cmds;
    const QList<QuickCmdItem> &items = m_quickCmdGroups[groupName];
    for (const QuickCmdItem &item : items) {
        QJsonObject cmdObj;
        cmdObj["name"] = item.name;
        cmdObj["data"] = item.data;
        cmds.append(cmdObj);
    }
    root["commands"] = cmds;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void MainWindow::on_quickCmdAddBtn_clicked()
{
    QString groupName = ui->quickCmdGroupCombo->currentText();
    if (groupName.isEmpty()) return;

    bool ok;
    QString name = QInputDialog::getText(this, "添加快捷指令", "指令名称:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString data = QInputDialog::getText(this, "添加快捷指令", "指令数据:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QuickCmdItem item;
    item.name = name;
    item.data = data;

    m_quickCmdGroups[groupName].append(item);
    loadQuickCmdGroup(groupName);
    saveQuickCmdGroup(groupName);

    statusBar()->showMessage("快捷指令已添加");
}

void MainWindow::on_quickCmdDelBtn_clicked()
{
    QString groupName = ui->quickCmdGroupCombo->currentText();
    if (groupName.isEmpty()) return;

    // 收集选中的行（从后往前删除避免索引偏移）
    QList<int> selectedRows;
    for (int i = 0; i < ui->quickCmdTable->rowCount(); i++) {
        QWidget *w = ui->quickCmdTable->cellWidget(i, 0);
        if (!w) continue;
        QCheckBox *cb = w->findChild<QCheckBox *>();
        if (cb && cb->isChecked()) {
            selectedRows.prepend(i);  // 从后往前
        }
    }

    if (selectedRows.isEmpty()) {
        statusBar()->showMessage("请先勾选要删除的指令");
        return;
    }

    QList<QuickCmdItem> &items = m_quickCmdGroups[groupName];
    for (int row : selectedRows) {
        if (row < items.size()) {
            items.removeAt(row);
        }
    }

    loadQuickCmdGroup(groupName);
    saveQuickCmdGroup(groupName);

    statusBar()->showMessage(QString("已删除 %1 条指令").arg(selectedRows.size()));
}

void MainWindow::on_quickCmdSelectAllBtn_clicked()
{
    for (int i = 0; i < ui->quickCmdTable->rowCount(); i++) {
        QWidget *w = ui->quickCmdTable->cellWidget(i, 0);
        if (!w) continue;
        QCheckBox *cb = w->findChild<QCheckBox *>();
        if (cb) cb->setChecked(true);
    }
}

void MainWindow::on_quickCmdUnselectBtn_clicked()
{
    for (int i = 0; i < ui->quickCmdTable->rowCount(); i++) {
        QWidget *w = ui->quickCmdTable->cellWidget(i, 0);
        if (!w) continue;
        QCheckBox *cb = w->findChild<QCheckBox *>();
        if (cb) cb->setChecked(false);
    }
}

void MainWindow::on_quickCmdAddGroupBtn_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "添加分组", "分组名称:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    if (m_quickCmdGroups.contains(name)) {
        QMessageBox::warning(this, "添加分组", "分组已存在");
        return;
    }

    // 创建空分组
    QList<QuickCmdItem> emptyList;
    m_quickCmdGroups.insert(name, emptyList);

    // 更新下拉框（阻止信号避免触发 loadQuickCmdGroup）
    ui->quickCmdGroupCombo->blockSignals(true);
    ui->quickCmdGroupCombo->clear();
    ui->quickCmdGroupCombo->addItems(m_quickCmdGroups.keys());
    ui->quickCmdGroupCombo->setCurrentText(name);
    ui->quickCmdGroupCombo->blockSignals(false);

    // 显示空分组
    loadQuickCmdGroup(name);

    // 保存到文件
    saveQuickCmdGroup(name);

    statusBar()->showMessage("分组已添加: " + name);
}

void MainWindow::on_quickCmdDelGroupBtn_clicked()
{
    QString groupName = ui->quickCmdGroupCombo->currentText();
    if (groupName.isEmpty()) return;

    if (m_quickCmdGroups.size() <= 1) {
        QMessageBox::warning(this, "删除分组", "至少保留一个分组");
        return;
    }

    int ret = QMessageBox::question(this, "删除分组",
                                     QString("确定删除分组 \"%1\"?").arg(groupName),
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // 从数据结构中移除
    m_quickCmdGroups.remove(groupName);

    // 删除对应的 JSON 文件
    QString cmdDir = findCommandsDir();
    if (!cmdDir.isEmpty()) {
        QDir dir(cmdDir);
        QStringList filters;
        filters << "*.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
        for (const QFileInfo &fi : files) {
            QFile f(fi.absoluteFilePath());
            if (f.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                f.close();
                if (doc.object().value("group").toString() == groupName) {
                    QFile::remove(fi.absoluteFilePath());
                    break;
                }
            }
        }
    }

    // 更新下拉框
    ui->quickCmdGroupCombo->blockSignals(true);
    ui->quickCmdGroupCombo->clear();
    ui->quickCmdGroupCombo->addItems(m_quickCmdGroups.keys());
    ui->quickCmdGroupCombo->setCurrentIndex(0);
    ui->quickCmdGroupCombo->blockSignals(false);

    loadQuickCmdGroup(ui->quickCmdGroupCombo->currentText());

    statusBar()->showMessage("分组已删除: " + groupName);
}

void MainWindow::applyTheme(int themeIndex)
{
    m_currentTheme = themeIndex;

    if (themeIndex == 0) {
        // 深色主题
        setStyleSheet(R"(
            QMainWindow { background-color: #1a1a2e; }
            QWidget[native="true"] { background-color: #16213e; }
            QTabWidget::pane { border: 1px solid #2a2a4a; background-color: #16213e; }
            QTabBar::tab { background-color: #16213e; color: #a0a0a0; padding: 6px 16px; border: 1px solid #2a2a4a; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }
            QTabBar::tab:selected { background-color: #0f3460; color: #eaeaea; }
            QTabBar::tab:hover { background-color: #0f3460; }
            QLabel { color: #a0a0a0; background: transparent; }
            QComboBox, QSpinBox, QLineEdit { background-color: #1a1a2e; color: #eaeaea; border: 1px solid #2a2a4a; padding: 6px; border-radius: 4px; }
            QComboBox:hover, QSpinBox:hover, QLineEdit:hover { border: 1px solid #e94560; }
            QComboBox::drop-down { border: none; }
            QPushButton { background-color: #0f3460; color: #eaeaea; border: 1px solid #2a2a4a; padding: 6px 16px; border-radius: 4px; }
            QPushButton:hover { background-color: #16213e; border: 1px solid #e94560; }
            QPushButton:pressed { background-color: #e94560; border-color: #e94560; }
            QTextEdit { background-color: #1a1a2e; color: #6eb5ff; border: 1px solid #2a2a4a; border-radius: 6px; padding: 8px; font-family: Consolas, 'Monaco', monospace; font-size: 13px; }
            QCheckBox { color: #eaeaea; background: transparent; spacing: 8px; }
            QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; border: 1px solid #2a2a4a; background-color: #1a1a2e; }
            QCheckBox::indicator:checked { background-color: #e94560; border-color: #e94560; }
            QStatusBar { background-color: #16213e; color: #a0a0a0; }
            QSplitter::handle { background-color: #2a2a4a; width: 2px; }
            QSplitter::handle:hover { background-color: #e94560; }
            QTableWidget { background-color: #1a1a2e; color: #eaeaea; gridline-color: #2a2a4a; border: 1px solid #2a2a4a; }
            QTableWidget::item { padding: 4px; }
            QHeaderView::section { background-color: #16213e; color: #a0a0a0; border: 1px solid #2a2a4a; padding: 4px; }
            QGroupBox { color: #eaeaea; border: 1px solid #2a2a4a; border-radius: 6px; margin-top: 12px; padding-top: 20px; background-color: #16213e; }
            QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #e94560; }
        )");
    } else {
        // 浅色主题
        setStyleSheet(R"(
            QMainWindow { background-color: #f0f0f0; }
            QWidget[native="true"] { background-color: #ffffff; }
            QTabWidget::pane { border: 1px solid #d0d0d0; background-color: #ffffff; }
            QTabBar::tab { background-color: #e8e8e8; color: #555555; padding: 6px 16px; border: 1px solid #d0d0d0; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }
            QTabBar::tab:selected { background-color: #ffffff; color: #333333; }
            QTabBar::tab:hover { background-color: #f5f5f5; }
            QLabel { color: #555555; background: transparent; }
            QComboBox, QSpinBox, QLineEdit { background-color: #ffffff; color: #333333; border: 1px solid #c0c0c0; padding: 6px; border-radius: 4px; }
            QComboBox:hover, QSpinBox:hover, QLineEdit:hover { border: 1px solid #0078d4; }
            QComboBox::drop-down { border: none; }
            QPushButton { background-color: #0078d4; color: #ffffff; border: 1px solid #0060b0; padding: 6px 16px; border-radius: 4px; }
            QPushButton:hover { background-color: #1084e0; border: 1px solid #0078d4; }
            QPushButton:pressed { background-color: #0060b0; border-color: #0050a0; }
            QTextEdit { background-color: #ffffff; color: #333333; border: 1px solid #d0d0d0; border-radius: 6px; padding: 8px; font-family: Consolas, 'Monaco', monospace; font-size: 13px; }
            QCheckBox { color: #333333; background: transparent; spacing: 8px; }
            QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; border: 1px solid #c0c0c0; background-color: #ffffff; }
            QCheckBox::indicator:checked { background-color: #0078d4; border-color: #0078d4; }
            QStatusBar { background-color: #e8e8e8; color: #555555; }
            QSplitter::handle { background-color: #d0d0d0; width: 2px; }
            QSplitter::handle:hover { background-color: #0078d4; }
            QTableWidget { background-color: #ffffff; color: #333333; gridline-color: #d0d0d0; border: 1px solid #d0d0d0; }
            QTableWidget::item { padding: 4px; }
            QHeaderView::section { background-color: #e8e8e8; color: #555555; border: 1px solid #d0d0d0; padding: 4px; }
            QGroupBox { color: #333333; border: 1px solid #d0d0d0; border-radius: 6px; margin-top: 12px; padding-top: 20px; background-color: #ffffff; }
            QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #0078d4; }
        )");
    }
}

void MainWindow::loadSettings()
{
    QSettings settings("QtSerialAssist", "QtSerialAssist");

    // 加载主题
    int theme = settings.value("theme", 0).toInt();
    if (theme != m_currentTheme) {
        applyTheme(theme);
    }

    // 加载语言
    int lang = settings.value("language", 0).toInt();
    m_currentLanguage = lang;
    if (lang == 1) {
        if (m_translator->load("QtSerialAssist_zh_CN.qm",
                QCoreApplication::applicationDirPath())) {
            qApp->installTranslator(m_translator);
        }
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("QtSerialAssist", "QtSerialAssist");
    settings.setValue("theme", m_currentTheme);

    settings.setValue("language", m_currentLanguage);
}

void MainWindow::on_settingsBtn_clicked()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        int theme = dialog.currentTheme();
        int lang = dialog.currentLanguage();

        // 应用主题
        if (theme != m_currentTheme) {
            applyTheme(theme);
        }

        // 应用语言
        if (lang != m_currentLanguage) {
            m_currentLanguage = lang;
            if (lang == 1) {
                // English
                if (!m_translator->isEmpty()) {
                    qApp->removeTranslator(m_translator);
                }
                if (m_translator->load("QtSerialAssist_zh_CN.qm",
                        QCoreApplication::applicationDirPath())) {
                    qApp->installTranslator(m_translator);
                }
            } else {
                // 中文
                if (!m_translator->isEmpty()) {
                    qApp->removeTranslator(m_translator);
                }
            }

            // 刷新 .ui 中的字符串（retranslateUi 会重置计数器等动态值，需保存恢复）
            QString savedIp = ui->ipLineEdit->text();
            QString savedRxLabel = ui->receiveBytesLabel->text();
            QString savedTxLabel = ui->sendBytesLabel->text();
            QString savedFrameLabel = ui->frameCountLabel->text();

            ui->retranslateUi(this);

            ui->ipLineEdit->setText(savedIp);
            ui->receiveBytesLabel->setText(savedRxLabel);
            ui->sendBytesLabel->setText(savedTxLabel);
            ui->frameCountLabel->setText(savedFrameLabel);

            // retranslateUi 会覆盖部分样式，重新应用主题
            applyTheme(m_currentTheme);
        }

        // 保存设置
        saveSettings();
    }
}

void MainWindow::on_terminalModeBtn_clicked()
{
    setTerminalMode(!m_terminalMode);
}

void MainWindow::setTerminalMode(bool enabled)
{
    m_terminalMode = enabled;

    if (enabled) {
        // 保存分界比例，然后折叠底部面板
        m_savedSplitterSizes = ui->rightSplitter->sizes();
        ui->modbusSplitter->hide();

        // 按钮切换为"普通模式"
        ui->terminalModeBtn->setText(QCoreApplication::translate("MainWindow", "普通模式"));

        // 接收区变为可编辑，作为终端输入输出
        ui->receiveTextEdit->setReadOnly(false);
        ui->receiveTextEdit->setFocus();
        // 光标移到末尾
        QTextCursor cursor = ui->receiveTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->receiveTextEdit->setTextCursor(cursor);

        // 安装事件过滤器，捕获按键发送数据
        ui->receiveTextEdit->installEventFilter(this);

        // 更改按钮样式提示
        ui->terminalModeBtn->setStyleSheet(
            "background-color: #e94560; color: #ffffff; border: 1px solid #e94560;"
            "padding: 6px 16px; border-radius: 4px;");

        statusBar()->showMessage("终端模式已开启 - 在接收区直接输入发送");
    } else {
        // 恢复底部发送面板
        ui->modbusSplitter->show();
        if (!m_savedSplitterSizes.isEmpty()) {
            ui->rightSplitter->setSizes(m_savedSplitterSizes);
        }

        // 按钮恢复为"终端模式"
        ui->terminalModeBtn->setText(QCoreApplication::translate("MainWindow", "终端模式"));

        // 接收区恢复只读
        ui->receiveTextEdit->setReadOnly(true);

        // 移除事件过滤器
        ui->receiveTextEdit->removeEventFilter(this);

        // 恢复按钮样式
        ui->terminalModeBtn->setStyleSheet("");
        applyTheme(m_currentTheme);  // 重新应用主题恢复按钮样式

        statusBar()->showMessage("终端模式已关闭");
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (m_terminalMode && obj == ui->receiveTextEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        QString text = keyEvent->text();

        // Enter/Return: 发送换行
        if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            QByteArray newline;
            QString nl = ui->newlineCombo->currentText();
            if (nl == "\\r\\n") {
                newline = "\r\n";
            } else if (nl == "\\n") {
                newline = "\n";
            } else if (nl == "\\r") {
                newline = "\r";
            } else {
                newline = "\r\n";
            }

            // 在显示区追加换行
            ui->receiveTextEdit->insertPlainText("\n");
            QTextCursor cursor = ui->receiveTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->receiveTextEdit->setTextCursor(cursor);

            // 发送换行数据
            sendTerminalData(newline);
            return true;
        }

        // Backspace: 删除本地最后一个字符（不发送）
        if (key == Qt::Key_Backspace) {
            QTextCursor cursor = ui->receiveTextEdit->textCursor();
            if (!cursor.atStart()) {
                cursor.deletePreviousChar();
            }
            return true;
        }

        // 普通可打印字符或空格：发送并显示
        if (!text.isEmpty() && text[0].isPrint()) {
            QByteArray data = text.toUtf8();

            // 在显示区追加字符（用发送颜色）
            QTextCursor cursor = ui->receiveTextEdit->textCursor();
            cursor.movePosition(QTextCursor::End);

            QTextCharFormat fmt;
            fmt.setForeground(QColor("#4ecca3"));  // 发送颜色（绿色）
            cursor.insertText(text, fmt);

            ui->receiveTextEdit->setTextCursor(cursor);

            // 发送数据
            sendTerminalData(data);
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::sendTerminalData(const QByteArray &data)
{
    if (data.isEmpty()) return;

    int tabIndex = ui->tabWidget->currentIndex();

#ifdef HAS_SERIAL_PORT
    if (tabIndex == 0 && m_serialConnected && m_serialPort) {
        m_serialPort->write(data);
        m_serialPort->flush();
        m_sendBytes += data.size();
        ui->sendBytesLabel->setText(QCoreApplication::translate("MainWindow", "发送: %1 字节").arg(m_sendBytes));
        return;
    }
#endif

    int protocol = ui->protocolCombo->currentIndex();
    if (protocol == 1 && m_tcpConnected && m_tcpSocket) {
        m_tcpSocket->write(data);
        m_tcpSocket->flush();
        m_sendBytes += data.size();
        ui->sendBytesLabel->setText(QCoreApplication::translate("MainWindow", "发送: %1 字节").arg(m_sendBytes));
    } else if (protocol == 2 && m_udpBound && m_udpSocket) {
        QString ip = ui->ipLineEdit->text();
        quint16 port = ui->portSpinBox->value();
        m_udpSocket->writeDatagram(data, QHostAddress(ip), port);
        m_sendBytes += data.size();
        ui->sendBytesLabel->setText(QCoreApplication::translate("MainWindow", "发送: %1 字节").arg(m_sendBytes));
    }
}
