#include "QuickCmdManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfoList>

QuickCmdManager::QuickCmdManager(QObject *parent)
    : QObject(parent)
    , m_sendTimer(new QTimer(this))
{
    m_sendTimer->setSingleShot(false);
    connect(m_sendTimer, &QTimer::timeout, this, &QuickCmdManager::onSendTimer);

    // Load groups from files
    QString cmdDir = findCommandsDir();
    if (!cmdDir.isEmpty()) {
        QDir dir(cmdDir);
        QStringList filters;
        filters << "*.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

        for (const QFileInfo &fileInfo : files) {
            QFile file(fileInfo.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) continue;

            QByteArray rawData = file.readAll();
            file.close();

            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
            if (error.error != QJsonParseError::NoError) continue;

            QJsonObject root = doc.object();
            QString groupName = root.value("group").toString();
            QJsonArray cmds = root.value("commands").toArray();

            QList<QuickCmdItem> items;
            for (const QJsonValue &val : cmds) {
                QJsonObject cmdObj = val.toObject();
                QuickCmdItem item;
                item.name = cmdObj.value("name").toString();
                item.data = cmdObj.value("data").toString();
                items.append(item);
            }

            if (!groupName.isEmpty()) {
                m_quickCmdGroups.insert(groupName, items);
            }
        }
    }

    // Default group if nothing loaded
    if (m_quickCmdGroups.isEmpty()) {
        QList<QuickCmdItem> defaultCmds;
        defaultCmds.append(QuickCmdItem("Custom1", ""));
        defaultCmds.append(QuickCmdItem("Custom2", ""));
        m_quickCmdGroups.insert("Default", defaultCmds);
    }

    rebuildGroupsList();

    if (!m_quickCmdGroups.isEmpty()) {
        m_currentGroup = m_quickCmdGroups.firstKey();
        loadGroup(m_currentGroup);
    }
}

QuickCmdManager::~QuickCmdManager() = default;

QVariantList QuickCmdManager::groups() const { return m_groupsList; }

QString QuickCmdManager::currentGroup() const { return m_currentGroup; }

void QuickCmdManager::setCurrentGroup(const QString &group)
{
    if (m_currentGroup == group) return;
    m_currentGroup = group;
    loadGroup(group);
    emit currentGroupChanged();
}

QVariantList QuickCmdManager::commands() const { return m_commandsList; }

int QuickCmdManager::sendDelay() const { return m_sendDelay; }

void QuickCmdManager::setSendDelay(int ms)
{
    if (m_sendDelay == ms) return;
    m_sendDelay = ms;
    emit sendDelayChanged();
}

bool QuickCmdManager::addGroup(const QString &name)
{
    if (name.isEmpty() || m_quickCmdGroups.contains(name)) {
        emit errorOccurred(tr("Group already exists or name is empty"));
        return false;
    }

    m_quickCmdGroups.insert(name, QList<QuickCmdItem>());
    rebuildGroupsList();
    saveGroup(name);
    emit groupsChanged();

    m_currentGroup = name;
    rebuildCommandsList();
    emit currentGroupChanged();

    return true;
}

bool QuickCmdManager::removeGroup(const QString &name)
{
    if (!m_quickCmdGroups.contains(name)) return false;
    if (m_quickCmdGroups.size() <= 1) {
        emit errorOccurred(tr("Must keep at least one group"));
        return false;
    }

    // Delete JSON file
    QString cmdDir = findCommandsDir();
    if (!cmdDir.isEmpty()) {
        QDir dir(cmdDir);
        QStringList filters;
        filters << "*.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
        for (const QFileInfo &fi : files) {
            QFile f(fi.absoluteFilePath());
            if (f.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                f.close();
                if (doc.object().value("group").toString() == name) {
                    QFile::remove(fi.absoluteFilePath());
                    break;
                }
            }
        }
    }

    m_quickCmdGroups.remove(name);
    rebuildGroupsList();
    emit groupsChanged();

    if (m_currentGroup == name) {
        m_currentGroup = m_quickCmdGroups.firstKey();
        loadGroup(m_currentGroup);
        emit currentGroupChanged();
    }

    return true;
}

bool QuickCmdManager::addCommand(const QString &name, const QString &data)
{
    if (m_currentGroup.isEmpty() || !m_quickCmdGroups.contains(m_currentGroup)) return false;

    m_quickCmdGroups[m_currentGroup].append(QuickCmdItem(name, data));
    rebuildCommandsList();
    saveGroup(m_currentGroup);
    emit commandsChanged();
    return true;
}

bool QuickCmdManager::removeCommand(int index)
{
    if (!m_quickCmdGroups.contains(m_currentGroup)) return false;

    QList<QuickCmdItem> &items = m_quickCmdGroups[m_currentGroup];
    if (index < 0 || index >= items.size()) return false;

    items.removeAt(index);
    rebuildCommandsList();
    saveGroup(m_currentGroup);
    emit commandsChanged();
    return true;
}

void QuickCmdManager::loadGroup(const QString &groupName)
{
    if (!m_quickCmdGroups.contains(groupName)) return;

    m_currentGroup = groupName;
    rebuildCommandsList();
    emit commandsChanged();
}

void QuickCmdManager::saveGroup(const QString &groupName)
{
    if (!m_quickCmdGroups.contains(groupName)) return;

    QString cmdDir = findCommandsDir();
    if (cmdDir.isEmpty()) return;

    // Find existing JSON file
    QDir dir(cmdDir);
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    QString filePath;
    for (const QFileInfo &fi : files) {
        QFile f(fi.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
            f.close();
            if (doc.object().value("group").toString() == groupName) {
                filePath = fi.absoluteFilePath();
                break;
            }
        }
    }

    if (filePath.isEmpty()) {
        filePath = cmdDir + "/" + groupName + ".json";
    }

    QJsonObject root;
    root["group"] = groupName;

    QJsonArray cmds;
    const QList<QuickCmdItem> &items = m_quickCmdGroups[groupName];
    for (const QuickCmdItem &item : items) {
        QJsonObject cmdObj;
        cmdObj["name"] = item.name;
        cmdObj["data"] = item.data;
        cmds.append(cmdObj);
    }
    root["commands"] = cmds;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void QuickCmdManager::sendSelectedCommands()
{
    m_sendTimer->stop();
    m_pendingSends.clear();
    m_sendIndex = 0;

    if (!m_quickCmdGroups.contains(m_currentGroup)) return;

    const QList<QuickCmdItem> &items = m_quickCmdGroups[m_currentGroup];
    for (const QuickCmdItem &item : items) {
        if (!item.data.isEmpty()) {
            m_pendingSends.append({item.name, item.data});
        }
    }

    if (m_pendingSends.isEmpty()) {
        emit errorOccurred(tr("No commands to send"));
        return;
    }

    m_sendTimer->setInterval(0); // immediate first send
    m_sendTimer->start();
}

void QuickCmdManager::stopSending()
{
    m_sendTimer->stop();
    m_pendingSends.clear();
    m_sendIndex = 0;
}

void QuickCmdManager::selectAll()
{
    // In QML, selection is handled differently; this is a placeholder
}

void QuickCmdManager::deselectAll()
{
    // In QML, selection is handled differently; this is a placeholder
}

void QuickCmdManager::onSendTimer()
{
    if (m_sendIndex >= m_pendingSends.size()) {
        m_sendTimer->stop();
        emit sendComplete();
        return;
    }

    const auto &item = m_pendingSends[m_sendIndex];
    emit sendData(item.second);

    m_sendIndex++;

    if (m_sendIndex < m_pendingSends.size()) {
        m_sendTimer->setInterval(m_sendDelay);
    }
}

QString QuickCmdManager::findCommandsDir()
{
    QString cmdDir = QCoreApplication::applicationDirPath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    cmdDir = QDir::currentPath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    cmdDir = dir.absolutePath() + "/commands";
    if (QDir(cmdDir).exists()) return cmdDir;

    return QString();
}

void QuickCmdManager::rebuildGroupsList()
{
    m_groupsList.clear();
    for (auto it = m_quickCmdGroups.constBegin(); it != m_quickCmdGroups.constEnd(); ++it) {
        m_groupsList.append(it.key());
    }
}

void QuickCmdManager::rebuildCommandsList()
{
    m_commandsList.clear();
    if (!m_quickCmdGroups.contains(m_currentGroup)) return;

    const QList<QuickCmdItem> &items = m_quickCmdGroups[m_currentGroup];
    for (int i = 0; i < items.size(); i++) {
        QVariantMap map;
        map["index"] = i;
        map["name"] = items[i].name;
        map["data"] = items[i].data;
        map["selected"] = true;
        m_commandsList.append(map);
    }
}
