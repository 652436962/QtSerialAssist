#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QTranslator>
#include <QCoreApplication>

class SettingsManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int themeIndex READ themeIndex WRITE setThemeIndex NOTIFY themeIndexChanged)
    Q_PROPERTY(int language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)
    Q_PROPERTY(int autoSendInterval READ autoSendInterval WRITE setAutoSendInterval NOTIFY autoSendIntervalChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override;

    int themeIndex() const;
    void setThemeIndex(int index);

    int language() const;
    void setLanguage(int lang);

    bool autoScroll() const;
    void setAutoScroll(bool enabled);

    int autoSendInterval() const;
    void setAutoSendInterval(int ms);

    Q_INVOKABLE void load();
    Q_INVOKABLE void save();

signals:
    void themeIndexChanged();
    void languageChanged();
    void autoScrollChanged();
    void autoSendIntervalChanged();

private:
    QSettings m_settings;
    QTranslator *m_translator = nullptr;
    int m_themeIndex = 0;
    int m_language = 0;
    bool m_autoScroll = true;
    int m_autoSendInterval = 1000;
};

#endif // SETTINGSMANAGER_H
