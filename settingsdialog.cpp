#include "settingsdialog.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFont>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置");
    setMinimumSize(420, 360);
    setupUI();
    applyStylesheet();
}

void SettingsDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    m_tabWidget = new QTabWidget(this);

    // ====== 主题页 ======
    auto *themePage = new QWidget();
    auto *themeLayout = new QVBoxLayout(themePage);

    auto *themeGroup = new QGroupBox("主题设置");
    auto *themeForm = new QFormLayout(themeGroup);
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem("深色主题");
    m_themeCombo->addItem("浅色主题");
    themeForm->addRow("选择主题:", m_themeCombo);
    themeLayout->addWidget(themeGroup);

    m_themePreview = new QLabel("预览: 主窗口背景 #1a1a2e / 面板 #16213e / 强调 #e94560");
    m_themePreview->setWordWrap(true);
    m_themePreview->setMinimumHeight(50);
    m_themePreview->setAlignment(Qt::AlignCenter);
    themeLayout->addWidget(m_themePreview);
    themeLayout->addStretch();

    m_tabWidget->addTab(themePage, "主题");

    // ====== 语言页 ======
    auto *langPage = new QWidget();
    auto *langLayout = new QVBoxLayout(langPage);

    auto *langGroup = new QGroupBox("语言设置");
    auto *langForm = new QFormLayout(langGroup);
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem("中文");
    m_languageCombo->addItem("English");
    langForm->addRow("选择语言:", m_languageCombo);

    auto *langNote = new QLabel("切换语言后需要重启应用生效");
    langNote->setWordWrap(true);
    langForm->addRow(langNote);
    langLayout->addWidget(langGroup);
    langLayout->addStretch();

    m_tabWidget->addTab(langPage, "语言");

    // ====== 版本页 ======
    auto *versionPage = new QWidget();
    auto *versionLayout = new QVBoxLayout(versionPage);

    auto *versionGroup = new QGroupBox("版本信息");
    auto *versionForm = new QFormLayout(versionGroup);

    m_versionLabel = new QLabel();
    m_versionLabel->setText(
        "版本号: 1.0.0\n"
        "Qt 版本: " QT_VERSION_STR "\n"
        "构建日期: " __DATE__ " " __TIME__ "\n"
        "平台: " +
        QString(QSysInfo::prettyProductName())
    );
    m_versionLabel->setWordWrap(true);
    versionForm->addRow(m_versionLabel);
    versionLayout->addWidget(versionGroup);
    versionLayout->addStretch();

    m_tabWidget->addTab(versionPage, "版本");

    // ====== 关于页 ======
    auto *aboutPage = new QWidget();
    auto *aboutLayout = new QVBoxLayout(aboutPage);

    auto *aboutGroup = new QGroupBox("关于 QtSerialAssist");
    auto *aboutInner = new QVBoxLayout(aboutGroup);

    auto *titleLabel = new QLabel("QtSerialAssist - 串口调试助手");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    aboutInner->addWidget(titleLabel);

    m_aboutLabel = new QLabel();
    m_aboutLabel->setText(
        "跨平台串口/网络调试助手\n\n"
        "支持功能:\n"
        "  • 串口通信 (RS232/RS485)\n"
        "  • TCP 服务器/客户端\n"
        "  • UDP 通信\n"
        "  • Modbus RTU/ASCII/TCP\n"
        "  • 快捷指令批量发送\n"
        "  • HEX/ASCII 数据收发\n\n"
        "© 2025 QtSerialAssist"
    );
    m_aboutLabel->setWordWrap(true);
    m_aboutLabel->setAlignment(Qt::AlignCenter);
    aboutInner->addWidget(m_aboutLabel);
    aboutLayout->addWidget(aboutGroup);
    aboutLayout->addStretch();

    m_tabWidget->addTab(aboutPage, "关于");

    mainLayout->addWidget(m_tabWidget);

    // 底部按钮
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto *cancelBtn = new QPushButton("取消");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);
    auto *okBtn = new QPushButton("确定");
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);
}

void SettingsDialog::applyStylesheet()
{
    setStyleSheet(R"(
        QDialog { background-color: #1a1a2e; }
        QTabWidget::pane { border: 1px solid #2a2a4a; background-color: #16213e; }
        QTabBar::tab { background-color: #16213e; color: #a0a0a0; padding: 6px 16px; border: 1px solid #2a2a4a; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; }
        QTabBar::tab:selected { background-color: #0f3460; color: #eaeaea; }
        QTabBar::tab:hover { background-color: #0f3460; }
        QLabel { color: #eaeaea; background: transparent; }
        QGroupBox { color: #eaeaea; border: 1px solid #2a2a4a; border-radius: 6px; margin-top: 12px; padding-top: 20px; background-color: #16213e; }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; color: #e94560; }
        QComboBox { background-color: #1a1a2e; color: #eaeaea; border: 1px solid #2a2a4a; padding: 6px; border-radius: 4px; min-width: 180px; }
        QComboBox:hover { border: 1px solid #e94560; }
        QComboBox::drop-down { border: none; }
        QPushButton { background-color: #0f3460; color: #eaeaea; border: 1px solid #2a2a4a; padding: 6px 20px; border-radius: 4px; }
        QPushButton:hover { background-color: #16213e; border: 1px solid #e94560; }
        QPushButton:pressed { background-color: #e94560; }
    )");
}

int SettingsDialog::currentTheme() const
{
    return m_themeCombo->currentIndex();
}

int SettingsDialog::currentLanguage() const
{
    return m_languageCombo->currentIndex();
}
