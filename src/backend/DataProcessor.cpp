#include "DataProcessor.h"

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
}

DataProcessor::~DataProcessor() = default;

int DataProcessor::displayMode() const { return m_displayMode; }

void DataProcessor::setDisplayMode(int mode)
{
    if (m_displayMode == mode) return;
    m_displayMode = mode;
    emit displayModeChanged();
}

quint64 DataProcessor::receiveBytes() const { return m_receiveBytes; }

quint64 DataProcessor::sendBytes() const { return m_sendBytes; }

int DataProcessor::frameCount() const { return m_frameCount; }

int DataProcessor::receiveMode() const { return m_receiveMode; }

void DataProcessor::setReceiveMode(int mode)
{
    if (m_receiveMode == mode) return;
    m_receiveMode = mode;
    emit receiveModeChanged();
}

QString DataProcessor::formatData(const QByteArray &data) const
{
    if (m_receiveMode == 0) {
        // HEX mode
        return data.toHex(' ').toUpper();
    }

    // ASCII mode
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

QByteArray DataProcessor::parseHexInput(const QString &input) const
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

void DataProcessor::resetStats()
{
    bool changed = false;
    if (m_receiveBytes != 0) { m_receiveBytes = 0; changed = true; }
    if (m_sendBytes != 0) { m_sendBytes = 0; changed = true; }
    if (m_frameCount != 0) { m_frameCount = 0; changed = true; }
    if (changed) {
        emit receiveBytesChanged();
        emit sendBytesChanged();
        emit frameCountChanged();
    }
}

void DataProcessor::addReceiveBytes(qint64 count)
{
    if (count <= 0) return;
    m_receiveBytes += count;
    emit receiveBytesChanged();
}

void DataProcessor::addSendBytes(qint64 count)
{
    if (count <= 0) return;
    m_sendBytes += count;
    emit sendBytesChanged();
}

void DataProcessor::incrementFrameCount()
{
    m_frameCount++;
    emit frameCountChanged();
}
