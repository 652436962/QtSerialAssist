#include "NetworkManager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
}

NetworkManager::~NetworkManager()
{
    closeConnection();
}

int NetworkManager::protocolIndex() const { return m_protocolIndex; }

void NetworkManager::setProtocolIndex(int index)
{
    if (m_protocolIndex == index) return;
    m_protocolIndex = index;
    emit protocolIndexChanged();
}

QString NetworkManager::ipAddress() const { return m_ipAddress; }

void NetworkManager::setIpAddress(const QString &ip)
{
    if (m_ipAddress == ip) return;
    m_ipAddress = ip;
    emit ipAddressChanged();
}

int NetworkManager::port() const { return m_port; }

void NetworkManager::setPort(int p)
{
    if (m_port == p) return;
    m_port = p;
    emit portChanged();
}

bool NetworkManager::isConnected() const { return m_connected; }

bool NetworkManager::openConnection()
{
    if (m_connected) {
        closeConnection();
    }

    switch (m_protocolIndex) {
    case 0: { // TCP Server
        m_tcpServer = new QTcpServer(this);
        connect(m_tcpServer, &QTcpServer::newConnection,
                this, &NetworkManager::onTcpServerNewConnection);

        if (m_tcpServer->listen(QHostAddress::Any, static_cast<quint16>(m_port))) {
            setConnected(true);
            return true;
        } else {
            emit errorOccurred(tr("TCP server failed to start"));
            delete m_tcpServer;
            m_tcpServer = nullptr;
            return false;
        }
    }
    case 1: { // TCP Client
        if (m_ipAddress.isEmpty()) {
            emit errorOccurred(tr("IP address is empty"));
            return false;
        }

        m_tcpSocket = new QTcpSocket(this);
        connect(m_tcpSocket, &QTcpSocket::readyRead,
                this, &NetworkManager::onTcpClientReadyRead);
        connect(m_tcpSocket, &QTcpSocket::disconnected,
                this, &NetworkManager::onTcpDisconnected);

        m_tcpSocket->connectToHost(m_ipAddress, static_cast<quint16>(m_port));
        if (m_tcpSocket->waitForConnected(3000)) {
            setConnected(true);
            return true;
        } else {
            emit errorOccurred(tr("Connection failed"));
            delete m_tcpSocket;
            m_tcpSocket = nullptr;
            return false;
        }
    }
    case 2: { // UDP
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead,
                this, &NetworkManager::onUdpReadyRead);

        if (m_udpSocket->bind(static_cast<quint16>(m_port))) {
            setConnected(true);
            return true;
        } else {
            emit errorOccurred(tr("UDP bind failed"));
            delete m_udpSocket;
            m_udpSocket = nullptr;
            return false;
        }
    }
    default:
        return false;
    }
}

void NetworkManager::closeConnection()
{
    if (m_tcpSocket) {
        m_tcpSocket->disconnectFromHost();
        delete m_tcpSocket;
        m_tcpSocket = nullptr;
    }
    if (m_tcpServer) {
        m_tcpServer->close();
        delete m_tcpServer;
        m_tcpServer = nullptr;
    }
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
    setConnected(false);
}

qint64 NetworkManager::writeData(const QByteArray &data)
{
    if (!m_connected) return -1;

    switch (m_protocolIndex) {
    case 1: { // TCP Client
        if (!m_tcpSocket) return -1;
        qint64 written = m_tcpSocket->write(data);
        m_tcpSocket->flush();
        return written;
    }
    case 2: { // UDP
        if (!m_udpSocket) return -1;
        return m_udpSocket->writeDatagram(data, QHostAddress(m_ipAddress), static_cast<quint16>(m_port));
    }
    default:
        return -1;
    }
}

void NetworkManager::onTcpClientReadyRead()
{
    if (m_tcpSocket) {
        emit dataReceived(m_tcpSocket->readAll());
    }
}

void NetworkManager::onTcpServerNewConnection()
{
    QTcpSocket *client = m_tcpServer->nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, this, [this, client]() {
        emit dataReceived(client->readAll());
    });
    connect(client, &QTcpSocket::disconnected, client, &QObject::deleteLater);
    emit newClientConnected();
}

void NetworkManager::onUdpReadyRead()
{
    if (!m_udpSocket) return;

    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        m_udpSocket->readDatagram(data.data(), data.size(), &sender, &senderPort);
        emit dataReceived(data);
    }
}

void NetworkManager::onTcpDisconnected()
{
    setConnected(false);
    emit clientDisconnected();
}

void NetworkManager::setConnected(bool c)
{
    if (m_connected == c) return;
    m_connected = c;
    emit connectedChanged();
}
