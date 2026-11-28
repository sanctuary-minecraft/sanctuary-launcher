// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryHome.h"
#include "SanctuaryAppearance.h"
#include "SanctuaryFeed.h"
#include "SanctuaryWordmark.h"
#include "SanctuarySession.h"
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QPointer>
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
    QPointer<QPushButton> connection;
    QPushButton* nextConnectionAction = nullptr;
    QMenu* connectionMenu = nullptr;
    QAction *refreshAccess = nullptr, *openProfile = nullptr, *changeMicrosoft = nullptr;
    QAction *disconnectDiscord = nullptr, *managePack = nullptr;
    std::array<Row, 5> rows;
    QLabel *eyebrow, *heading, *subtitle, *server, *notice, *historical, *help, *footer, *code, *codeHelp;
    QWidget *previewControls, *codePanel, *setupPanel, *community, *readyCodePanel, *toolsPanel, *dashboardEmpty;
    QHBoxLayout* contentLayout;
    QWidget* contentColumn;
    QLabel* dashboardMessage;
    QPushButton* startSetup;
    QFrame *chromeHeader, *chromeFooter;
    QList<SanctuaryFeed*> feeds;
    QList<QLabel*> feedTitles;
    QLabel* readyCode;
    QPushButton *readyCodeToggle, *readyCodeCopy;
    QComboBox* scenarios;
    QPushButton *discordConnect, *discordRefresh;
    QStackedWidget* body;
    QLabel *packSummary, *feedback;
    QPushButton* setupToggle;
    QPushButton *reset, *play, *mods, *shaders, *instanceSettings, *tools, *settings, *news, *site, *wiki, *copy, *sponsor;
    QPushButton* playModes;
    QAction *playMinecraft, *joinSanctuary;
    QProgressBar* progress;
};

SanctuaryHome::SanctuaryHome(QWidget* parent, View view) : QWidget(parent), m_ui(std::make_unique<Ui>()), m_view(view)
{
    initializeSanctuaryResources();
    setObjectName("sanctuaryHome");
    new SanctuaryAppearance::PaletteWatcher(this, [this] { applyAppearance(); });
    buildUi();
    if (m_view == View::Dashboard) {
        auto* clock = new QTimer(this);
        clock->setInterval(1000);
        connect(clock, &QTimer::timeout, this, [this] { if (isVisible()) refreshClock(); });
        clock->start();
    }
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
void SanctuaryHome::applyAppearance()
{
    SanctuaryAppearance::apply(this, QStringLiteral(R"(
        QWidget#sanctuaryHome, #sanctuaryHome QWidget#surface { background: @surface; color: @text; }
        #sanctuaryHome { font-family: "SF Pro Text", "Segoe UI", sans-serif; font-size: 14px; }
        #sanctuaryHome QLabel { color: @text; border: none; background: transparent; }
        #sanctuaryHome QLabel#heading { font-size: 21px; font-weight: 700; }
        #sanctuaryHome QLabel#heading[dashboard="true"] { font-size: 23px; font-weight: 600; }
        #sanctuaryHome QLabel#eyebrow { font-size: 14px; font-weight: 600; color: @muted; }
        #sanctuaryHome QLabel#subtitle { font-size: 13px; color: @muted; }
        #sanctuaryHome QLabel[communityTitle="true"] { font-size: 14px; font-weight: 600; color: @muted; }
        #sanctuaryHome QLabel[muted="true"] { color: @muted; font-size: 12px; }
        #sanctuaryHome QFrame#header { border: none; border-bottom: 1px solid @border; }
        #sanctuaryHome QFrame#footer { border: none; border-top: 1px solid @border; }
        #sanctuaryHome QFrame[communityCard="true"] { border: 1px solid @border; border-radius: 4px; background: @panel; }
        #sanctuaryHome QFrame[communityCard="true"] QWidget { background: transparent; }
        #sanctuaryHome QFrame#steps { border: 1px solid @border; border-radius: 6px; background: @surface; }
        #sanctuaryHome QFrame[stepRow="true"] { border: none; border-bottom: 1px solid @border; background: transparent; }
        #sanctuaryHome QFrame[active="true"] { background: @panel; }
        #sanctuaryHome QFrame[waiting="true"] { background: @alertRow; }
        #sanctuaryHome QLabel[rowTitle="true"] { font-size: 14px; font-weight: 600; }
        #sanctuaryHome QLabel[number="true"] { border: 1px solid @controlBorder; border-radius: 14px; color: @muted; font-size: 14px; font-weight: 600; }
        #sanctuaryHome QLabel[number="true"][kind="complete"] { border-color: @greenText; color: @greenText; }
        #sanctuaryHome QLabel[number="true"][kind="current"] { background: @greenButton; border-color: @greenButton; color: white; }
        #sanctuaryHome QLabel[number="true"][kind="waiting"] { border-color: @alertText; color: @alertText; }
        #sanctuaryHome QLabel[badge="true"] { padding: 4px 8px; border: 1px solid @border; border-radius: 4px; font-size: 12px; }
        #sanctuaryHome QLabel[badge="true"][kind="complete"] { background: @greenPanel; border-color: @greenPanel; color: @greenText; }
        #sanctuaryHome QLabel[badge="true"][kind="waiting"] { background: @alertPanel; border-color: @alertPanel; color: @alertText; }
        #sanctuaryHome QLabel[badge="true"][kind="error"] { background: @errorPanel; border-color: @errorPanel; color: @errorText; }
        #sanctuaryHome QPushButton { color: @text; background: @panel; border: 1px solid @border; border-radius: 5px; padding: 5px 14px; font-size: 13px; font-weight: 500; }
        #sanctuaryHome QPushButton:hover { background: @hover; border-color: @controlBorder; }
        #sanctuaryHome QPushButton:focus { border: 2px solid @link; }
        #sanctuaryHome QPushButton[primary="true"] { background: @greenButton; color: white; border-color: @greenButton; font-weight: 600; }
        #sanctuaryHome QPushButton[primary="true"]:hover { background: @greenButtonHover; }
        #sanctuaryHome QPushButton:disabled { background: @disabledPanel; color: @disabledText; border-color: @disabledPanel; }
        #sanctuaryHome QPushButton#play { border-top-right-radius: 0; border-bottom-right-radius: 0; }
        #sanctuaryHome QPushButton#playModes { border-top-left-radius: 0; border-bottom-left-radius: 0; padding: 0; }
        #sanctuaryHome QPushButton#playModes::menu-indicator { image: none; }
        #sanctuaryHome QPushButton[link="true"] { color: @link; background: transparent; border: none; padding: 3px; font-weight: 400; text-align: left; }
        #sanctuaryHome QPushButton[link="true"]:focus { border: 1px solid @link; }

        #sanctuaryHome QComboBox { background: @surface; color: @text; border: 1px solid @border; border-radius: 4px; padding: 5px 10px; min-height: 22px; }
        #sanctuaryHome QProgressBar { border: 1px solid @border; border-radius: 3px; background: @surface; color: @text; text-align: center; }
        #sanctuaryHome QProgressBar::chunk { background: @greenPanel; }
    )"));
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
    m_ui->contentLayout->setContentsMargins(embedded ? 0 : 12, 0, embedded ? 0 : 12, 0);
    m_ui->contentLayout->setSpacing(embedded ? 0 : -1);
    m_ui->contentColumn->setMaximumWidth(embedded ? QWIDGETSIZE_MAX : 960);
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

void SanctuaryHome::setMicrosoftSignInState(MicrosoftSignIn mode, bool verified, const QString& profile, const QString& message)
{
    m_microsoftSignIn = mode;
    m_microsoftVerified = hasMicrosoftSignIn() && verified;
    m_microsoftProfile = profile;
    m_microsoftMessage = message;
    refresh();
}

void SanctuaryHome::setGameBusy(bool busy)
{
    m_gameBusy = busy;
    refresh();
}

void SanctuaryHome::setJoinOnPlay(bool join)
{
    m_joinOnPlay = join;
    refresh();
}

QPushButton* SanctuaryHome::createConnectionButton()
{
    auto& u = *m_ui;
    Q_ASSERT(!u.connection);
    u.connection = button(this, "nav-connection");
    u.connectionMenu = new QMenu(u.connection);
    u.refreshAccess = u.connectionMenu->addAction(QString(), u.discordRefresh, &QPushButton::click);
    u.openProfile = u.connectionMenu->addAction(QString(), u.rows[2].action, &QPushButton::click);
    u.changeMicrosoft = u.connectionMenu->addAction(QString(), u.rows[1].action, &QPushButton::click);
    u.disconnectDiscord = u.connectionMenu->addAction(QString(), this, &SanctuaryHome::discordDisconnectRequested);
    u.connectionMenu->addSeparator();
    u.managePack = u.connectionMenu->addAction(QString(), u.rows[4].action, &QPushButton::click);
    u.connection->addActions(u.connectionMenu->actions());
    u.connection->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(u.connection, &QPushButton::clicked, this, [this] {
        if (m_ui->nextConnectionAction) m_ui->nextConnectionAction->click();
    });
    refresh();
    return u.connection;
}

void SanctuaryHome::refreshConnectionButton(int step, bool started, bool complete)
{
    auto& u = *m_ui;
    if (!u.connection) return;

    u.refreshAccess->setText(text("Actualiser mon accès", "Refresh my access"));
    u.openProfile->setText(text("Ouvrir mon dossier Sanctuary", "Open my Sanctuary profile"));
    u.changeMicrosoft->setText(text("Changer de compte Microsoft", "Change Microsoft account"));
    u.disconnectDiscord->setText(text("Déconnecter Discord", "Disconnect Discord"));
    u.managePack->setText(m_packVerified && !m_packBusy ? text("Mettre à jour le pack", "Update the pack") : u.rows[4].action->text());
    u.refreshAccess->setEnabled(m_discordConnected && !m_discordBusy);
    u.openProfile->setEnabled(m_discordConnected);
    u.changeMicrosoft->setEnabled(m_microsoftVerified && u.rows[1].action->isEnabled());
    u.disconnectDiscord->setEnabled(m_discordConnected);
    u.managePack->setEnabled(m_realPack && u.rows[4].action->isEnabled());

    const auto progress = complete ? "complete" : started ? "partial" : "start";
    if (u.connection->property("progress").toString() != progress)
        setRole(u.connection, "progress", progress);
    auto* menu = complete ? u.connectionMenu : nullptr;
    if (u.connection->menu() != menu) u.connection->setMenu(menu);
    u.nextConnectionAction = nullptr;
    QString title, instructions;
    if (complete) {
        title = text("Connecté", "Connected");
        instructions = text("Cliquez pour gérer vos comptes ou vérifier le pack.",
                            "Click to manage your accounts or verify the pack.");
    } else if (step == 1) {
        u.nextConnectionAction = m_onlineSession ? u.discordConnect : u.rows[0].action;
        title = m_discordBusy ? text("Annuler la connexion", "Cancel sign-in") : text("Connecter Discord", "Connect Discord");
        instructions = text("Validez la connexion Discord dans le navigateur, puis revenez au launcher.",
                            "Confirm Discord sign-in in your browser, then return to the launcher.");
        if (!m_discordMessage.isEmpty()) instructions.prepend(m_discordMessage + '\n');
    } else if (step == 2) {
        u.nextConnectionAction = u.rows[1].action;
        title = text("Connecter Microsoft", "Connect Microsoft");
        instructions = hasMicrosoftSignIn()
            ? text("Suivez les instructions du code Microsoft avec le compte qui possède Minecraft Java.",
                   "Follow the Microsoft code instructions using the account that owns Minecraft Java.")
            : u.rows[1].detail->text();
    } else if (step == 3) {
        const auto status = m_discordProfile.value("access").toObject().value("status").toString();
        const bool refreshAccess = m_onlineSession && (status.isEmpty() || status == "pending" || status == "synchronizing");
        u.nextConnectionAction = refreshAccess ? u.discordRefresh : u.rows[2].action;
        title = refreshAccess ? text("Actualiser mon accès", "Refresh my access")
            : status == "guild_required" ? text("Rejoindre le Discord", "Join the Discord")
            : m_onlineSession ? text("Ouvrir mon dossier", "Open my profile") : u.rows[2].action->text();
        instructions = u.rows[2].detail->text();
        if (refreshAccess) instructions += text("\nCliquez pour vérifier votre accès.", "\nClick to check your access.");
    } else {
        u.nextConnectionAction = u.rows[4].action;
        title = u.rows[4].action->text();
        instructions = m_packBusy
            ? text("Le pack est en cours de préparation. Cliquez pour interrompre la synchronisation.",
                   "The pack is being prepared. Click to cancel synchronization.")
            : text("Installez ou vérifiez le pack officiel avant de lancer Minecraft.",
                   "Install or verify the official pack before launching Minecraft.");
    }
    u.connection->setText(title);
    u.connection->setToolTip(instructions);
    u.connection->setAccessibleDescription(instructions);
    u.connection->setEnabled(complete || (u.nextConnectionAction && u.nextConnectionAction->isEnabled()));
}

bool SanctuaryHome::canPlayMinecraft() const
{
    return m_microsoftVerified && m_instanceReady && m_realPack && m_packVerified && !m_packBusy && !m_gameBusy;
}

bool SanctuaryHome::canLaunchGame() const
{
    const auto minecraftName = m_discordProfile.value("user").toObject().value("minecraft_username").toString();
    return canPlayMinecraft() && m_onlineSession && m_discordConnected && !m_discordBusy
        && !minecraftName.isEmpty() && minecraftName.compare(m_microsoftProfile, Qt::CaseInsensitive) == 0
        && m_discordProfile.value("access").toObject().value("status") == "granted";
}

bool SanctuaryHome::communityUnlocked() const
{
    if (m_view == View::Setup) return false;
    if (m_onlineSession) return m_discordConnected && m_discordProfile.value("access").toObject().value("status") == "granted";
    if (m_view == View::Dashboard) return false;
    const bool installed = m_realPack ? m_packVerified : m_local.installed;
    return m_localMode && m_local.discord && m_local.microsoft && m_local.access == Access::Granted && installed && !m_packBusy;
}

void SanctuaryHome::openedImpl()
{
    refreshClock();
    if (communityUnlocked()) for (auto* feed : m_ui->feeds) feed->reload();
}

void SanctuaryHome::refreshClock()
{
    if (m_view != View::Dashboard) return;
    const QLocale locale(m_french ? QLocale::French : QLocale::English,
                         m_french ? QLocale::France : QLocale::UnitedKingdom);
    auto title = locale.toString(QDateTime::currentDateTime(), "dddd d MMMM yyyy · HH:mm");
    if (!title.isEmpty()) title[0] = title[0].toUpper();
    m_ui->subtitle->setText(title);
}

void SanctuaryHome::buildUi()
{
    auto& u = *m_ui;
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* surface = new QWidget(this);
    surface->setObjectName("surface");
    auto* horizontal = new QHBoxLayout(surface);
    u.contentLayout = horizontal;
    horizontal->setContentsMargins(12, 0, 12, 0);
    horizontal->addStretch();
    auto* column = new QWidget(surface);
    u.contentColumn = column;
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
    auto* wordmark = new SanctuaryWordmark(u.chromeHeader);
    wordmark->setObjectName("wordmark");
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
    u.heading->setProperty("dashboard", m_view == View::Dashboard);
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
        {"article","Gazette","Gazette",1,0,1}, {"notice","Tableau d’affichage","Notice board",1,1,1}};
    for (const auto& section : sections) {
        auto* card = new QFrame(u.community);
        card->setObjectName("communityCard-" + QString(section.kind));
        card->setProperty("communityCard", true);
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 12, 12, 12);
        cardLayout->setSpacing(5);
        auto* title = label(card, "communityTitle-" + QString(section.kind));
        title->setProperty("communityTitle", true);
        title->setProperty("labelFr", QString(section.fr));
        title->setProperty("labelEn", QString(section.en));
        u.feedTitles.append(title);
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

    u.dashboardEmpty = new QWidget(body);
    auto* empty = new QVBoxLayout(u.dashboardEmpty);
    empty->setContentsMargins(0,0,0,0);
    u.dashboardMessage = label(u.dashboardEmpty,"dashboardMessage");
    empty->addWidget(u.dashboardMessage);
    u.startSetup = button(u.dashboardEmpty,"startSetup");
    empty->addWidget(u.startSetup,0,Qt::AlignLeft);
    empty->addStretch();
    body->addWidget(u.dashboardEmpty);
    connect(u.startSetup,&QPushButton::clicked,this,[this] { emit instanceToolRequested("setup"); });

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
    u.play->setMinimumSize(88, 38);
    u.play->setProperty("primary", true);
    u.playModes = button(footer, "playModes");
    u.playModes->setFixedSize(30, 38);
    u.playModes->setText(QStringLiteral("▾"));
    auto* modes = new QMenu(u.playModes);
    u.playMinecraft = modes->addAction(QString());
    u.joinSanctuary = modes->addAction(QString());
    u.playMinecraft->setCheckable(true);
    u.joinSanctuary->setCheckable(true);
    u.playModes->setMenu(modes);
    auto* playGroup = new QHBoxLayout;
    playGroup->setSpacing(0);
    playGroup->addWidget(u.play);
    playGroup->addWidget(u.playModes);
    foot->addLayout(playGroup);
    foot->setAlignment(playGroup, Qt::AlignBottom);
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
    connect(u.setupToggle, &QPushButton::clicked, this, [this] {
        if (m_view == View::Dashboard) emit instanceToolRequested("setup");
        else if (m_view == View::Setup) emit instanceToolRequested("home");
        else emit accountsRequested();
    });
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
        if (hasMicrosoftSignIn()) { if (!m_gameBusy) emit microsoftSignInRequested(); return; }
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
    auto selectMode = [this](bool join) {
        setJoinOnPlay(join);
        emit playModeChanged(join);
    };
    connect(u.playMinecraft, &QAction::triggered, this, [selectMode] { selectMode(false); });
    connect(u.joinSanctuary, &QAction::triggered, this, [selectMode] { selectMode(true); });
    connect(u.play, &QPushButton::clicked, this, [this] {
        if (m_joinOnPlay ? canLaunchGame() : canPlayMinecraft()) emit playRequested(m_joinOnPlay);
        else if (!hasMicrosoftSignIn() && !m_onlineSession && !m_gameBusy && m_localMode && m_local.discord && m_local.microsoft && m_local.access == Access::Granted && (m_realPack ? m_packVerified && !m_packBusy : m_local.installed))
            emit localPlayRequested();
    });
    connect(u.tools, &QPushButton::clicked, this, &SanctuaryHome::advancedRequested);
    connect(u.settings, &QPushButton::clicked, this, &SanctuaryHome::settingsRequested);
    connect(u.mods, &QPushButton::clicked, this, [this] { emit instanceToolRequested("mods"); });
    connect(u.shaders, &QPushButton::clicked, this, [this] { emit instanceToolRequested("shaderpacks"); });
    connect(u.instanceSettings, &QPushButton::clicked, this, [this] { emit instanceToolRequested("settings"); });
    connect(u.news, &QPushButton::clicked, this, [this] { emit instanceToolRequested("news"); });
    connect(u.site, &QPushButton::clicked, this, [this] { emit linkRequested(QUrl("https://sanctuary-minecraft.net/")); });
    connect(u.wiki, &QPushButton::clicked, this, [this] { emit instanceToolRequested("wiki"); });
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
    if (m_onlineSession) {
        if (!m_discordConnected) return 1;
        if (m_discordProfile.value("access").toObject().value("status") != "granted") return 3;
        return m_microsoftVerified ? 5 : 2;
    }
    if (!m_localMode || !m_local.discord) return 1;
    if (hasMicrosoftSignIn() ? !m_microsoftVerified : !m_local.microsoft) return 2;
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
    const bool microsoft = hasMicrosoftSignIn() ? m_microsoftVerified : !m_onlineSession && m_localMode && m_local.microsoft;
    const bool admitted = m_onlineSession ? discord && access == Access::Granted : m_localMode && access == Access::Granted;
    const bool installed = m_realPack ? m_packVerified : m_localMode && m_local.installed;
    const bool installing = m_realPack ? m_packBusy : m_installTimer && m_installTimer->isActive();
    const int active = activeStep();
    const bool waiting = (m_onlineSession || m_localMode) && (access == Access::Pending || access == Access::Synchronizing);
    u.previewControls->setVisible(m_view != View::Dashboard && (m_localMode || m_onlineSession));
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
    if (m_view == View::Dashboard) u.heading->setText(displayName());
    u.subtitle->setText(m_onlineSession && communityUnlocked() ? text("Accès Sanctuary confirmé. Vos publications sont disponibles ; le jeu attend Microsoft.", "Sanctuary access confirmed. Your posts are available; the game awaits Microsoft.") : communityUnlocked() ? text("Les nouvelles de votre communauté.", "The latest from your community.") : subtitle);
    u.eyebrow->setVisible(m_view != View::Dashboard && !communityUnlocked());
    refreshClock();
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
        if (m_onlineSession) {
            m.detail->setText(text("Connexion en attente d’autorisation Microsoft/Mojang.", "Sign-in awaits Microsoft/Mojang approval."));
            m.action->setToolTip(m.detail->text());
        }
    }
    if (hasMicrosoftSignIn()) {
        const bool development = m_microsoftSignIn == MicrosoftSignIn::Development;
        const auto description = development ? text("Test réel Microsoft → Xbox → Minecraft disponible.", "Live Microsoft → Xbox → Minecraft test available.")
                                             : text("Connectez le compte qui possède Minecraft Java.", "Sign in with the account that owns Minecraft Java.");
        m.detail->setText(m_microsoftMessage.isEmpty()
            ? (m_microsoftVerified ? m_microsoftProfile : description)
            : m_microsoftMessage.section('\n',0,0));
        m.detail->setToolTip(m_microsoftMessage);
        const auto connectLabel = development ? text("Tester Microsoft · dev", "Test Microsoft · dev") : text("Connecter Microsoft", "Connect Microsoft");
        action(1, m_microsoftVerified ? text("Changer de compte", "Change account") : connectLabel, active == 2);
        m.action->setEnabled(m_microsoftAvailable && !m_gameBusy);
        m.action->setToolTip(text("Connexion réelle par code avec l’application Sanctuary.", "Live device-code sign-in with the Sanctuary application."));
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
    u.body->setCurrentWidget(ready ? u.community : m_view == View::Dashboard ? u.dashboardEmpty : u.setupPanel);
    u.dashboardMessage->setText(u.connection
        ? text("Suivez l’étape indiquée dans la barre latérale pour préparer Sanctuary.",
               "Follow the step shown in the sidebar to set up Sanctuary.")
        : text("Retrouvez votre accès et les publications de Sanctuary dans Bien démarrer.", "Find your Sanctuary access and posts in Getting started."));
    u.startSetup->setText(text("Bien démarrer", "Getting started"));
    u.startSetup->setVisible(!u.connection);
    u.setupToggle->setVisible(!u.connection && (ready || m_view != View::Adaptive));
    u.setupToggle->setText(m_view == View::Dashboard ? text("Bien démarrer", "Getting started") : m_view == View::Setup ? text("Voir l’accueil", "View home") : text("Mes comptes", "My accounts"));
    u.readyCodeToggle->setVisible(ready && m_view == View::Adaptive);
    u.readyCodeToggle->setText(m_codeVisible ? text("Masquer le code SGA", "Hide SGA code") : text("Code SGA · facultatif", "SGA code · optional"));
    if (m_onlineSession) u.readyCodeToggle->setText(text("Mon dossier Sanctuary", "My Sanctuary profile"));
    u.readyCodePanel->setVisible(!m_onlineSession && ready && m_codeVisible);
    u.readyCode->setText("SANCTUARYDEMO");
    u.readyCodeCopy->setText(text("Copier le code fictif", "Copy fictional code"));
    for (auto* title : u.feedTitles) title->setText(title->property(m_french ? "labelFr" : "labelEn").toString());
    for (auto* feed : u.feeds) feed->setFrench(m_french);
    if (ready && !m_communityVisible) for (auto* feed : u.feeds) feed->reload();
    m_communityVisible = ready;
    u.playMinecraft->setText(text("Jouer", "Play"));
    u.joinSanctuary->setText(text("Rejoindre Sanctuary", "Join Sanctuary"));
    u.playMinecraft->setChecked(!m_joinOnPlay);
    u.joinSanctuary->setChecked(m_joinOnPlay);
    u.playModes->setAccessibleName(text("Mode de lancement", "Launch mode"));
    u.playModes->setToolTip(u.playModes->accessibleName());
    u.playModes->setEnabled(!m_gameBusy);
    const bool canPlay = m_joinOnPlay ? canLaunchGame() : canPlayMinecraft();
    u.play->setText(m_gameBusy ? text("Jeu en cours", "Game running") : (m_joinOnPlay ? u.joinSanctuary : u.playMinecraft)->text());
    u.play->setEnabled(canPlay || (!m_gameBusy && !hasMicrosoftSignIn() && !m_onlineSession && m_localMode && discord && microsoft && admitted && installed && !installing));
    u.play->setToolTip(text("En mode local, ce bouton vérifie le parcours sans lancer Minecraft.", "In local mode, this checks the flow without launching Minecraft."));
    if (m_onlineSession) u.play->setToolTip(text("Le lancement attend le raccordement Microsoft/Mojang.", "Game launch awaits Microsoft/Mojang integration."));
    u.help->setText(m_realPack ? text("Comptes et accès locaux. Les essais du jeu et du serveur attendent Mojang.",
                                    "Local accounts and access. Game and server tests await Mojang.")
                    : m_localMode ? text("Simulation locale uniquement. Minecraft, serveur et personnage seront testés après l'approbation Mojang.",
                                                  "Local simulation only. Minecraft, server and character tests await Mojang approval.")
                                    : text("Connexion et accès au serveur en attente du raccordement Microsoft/Mojang.", "Sign-in and server access await Microsoft/Mojang integration."));
    if (m_onlineSession) u.help->setText(text("Discord et accès réels · lancement Minecraft en attente de Microsoft/Mojang.", "Real Discord account and access · Minecraft launch awaits Microsoft/Mojang."));
    if (hasMicrosoftSignIn()) {
        u.help->setText(text("Connectez vos comptes et vérifiez le pack pour rejoindre Sanctuary.", "Connect your accounts and verify the pack to join Sanctuary."));
        u.play->setToolTip(u.help->text());
        if (m_onlineSession && m_microsoftVerified && admitted) {
            const auto name = m_discordProfile.value("user").toObject().value("minecraft_username").toString();
            if (name.isEmpty())
                u.help->setText(text("Le dossier Sanctuary ne fournit pas de pseudo Minecraft. Actualisez votre accès.", "Your Sanctuary profile has no Minecraft name. Refresh your access."));
            else if (name.compare(m_microsoftProfile, Qt::CaseInsensitive) != 0)
                u.help->setText(text("Compte Minecraft : %1 · Dossier Sanctuary : %2. Connectez le compte correspondant.", "Minecraft account: %1 · Sanctuary profile: %2. Connect the matching account.").arg(m_microsoftProfile, name));
        }
        if (m_joinOnPlay && canLaunchGame()) {
            u.help->setText(text("Prêt à rejoindre Sanctuary avec %1.", "Ready to join Sanctuary as %1.").arg(m_microsoftProfile));
            u.play->setToolTip(text("Rejoindre directement le serveur Sanctuary Beta.", "Join the Sanctuary Beta server directly."));
        }
        if (!m_joinOnPlay && canPlayMinecraft()) {
            u.help->setText(text("Minecraft s’ouvrira sur son menu principal.", "Minecraft will open at its main menu."));
            u.play->setToolTip(u.help->text());
        }
    }
    if (m_gameBusy) {
        u.help->setText(text("Le jeu est en cours de lancement ou ouvert. Les journaux restent disponibles.", "The game is starting or running. Game logs remain available."));
        u.play->setToolTip(u.help->text());
    }
    if (m_gameBusy) u.rows[4].action->setEnabled(false);
    if (u.connection) u.rows[4].action->hide();
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
    const int connectionStep = !discord ? 1 : !microsoft ? 2 : !admitted ? 3 : 5;
    refreshConnectionButton(connectionStep, discord || microsoft || admitted || installed, discord && microsoft && admitted && installed);
}
