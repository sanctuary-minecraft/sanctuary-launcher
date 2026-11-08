// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryHome.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QUrl>
#include <QVBoxLayout>

// Explicitly reference the static resource object, including in standalone Qt tests.
static void initializeSanctuaryResources() { Q_INIT_RESOURCE(sanctuary); }

namespace {
class Island : public QWidget {
public:
    explicit Island(QWidget* parent) : QWidget(parent), m_image(":/sanctuary/island.png")
    {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setAccessibleName("Sanctuary");
    }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const auto scaled = m_image.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap((width() - scaled.width()) / 2, (height() - scaled.height()) / 2, scaled);
    }
private:
    QPixmap m_image;
};

QLabel* label(const QString& value, QWidget* parent, const QString& name = {})
{
    auto* result = new QLabel(value, parent);
    result->setObjectName(name);
    result->setWordWrap(true);
    result->setTextFormat(Qt::PlainText);
    return result;
}

QPushButton* button(const QString& value, QWidget* parent, const QString& name)
{
    auto* result = new QPushButton(value, parent);
    result->setObjectName(name);
    result->setCursor(Qt::PointingHandCursor);
    return result;
}
}

SanctuaryHome::SanctuaryHome(QWidget* parent) : QWidget(parent)
{
    initializeSanctuaryResources();
    setObjectName("sanctuaryHome");
    setStyleSheet(R"(
        QWidget#sanctuaryHome { color: #fff; font-family: Menlo, Consolas, monospace; font-size: 14px; }
        #sanctuaryHome QWidget { font-family: Menlo, Consolas, monospace; }
        #sanctuaryHome QLabel { color: #fff; background: transparent; border: none; }
        #sanctuaryHome QFrame#frame { background: rgba(0, 26, 39, 180); border: 1px solid rgba(255,255,255,150); }
        #sanctuaryHome QFrame#sidebar { background: rgba(0,0,0,20); border-right: 1px solid rgba(255,255,255,120); }
        #sanctuaryHome QFrame#header, #sanctuaryHome QFrame#article { border-bottom: 1px solid rgba(255,255,255,100); }
        #sanctuaryHome QFrame#launch { border-left: 1px solid rgba(255,255,255,120); }
        #sanctuaryHome QFrame#footer { border-top: 1px solid rgba(255,255,255,120); }
        #sanctuaryHome QPushButton { color: white; font-size: 14px; background: transparent; border: none; padding: 10px 3px; text-align: left; }
        #sanctuaryHome QPushButton:hover, #sanctuaryHome QPushButton:focus { color: #ffff00; }
        #sanctuaryHome QPushButton:focus { outline: none; border: 1px solid #ffff00; }
        #sanctuaryHome QPushButton#selected, #sanctuaryHome QPushButton#account, #sanctuaryHome QPushButton#primary {
            border-image: url(:/sanctuary/button.png) 2 2 2 2; border-width: 3px; border-style: solid;
            text-align: center; padding: 12px; font-weight: bold;
        }
        #sanctuaryHome QPushButton:disabled { color: #b9b9b9; }
        #sanctuaryHome QPushButton#primary { font-size: 28px; padding: 22px 10px; }
        #sanctuaryHome QLabel#heading { font-size: 19px; font-weight: bold; }
        #sanctuaryHome QLabel#title { font-size: 15px; font-weight: bold; }
        #sanctuaryHome QLabel#muted { color: #d0dce2; font-size: 12px; }
        #sanctuaryHome QScrollArea { background: transparent; border: none; }
        #sanctuaryHome QScrollArea > QWidget > QWidget { background: transparent; }
    )");
    rebuild();
}

QString SanctuaryHome::text(const char* french, const char* english) const
{
    return QString::fromUtf8(m_french ? french : english);
}

void SanctuaryHome::setState(bool ready, bool microsoft, bool french)
{
    if (ready == m_instanceReady && microsoft == m_microsoftAvailable && french == m_french)
        return;
    m_instanceReady = ready;
    m_microsoftAvailable = microsoft;
    m_french = french;
    rebuild();
}

void SanctuaryHome::rebuild()
{
    // This view owns its widgets; rebuilding after a language/settings change
    // never touches an instance, account, or game configuration.
    delete layout();
    const auto children = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (auto* child : children)
        delete child;
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(22, 22, 22, 22);
    auto* frame = new QFrame(this);
    frame->setObjectName("frame");
    outer->addWidget(frame);
    auto* grid = new QGridLayout(frame);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);
    grid->setColumnMinimumWidth(0, 205);
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(0, 1);

    auto* sidebar = new QFrame(frame);
    sidebar->setObjectName("sidebar");
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(18, 22, 18, 16);
    auto* wordmark = new QLabel(sidebar);
    wordmark->setPixmap(QPixmap(":/sanctuary/wordmark.png").scaledToWidth(177, Qt::SmoothTransformation));
    wordmark->setAccessibleName("Sanctuary Launcher");
    side->addWidget(wordmark);
    side->addWidget(label("L A U N C H E R", sidebar, "muted"), 0, Qt::AlignHCenter);
    side->addSpacing(26);
    side->addWidget(button(text("ACCUEIL", "HOME"), sidebar, "selected"));
    const auto addLink = [this](QBoxLayout* layout, QWidget* parent, const QString& title, const QString& path) {
        auto* link = button(title, parent, "link");
        layout->addWidget(link);
        connect(link, &QPushButton::clicked, this, [this, path] {
            emit linkRequested(QUrl("https://sanctuary-minecraft.net" + path));
        });
    };
    addLink(side, sidebar, text("ACTUALITÉS", "NEWS"), "/actualites");
    addLink(side, sidebar, "WIKI", "/wiki");
    addLink(side, sidebar, text("COMMUNAUTÉ", "COMMUNITY"), "/communaute");
    side->addStretch();
    auto* settings = button(text("RÉGLAGES", "SETTINGS"), sidebar, "settings");
    side->addWidget(settings);
    connect(settings, &QPushButton::clicked, this, &SanctuaryHome::settingsRequested);
    auto* advanced = button(text("Outils avancés Prism", "Advanced Prism tools"), sidebar, "advanced");
    side->addWidget(advanced);
    connect(advanced, &QPushButton::clicked, this, &SanctuaryHome::advancedRequested);
    grid->addWidget(sidebar, 0, 0);

    auto* main = new QWidget(frame);
    auto* body = new QVBoxLayout(main);
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);
    auto* header = new QFrame(main);
    header->setObjectName("header");
    auto* top = new QHBoxLayout(header);
    top->setContentsMargins(24, 14, 18, 14);
    top->addWidget(label(text("ACCUEIL", "HOME"), header));
    top->addStretch();
    top->addWidget(label(text("Non connecté", "Not signed in"), header));
    top->addSpacing(18);
    auto* account = button(text("Se connecter", "Sign in"), header, "account");
    account->setEnabled(m_microsoftAvailable);
    account->setToolTip(text("La connexion Microsoft sera raccordée à l'application Sanctuary.",
                             "Microsoft sign-in will be connected to the Sanctuary application."));
    connect(account, &QPushButton::clicked, this, &SanctuaryHome::accountsRequested);
    top->addWidget(account);
    body->addWidget(header);
    body->addWidget(new Island(main), 7);

    auto* lower = new QWidget(main);
    auto* columns = new QHBoxLayout(lower);
    columns->setSpacing(0);
    columns->setContentsMargins(0, 0, 0, 0);
    auto* news = new QWidget(lower);
    auto* newsLayout = new QVBoxLayout(news);
    newsLayout->setContentsMargins(22, 16, 22, 12);
    newsLayout->setSpacing(4);
    newsLayout->addWidget(label(text("DERNIÈRES ACTUALITÉS", "LATEST NEWS"), news, "heading"));
    auto* scroll = new QScrollArea(news);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* list = new QWidget(scroll);
    auto* entries = new QVBoxLayout(list);
    entries->setContentsMargins(0, 0, 0, 0);
    entries->setSpacing(0);
    struct Article { const char* version; const char* date; const char* titleFr; const char* titleEn; const char* fr; const char* en; };
    const Article articles[] = {
        {"beta.372", "2026-11-07", "Les emotes arrivent dans le chat", "Emotes join the chat",
         "BTTV, Twitch, FrankerFaceZ et 7TV rejoignent Sanctuary.", "BTTV, Twitch, FrankerFaceZ and 7TV join Sanctuary."},
        {"beta.370", "2026-11-06", "Présence Discord et carte du monde", "Discord presence and world map",
         "Votre activité Discord et la carte du monde.", "Your Discord activity and the world map."},
        {"beta.369", "2026-11-05", "De nouvelles terres à explorer", "New lands to explore",
         "De nouveaux lieux et des écrans vidéo mis à jour.", "New places and updated video screens."}
    };
    for (const auto& article : articles) {
        auto* row = new QFrame(list);
        row->setObjectName("article");
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 12, 0, 12);
        auto* date = label(QString::fromUtf8(article.version) + "\n" + article.date, row, "muted");
        date->setFixedWidth(92);
        rowLayout->addWidget(date, 0, Qt::AlignTop);
        auto* content = new QVBoxLayout;
        content->setSpacing(5);
        auto* title = button(text(article.titleFr, article.titleEn), row, "articleLink");
        title->setStyleSheet("font-weight: bold; padding: 0;");
        content->addWidget(title);
        content->addWidget(label(text(article.fr, article.en), row));
        const QString fragment = QString::fromUtf8(article.version).replace('.', '_');
        connect(title, &QPushButton::clicked, this, [this, fragment] {
            emit linkRequested(QUrl("https://sanctuary-minecraft.net/actualites#version-" + fragment));
        });
        rowLayout->addLayout(content, 1);
        entries->addWidget(row);
    }
    entries->addStretch();
    scroll->setWidget(list);
    newsLayout->addWidget(scroll, 1);
    addLink(newsLayout, news, text("Toutes les actualités", "All news"), "/actualites");
    columns->addWidget(news, 2);

    auto* launch = new QFrame(lower);
    launch->setObjectName("launch");
    launch->setMinimumWidth(285);
    launch->setMaximumWidth(390);
    auto* launchLayout = new QVBoxLayout(launch);
    launchLayout->setContentsMargins(24, 20, 24, 20);
    launchLayout->setSpacing(14);
    launchLayout->addWidget(label("SANCTUARY BETA", launch, "heading"));
    launchLayout->addWidget(label(m_instanceReady ? text("Profil Sanctuary préparé", "Sanctuary profile ready")
                                                : text("Pack non installé", "Pack not installed"), launch));
    launchLayout->addWidget(label(text("Statut du serveur non vérifié", "Server status not checked"), launch, "muted"));
    launchLayout->addStretch();
    auto* primary = button(m_instanceReady ? text("JOUER", "PLAY") : text("INSTALLER", "INSTALL"), launch, "primary");
    primary->setEnabled(!m_instanceReady || m_microsoftAvailable);
    connect(primary, &QPushButton::clicked, this, [this] {
        if (!m_instanceReady) emit installRequested();
        else if (m_microsoftAvailable) emit playRequested();
    });
    launchLayout->addWidget(primary);
    launchLayout->addWidget(label(m_instanceReady
        ? (m_microsoftAvailable ? text("Se connecter pour lancer Sanctuary.", "Sign in to launch Sanctuary.")
                                : text("Connexion Microsoft en attente. Votre profil est conservé.", "Microsoft sign-in is pending. Your profile is preserved."))
        : text("Préparer le pack avant de connecter votre compte.", "Prepare the pack before connecting your account."), launch, "muted"));
    launchLayout->addStretch();
    columns->addWidget(launch, 1);
    body->addWidget(lower, 6);
    grid->addWidget(main, 0, 1);

    auto* footer = new QFrame(frame);
    footer->setObjectName("footer");
    auto* foot = new QHBoxLayout(footer);
    foot->setContentsMargins(20, 6, 20, 6);
    foot->addWidget(label("Sanctuary Launcher 0.1.0 · Prism 11.1.1", footer, "muted"));
    foot->addStretch();
    addLink(foot, footer, "Site Sanctuary", "/");
    grid->addWidget(footer, 1, 0, 1, 2);
}

void SanctuaryHome::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QPixmap sky(":/sanctuary/sky.png");
    painter.drawPixmap(rect(), sky);
}
