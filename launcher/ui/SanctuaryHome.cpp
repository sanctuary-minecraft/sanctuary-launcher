// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryHome.h"
#include "SanctuaryFeed.h"
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
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
    QWidget *previewControls, *codePanel, *setupPanel, *community, *readyCodePanel;
    QFrame *chromeHeader, *chromeFooter;
    QList<SanctuaryFeed*> feeds;
    QLabel* readyCode;
    QPushButton *readyCodeToggle, *readyCodeCopy;
    QComboBox* scenarios;
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
        QWidget#sanctuaryHome, #sanctuaryHome QScrollArea, #sanctuaryHome QWidget#surface { background: #fff; color: #1f2328; }
        #sanctuaryHome { font-family: "SF Pro Text", "Segoe UI", sans-serif; font-size: 14px; }
        #sanctuaryHome QLabel { color: #1f2328; border: none; background: transparent; }
        #sanctuaryHome QLabel#heading { font-size: 21px; font-weight: 700; }
        #sanctuaryHome QLabel#eyebrow { font-size: 14px; font-weight: 600; color: #57606a; }
        #sanctuaryHome QLabel#subtitle { font-size: 16px; color: #57606a; }
        #sanctuaryHome QLabel[muted="true"] { color: #57606a; font-size: 12px; }
        #sanctuaryHome QFrame#header { border: none; border-bottom: 1px solid #d0d7de; }
        #sanctuaryHome QFrame#footer { border: none; border-top: 1px solid #d0d7de; }
        #sanctuaryHome QFrame#steps { border: 1px solid #d0d7de; border-radius: 6px; background: white; }
        #sanctuaryHome QFrame[stepRow="true"] { border: none; border-bottom: 1px solid #d8dee4; background: transparent; }
        #sanctuaryHome QFrame[active="true"] { background: #f6f8fa; }
        #sanctuaryHome QFrame[waiting="true"] { background: #fff8e5; }
        #sanctuaryHome QLabel[rowTitle="true"] { font-size: 16px; font-weight: 600; }
        #sanctuaryHome QLabel[number="true"] { border: 1px solid #8c959f; border-radius: 16px; color: #57606a; font-size: 17px; font-weight: 600; }
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
        #sanctuaryHome QScrollArea { border: none; }
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
}

void SanctuaryHome::setEmbedded(bool embedded)
{
    m_embedded = embedded;
    m_ui->chromeHeader->setVisible(!embedded);
    m_ui->chromeFooter->setVisible(true);
    refresh();
}

bool SanctuaryHome::communityUnlocked() const
{
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
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* surface = new QWidget(scroll);
    surface->setObjectName("surface");
    auto* horizontal = new QHBoxLayout(surface);
    horizontal->setContentsMargins(24, 18, 24, 18);
    horizontal->addStretch();
    auto* column = new QWidget(surface);
    column->setMaximumWidth(960);
    auto* body = new QVBoxLayout(column);
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(9);
    horizontal->addWidget(column, 1);
    horizontal->addStretch();
    scroll->setWidget(surface);
    outer->addWidget(scroll);
    auto* header = new QFrame(column);
    header->setObjectName("header");
    u.chromeHeader = header;
    auto* top = new QHBoxLayout(header);
    top->setContentsMargins(0, 0, 0, 15);
    auto* wordmark = label(header, "wordmark");
    wordmark->setPixmap(QPixmap(":/sanctuary/wordmark.png").scaledToWidth(215, Qt::SmoothTransformation));
    wordmark->setAccessibleName("Sanctuary");
    top->addWidget(wordmark);
    auto* launcher = label(header, "launcherTitle");
    launcher->setText("Launcher");
    top->addWidget(launcher);
    top->addStretch();
    u.server = label(header, "serverStatus");
    u.server->setProperty("muted", true);
    u.server->setWordWrap(false);
    top->addWidget(u.server);
    body->addWidget(header);
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
    connect(u.reset, &QPushButton::clicked, this, [this] { setLocalScenario("start"); });
    body->addWidget(u.previewControls);
    auto* packHeading = label(column, "packHeading");
    packHeading->setText("Sanctuary Beta");
    packHeading->setStyleSheet("font-size: 28px; font-weight: 700;");
    body->addWidget(packHeading);
    u.packSummary = label(column, "packSummary");
    body->addWidget(u.packSummary);
    u.feedback = label(column, "feedback");
    u.feedback->setProperty("muted", true);
    body->addWidget(u.feedback);
    u.setupPanel = new QWidget(column);
    u.setupPanel->setObjectName("inlineAccounts");
    auto* details = new QVBoxLayout(u.setupPanel);
    details->setContentsMargins(0, 5, 0, 5);
    details->setSpacing(6);
    u.eyebrow = label(column, "eyebrow");
    u.heading = label(column, "heading");
    u.subtitle = label(column, "subtitle");
    details->addWidget(u.eyebrow);
    details->addWidget(u.heading);
    details->addWidget(u.subtitle);
    auto* steps = new QFrame(column);
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
        line->setContentsMargins(16, 11, 16, 11);
        line->setHorizontalSpacing(18);
        line->setVerticalSpacing(4);
        r.number = label(r.frame, QString("number%1").arg(i + 1));
        r.number->setText(QString::number(i + 1));
        r.number->setProperty("number", true);
        r.number->setFixedSize(34, 34);
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
        line->addWidget(r.status, 0, 2, 2, 1, Qt::AlignVCenter);
        line->addWidget(r.action, 0, 2, 2, 1, Qt::AlignVCenter);
        if (i == 2) {
            u.historical = label(r.frame, "historicalBadge");
            u.historical->setProperty("badge", true);
            u.historical->setWordWrap(false);
            line->addWidget(u.historical, 2, 1, 1, 1, Qt::AlignLeft);
            u.sponsor = button(r.frame, "sponsor");
            u.sponsor->setProperty("link", true);
            line->addWidget(u.sponsor, 3, 1);
        }
        if (i == 3) {
            u.codePanel = new QWidget(r.frame);
            auto* codeLayout = new QHBoxLayout(u.codePanel);
            codeLayout->setContentsMargins(0, 3, 0, 0);
            u.code = label(u.codePanel, "codeValue");
            const int fontId = QFontDatabase::addApplicationFont(":/sanctuary/Sga-Regular.ttf");
            const auto families = QFontDatabase::applicationFontFamilies(fontId);
            if (!families.isEmpty()) {
                QFont font(families.first(), 22);
                font.setLetterSpacing(QFont::AbsoluteSpacing, 5);
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
        if (i == 4) {
            u.progress = new QProgressBar(r.frame);
            u.progress->setObjectName("installationProgress");
            u.progress->setRange(0, 100);

        }
        rows->addWidget(r.frame);
    }
    details->addWidget(steps);
    body->addWidget(u.progress);
    u.help = label(column, "helpText");
    u.help->setProperty("muted", true);
    body->addWidget(u.help);
    auto* launch = new QHBoxLayout;
    u.play = button(column, "play");
    u.play->setMinimumWidth(135);
    u.play->setProperty("primary", true);
    launch->addWidget(u.play);
    launch->addWidget(u.rows[4].action);
    u.mods = button(column, "mods");
    u.shaders = button(column, "shaders");
    u.instanceSettings = button(column, "instanceSettings");

    launch->addStretch();
    u.tools = button(column, "advanced");
    launch->addWidget(u.tools);
    body->addLayout(launch);
    auto* shortcuts = new QGridLayout;
    shortcuts->setSpacing(8);
    shortcuts->addWidget(u.mods, 0, 0);
    shortcuts->addWidget(u.shaders, 0, 2);
    shortcuts->addWidget(u.instanceSettings, 2, 0);
    struct Shortcut { const char* id; const char* fr; const char* en; int row; int column; };
    const Shortcut extra[] = {
        {"resourcepacks", "Packs de ressources", "Resource packs", 0, 1},
        {"screenshots", "Captures d'écran", "Screenshots", 1, 0},
        {"worlds", "Mondes", "Worlds", 1, 1},
        {"servers", "Serveurs", "Servers", 1, 2},
        {"version", "Versions", "Versions", 2, 1},
        {"console", "Journaux", "Logs", 2, 2}
    };
    for (const auto& shortcut : extra) {
        auto* entry = button(column, shortcut.id);
        entry->setProperty("labelFr", shortcut.fr);
        entry->setProperty("labelEn", shortcut.en);
        shortcuts->addWidget(entry, shortcut.row, shortcut.column);
        const QString page(shortcut.id);
        connect(entry, &QPushButton::clicked, this, [this, page] { emit instanceToolRequested(page); });
    }
    body->addLayout(shortcuts);
    u.setupToggle = button(column, "setupToggle");
    body->addWidget(u.setupToggle);
    body->addWidget(u.setupPanel);
    u.setupPanel->show();
    connect(u.setupToggle, &QPushButton::clicked, this, &SanctuaryHome::accountsRequested);
    u.readyCodeToggle = button(column, "readyCodeToggle");
    body->addWidget(u.readyCodeToggle);
    u.readyCodePanel = new QWidget(column);
    auto* readyCodeLayout = new QHBoxLayout(u.readyCodePanel);
    readyCodeLayout->setContentsMargins(0, 0, 0, 0);
    u.readyCode = label(u.readyCodePanel, "readyCodeValue");
    u.readyCode->setFont(u.code->font());
    readyCodeLayout->addWidget(u.readyCode);
    u.readyCodeCopy = button(u.readyCodePanel, "readyCodeCopy");
    readyCodeLayout->addWidget(u.readyCodeCopy);
    readyCodeLayout->addStretch();
    body->addWidget(u.readyCodePanel);
    connect(u.readyCodeToggle, &QPushButton::clicked, this, [this] { m_codeVisible = !m_codeVisible; refresh(); });
    connect(u.readyCodeCopy, &QPushButton::clicked, u.copy, &QPushButton::click);
    u.community = new QWidget(column);
    u.community->setObjectName("communityHome");
    auto* community = new QVBoxLayout(u.community);
    community->setContentsMargins(0, 0, 0, 0);
    struct FeedSection { const char* kind; const char* fr; const char* en; };
    const FeedSection sections[] = {{"bulletin","Intendance","Stewardship"},{"article","Gazette","Gazette"},{"notice","Tableau d'affichage","Notice board"}};
    for (const auto& section : sections) {
        auto* title = label(u.community, "communityTitle-" + QString(section.kind));
        title->setProperty("labelFr", section.fr);
        title->setProperty("labelEn", section.en);
        title->setStyleSheet("font-size: 18px; font-weight: 600; padding-top: 10px;");
        community->addWidget(title);
        auto* feed = new SanctuaryFeed(section.kind, u.community);
        u.feeds.append(feed);
        community->addWidget(feed);
        connect(feed, &SanctuaryFeed::linkRequested, this, &SanctuaryHome::linkRequested);
    }
    body->addWidget(u.community);
    auto* footer = new QFrame(column);
    footer->setObjectName("footer");
    u.chromeFooter = footer;
    auto* foot = new QHBoxLayout(footer);
    foot->setContentsMargins(0, 10, 0, 0);
    u.news = button(footer, "news");
    u.site = button(footer, "site");
    u.wiki = button(footer, "help");
    u.settings = button(footer, "settings");
    for (auto* link : {u.news, u.site, u.wiki, u.settings}) {
        link->setProperty("link", true);
        foot->addWidget(link);
    }
    foot->addStretch();
    u.footer = label(footer, "footerVersion");
    u.footer->setProperty("muted", true);
    u.footer->setWordWrap(false);
    foot->addWidget(u.footer);
    body->addWidget(footer);
    body->addStretch();
    connect(u.rows[0].action, &QPushButton::clicked, this, [this] {
        if (m_localMode) { m_local.discord = true; commitLocalChange(); }
        else emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
    });
    connect(u.rows[1].action, &QPushButton::clicked, this, [this] {
        if (m_localMode) { m_local.microsoft = true; commitLocalChange(); }
        else if (m_microsoftAvailable) emit accountsRequested();
    });
    connect(u.rows[2].action, &QPushButton::clicked, this, [this] {
        if (m_localMode && m_local.access == Access::None) { m_local.access = Access::Pending; commitLocalChange(); }
        else if (!m_localMode) emit linkRequested(QUrl("https://sanctuary-minecraft.net/candidature"));
    });
    connect(u.rows[3].action, &QPushButton::clicked, this, [this] {
        if (m_localMode && m_local.access == Access::Granted) { m_codeVisible = !m_codeVisible; refresh(); }
    });
    connect(u.copy, &QPushButton::clicked, this, [this] {
        if (m_localMode && m_local.access == Access::Granted && m_codeVisible)
            QApplication::clipboard()->setText("DEMO-SANCTUARY");
    });
    connect(u.rows[4].action, &QPushButton::clicked, this, &SanctuaryHome::install);
    connect(u.play, &QPushButton::clicked, this, [this] {
        if (m_localMode && m_local.discord && m_local.microsoft && m_local.access == Access::Granted && (m_realPack ? m_packVerified : m_local.installed))
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
    const bool discord = m_localMode && m_local.discord;
    const bool microsoft = m_localMode && m_local.microsoft;
    const bool admitted = m_localMode && m_local.access == Access::Granted;
    const bool installed = m_realPack ? m_packVerified : m_localMode && m_local.installed;
    const bool installing = m_realPack ? m_packBusy : m_installTimer && m_installTimer->isActive();
    const int active = activeStep();
    const bool waiting = m_localMode && (m_local.access == Access::Pending || m_local.access == Access::Synchronizing);
    u.previewControls->setVisible(m_localMode);
    u.notice->setText(m_realPack ? text("Mode local · comptes et accès simulés · pack officiel", "Local mode · simulated accounts and access · official pack")
                                : text("Mode local · comptes, accès et installation simulés", "Local mode · simulated accounts, access and installation"));
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
        switch (m_local.access) {
            case Access::None: heading = text("Demander à rejoindre Sanctuary", "Apply to join Sanctuary"); break;
            case Access::Pending: heading = text("Accès Sanctuary — En attente", "Sanctuary access — Pending"); break;
            case Access::Synchronizing: heading = text("Accès accordé — Préparation en cours", "Access granted — Preparing"); break;
            case Access::Rejected: heading = text("Candidature non retenue", "Application not accepted"); break;
            case Access::Revoked: heading = text("Accès Sanctuary retiré", "Sanctuary access revoked"); break;
            case Access::Granted: break;
        }
        subtitle = m_local.access == Access::None
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
    u.heading->setText(heading);
    u.subtitle->setText(subtitle);
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
    d.detail->setText(discord ? text("Compte de démonstration", "Demo account") : text("Votre identité Sanctuary", "Your Sanctuary identity"));
    d.status->setText(text("Connecté", "Connected"));
    if (!discord) action(0, m_localMode ? text("Connecter Discord", "Connect Discord") : text("Ouvrir le site", "Open website"), active == 1);
    auto& m = u.rows[1];
    m.detail->setText(microsoft ? text("Minecraft Java · simulation", "Minecraft Java · simulation") : text("Votre compte Minecraft Java", "Your Minecraft Java account"));
    m.status->setText(text("Connecté", "Connected"));
    if (!microsoft) {
        action(1, text("Connecter Microsoft", "Connect Microsoft"), active == 2);
        m.action->setEnabled(m_localMode || m_microsoftAvailable);
    }
    auto& a = u.rows[2];
    a.detail->setText(text("Candidature ou parrainage", "Application or sponsorship"));
    a.status->setText(text("À vérifier", "Not checked"));
    u.sponsor->setVisible(m_localMode && discord && microsoft && m_local.access == Access::None);
    u.sponsor->setText(text("Simuler un parrainage", "Simulate sponsorship"));
    if (m_localMode && discord && microsoft) {
        switch (m_local.access) {
            case Access::None: action(2, text("Envoyer ma candidature", "Submit application"), true); break;
            case Access::Pending:
                a.detail->setText(text("Votre candidature est en cours d'examen.", "Your application is being reviewed."));
                a.status->setText(text("En attente", "Pending")); setRole(a.status, "kind", "waiting"); break;
            case Access::Synchronizing:
                a.detail->setText(text("Confirmation serveur en attente.", "Waiting for server confirmation."));
                a.status->setText(text("Préparation en cours", "Preparing")); setRole(a.status, "kind", "waiting"); break;
            case Access::Granted:
                a.detail->setText(text("Accès de démonstration uniquement.", "Demo access only."));
                a.status->setText(text("Validé", "Granted")); break;
            case Access::Rejected:
                a.detail->setText(text("Consultez l'équipe pour connaître la suite.", "Contact the team for next steps."));
                a.status->setText(text("Refusée", "Rejected")); setRole(a.status, "kind", "error"); break;
            case Access::Revoked:
                a.detail->setText(text("Vos fichiers et réglages sont conservés.", "Your files and preferences are preserved."));
                a.status->setText(text("Accès retiré", "Access revoked")); setRole(a.status, "kind", "error"); break;
        }
    }
    u.historical->setText(text("Joueur historique de la bêta", "Historical Beta player"));
    u.historical->setVisible(m_localMode && m_local.historical);
    auto& c = u.rows[3];
    c.detail->setText(admitted ? text("Aucun code à saisir pour jouer.", "No code entry needed to play.") : text("Disponible après admission.", "Available after admission."));
    c.status->setText(text("Masqué", "Hidden"));
    if (admitted) action(3, m_codeVisible ? text("Masquer mon code", "Hide my code") : text("Afficher mon code SGA", "Show my SGA code"));
    u.codePanel->setVisible(admitted && m_codeVisible);
    u.codeHelp->setVisible(admitted && m_codeVisible);
    u.code->setText("SANCTUARYDEMO");
    u.code->setAccessibleName(text("Code fictif de démonstration", "Fictional demo code"));
    u.codeHelp->setText(text("Code fictif · ne permet aucune admission réelle", "Fictional code · grants no real admission"));
    u.copy->setText(text("Copier", "Copy"));
    auto& p = u.rows[4];
    p.detail->setText(m_localMode ? text("Java et pack · simulation locale", "Java and pack · local simulation")
                                : text("Profil Prism · contenu à vérifier", "Prism profile · contents not verified"));
    p.status->setText(m_localMode ? text("Installé · simulation", "Installed · simulation") : text("Profil présent", "Profile present"));
    if (m_realPack || (!installed && !(m_instanceReady && !m_localMode))) {
        const bool primary = m_realPack ? !installed : active == 5 || (active == 3 && m_local.access != Access::None);
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
    const bool ready = communityUnlocked();
    u.setupPanel->setVisible(!ready);
    u.setupToggle->setVisible(ready);
    u.setupToggle->setText(text("Gérer mes comptes", "Manage my accounts"));
    u.community->setVisible(ready);
    u.readyCodeToggle->setVisible(ready);
    u.readyCodeToggle->setText(m_codeVisible ? text("Masquer mon code SGA · facultatif", "Hide my SGA code · optional")
                                          : text("Afficher mon code SGA · facultatif", "Show my SGA code · optional"));
    u.readyCodePanel->setVisible(ready && m_codeVisible);
    u.readyCode->setText("SANCTUARYDEMO");
    u.readyCodeCopy->setText(text("Copier le code fictif", "Copy fictional code"));
    for (auto* title : u.community->findChildren<QLabel*>()) {
        if (title->property("labelFr").isValid()) title->setText(title->property(m_french ? "labelFr" : "labelEn").toString());
    }
    for (auto* feed : u.feeds) feed->setFrench(m_french);
    if (ready && !m_communityVisible) for (auto* feed : u.feeds) feed->reload();
    m_communityVisible = ready;
    for (auto* shortcut : findChildren<QPushButton*>()) {
        if (shortcut->property("labelFr").isValid()) {
            shortcut->setText(shortcut->property(m_french ? "labelFr" : "labelEn").toString());
            shortcut->setVisible(!m_embedded);
        }
    }
    u.play->setText(text("Jouer", "Play"));
    u.play->setEnabled(m_localMode && discord && microsoft && admitted && installed && !installing);
    u.play->setToolTip(text("En mode local, ce bouton vérifie le parcours sans lancer Minecraft.", "In local mode, this checks the flow without launching Minecraft."));
    u.help->setText(m_realPack ? text("Pack officiel réel. En mode local, comptes et accès restent simulés ; les essais du serveur et du personnage attendent Mojang.",
                                    "Real official pack. In local mode, accounts and access remain simulated; server and character tests await Mojang.")
                    : m_localMode ? text("Simulation locale uniquement. Minecraft, serveur et personnage seront testés après l'approbation Mojang.",
                                                  "Local simulation only. Minecraft, server and character tests await Mojang approval.")
                                    : text("Connexion et accès au serveur en attente du raccordement Microsoft/Mojang.", "Sign-in and server access await Microsoft/Mojang integration."));
    u.mods->setText("Mods");
    u.shaders->setText("Shaders");
    u.instanceSettings->setText(text("Réglages", "Settings"));
    for (auto* shortcut : {u.mods, u.shaders, u.instanceSettings}) shortcut->setVisible(!m_embedded && (m_localMode || m_instanceReady));
    u.tools->setText(text("Tous les outils", "All tools"));
    u.tools->setVisible(!m_embedded);
    u.settings->setText(text("Réglages du launcher", "Launcher settings"));
    u.news->setText(text("Actualités", "News"));
    u.site->setText("Site Sanctuary");
    u.wiki->setText(text("Aide", "Help"));
    u.footer->setText(m_localMode ? text("Mode local · aucune connexion serveur", "Local mode · no server connection") : "Sanctuary · Prism 11.1.1");
}
