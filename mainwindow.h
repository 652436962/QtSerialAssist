#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QTimer>
#include <QTextCursor>
#include <QDateTime>
#include <QMap>

struct QuickCmdItem {
    QString name;
    QString data;
    QuickCmdItem() = default;
    QuickCmdItem(const QString &n, const QString &d) : name(n), data(d) {}
};

#ifdef HAS_SERIAL_PORT
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_connectBtn_clicked();
    void on_sendBtn_clicked(bool appendNewline = false);
    void on_clearReceiveBtn_clicked();
    void on_clearSendBtn_clicked();
    void on_autoSendCheckBox_toggled(bool checked);
    void on_intervalSpinBox_valueChanged(int value);

    void on_tcpClientReadyRead();
    void on_tcpServerNewConnection();
    void on_udpReadyRead();

    void on_autoSendTimer();

    void on_tabWidget_currentChanged(int index);

    void on_modbusUpdateBtn_clicked();
    void on_quickCmdLoadBtn_clicked(int row);
    void on_quickCmdSendBtn_clicked();
    void on_quickCmdAddBtn_clicked();
    void on_quickCmdDelBtn_clicked();
    void on_quickCmdSelectAllBtn_clicked();
    void on_quickCmdUnselectBtn_clicked();
    void on_quickCmdAddGroupBtn_clicked();
    void on_quickCmdDelGroupBtn_clicked();

#ifdef HAS_SERIAL_PORT
    void on_serialReadyRead();
    void refreshSerialPorts();
#endif

private:
    Ui::MainWindow *ui;

#ifdef HAS_SERIAL_PORT
    QSerialPort *m_serialPort = nullptr;
#endif
    QTcpSocket *m_tcpSocket = nullptr;
    QTcpServer *m_tcpServer = nullptr;
    QUdpSocket *m_udpSocket = nullptr;
    QTimer *m_autoSendTimer = nullptr;
    QTimer *m_autoRefreshTimer = nullptr;

    bool m_serialConnected = false;
    bool m_tcpConnected = false;
    bool m_udpBound = false;

    QString m_currentProtocol;

    void initUI();
    void initConnections();

    void appendReceiveData(const QString &data, bool isSend = false);
    void updateStatus(const QString &status);

    QByteArray formatHexData(const QByteArray &data);
    QByteArray parseHexInput(const QString &input);
    quint16 modbusCRC16(const QByteArray &data);

    void initQuickCmdGroups();
    void loadQuickCmdGroup(const QString &groupName);
    void saveQuickCmdGroup(const QString &groupName);
    QString findCommandsDir();

    QTimer *m_quickCmdTimer = nullptr;
    int m_quickCmdIndex = 0;
    QMap<QString, QList<QuickCmdItem>> m_quickCmdGroups;

    quint64 m_sendBytes = 0;
    quint64 m_receiveBytes = 0;
    int m_frameCount = 0;
};
#endif
