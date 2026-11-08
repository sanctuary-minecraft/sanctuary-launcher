// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryHome.h"
#include "SanctuaryFeed.h"
#include "SanctuarySession.h"
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStyle>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <array>

static void initializeSanctuaryResources() { Q_INIT_RESOURCE(sanctuary); }
namespace {
QLabel* label(QWidget* parent, const QString& name)
{
    auto* result = new QLabel(parent);
    result->setObjectName(name);
    result->setWordWrap(true);
    result->setTextFormat(Qt::PlainText);
    return result;
}
QPushButton* button(QWidget* parent, const QString& name)
{
    auto* result = new QPushButton(parent);
    result->setObjectName(name);
    result->setCursor(Qt::PointingHandCursor);
    result->setMinimumHeight(34);
    return result;
}
void setRole(QWidget* widget, const char* name, const QVariant& value)
{
    widget->setProperty(name, value);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
const QStringList scenarios{"start", "microsoft", "apply", "pending", "pending-ready", "sync", "accepted", "ready", "rejected", "revoked"};
}

struct SanctuaryHome::Ui {
    struct Row { QFrame* frame; QLabel *number, *title, *detail, *status; QPushButton* action; };
    std::array<Row, 5> rows;
    QLabel *eyebrow, *heading, *subtitle, *server, *notice, *historical, *help, *footer, *code, *codeHelp;
    QWidget *previewControls, *codePanel, *setupPanel, *community, *readyCodePanel, *toolsPanel;
    QFrame *chromeHeader, *chromeFooter;
    QList<SanctuaryFeed*> feeds;
    QLabel* readyCode;
    QPushButton *readyCodeToggle, *readyCodeCopy;
    QComboBox* scenarios;
    QPushButton *discordConnect, *discordRefresh;
    QStackedWidget* body;
    QLabel *packSummary, *feedback;
    QPushButton* setupToggle;
    QPushButton *reset, *play, *mods, *shaders, *instanceSettings, *tools, *settings, *news, *site, *wiki, *copy, *sponsor;
    QProgressBar* progress;
};

SanctuaryHome::SanctuaryHome(QWidget* parent) : QWidget(parent), m_ui(std::make_unique<Ui>())
{
    initializeSanctuaryResources();
    setObjectName("sanctuaryHome");
    setStyleSheet(R"(
        QWidget#sanctuaryHome, #sanctuaryHome QWidget#surface { background: #fff; color: #1f2328; }
        #sanctuaryHome { font-family: "SF Pro Text", "Segoe UI", sans-serif; font-size: 14px; }
        #sanctuaryHome QLabel { color: #1f2328; border: none; background: transparent; }
        #sanctuaryHome QLabel#heading { font-size: 21px; font-weight: 700; }
        #sanctuaryHome QLabel#eyebrow { font-size: 14px; font-weight: 600; color: #57606a; }
        #sanctuaryHome QLabel#subtitle { font-size: 13px; color: #57606a; }
        #sanctuaryHome QLabel[muted="true"] { color: #57606a; font-size: 12px; }
        #sanctuaryHome QFrame#header { border: none; border-bottom: 1px solid #d0d7de; }
        #sanctuaryHome QFrame#footer { border: none; border-top: 1px solid #d0d7de; }
        #sanctuaryHome QFrame#steps { border: 1px solid #d0d7de; border-radius: 6px; background: white; }
        #sanctuaryHome QFrame[stepRow="true"] { border: none; border-bottom: 1px solid #d8dee4; background: transparent; }
        #sanctuaryHome QFrame[active="true"] { background: #f6f8fa; }
        #sanctuaryHome QFrame[waiting="true"] { background: #fff8e5; }
        #sanctuaryHome QLabel[rowTitle="true"] { font-size: 14px; font-weight: 600; }
        #sanctuaryHome QLabel[number="true"] { border: 1px solid #8c959f; border-radius: 14px; color: #57606a; font-size: 14px; font-weight: 600; }
        #sanctuaryHome QLabel[number="true"][kind="complete"] { border-color: #1a7f37; color: #1a7f37; }
        #sanctuaryHome QLabel[number="true"][kind="current"] { background: #1a7f37; border-color: #1a7f37; color: white; }
        #sanctuaryHome QLabel[number="true"][kind="waiting"] { border-color: #9a6700; color: #9a6700; }
        #sanctuaryHome QLabel[badge="true"] { padding: 4px 8px; border: 1px solid #d0d7de; border-radius: 4px; font-size: 12px; }
        #sanctuaryHome QLabel[badge="true"][kind="complete"] { background: #dafbe1; border-color: #dafbe1; color: #1a7f37; }
        #sanctuaryHome QLabel[badge="true"][kind="waiting"] { background: #fff1c2; border-color: #fff1c2; color: #7d4e00; }
        #sanctuaryHome QLabel[badge="true"][kind="error"] { background: #ffebe9; border-color: #ffebe9; color: #a40e26; }
        #sanctuaryHome QPushButton { color: #24292f; background: #f6f8fa; border: 1px solid #d0d7de; border-radius: 5px; padding: 5px 14px; font-size: 13px; font-weight: 500; }
        #sanctuaryHome QPushButton:hover { background: #eef1f4; border-color: #8c959f; }
        #sanctuaryHome QPushButton:focus { border: 2px solid #0969da; }
        #sanctuaryHome QPushButton[primary="true"] { background: #1f883d; color: white; border-color: #1a7f37; font-weight: 600; }
        #sanctuaryHome QPushButton[primary="true"]:hover { background: #1a7f37; }
        #sanctuaryHome QPushButton:disabled { background: #eaeef2; color: #6e7781; border-color: #eaeef2; }
        #sanctuaryHome QPushButton[link="true"] { color: #0969da; background: transparent; border: none; padding: 3px; font-weight: 400; text-align: left; }
        #sanctuaryHome QPushButton[link="true"]:focus { border: 1px solid #0969da; }
        #sanctuaryHome QFrame[communityCard="true"] { border: 1px solid #d0d7de; border-radius: 5px; }
        #sanctuaryHome QComboBox { background: white; color: #24292f; border: 1px solid #d0d7de; border-radius: 4px; padding: 5px 10px; min-height: 22px; }
        #sanctuaryHome QProgressBar { border: 1px solid #d0d7de; border-radius: 3px; background: white; color: #24292f; text-align: center; }
        #sanctuaryHome QProgressBar::chunk { background: #dafbe1; }
    )");
    buildUi();
    m_installTimer = new QTimer(this);
    m_installTimer->setInterval(150);
    connect(m_installTimer, &QTimer::timeout, this, [this] {
        m_progress = qMin(100, m_progress + 5);
        if (m_progress == 100) {
            m_installTimer->stop();
            m_local.installed = true;
            commitLocalChange();
        } else refresh();
    });
    refresh();
}
SanctuaryHome::~SanctuaryHome() = default;
QString SanctuaryHome::text(const char* fr, const char* en) const { return QString::fromUtf8(m_french ? fr : en); }

void SanctuaryHome::setPackState(bool verified, bool busy, const QString& version, const QString& message, int percent)
{
    m_installTimer->stop();
    m_realPack = true;
    m_packVerified = verified;
    m_packBusy = busy;
    m_packVersion = version;
    m_packMessage = message;
    m_packProgress = percent;
    refresh();
}

void SanctuaryHome::showFeedback(const QString& message)
{
    m_ui->feedback->setText(message);
    m_ui->feedback->setToolTip(message);
    m_ui->feedback->setVisible(!message.isEmpty());
}

void SanctuaryHome::setEmbedded(bool embedded)
{
    m_embedded = embedded;
    m_ui->chromeHeader->setVisible(!embedded);
    m_ui->chromeFooter->setVisible(true);
    refresh();
}

void SanctuaryHome::setDiscordState(bool connected, bool busy, const QJsonObject& profile, const QString& message)
{
    m_onlineSession = true;
    m_discordConnected = connected;
    m_discordBusy = busy;
    m_discordProfile = SanctuarySession::validProfile(profile) ? profile : QJsonObject{};
    m_discordMessage = message;
    m_codeVisible = false;
    refresh();
}
void SanctuaryHome::setDiscordSession(SanctuarySession* session)
{
    for (auto* feed : m_ui->feeds) feed->setSession(session);
    auto update = [this, session] { setDiscordState(session->connected(), session->busy(), session->profile(), session->message()); };
    connect(session, &SanctuarySession::changed, this, update);
    update();
}

bool SanctuaryHome::communityUnlocked() const
{
    if (m_onlineSession) return m_discordConnected && m_discordProfile.value("access").toObject().value("status") == "granted";
    const bool installed = m_realPack ? m_packVerified : m_local.installed;
    return m_localMode && m_local.discord && m_local.microsoft && m_local.access == Access::Granted && installed && !m_packBusy;
}

void SanctuaryHome::openedImpl()
{
    if (communityUnlocked()) for (auto* feed : m_ui->feeds) feed->reload();
}

void SanctuaryHome::buildUi()
{
    auto& u = *m_ui;
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* surface = new QWidget(this);
    surface->setObjectName("surface");
    auto* horizontal = new QHBoxLayout(surface);
    horizontal->setContentsMargins(12, 0, 12, 0);
    horizontal->addStretch();
    auto* column = new QWidget(surface);
    column->setMaximumWidth(960);
    auto* page = new QVBoxLayout(column);
    page->setContentsMargins(0, 0, 0, 0);
    page->setSpacing(8);
    horizontal->addWidget(column, 1);
    horizontal->addStretch();
    outer->addWidget(surface);

    u.chromeHeader = new QFrame(column);
    u.chromeHeader->setObjectName("header");
    auto* brand = new QHBoxLayout(u.chromeHeader);
    brand->setContentsMargins(0, 0, 0, 6);
    auto* wordmark = label(u.chromeHeader, "wordmark");
    wordmark->setPixmap(QPixmap(":/sanctuary/wordmark.png").scaledToWidth(180, Qt::SmoothTransformation));
    wordmark->setAccessibleName("Sanctuary");
    brand->addWidget(wordmark);
    brand->addStretch();
    u.server = label(u.chromeHeader, "serverStatus");
    u.server->setProperty("muted", true);
    brand->addWidget(u.server);
    page->addWidget(u.chromeHeader);

    u.previewControls = new QWidget(column);
    auto* preview = new QHBoxLayout(u.previewControls);
    preview->setContentsMargins(0, 0, 0, 0);
    u.notice = label(u.previewControls, "previewNotice");
    u.notice->setProperty("muted", true);
    preview->addWidget(u.notice, 1);
    u.scenarios = new QComboBox(u.previewControls);
    u.scenarios->setObjectName("scenarios");
    u.scenarios->addItem("Parcours courant", QString());
    for (const auto& scenario : scenarios) u.scenarios->addItem(scenario, scenario);
    connect(u.scenarios, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        const auto scenario = m_ui->scenarios->itemData(index).toString();
        if (!scenario.isEmpty()) setLocalScenario(scenario);
    });
    preview->addWidget(u.scenarios);
    u.reset = button(u.previewControls, "resetPreview");
    preview->addWidget(u.reset);
    u.discordConnect = button(u.previewControls, "discordSessionConnect");
    u.discordRefresh = button(u.previewControls, "discordSessionRefresh");
    preview->addWidget(u.discordConnect);
    preview->addWidget(u.discordRefresh);
    connect(u.discordConnect, &QPushButton::clicked, this, [this] {
        if (m_discordConnected || m_discordBusy) emit discordDisconnectRequested();
        else emit discordRequested();
    });
    connect(u.discordRefresh, &QPushButton::clicked, this, &SanctuaryHome::discordRefreshRequested);
    connect(u.reset, &QPushButton::clicked, this, [this] { setLocalScenario("start"); });

    auto* header = new QFrame(column);
    header->setObjectName("homeHeader");
    auto* heading = new QVBoxLayout(header);
    heading->setContentsMargins(0, 0, 0, 4);
    heading->setSpacing(2);
    u.eyebrow = label(header, "eyebrow");
    u.heading = label(header, "heading");
    u.subtitle = label(header, "subtitle");
    heading->addWidget(u.eyebrow);
    heading->addWidget(u.heading);
    heading->addWidget(u.subtitle);
    u.feedback = label(header, "feedback");
    u.feedback->setProperty("muted", true);
    u.feedback->setMaximumHeight(u.feedback->fontMetrics().lineSpacing() * 2);
    u.feedback->hide();
    heading->addWidget(u.feedback);
    page->addWidget(header);
    page->addWidget(u.previewControls);

    auto* body = new QStackedWidget(column);
    u.body = body;
    body->setObjectName("homeBody");
    page->addWidget(body, 1);
    u.setupPanel = new QWidget(body);
    u.setupPanel->setObjectName("inlineAccounts");
    auto* details = new QVBoxLayout(u.setupPanel);
    details->setContentsMargins(0, 0, 0, 0);
    details->setSpacing(0);
    auto* steps = new QFrame(u.setupPanel);
    steps->setObjectName("steps");
    auto* rows = new QVBoxLayout(steps);
    rows->setContentsMargins(1, 1, 1, 1);
    rows->setSpacing(0);
    for (int i = 0; i < 5; ++i) {
        auto& r = u.rows[i];
        r.frame = new QFrame(steps);
        r.frame->setObjectName(QString("step%1").arg(i + 1));
        r.frame->setProperty("stepRow", true);
        auto* line = new QGridLayout(r.frame);
        line->setContentsMargins(12, 5, 12, 5);
        line->setHorizontalSpacing(12);
        line->setVerticalSpacing(2);
        r.number = label(r.frame, QString("number%1").arg(i + 1));
        r.number->setText(QString::number(i + 1));
        r.number->setProperty("number", true);
        r.number->setFixedSize(28, 28);
        r.number->setAlignment(Qt::AlignCenter);
        line->addWidget(r.number, 0, 0, 2, 1, Qt::AlignVCenter);
        r.title = label(r.frame, QString("title%1").arg(i + 1));
        r.title->setProperty("rowTitle", true);
        r.detail = label(r.frame, QString("detail%1").arg(i + 1));
        r.detail->setProperty("muted", true);
        line->addWidget(r.title, 0, 1);
        line->addWidget(r.detail, 1, 1);
        line->setColumnStretch(1, 1);
        r.status = label(r.frame, QString("status%1").arg(i + 1));
        r.status->setProperty("badge", true);
        r.status->setAlignment(Qt::AlignCenter);
        r.action = button(r.frame, QStringList{"discord", "microsoft", "access", "codeToggle", "install"}[i]);
        r.action->setMinimumHeight(28);
        line->addWidget(r.status, 0, 2, 2, 1, Qt::AlignVCenter);
        line->addWidget(r.action, 0, 2, 2, 1, Qt::AlignVCenter);
        if (i == 2) {
            u.historical = label(r.frame, "historicalBadge");
            u.historical->setProperty("badge", true);
            u.historical->setWordWrap(false);
            line->addWidget(u.historical, 2, 1, 1, 2, Qt::AlignLeft);
            u.sponsor = button(r.frame, "sponsor");
            u.sponsor->setProperty("link", true);
            u.sponsor->setMinimumHeight(24);
            line->addWidget(u.sponsor, 3, 1);
        }
        if (i == 3) {
            u.codePanel = new QWidget(r.frame);
            auto* codeLayout = new QHBoxLayout(u.codePanel);
            codeLayout->setContentsMargins(0, 2, 0, 0);
            u.code = label(u.codePanel, "codeValue");
            const int fontId = QFontDatabase::addApplicationFont(":/sanctuary/Sga-Regular.ttf");
            const auto families = QFontDatabase::applicationFontFamilies(fontId);
            if (!families.isEmpty()) {
                QFont font(families.first(), 20);
                font.setLetterSpacing(QFont::AbsoluteSpacing, 4);
                u.code->setFont(font);
            }
            codeLayout->addWidget(u.code);
            u.copy = button(u.codePanel, "copyCode");
            codeLayout->addWidget(u.copy);
            codeLayout->addStretch();
            line->addWidget(u.codePanel, 2, 1, 1, 2);
            u.codeHelp = label(r.frame, "codeHelp");
            u.codeHelp->setProperty("muted", true);
            line->addWidget(u.codeHelp, 3, 1, 1, 2);
        }
        rows->addWidget(r.frame);
    }
    details->addWidget(steps);
    details->addStretch();
    body->addWidget(u.setupPanel);

    u.community = new QWidget(body);
    u.community->setObjectName("communityHome");
    auto* community = new QGridLayout(u.community);
    community->setContentsMargins(0, 0, 0, 0);
    community->setSpacing(10);
    struct FeedSection { const char* kind; const char* fr; const char* en; int row; int column; int columns; };
    const FeedSection sections[] = {{"bulletin","Intendance","Stewardship",0,0,2},
        {"article","Gazette","Gazette",1,0,1}, {"notice","Tableau d'affichage","Notice board",1,1,1}};
    for (const auto& section : sections) {
        auto* card = new QFrame(u.community);
        card->setObjectName("communityCard-" + QString(section.kind));
        card->setProperty("communityCard", true);
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);
        cardLayout->setSpacing(5);
        auto* title = label(card, "communityTitle-" + QString(section.kind));
        title->setProperty("labelFr", section.fr);
        title->setProperty("labelEn", section.en);
        title->setStyleSheet("font-size: 16px; font-weight: 600;");
        cardLayout->addWidget(title);
        auto* feed = new SanctuaryFeed(section.kind, card);
        u.feeds.append(feed);
        cardLayout->addWidget(feed, 1);
        community->addWidget(card, section.row, section.column, 1, section.columns);
        connect(feed, &SanctuaryFeed::linkRequested, this, &SanctuaryHome::linkRequested);
    }
    community->setColumnStretch(0, 1);
    community->setColumnStretch(1, 1);
    community->setRowStretch(0, 1);
    community->setRowStretch(1, 2);
    body->addWidget(u.community);

    // Standalone widget tools are hidden when the persistent native sidebar is present.
    u.toolsPanel = new QWidget(column);
    auto* shortcuts = new QHBoxLayout(u.toolsPanel);
    shortcuts->setContentsMargins(0, 0, 0, 0);
    u.mods = button(u.toolsPanel, "mods");
    u.shaders = button(u.toolsPanel, "shaders");
    u.instanceSettings = button(u.toolsPanel, "instanceSettings");
    u.tools = button(u.toolsPanel, "advanced");
    for (auto* entry : {u.mods, u.shaders, u.instanceSettings, u.tools}) shortcuts->addWidget(entry);
    page->addWidget(u.toolsPanel);

    auto* footer = new QFrame(column);
    footer->setObjectName("footer");
    u.chromeFooter = footer;
    auto* footerLayout = new QVBoxLayout(footer);
    footerLayout->setContentsMargins(0, 8, 0, 0);
    footerLayout->setSpacing(4);
    u.progress = new QProgressBar(footer);
    u.progress->setObjectName("installationProgress");
    footerLayout->addWidget(u.progress);
    auto* foot = new QHBoxLayout;
    auto* info = new QVBoxLayout;
    info->setSpacing(2);
    auto* packLine = new QHBoxLayout;
    auto* packHeading = label(footer, "packHeading");
    packHeading->setText("Sanctuary Beta");
    packHeading->setWordWrap(false);
    packHeading->setStyleSheet("font-weight:600;");
    packLine->addWidget(packHeading);
    u.packSummary = label(footer, "packSummary");
    u.packSummary->setProperty("muted", true);
    u.packSummary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    u.packSummary->setMaximumHeight(u.packSummary->fontMetrics().lineSpacing() * 2);
    packLine->addWidget(u.packSummary, 1);
    info->addLayout(packLine);
    auto* links = new QHBoxLayout;
    u.news = button(footer, "news");
    u.site = button(footer, "site");
    u.wiki = button(footer, "help");
    u.settings = button(footer, "settings");
    u.setupToggle = button(footer, "setupToggle");
    u.readyCodeToggle = button(footer, "readyCodeToggle");
    for (auto* link : {u.news, u.site, u.wiki, u.settings, u.setupToggle, u.readyCodeToggle}) {
        link->setProperty("link", true);
        link->setMinimumHeight(22);
        links->addWidget(link);
    }
    links->addStretch();
    info->addLayout(links);
    foot->addLayout(info, 1);
    foot->addWidget(u.rows[4].action, 0, Qt::AlignBottom);
    u.play = button(footer, "play");
    u.play->setMinimumSize(120, 38);
    u.play->setProperty("primary", true);
    foot->addWidget(u.play, 0, Qt::AlignRight | Qt::AlignBottom);
    u.help = label(footer, "helpText");
    u.help->setProperty("muted", true);
    info->addWidget(u.help);
    u.footer = label(footer, "footerVersion");
    u.footer->setProperty("muted", true);
    info->addWidget(u.footer);
    footerLayout->addLayout(foot);
    page->addWidget(footer);

    // Optional code appears in a compact panel above the footer, never as a prerequisite.
    u.readyCodePanel = new QWidget(column);
    auto* readyCodeLayout = new QHBoxLayout(u.readyCodePanel);
    readyCodeLayout->setContentsMargins(0, 0, 0, 0);
    u.readyCode = label(u.readyCodePanel, "readyCodeValue");
    u.readyCode->setFont(u.code->font());
    readyCodeLayout->addWidget(u.readyCode);
    u.readyCodeCopy = button(u.readyCodePanel, "readyCodeCopy");
    readyCodeLayout->addWidget(u.readyCodeCopy);
    readyCodeLayout->addStretch();
    page->insertWidget(page->count() - 1, u.readyCodePanel);
    connect(u.setupToggle, &QPushButton::clicked, this, &SanctuaryHome::accountsRequested);
    connect(u.readyCodeToggle, &QPushButton::clicked, this, [this] {
        if (m_onlineSession) emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
        else { m_codeVisible = !m_codeVisible; refresh(); }
    });
    connect(u.readyCodeCopy, &QPushButton::clicked, u.copy, &QPushButton::click);
    connect(u.rows[0].action, &QPushButton::clicked, this, [this] {
        if (m_onlineSession) emit discordRequested();
        else if (m_localMode) { m_local.discord = true; commitLocalChange(); }
        else emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
    });
    connect(u.rows[1].action, &QPushButton::clicked, this, [this] {
        if (m_onlineSession) return;
        if (m_localMode) { m_local.microsoft = true; commitLocalChange(); }
        else if (m_microsoftAvailable) emit accountsRequested();
    });
    connect(u.rows[2].action, &QPushButton::clicked, this, [this] {
        if (m_onlineSession) emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
        else if (m_localMode && m_local.access == Access::None) { m_local.access = Access::Pending; commitLocalChange(); }
        else if (!m_localMode) emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
    });
    connect(u.rows[3].action, &QPushButton::clicked, this, [this] {
        if (m_onlineSession) emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
        else if (m_localMode && m_local.access == Access::Granted) { m_codeVisible = !m_codeVisible; refresh(); }
    });
    connect(u.copy, &QPushButton::clicked, this, [this] {
        if (!m_onlineSession && m_localMode && m_local.access == Access::Granted && m_codeVisible)
            QApplication::clipboard()->setText("DEMO-SANCTUARY");
    });
    connect(u.rows[4].action, &QPushButton::clicked, this, &SanctuaryHome::install);
    connect(u.play, &QPushButton::clicked, this, [this] {
        if (!m_onlineSession && m_localMode && m_local.discord && m_local.microsoft && m_local.access == Access::Granted && (m_realPack ? m_packVerified : m_local.installed))
            emit localPlayRequested();
    });
    connect(u.tools, &QPushButton::clicked, this, &SanctuaryHome::advancedRequested);
    connect(u.settings, &QPushButton::clicked, this, &SanctuaryHome::settingsRequested);
    connect(u.mods, &QPushButton::clicked, this, [this] { emit instanceToolRequested("mods"); });
    connect(u.shaders, &QPushButton::clicked, this, [this] { emit instanceToolRequested("shaderpacks"); });
    connect(u.instanceSettings, &QPushButton::clicked, this, [this] { emit instanceToolRequested("settings"); });
    connect(u.news, &QPushButton::clicked, this, [this] { emit instanceToolRequested("news"); });
    connect(u.site, &QPushButton::clicked, this, [this] { emit linkRequested(QUrl("https://sanctuary-minecraft.net/")); });
    connect(u.wiki, &QPushButton::clicked, this, [this] { emit linkRequested(QUrl("https://sanctuary-minecraft.net/wiki")); });
    connect(u.sponsor, &QPushButton::clicked, this, [this] {
        if (m_localMode) { m_local.access = Access::Granted; commitLocalChange(); }
        else emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
    });
}

void SanctuaryHome::setState(bool ready, bool microsoft, bool french)
{
    m_instanceReady = ready;
    m_microsoftAvailable = microsoft;
    m_french = french;
    refresh();
}
void SanctuaryHome::setLocalMode(bool enabled, const QJsonObject& saved)
{
    m_installTimer->stop();
    m_localMode = enabled;
    m_local = {};
    m_progress = 0;
    m_codeVisible = false;
    if (enabled && saved.value("version").toInt() == 1) {
        m_local.discord = saved.value("discord").toBool();
        m_local.microsoft = saved.value("microsoft").toBool();
        m_local.installed = saved.value("installed").toBool();
        m_local.historical = saved.value("historical").toBool();
        const int access = saved.value("access").toInt();
        if (m_local.discord && m_local.microsoft && access >= int(Access::None) && access <= int(Access::Revoked))
            m_local.access = Access(access);
    }
    refresh();
}
QJsonObject SanctuaryHome::localState() const
{
    return {{"version", 1}, {"discord", m_local.discord}, {"microsoft", m_local.microsoft},
            {"access", int(m_local.access)}, {"installed", m_local.installed}, {"historical", m_local.historical}};
}
void SanctuaryHome::commitLocalChange() { refresh(); emit localStateChanged(localState()); }
void SanctuaryHome::setLocalScenario(const QString& scenario)
{
    if (!m_localMode || !scenarios.contains(scenario)) return;
    const bool preparedWhileWaiting = m_local.installed;
    m_installTimer->stop();
    m_local = {};
    m_codeVisible = false;
    m_progress = 0;
    m_local.discord = scenario != "start";
    m_local.microsoft = scenario != "start" && scenario != "microsoft";
    if (scenario == "pending" || scenario == "pending-ready") m_local.access = Access::Pending;
    if (scenario == "sync") m_local.access = Access::Synchronizing;
    if (scenario == "accepted" || scenario == "ready") m_local.access = Access::Granted;
    if (scenario == "rejected") m_local.access = Access::Rejected;
    if (scenario == "revoked") m_local.access = Access::Revoked;
    m_local.installed = QStringList{"pending-ready", "sync", "ready", "rejected", "revoked"}.contains(scenario);
    if (scenario == "accepted") m_local.installed = preparedWhileWaiting;
    m_local.historical = scenario == "accepted" || scenario == "ready";
    commitLocalChange();
}
int SanctuaryHome::activeStep() const
{
    if (m_onlineSession) return !m_discordConnected ? 1 : m_discordProfile.value("access").toObject().value("status") == "granted" ? 2 : 3;
    if (!m_localMode || !m_local.discord) return 1;
    if (!m_local.microsoft) return 2;
    if (m_local.access != Access::Granted) return 3;
    return m_codeVisible ? 4 : 5;
}
void SanctuaryHome::install()
{
    if (m_realPack || !m_localMode) { emit installRequested(); return; }
    if (m_installTimer->isActive() || m_local.installed) return;
    m_progress = 0;
    m_installTimer->start();
    refresh();
}
void SanctuaryHome::refresh()
{
    auto& u = *m_ui;
    const auto remoteStatus = m_discordProfile.value("access").toObject().value("status").toString();
    const QHash<QString, Access> accessStates{{"none", Access::None}, {"pending", Access::Pending}, {"granted", Access::Granted},
        {"synchronizing", Access::Synchronizing}, {"rejected", Access::Rejected}, {"revoked", Access::Revoked}};
    const auto access = m_onlineSession ? accessStates.value(remoteStatus, Access::None) : m_local.access;
    const bool discord = m_onlineSession ? m_discordConnected : m_localMode && m_local.discord;
    const bool microsoft = !m_onlineSession && m_localMode && m_local.microsoft;
    const bool admitted = m_onlineSession ? discord && access == Access::Granted : m_localMode && access == Access::Granted;
    const bool installed = m_realPack ? m_packVerified : m_localMode && m_local.installed;
    const bool installing = m_realPack ? m_packBusy : m_installTimer && m_installTimer->isActive();
    const int active = activeStep();
    const bool waiting = (m_onlineSession || m_localMode) && (access == Access::Pending || access == Access::Synchronizing);
    u.previewControls->setVisible(m_localMode || m_onlineSession);
    u.scenarios->setVisible(m_localMode && !m_onlineSession);
    u.reset->setVisible(m_localMode && !m_onlineSession);
    u.discordConnect->setVisible(m_onlineSession);
    u.discordRefresh->setVisible(m_onlineSession && m_discordConnected);
    u.discordRefresh->setEnabled(!m_discordBusy);
    u.discordRefresh->setText(text("Actualiser mon accès", "Refresh my access"));
    u.discordConnect->setText(m_discordConnected ? text("Déconnecter Discord", "Disconnect Discord")
        : m_discordBusy ? text("Annuler", "Cancel") : text("Connecter Discord", "Connect Discord"));
    u.notice->setText(m_realPack ? text("Mode local · comptes et accès simulés · pack officiel", "Local mode · simulated accounts and access · official pack")
                                : text("Mode local · comptes, accès et installation simulés", "Local mode · simulated accounts, access and installation"));
    if (m_onlineSession) u.notice->setText(m_discordMessage.isEmpty() ? text("Retrouvez votre compte Sanctuary avec Discord.", "Find your Sanctuary account with Discord.") : m_discordMessage);
    const QStringList scenarioNames{text("Scénarios locaux", "Local scenarios"), "1 · Discord", "2 · Microsoft",
        text("Demande d'accès", "Access application"), text("En attente", "Pending"), text("Attente après préparation", "Pending after preparation"),
        text("Synchronisation d'accès", "Access synchronization"), text("Accorder l'accès local", "Grant local access"),
        text("Accès local prêt", "Local access ready"), text("Refus", "Rejected"), text("Accès retiré", "Revoked")};
    for (int i = 0; i < scenarioNames.size(); ++i) u.scenarios->setItemText(i, scenarioNames[i]);
    u.reset->setText(text("Recommencer", "Start over"));
    u.server->setText(text("Serveur · statut non vérifié", "Server · status not checked"));
    const QStringList titles{"Discord", "Microsoft", text("Accès Sanctuary", "Sanctuary access"),
        text("Code de personnage · Facultatif", "Character code · Optional"), "Installation"};
    QString heading, subtitle;
    if (active == 1) {
        heading = text("Connecter Discord", "Connect Discord");
        subtitle = text("Retrouvez votre dossier Sanctuary avec votre compte Discord.", "Find your Sanctuary profile with your Discord account.");
    } else if (active == 2) {
        heading = text("Connecter Microsoft", "Connect Microsoft");
        subtitle = text("Vérifiez votre compte et votre licence Minecraft Java.", "Verify your account and Minecraft Java license.");
    } else if (active == 3) {
        switch (access) {
            case Access::None: heading = text("Demander à rejoindre Sanctuary", "Apply to join Sanctuary"); break;
            case Access::Pending: heading = text("Accès Sanctuary — En attente", "Sanctuary access — Pending"); break;
            case Access::Synchronizing: heading = text("Accès accordé — Préparation en cours", "Access granted — Preparing"); break;
            case Access::Rejected: heading = text("Candidature non retenue", "Application not accepted"); break;
            case Access::Revoked: heading = text("Accès Sanctuary retiré", "Sanctuary access revoked"); break;
            case Access::Granted: break;
        }
        subtitle = access == Access::None
            ? text("Vos comptes sont liés. Envoyez votre candidature.", "Your accounts are linked. Submit your application.")
            : text("Vous pouvez préparer votre jeu ; Jouer attend la confirmation d'accès.", "You can prepare your game; Play requires access confirmation.");
    } else if (active == 4) {
        heading = text("Mon code de personnage", "My character code");
        subtitle = text("Ce code reste facultatif pour rejoindre Sanctuary.", "This code is optional when joining Sanctuary.");
    } else {
        heading = installing ? text("Installation de Sanctuary", "Installing Sanctuary")
                            : (installed ? text("Jouer à Sanctuary", "Play Sanctuary") : text("Installer Sanctuary", "Install Sanctuary"));
        subtitle = text("Votre progression est conservée entre les ouvertures.", "Your progress is kept between sessions.");
    }
    u.eyebrow->setText(text("Étape %1 · %2", "Step %1 · %2").arg(active).arg(titles[active - 1]));
    if (m_onlineSession && discord && m_discordProfile.isEmpty()) heading = text("Vérifier votre accès Sanctuary", "Verify your Sanctuary access");
    if (m_onlineSession && remoteStatus == "guild_required") heading = text("Rejoindre le Discord Sanctuary", "Join the Sanctuary Discord");
    u.heading->setText(communityUnlocked() ? (m_onlineSession ? text("Connexion Microsoft en attente", "Microsoft sign-in pending") : text("Sanctuary", "Sanctuary")) : heading);
    u.subtitle->setText(m_onlineSession && communityUnlocked() ? text("Accès Sanctuary confirmé. Vos publications sont disponibles ; le jeu attend Microsoft.", "Sanctuary access confirmed. Your posts are available; the game awaits Microsoft.") : communityUnlocked() ? text("Les nouvelles de votre communauté.", "The latest from your community.") : subtitle);
    u.eyebrow->setVisible(!communityUnlocked());
    const std::array<bool, 5> complete{discord, microsoft, admitted, false, installed};
    for (int i = 0; i < 5; ++i) {
        auto& r = u.rows[i];
        r.title->setText(titles[i]);
        r.status->show();
        r.action->hide();
        r.action->setEnabled(true);
        setRole(r.action, "primary", false);
        setRole(r.frame, "active", active == i + 1);
        setRole(r.frame, "waiting", i == 2 && waiting);
        const QString kind = i == 2 && waiting ? "waiting" : complete[i] ? "complete" : (active == i + 1 && i != 3 ? "current" : "neutral");
        setRole(r.number, "kind", kind);
        setRole(r.status, "kind", complete[i] ? "complete" : "neutral");
    }
    const auto action = [&u](int index, const QString& value, bool primary = false) {
        if (index != 4) u.rows[index].status->hide();
        u.rows[index].action->setText(value);
        u.rows[index].action->show();
        setRole(u.rows[index].action, "primary", primary);
    };
    auto& d = u.rows[0];
    d.detail->setText(m_onlineSession && discord ? m_discordProfile.value("user").toObject().value("display_name").toString(text("Compte Discord", "Discord account")) : discord ? text("Compte de démonstration", "Demo account") : text("Votre identité Sanctuary", "Your Sanctuary identity"));
    d.status->setText(text("Connecté", "Connected"));
    if (!discord) action(0, (m_onlineSession || m_localMode) ? text("Connecter Discord", "Connect Discord") : text("Ouvrir le site", "Open website"), active == 1);
    auto& m = u.rows[1];
    m.detail->setText(microsoft ? text("Minecraft Java · simulation", "Minecraft Java · simulation") : text("Votre compte Minecraft Java", "Your Minecraft Java account"));
    m.status->setText(text("Connecté", "Connected"));
    if (!microsoft) {
        action(1, text("Connecter Microsoft", "Connect Microsoft"), active == 2);
        m.action->setEnabled(!m_onlineSession && (m_localMode || m_microsoftAvailable));
    }
    auto& a = u.rows[2];
    a.detail->setText(text("Candidature ou parrainage", "Application or sponsorship"));
    a.status->setText(text("À vérifier", "Not checked"));
    u.sponsor->setVisible(!m_onlineSession && m_localMode && discord && microsoft && access == Access::None);
    u.sponsor->setText(text("Simuler un parrainage", "Simulate sponsorship"));
    if (discord && (m_onlineSession || microsoft)) {
        switch (access) {
            case Access::None: action(2, m_onlineSession ? text("Ouvrir mon dossier", "Open my profile") : text("Envoyer ma candidature", "Submit application"), true); break;
            case Access::Pending:
                a.detail->setText(text("Votre candidature est en cours d'examen.", "Your application is being reviewed."));
                a.status->setText(text("En attente", "Pending")); setRole(a.status, "kind", "waiting"); break;
            case Access::Synchronizing:
                a.detail->setText(text("Confirmation serveur en attente.", "Waiting for server confirmation."));
                a.status->setText(text("Préparation en cours", "Preparing")); setRole(a.status, "kind", "waiting"); break;
            case Access::Granted:
                a.detail->setText(m_onlineSession ? text("Accès confirmé par le site Sanctuary.", "Access confirmed by the Sanctuary website.") : text("Accès de démonstration uniquement.", "Demo access only."));
                a.status->setText(text("Validé", "Granted")); break;
            case Access::Rejected:
                a.detail->setText(text("Consultez l'équipe pour connaître la suite.", "Contact the team for next steps."));
                a.status->setText(text("Refusée", "Rejected")); setRole(a.status, "kind", "error"); break;
            case Access::Revoked:
                a.detail->setText(text("Vos fichiers et réglages sont conservés.", "Your files and preferences are preserved."));
                a.status->setText(text("Accès retiré", "Access revoked")); setRole(a.status, "kind", "error"); break;
        }
    }
    if (m_onlineSession && m_discordProfile.isEmpty()) a.detail->setText(text("Actualisez pour vérifier votre dossier.", "Refresh to verify your profile."));
    if (m_onlineSession && remoteStatus == "guild_required") a.detail->setText(text("Rejoignez le Discord depuis votre dossier sur le site.", "Join Discord from your profile on the website."));
    u.historical->setText(text("Joueur historique de la bêta", "Historical Beta player"));
    u.historical->setVisible(!m_onlineSession && m_localMode && m_local.historical);
    auto& c = u.rows[3];
    c.detail->setText(admitted ? text("Aucun code à saisir pour jouer.", "No code entry needed to play.") : text("Disponible après admission.", "Available after admission."));
    c.status->setText(text("Masqué", "Hidden"));
    if (admitted) action(3, m_codeVisible ? text("Masquer mon code", "Hide my code") : text("Afficher mon code SGA", "Show my SGA code"));
    if (m_onlineSession && admitted) action(3, text("Voir mon dossier", "View my profile"));
    u.codePanel->setVisible(!m_onlineSession && admitted && m_codeVisible);
    u.codeHelp->setVisible(!m_onlineSession && admitted && m_codeVisible);
    u.code->setText("SANCTUARYDEMO");
    u.code->setAccessibleName(text("Code fictif de démonstration", "Fictional demo code"));
    u.codeHelp->setText(text("Code fictif · ne permet aucune admission réelle", "Fictional code · grants no real admission"));
    u.copy->setText(text("Copier", "Copy"));
    auto& p = u.rows[4];
    p.detail->setText(m_localMode ? text("Java et pack · simulation locale", "Java and pack · local simulation")
                                : text("Profil Prism · contenu à vérifier", "Prism profile · contents not verified"));
    p.status->setText(m_localMode ? text("Installé · simulation", "Installed · simulation") : text("Profil présent", "Profile present"));
    if (m_realPack || (!installed && !(m_instanceReady && !m_localMode))) {
        const bool primary = m_realPack ? !installed : active == 5 || (active == 3 && access != Access::None);
        action(4, m_realPack ? (installing ? text("Annuler", "Cancel") : installed ? text("Mettre à jour", "Update")
                                 : m_instanceReady ? text("Vérifier / réparer", "Verify / repair") : text("Installer Sanctuary", "Install Sanctuary"))
                            : installing ? text("Simulation en cours", "Simulation in progress") : text("Installer Sanctuary", "Install Sanctuary"), primary);
        p.action->setEnabled(m_realPack || !installing);
    }
    if (m_realPack) {
        p.detail->setText(m_packVersion.isEmpty() ? text("Pack officiel du site", "Official website pack") : "Sanctuary " + m_packVersion);
        p.status->setText(installed ? text("Pack vérifié", "Pack verified") : text("À préparer", "Not prepared"));
    }
    u.progress->setVisible(installing);
    const int percent = m_realPack ? m_packProgress : m_progress;
    u.progress->setRange(0, installing && percent < 0 ? 0 : 100);
    u.progress->setValue(qMax(0, percent));
    u.progress->setFormat(m_realPack ? "%p%" : text("Simulation · %p%", "Simulation · %p%"));
    u.packSummary->setText(m_realPack ? m_packMessage : text("Installation locale de développement", "Local development installation"));
    u.packSummary->setToolTip(u.packSummary->text());
    const bool ready = communityUnlocked();
    u.body->setCurrentWidget(ready ? u.community : u.setupPanel);
    u.setupToggle->setVisible(ready);
    u.setupToggle->setText(text("Mes comptes", "My accounts"));
    u.readyCodeToggle->setVisible(ready);
    u.readyCodeToggle->setText(m_codeVisible ? text("Masquer le code SGA", "Hide SGA code") : text("Code SGA · facultatif", "SGA code · optional"));
    if (m_onlineSession) u.readyCodeToggle->setText(text("Mon dossier Sanctuary", "My Sanctuary profile"));
    u.readyCodePanel->setVisible(!m_onlineSession && ready && m_codeVisible);
    u.readyCode->setText("SANCTUARYDEMO");
    u.readyCodeCopy->setText(text("Copier le code fictif", "Copy fictional code"));
    for (auto* title : u.community->findChildren<QLabel*>()) {
        if (title->property("labelFr").isValid()) title->setText(title->property(m_french ? "labelFr" : "labelEn").toString());
    }
    for (auto* feed : u.feeds) feed->setFrench(m_french);
    if (ready && !m_communityVisible) for (auto* feed : u.feeds) feed->reload();
    m_communityVisible = ready;
    u.play->setText(text("Jouer", "Play"));
    u.play->setEnabled(!m_onlineSession && m_localMode && discord && microsoft && admitted && installed && !installing);
    u.play->setToolTip(text("En mode local, ce bouton vérifie le parcours sans lancer Minecraft.", "In local mode, this checks the flow without launching Minecraft."));
    if (m_onlineSession) u.play->setToolTip(text("Le lancement attend le raccordement Microsoft/Mojang.", "Game launch awaits Microsoft/Mojang integration."));
    u.help->setText(m_realPack ? text("Comptes et accès locaux. Les essais du jeu et du serveur attendent Mojang.",
                                    "Local accounts and access. Game and server tests await Mojang.")
                    : m_localMode ? text("Simulation locale uniquement. Minecraft, serveur et personnage seront testés après l'approbation Mojang.",
                                                  "Local simulation only. Minecraft, server and character tests await Mojang approval.")
                                    : text("Connexion et accès au serveur en attente du raccordement Microsoft/Mojang.", "Sign-in and server access await Microsoft/Mojang integration."));
    if (m_onlineSession) u.help->setText(text("Discord et accès réels · lancement Minecraft en attente de Microsoft/Mojang.", "Real Discord account and access · Minecraft launch awaits Microsoft/Mojang."));
    u.mods->setText("Mods");
    u.shaders->setText("Shaders");
    u.instanceSettings->setText(text("Réglages", "Settings"));
    for (auto* shortcut : {u.mods, u.shaders, u.instanceSettings}) shortcut->setVisible(!m_embedded && (m_localMode || m_instanceReady));
    u.tools->setText(text("Tous les outils", "All tools"));
    u.tools->setVisible(!m_embedded);
    u.toolsPanel->setVisible(!m_embedded);
    u.site->setVisible(!m_embedded);
    u.news->setVisible(!m_embedded);
    u.footer->setVisible(!m_embedded);
    u.settings->setText(text("Réglages du launcher", "Launcher settings"));
    u.news->setText(text("Actualités", "News"));
    u.site->setText("Site Sanctuary");
    u.wiki->setText(text("Aide", "Help"));
    u.footer->setText(m_localMode ? text("Mode local · aucune connexion serveur", "Local mode · no server connection") : "Sanctuary · Prism 11.1.1");
}
