#ifndef QUICKCMDMANAGER_H
#define QUICKCMDMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QTimer>
#include <QMap>

struct QuickCmdItem {
    QString name;
    QString data;
    QuickCmdItem() = default;
    QuickCmdItem(const QString &n, const QString &d) : name(n), data(d) {}
};

class QuickCmdManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QString currentGroup READ currentGroup WRITE setCurrentGroup NOTIFY currentGroupChanged)
    Q_PROPERTY(QVariantList commands READ commands NOTIFY commandsChanged)
    Q_PROPERTY(int sendDelay READ sendDelay WRITE setSendDelay NOTIFY sendDelayChanged)

public:
    explicit QuickCmdManager(QObject *parent = nullptr);
    ~QuickCmdManager() override;

    QVariantList groups() const;
    QString currentGroup() const;
    void setCurrentGroup(const QString &group);
    QVariantList commands() const;
    int sendDelay() const;
    void setSendDelay(int ms);

    Q_INVOKABLE bool addGroup(const QString &name);
    Q_INVOKABLE bool removeGroup(const QString &name);
    Q_INVOKABLE bool addCommand(const QString &name, const QString &data);
    Q_INVOKABLE bool removeCommand(int index);
    Q_INVOKABLE void loadGroup(const QString &groupName);
    Q_INVOKABLE void saveGroup(const QString &groupName);
    Q_INVOKABLE void sendSelectedCommands();
    Q_INVOKABLE void stopSending();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void deselectAll();

signals:
    void groupsChanged();
    void currentGroupChanged();
    void commandsChanged();
    void sendDelayChanged();
    void sendData(const QString &data);
    void sendComplete();
    void errorOccurred(const QString &error);

private slots:
    void onSendTimer();

private:
    QMap<QString, QList<QuickCmdItem>> m_quickCmdGroups;
    QVariantList m_groupsList;
    QVariantList m_commandsList;
    QString m_currentGroup;
    int m_sendDelay = 100;

    QTimer *m_sendTimer = nullptr;
    QList<QPair<QString, QString>> m_pendingSends;
    int m_sendIndex = 0;

    QString findCommandsDir();
    void rebuildGroupsList();
    void rebuildCommandsList();
};

#endif // QUICKCMDMANAGER_H
