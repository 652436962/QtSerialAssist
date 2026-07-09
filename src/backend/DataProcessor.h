#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QByteArray>

class DataProcessor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int displayMode READ displayMode WRITE setDisplayMode NOTIFY displayModeChanged)
    Q_PROPERTY(quint64 receiveBytes READ receiveBytes NOTIFY receiveBytesChanged)
    Q_PROPERTY(quint64 sendBytes READ sendBytes NOTIFY sendBytesChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(int receiveMode READ receiveMode WRITE setReceiveMode NOTIFY receiveModeChanged)

public:
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor() override;

    int displayMode() const;
    void setDisplayMode(int mode);

    quint64 receiveBytes() const;
    quint64 sendBytes() const;
    int frameCount() const;

    int receiveMode() const;
    void setReceiveMode(int mode);

    Q_INVOKABLE QString formatData(const QByteArray &data) const;
    Q_INVOKABLE QByteArray parseHexInput(const QString &input) const;
    Q_INVOKABLE void resetStats();
    Q_INVOKABLE void addReceiveBytes(qint64 count);
    Q_INVOKABLE void addSendBytes(qint64 count);
    Q_INVOKABLE void incrementFrameCount();

signals:
    void displayModeChanged();
    void receiveBytesChanged();
    void sendBytesChanged();
    void frameCountChanged();
    void receiveModeChanged();

private:
    int m_displayMode = 0;  // 0=HEX, 1=ASCII
    quint64 m_receiveBytes = 0;
    quint64 m_sendBytes = 0;
    int m_frameCount = 0;
    int m_receiveMode = 1;  // 0=HEX, 1=ASCII
};

#endif // DATAPROCESSOR_H
