#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QHostAddress>

class NetworkManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int protocolIndex READ protocolIndex WRITE setProtocolIndex NOTIFY protocolIndexChanged)
    Q_PROPERTY(QString ipAddress READ ipAddress WRITE setIpAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() override;

    int protocolIndex() const;
    void setProtocolIndex(int index);

    QString ipAddress() const;
    void setIpAddress(const QString &ip);

    int port() const;
    void setPort(int p);

    bool isConnected() const;

    Q_INVOKABLE bool openConnection();
    Q_INVOKABLE void closeConnection();
    Q_INVOKABLE qint64 writeData(const QByteArray &data);

signals:
    void protocolIndexChanged();
    void ipAddressChanged();
    void portChanged();
    void connectedChanged();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void newClientConnected();
    void clientDisconnected();

private slots:
    void onTcpClientReadyRead();
    void onTcpServerNewConnection();
    void onUdpReadyRead();
    void onTcpDisconnected();

private:
    QTcpSocket *m_tcpSocket = nullptr;
    QTcpServer *m_tcpServer = nullptr;
    QUdpSocket *m_udpSocket = nullptr;

    int m_protocolIndex = 0; // 0=TCP Server, 1=TCP Client, 2=UDP
    QString m_ipAddress;
    int m_port = 8080;
    bool m_connected = false;

    void setConnected(bool c);
};

#endif // NETWORKMANAGER_H
