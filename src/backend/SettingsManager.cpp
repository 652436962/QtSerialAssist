#include "SettingsManager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("QtSerialAssist", "QtSerialAssist")
    , m_translator(new QTranslator(this))
{
    load();
}

SettingsManager::~SettingsManager() = default;

int SettingsManager::themeIndex() const { return m_themeIndex; }

void SettingsManager::setThemeIndex(int index)
{
    if (m_themeIndex == index) return;
    m_themeIndex = index;
    emit themeIndexChanged();
}

int SettingsManager::language() const { return m_language; }

void SettingsManager::setLanguage(int lang)
{
    if (m_language == lang) return;
    m_language = lang;
    if (lang == 1) {
        if (m_translator->load("QtSerialAssist_zh_CN.qm",
                               QCoreApplication::applicationDirPath())) {
            qApp->installTranslator(m_translator);
        }
    } else {
        if (!m_translator->isEmpty()) {
            qApp->removeTranslator(m_translator);
        }
    }
    emit languageChanged();
}

bool SettingsManager::autoScroll() const { return m_autoScroll; }

void SettingsManager::setAutoScroll(bool enabled)
{
    if (m_autoScroll == enabled) return;
    m_autoScroll = enabled;
    emit autoScrollChanged();
}

int SettingsManager::autoSendInterval() const { return m_autoSendInterval; }

void SettingsManager::setAutoSendInterval(int ms)
{
    if (m_autoSendInterval == ms) return;
    m_autoSendInterval = ms;
    emit autoSendIntervalChanged();
}

void SettingsManager::load()
{
    int theme = m_settings.value("theme", 0).toInt();
    if (theme != m_themeIndex) {
        m_themeIndex = theme;
        emit themeIndexChanged();
    }

    int lang = m_settings.value("language", 0).toInt();
    if (lang != m_language) {
        setLanguage(lang);
    }

    m_autoScroll = m_settings.value("autoScroll", true).toBool();
    m_autoSendInterval = m_settings.value("autoSendInterval", 1000).toInt();
}

void SettingsManager::save()
{
    m_settings.setValue("theme", m_themeIndex);
    m_settings.setValue("language", m_language);
    m_settings.setValue("autoScroll", m_autoScroll);
    m_settings.setValue("autoSendInterval", m_autoSendInterval);
}
