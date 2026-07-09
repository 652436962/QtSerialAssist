#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QTimer>
#include <QByteArray>

class SettingsManager;
class SerialPortManager;
class NetworkManager;
class DataProcessor;
class ModbusEngine;
class QuickCmdManager;

class AppCore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* serialPort READ serialPort CONSTANT)
    Q_PROPERTY(QObject* network READ network CONSTANT)
    Q_PROPERTY(QObject* dataProcessor READ dataProcessor CONSTANT)
    Q_PROPERTY(QObject* modbus READ modbus CONSTANT)
    Q_PROPERTY(QObject* quickCmd READ quickCmd CONSTANT)
    Q_PROPERTY(QObject* settings READ settings CONSTANT)
    Q_PROPERTY(bool terminalMode READ terminalMode WRITE setTerminalMode NOTIFY terminalModeChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool autoSendEnabled READ autoSendEnabled WRITE setAutoSendEnabled NOTIFY autoSendEnabledChanged)
    Q_PROPERTY(int autoSendInterval READ autoSendInterval NOTIFY autoSendIntervalChanged)

public:
    explicit AppCore(QObject *parent = nullptr);
    ~AppCore() override;

    QObject* serialPort() const;
    QObject* network() const;
    QObject* dataProcessor() const;
    QObject* modbus() const;
    QObject* quickCmd() const;
    QObject* settings() const;

    bool terminalMode() const;
    void setTerminalMode(bool enabled);

    QString statusMessage() const;
    QString version() const;

    bool autoSendEnabled() const;
    void setAutoSendEnabled(bool enabled);
    int autoSendInterval() const;

    Q_INVOKABLE bool sendData(const QString &data, bool hexMode = false, bool appendNewline = false, int newlineMode = 0);
    Q_INVOKABLE bool sendRawData(const QByteArray &data);
    Q_INVOKABLE void setAutoSend(bool enabled, int intervalMs);
    Q_INVOKABLE void stopAutoSend();

signals:
    void terminalModeChanged();
    void statusMessageChanged();
    void dataReceived(const QString &formattedData);
    void dataSent(const QString &formattedData);
    void autoSendEnabledChanged();
    void autoSendIntervalChanged();
    void autoSendRequested();
    void errorOccurred(const QString &error);

private slots:
    void onSerialDataReceived(const QByteArray &data);
    void onNetworkDataReceived(const QByteArray &data);
    void onQuickCmdSendData(const QString &data);
    void onAutoSendTimer();

private:
    SettingsManager *m_settings = nullptr;
    SerialPortManager *m_serialPort = nullptr;
    NetworkManager *m_network = nullptr;
    DataProcessor *m_dataProcessor = nullptr;
    ModbusEngine *m_modbus = nullptr;
    QuickCmdManager *m_quickCmd = nullptr;

    QTimer *m_autoSendTimer = nullptr;
    QByteArray m_lineBuffer;
    bool m_terminalMode = false;
    QString m_statusMessage;
    bool m_autoSendEnabled = false;

    void processIncomingData(const QByteArray &data);
    void updateStatus(const QString &msg);
};

#endif // APPCORE_H
