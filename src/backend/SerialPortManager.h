#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QTimer>

#ifdef HAS_SERIAL_PORT
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

class SerialPortManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList availablePorts READ availablePorts NOTIFY availablePortsChanged)
    Q_PROPERTY(QString currentPort READ currentPort WRITE setCurrentPort NOTIFY currentPortChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
    Q_PROPERTY(int dataBits READ dataBits WRITE setDataBits NOTIFY dataBitsChanged)
    Q_PROPERTY(int stopBits READ stopBits WRITE setStopBits NOTIFY stopBitsChanged)
    Q_PROPERTY(int parity READ parity WRITE setParity NOTIFY parityChanged)
    Q_PROPERTY(int flowControl READ flowControl WRITE setFlowControl NOTIFY flowControlChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager() override;

    QVariantList availablePorts() const;
    QString currentPort() const;
    void setCurrentPort(const QString &port);
    int baudRate() const;
    void setBaudRate(int rate);
    int dataBits() const;
    void setDataBits(int bits);
    int stopBits() const;
    void setStopBits(int bits);
    int parity() const;
    void setParity(int p);
    int flowControl() const;
    void setFlowControl(int fc);
    bool isConnected() const;

    Q_INVOKABLE bool openPort();
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE qint64 writeData(const QByteArray &data);

signals:
    void availablePortsChanged();
    void currentPortChanged();
    void baudRateChanged();
    void dataBitsChanged();
    void stopBitsChanged();
    void parityChanged();
    void flowControlChanged();
    void connectedChanged();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead();

private:
#ifdef HAS_SERIAL_PORT
    QSerialPort *m_serialPort = nullptr;
    QTimer *m_refreshTimer = nullptr;
#endif
    QVariantList m_availablePorts;
    QString m_currentPort;
    int m_baudRate = 115200;
    int m_dataBits = 3; // Data8
    int m_stopBits = 0; // OneStop
    int m_parity = 0;   // NoParity
    int m_flowControl = 0; // NoFlowControl
    bool m_connected = false;

    void setConnected(bool c);
};

#endif // SERIALPORTMANAGER_H
