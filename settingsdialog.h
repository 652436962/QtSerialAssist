#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    int currentTheme() const;
    int currentLanguage() const;

private:
    void setupUI();
    void applyStylesheet();

    QTabWidget *m_tabWidget;

    // 主题页
    QComboBox *m_themeCombo;
    QLabel *m_themePreview;

    // 语言页
    QComboBox *m_languageCombo;

    // 版本页
    QLabel *m_versionLabel;

    // 关于页
    QLabel *m_aboutLabel;
};

#endif // SETTINGSDIALOG_H
