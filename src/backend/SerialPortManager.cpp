#include "SerialPortManager.h"

#ifdef HAS_SERIAL_PORT

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_refreshTimer(new QTimer(this))
{
    m_refreshTimer->setInterval(2000);
    connect(m_refreshTimer, &QTimer::timeout, this, &SerialPortManager::refreshPorts);
    m_refreshTimer->start();
    refreshPorts();
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

QVariantList SerialPortManager::availablePorts() const { return m_availablePorts; }

QString SerialPortManager::currentPort() const { return m_currentPort; }

void SerialPortManager::setCurrentPort(const QString &port)
{
    if (m_currentPort == port) return;
    m_currentPort = port;
    emit currentPortChanged();
}

int SerialPortManager::baudRate() const { return m_baudRate; }

void SerialPortManager::setBaudRate(int rate)
{
    if (m_baudRate == rate) return;
    m_baudRate = rate;
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->setBaudRate(rate);
    }
    emit baudRateChanged();
}

int SerialPortManager::dataBits() const { return m_dataBits; }

void SerialPortManager::setDataBits(int bits)
{
    if (m_dataBits == bits) return;
    m_dataBits = bits;
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(bits));
    }
    emit dataBitsChanged();
}

int SerialPortManager::stopBits() const { return m_stopBits; }

void SerialPortManager::setStopBits(int bits)
{
    if (m_stopBits == bits) return;
    m_stopBits = bits;
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(bits));
    }
    emit stopBitsChanged();
}

int SerialPortManager::parity() const { return m_parity; }

void SerialPortManager::setParity(int p)
{
    if (m_parity == p) return;
    m_parity = p;
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->setParity(static_cast<QSerialPort::Parity>(p));
    }
    emit parityChanged();
}

int SerialPortManager::flowControl() const { return m_flowControl; }

void SerialPortManager::setFlowControl(int fc)
{
    if (m_flowControl == fc) return;
    m_flowControl = fc;
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(fc));
    }
    emit flowControlChanged();
}

bool SerialPortManager::isConnected() const { return m_connected; }

bool SerialPortManager::openPort()
{
    if (m_connected) {
        closePort();
    }

    QString portName = m_currentPort.section(" - ", 0, 0);

    if (portName.isEmpty()) {
        emit errorOccurred(tr("No port selected"));
        return false;
    }

    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName(portName);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serialPort->errorString());
        delete m_serialPort;
        m_serialPort = nullptr;
        return false;
    }

    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(m_dataBits));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(m_stopBits));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(m_parity));
    m_serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(m_flowControl));

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);

    setConnected(true);
    return true;
}

void SerialPortManager::closePort()
{
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
        delete m_serialPort;
        m_serialPort = nullptr;
    }
    setConnected(false);
}

qint64 SerialPortManager::writeData(const QByteArray &data)
{
    if (!m_serialPort || !m_serialPort->isOpen()) return -1;
    qint64 written = m_serialPort->write(data);
    m_serialPort->flush();
    return written;
}

void SerialPortManager::refreshPorts()
{
    QVariantList ports;
    const auto portInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : portInfos) {
        QString display = info.portName();
        QString desc = info.description();
        if (!desc.isEmpty()) {
            display += " - " + desc;
        }
        ports.append(display);
    }

    if (m_availablePorts != ports) {
        m_availablePorts = ports;
        emit availablePortsChanged();
    }
}

void SerialPortManager::onReadyRead()
{
    if (m_serialPort) {
        emit dataReceived(m_serialPort->readAll());
    }
}

void SerialPortManager::setConnected(bool c)
{
    if (m_connected == c) return;
    m_connected = c;
    emit connectedChanged();
}

#else // !HAS_SERIAL_PORT

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
{
}

SerialPortManager::~SerialPortManager() = default;
QVariantList SerialPortManager::availablePorts() const { return {}; }
QString SerialPortManager::currentPort() const { return {}; }
void SerialPortManager::setCurrentPort(const &) {}
int SerialPortManager::baudRate() const { return 115200; }
void SerialPortManager::setBaudRate(int) {}
int SerialPortManager::dataBits() const { return 3; }
void SerialPortManager::setDataBits(int) {}
int SerialPortManager::stopBits() const { return 0; }
void SerialPortManager::setStopBits(int) {}
int SerialPortManager::parity() const { return 0; }
void SerialPortManager::setParity(int) {}
int SerialPortManager::flowControl() const { return 0; }
void SerialPortManager::setFlowControl(int) {}
bool SerialPortManager::isConnected() const { return false; }
bool SerialPortManager::openPort() { emit errorOccurred(tr("Serial port support not available")); return false; }
void SerialPortManager::closePort() {}
void SerialPortManager::refreshPorts() {}
void SerialPortManager::onReadyRead() {}
void SerialPortManager::setConnected(bool) {}
qint64 SerialPortManager::writeData(const QByteArray &) { return -1; }

#endif // HAS_SERIAL_PORT
