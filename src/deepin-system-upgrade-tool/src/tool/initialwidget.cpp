// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DPalette>
#include <DDialog>
#include <DFontSizeManager>
#include <DTextBrowser>

#include <QPalette>
#include <QFile>
#include <QtDebug>
#include <QGraphicsOpacityEffect>

#include <iostream>

#include "initialwidget.h"
#include "../core/dbusworker.h"
#include "../core/utils.h"
#include "../widgets/backgroundframe.h"

#define LOGO_W 248
#define LOGO_H 136

InitialWidget::InitialWidget(QWidget *parent)
    : BaseContainerWidget(parent)
    , m_checkedCount(0)
    , m_iconLabel(new IconLabel(this, Orientation::Vertical))
    , m_titleLabel(new DLabel(this))
    , m_currentVersionLabel(new SimpleLabel(this))
    , m_textLabel(new SimpleLabel(this))
    , m_userLicenseLayout(new QHBoxLayout(this))
    , m_userLicenseCheckBox(new QCheckBox(this))
    , m_userLicenseLabel(new DLabel(this))
    , m_privacyLicenseLabel(new DLabel(this))
{
    initUI();
    initConnections();
}

void InitialWidget::initUI()
{
    m_threeDotsWidget->setVisible(false);
    m_contentLayout->setContentsMargins(0, 20, 0, 0);
    if (isDarkMode())
        m_iconLabel->setIcon(QString::fromUtf8(":/icons/initial_widget_logo_dark.png"));
    else
        m_iconLabel->setIcon(QString::fromUtf8(":/icons/initial_widget_logo.png"));
    m_iconLabel->scale(LOGO_W, LOGO_H);

    addContentWidget(m_iconLabel);

    QFont titleFont("SourceHanSansSC", 18, QFont::DemiBold);

    m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_titleLabel->setText(tr("Deepin Upgrade"));
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setForegroundRole(DPalette::TextTitle);

    // iconLabel->addSpacing(12);
    m_iconLabel->addWidget(m_titleLabel);

    QFont curVerFont("SourceHanSansSC", 9, QFont::Normal);

    m_currentVersionLabel->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    m_currentVersionLabel->setText(tr("Current version: V%1").arg(DBusWorker::getInstance()->GetDistroVer()));
    m_currentVersionLabel->setFont(curVerFont);
    m_currentVersionLabel->setForegroundRole(DPalette::TextTips);
    // currentVersionLabel->setColor(QColor("#6b7f91"));
    // DFontSizeManager::instance()->bind(currentVersionLabel, DFontSizeManager::T7);

    m_iconLabel->addWidget(m_currentVersionLabel);

    m_contentLayout->addSpacing(30);

    QFont textFont("SourceHanSansSC", 10, QFont::Thin);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    if (isDarkMode())
        effect->setOpacity(1);
    else
        effect->setOpacity(0.7);
    m_textLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_textLabel->setText(tr("Take a few simple steps to upgrade your system to"));
    m_textLabel->setFont(textFont);
    m_textLabel->setGraphicsEffect(effect);
    m_textLabel->setForegroundRole(DPalette::BrightText);

    addContentWidget(m_textLabel);

    m_contentLayout->addSpacing(10);

    // TODO(Yutao Meng): 设置实际需要升级到的目标版本系统标签
    BackgroundFrame *targetVersionFrame = new BackgroundFrame(this, 18);
    QHBoxLayout *frameLayout = new QHBoxLayout;
    QHBoxLayout *targetVersionLayout = new QHBoxLayout;
    frameLayout->setContentsMargins(270, 0, 270, 0);
    targetVersionLayout->setContentsMargins(0,0,0,0);

    DLabel *targetVersionTextLabel = new DLabel(tr("Deepin 23"));

    textFont = QFont("SourceHanSansSC", 12, QFont::DemiBold);
    targetVersionTextLabel->setFont(textFont);
    targetVersionTextLabel->setForegroundRole(DPalette::TextTitle);
    targetVersionTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    targetVersionLayout->setContentsMargins(0, 4, 0, 6);
    targetVersionLayout->addWidget(targetVersionTextLabel);
    targetVersionFrame->setLayout(targetVersionLayout);
    frameLayout->addWidget(targetVersionFrame);
    m_contentLayout->addLayout(frameLayout);

    QFont licenseFont("SourceHanSansSC", 9, QFont::Normal);

    m_userLicenseLabel->setFont(licenseFont);
    m_userLicenseLabel->setText(tr("I have read and agreed to the <a href=\"user\" style=\"text-decoration: none;\">%1</a> and <a href=\"privacy\" style=\"text-decoration: none;\">%2</a>.")
                                .arg(tr("\"End User License Agreement for Deepin Upgrade Tool\""))
                                .arg(tr("\"Privacy Protection Guide\"")));
    m_userLicenseLayout->addSpacerItem(new QSpacerItem(238, 0, QSizePolicy::Maximum));
    m_userLicenseLayout->addWidget(m_userLicenseCheckBox, Qt::AlignCenter);
    m_userLicenseLayout->addWidget(m_userLicenseLabel, Qt::AlignCenter);
    m_userLicenseLayout->addSpacerItem(new QSpacerItem(240, 0, QSizePolicy::Maximum));

    m_bottomUpperLayout->addLayout(m_userLicenseLayout);
    m_bottomUpperLayout->addSpacing(12);

    m_midButton->setText(tr("Pre-Check"));
    QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy2.setHeightForWidth(m_midButton->sizePolicy().hasHeightForWidth());
    m_midButton->setSizePolicy(sizePolicy2);
    m_midButton->setVisible(true);
    m_midButton->setEnabled(false);
    m_midButton->setFixedWidth(250);

    m_spacerItem->changeSize(0, 60);
}

void InitialWidget::initConnections()
{
    connect(m_midButton, &QPushButton::clicked, this, &InitialWidget::upgradeCheckButtonClicked);
    connect(m_userLicenseCheckBox, &QCheckBox::stateChanged, this, &InitialWidget::enableUpgradeCheckButton);
    connect(m_userLicenseLabel, &DLabel::linkActivated, [this] (const QString &link) {
        if (link == "user")
            openLicenseDetails(LicenseType::USER);
        else
            openLicenseDetails(LicenseType::PRIVACY);
    });
    // 浅色/暗色主题切换
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType themeType) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
        QString logoPath;
        if (themeType == DGuiApplicationHelper::DarkType)
        {
            effect->setOpacity(1);
            logoPath = QString::fromUtf8(":/icons/initial_widget_logo_dark.png");
        }
        else
        {
            effect->setOpacity(0.7);
            logoPath = QString::fromUtf8(":/icons/initial_widget_logo.png");
        }
        m_textLabel->setGraphicsEffect(effect);
        m_iconLabel->setIcon(logoPath);
        m_iconLabel->scale(248, 136);
    });
}

void InitialWidget::enableUpgradeCheckButton(int state)
{
    if (state == Qt::Unchecked)
    {
        m_midButton->setEnabled(false);
    }
    else
    {
        m_midButton->setEnabled(true);
    }
}

void InitialWidget::openLicenseDetails(LicenseType type)
{
    QString licenseFileName;
    QString title;
    QCheckBox *checkBox = m_userLicenseCheckBox;

    switch (type) {
    case LicenseType::USER:
        title = tr("End User License Agreement for Deepin Upgrade Tool");
        licenseFileName = tr(":/text/end_user_license_en.txt");
        break;
    case LicenseType::PRIVACY:
        title = tr("Privacy Protection Guide for Deepin Upgrade Tool");
        licenseFileName = tr(":/text/privacy_protection_guide_en.txt");
        break;
    default:
        break;
    }

    QTextEdit* acknowledgementTextEdit = new QTextEdit(this);
    DDialog dlg(this);
    dlg.setFixedSize(MAIN_WINDOW_W, MAIN_WINDOW_H);

    DLabel *titleLabel = new DLabel(title);
    QFont textFont("SourceHanSansSC", 18, QFont::Medium);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(textFont);
    titleLabel->setForegroundRole(DPalette::TextTitle);
    dlg.addContent(titleLabel);

    dlg.addSpacing(10);

    // 设置协议正文字体和颜色
    QFile textFile(licenseFileName);
    acknowledgementTextEdit->setCurrentFont(QFont("SourceHanSansSC", 10, QFont::Normal));
    acknowledgementTextEdit->setForegroundRole(DPalette::BrightText);
    acknowledgementTextEdit->setBackgroundRole(DPalette::Window);
    if (!textFile.open(QIODevice::ReadOnly))
    {
        qWarning() << QString("Cannot find file:%1").arg(licenseFileName);
        return;
    }
//    // 设置可鼠标点击外部链接
//    acknowledgementTextEdit->setOpenExternalLinks(true);

    acknowledgementTextEdit->setText(textFile.readAll());
    textFile.close();
    // 消除边框，并应用对话框背景色
    acknowledgementTextEdit->setReadOnly(true);
    acknowledgementTextEdit->setStyleSheet("QTextEdit { border: 0; background: #00000000; }");
    // 设置更高的伸展权重，用以填充和标题之间的剩余空间
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setVerticalStretch(10);
    acknowledgementTextEdit->setSizePolicy(sizePolicy);
    QVBoxLayout *textWrapingLayout = new QVBoxLayout;
    QWidget *textWrapingWidget = new QWidget();
    textWrapingLayout->setContentsMargins(12, 6, 12, 6);
    textWrapingLayout->addWidget(acknowledgementTextEdit);
    textWrapingWidget->setLayout(textWrapingLayout);
    dlg.addContent(textWrapingWidget);

    QWidget *buttonsWidget = new QWidget();
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QSizePolicy fixedSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    DPushButton *disagreeButton = new DPushButton();
    DSuggestButton *agreeButton = new DSuggestButton();
    disagreeButton->setText(tr("Disagree"));
    disagreeButton->setSizePolicy(fixedSizePolicy);
    disagreeButton->setFixedSize(120, 36);
    agreeButton->setText(tr("Agree"));
    agreeButton->setSizePolicy(fixedSizePolicy);
    agreeButton->setFixedSize(120, 36);

    connect(disagreeButton, &DPushButton::clicked, &dlg, &DDialog::reject);
    connect(agreeButton, &DPushButton::clicked, &dlg, &DDialog::accept);

    buttonsLayout->addWidget(disagreeButton, 0, Qt::AlignRight);
    buttonsLayout->addWidget(agreeButton, 0, Qt::AlignLeft);
    buttonsLayout->setSpacing(10);

    buttonsWidget->setLayout(buttonsLayout);
    dlg.addContent(buttonsWidget);

    if (dlg.exec() == DDialog::Accepted)
    {
        checkBox->setCheckState(Qt::Checked);
    }
    else
    {
        checkBox->setCheckState(Qt::Unchecked);
    }
}
