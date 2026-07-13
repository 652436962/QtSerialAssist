#include "AppCore.h"
#include "SettingsManager.h"
#include "SerialPortManager.h"
#include "NetworkManager.h"
#include "DataProcessor.h"
#include "ModbusEngine.h"
#include "QuickCmdManager.h"

AppCore::AppCore(QObject *parent)
    : QObject(parent)
    , m_settings(new SettingsManager(this))
    , m_serialPort(new SerialPortManager(this))
    , m_network(new NetworkManager(this))
    , m_dataProcessor(new DataProcessor(this))
    , m_modbus(new ModbusEngine(this))
    , m_quickCmd(new QuickCmdManager(this))
    , m_autoSendTimer(new QTimer(this))
{
    // Connect serial port data
    connect(m_serialPort, &SerialPortManager::dataReceived,
            this, &AppCore::onSerialDataReceived);
    connect(m_serialPort, &SerialPortManager::errorOccurred,
            this, [this](const QString &msg) { updateStatus(msg); });

    // Connect network data
    connect(m_network, &NetworkManager::dataReceived,
            this, &AppCore::onNetworkDataReceived);
    connect(m_network, &NetworkManager::errorOccurred,
            this, [this](const QString &msg) { updateStatus(msg); });

    // Connect quick command send
    connect(m_quickCmd, &QuickCmdManager::sendData,
            this, &AppCore::onQuickCmdSendData);
    connect(m_quickCmd, &QuickCmdManager::errorOccurred,
            this, [this](const QString &msg) { updateStatus(msg); });

    // Auto-send timer
    m_autoSendTimer->setSingleShot(false);
    connect(m_autoSendTimer, &QTimer::timeout, this, &AppCore::onAutoSendTimer);

    // Status updates
    connect(m_serialPort, &SerialPortManager::connectedChanged, this, [this]() {
        if (m_serialPort->isConnected())
            updateStatus(tr("串口已连接"));
        else
            updateStatus(tr("串口已断开"));
    });
    connect(m_network, &NetworkManager::connectedChanged, this, [this]() {
        if (m_network->isConnected())
            updateStatus(tr("网络已连接"));
        else
            updateStatus(tr("网络已断开"));
    });

    // Apply saved theme
    m_settings->load();

    updateStatus(tr("Ready"));
}

AppCore::~AppCore() = default;

QObject* AppCore::serialPort() const { return m_serialPort; }
QObject* AppCore::network() const { return m_network; }
QObject* AppCore::dataProcessor() const { return m_dataProcessor; }
QObject* AppCore::modbus() const { return m_modbus; }
QObject* AppCore::quickCmd() const { return m_quickCmd; }
QObject* AppCore::settings() const { return m_settings; }

bool AppCore::terminalMode() const { return m_terminalMode; }

void AppCore::setTerminalMode(bool enabled)
{
    if (m_terminalMode == enabled) return;
    m_terminalMode = enabled;
    updateStatus(enabled ? tr("Terminal mode ON") : tr("Terminal mode OFF"));
    emit terminalModeChanged();
}

QString AppCore::statusMessage() const { return m_statusMessage; }

QString AppCore::version() const
{
    return QString("2.1.8 (Qt %1)").arg(QT_VERSION_STR);
}

bool AppCore::autoSendEnabled() const { return m_autoSendEnabled; }

int AppCore::autoSendInterval() const { return m_autoSendTimer->interval(); }

void AppCore::setAutoSend(bool enabled, int intervalMs)
{
    setAutoSendEnabled(enabled);
    if (enabled && intervalMs > 0) {
        m_autoSendTimer->setInterval(intervalMs);
    }
}

void AppCore::stopAutoSend()
{
    setAutoSendEnabled(false);
}

void AppCore::setAutoSendEnabled(bool enabled)
{
    if (m_autoSendEnabled == enabled) return;
    m_autoSendEnabled = enabled;

    if (enabled) {
        m_autoSendTimer->setInterval(m_settings->autoSendInterval());
        m_autoSendTimer->start();
        updateStatus(tr("Auto send ON, interval %1 ms").arg(m_autoSendTimer->interval()));
    } else {
        m_autoSendTimer->stop();
        updateStatus(tr("Auto send OFF"));
    }
    emit autoSendEnabledChanged();
}

bool AppCore::sendData(const QString &data, bool hexMode, bool appendNewline, int newlineMode)
{
    if (data.isEmpty()) {
        updateStatus(tr("No data to send"));
        return false;
    }

    QByteArray bytes;
    if (hexMode) {
        bytes = m_dataProcessor->parseHexInput(data);
        if (bytes.isEmpty()) {
            updateStatus(tr("HEX parse failed - invalid hex input"));
            return false;
        }
    } else {
        bytes = data.toUtf8();
    }

    if (bytes.isEmpty()) return false;

    if (appendNewline) {
        switch (newlineMode) {
        case 0: bytes.append("\r\n"); break;
        case 1: bytes.append("\n"); break;
        case 2: bytes.append("\r"); break;
        default: bytes.append("\r\n"); break;
        }
    }

    return sendRawData(bytes);
}

bool AppCore::sendRawData(const QByteArray &data)
{
    if (data.isEmpty()) return false;

    bool sent = false;

    // Try serial first
    if (m_serialPort->isConnected()) {
        qint64 written = m_serialPort->writeData(data);
        sent = (written != -1);
    }

    // Try network if serial didn't send
    if (!sent && m_network->isConnected()) {
        qint64 written = m_network->writeData(data);
        sent = (written != -1);
    }

    if (sent) {
        m_dataProcessor->addSendBytes(data.size());
        // Emit dataSent with formatted data (split lines like processIncomingData)
        QByteArray temp = data;
        temp.replace("\r\n", "\n");
        temp.replace('\r', '\n');
        QList<QByteArray> lines = temp.split('\n');
        for (const QByteArray &line : lines) {
            QString formatted = m_dataProcessor->formatData(line);
            if (!formatted.isEmpty())
                emit dataSent(formatted);
        }
    }

    return sent;
}

void AppCore::onSerialDataReceived(const QByteArray &data)
{
    m_dataProcessor->addReceiveBytes(data.size());
    m_dataProcessor->incrementFrameCount();
    processIncomingData(data);
}

void AppCore::onNetworkDataReceived(const QByteArray &data)
{
    m_dataProcessor->addReceiveBytes(data.size());
    m_dataProcessor->incrementFrameCount();
    processIncomingData(data);
}

void AppCore::onQuickCmdSendData(const QString &data)
{
    QByteArray bytes = m_dataProcessor->parseHexInput(data);
    if (bytes.isEmpty()) {
        bytes = data.toUtf8();
    }
    sendRawData(bytes);
}

void AppCore::onAutoSendTimer()
{
    emit autoSendRequested();
}

void AppCore::processIncomingData(const QByteArray &data)
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
            // Strip ANSI escape sequences
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

            QString formatted = m_dataProcessor->formatData(cleanLine);
            emit dataReceived(formatted);
        }
    }
}

void AppCore::updateStatus(const QString &msg)
{
    if (m_statusMessage == msg) return;
    m_statusMessage = msg;
    emit statusMessageChanged();
}
