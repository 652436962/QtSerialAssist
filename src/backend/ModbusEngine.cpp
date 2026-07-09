#include "ModbusEngine.h"

ModbusEngine::ModbusEngine(QObject *parent)
    : QObject(parent)
{
}

ModbusEngine::~ModbusEngine() = default;

int ModbusEngine::slaveAddr() const { return m_slaveAddr; }

void ModbusEngine::setSlaveAddr(int addr)
{
    if (m_slaveAddr == addr) return;
    m_slaveAddr = addr;
    emit slaveAddrChanged();
}

int ModbusEngine::functionCode() const { return m_functionCode; }

void ModbusEngine::setFunctionCode(int fc)
{
    if (m_functionCode == fc) return;
    m_functionCode = fc;
    emit functionCodeChanged();
}

int ModbusEngine::registerAddr() const { return m_registerAddr; }

void ModbusEngine::setRegisterAddr(int addr)
{
    if (m_registerAddr == addr) return;
    m_registerAddr = addr;
    emit registerAddrChanged();
}

int ModbusEngine::registerCount() const { return m_registerCount; }

void ModbusEngine::setRegisterCount(int count)
{
    if (m_registerCount == count) return;
    m_registerCount = count;
    emit registerCountChanged();
}

int ModbusEngine::format() const { return m_format; }

void ModbusEngine::setFormat(int f)
{
    if (m_format == f) return;
    m_format = f;
    emit formatChanged();
}

QString ModbusEngine::resultFrame() const { return m_resultFrame; }

QString ModbusEngine::generateFrame()
{
    // Function codes: 01, 03, 05, 06, 10
    const quint8 funcCodes[] = {0x01, 0x03, 0x05, 0x06, 0x10};
    quint8 func = funcCodes[m_functionCode];

    QByteArray frame;
    frame.append(static_cast<char>(m_slaveAddr));
    frame.append(static_cast<char>(func));
    frame.append(static_cast<char>((m_registerAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(m_registerAddr & 0xFF));
    frame.append(static_cast<char>((m_registerCount >> 8) & 0xFF));
    frame.append(static_cast<char>(m_registerCount & 0xFF));

    QString text;
    if (m_format == 0) {
        // RTU: append CRC16
        quint16 crc = crc16(frame);
        frame.append(static_cast<char>(crc & 0xFF));
        frame.append(static_cast<char>((crc >> 8) & 0xFF));
        text = frame.toHex(' ').toUpper();
    } else if (m_format == 1) {
        // ASCII: ':' prefix, LRC check, \r\n suffix
        quint8 lrcVal = lrc(frame);
        QByteArray asciiFrame;
        asciiFrame.append(':');
        for (int i = 0; i < frame.size(); i++) {
            asciiFrame.append(QByteArray::number(static_cast<quint8>(frame[i]), 16)
                                  .rightJustified(2, '0')
                                  .toUpper());
        }
        asciiFrame.append(QByteArray::number(lrcVal, 16)
                              .rightJustified(2, '0')
                              .toUpper());
        asciiFrame.append("\r\n");
        text = QString::fromLatin1(asciiFrame);
    } else {
        // TCP: add MBAP header
        QByteArray tcpFrame;
        tcpFrame.append(static_cast<char>(0x00)); // Transaction ID high
        tcpFrame.append(static_cast<char>(0x01)); // Transaction ID low
        tcpFrame.append(static_cast<char>(0x00)); // Protocol ID high
        tcpFrame.append(static_cast<char>(0x00)); // Protocol ID low
        quint16 len = frame.size();
        tcpFrame.append(static_cast<char>((len >> 8) & 0xFF));
        tcpFrame.append(static_cast<char>(len & 0xFF));
        tcpFrame.append(frame);
        text = tcpFrame.toHex(' ').toUpper();
    }

    if (m_resultFrame != text) {
        m_resultFrame = text;
        emit resultFrameChanged();
    }
    return text;
}

quint16 ModbusEngine::crc16(const QByteArray &data)
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

quint8 ModbusEngine::lrc(const QByteArray &data)
{
    quint8 lrc = 0;
    for (int i = 0; i < data.size(); i++) {
        lrc += static_cast<quint8>(data[i]);
    }
    return static_cast<quint8>(-lrc);
}
