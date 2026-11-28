// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryWorkspace.h"
#include "SanctuaryAppearance.h"
#include "SanctuaryFeed.h"
#include "SanctuaryWordmark.h"
#include "ui/pages/BasePage.h"
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

static void initializeWorkspaceResources() { Q_INIT_RESOURCE(sanctuary); }

static QString navigationGroup(const QString& page)
{
    if (page == "home") return QStringLiteral("play");
    if (page == "news" || page == "map" || page == "wiki") return QStringLiteral("community");
    return QStringLiteral("game");
}

SanctuaryWorkspace::SanctuaryWorkspace(QWidget* parent) : QWidget(parent), m_navSelection(new QButtonGroup(this))
{
    initializeWorkspaceResources();
    setObjectName("sanctuaryWorkspace");
    new SanctuaryAppearance::PaletteWatcher(this, [this] { applyAppearance(); });
    auto* body = new QHBoxLayout(this);
    body->setContentsMargins(22, 18, 22, 18);
    auto* sidebar = new QVBoxLayout;
    auto* wordmark = new SanctuaryWordmark(this);
    wordmark->setObjectName("workspaceWordmark");
    sidebar->addWidget(wordmark, 0, Qt::AlignLeft | Qt::AlignTop);
    auto* navScroll = new QScrollArea(this);
    navScroll->setWidgetResizable(true);
    navScroll->setFixedWidth(210);
    auto* nav = new QWidget(navScroll);
    m_navLayout = new QVBoxLayout(nav);
    m_navLayout->setContentsMargins(0, 10, 10, 0);
    m_navLayout->setSpacing(2);
    m_navLayout->addStretch();
    navScroll->setWidget(nav);
    sidebar->addWidget(navScroll, 1);
    body->addLayout(sidebar);
    auto* right = new QVBoxLayout;
    m_toolbar = new QWidget(this);
    auto* header = new QHBoxLayout(m_toolbar);
    header->setContentsMargins(0, 0, 0, 0);
    m_title = new QLabel(m_toolbar);
    m_title->setObjectName("workspaceTitle");
    header->addWidget(m_title, 1);
    m_home = new QPushButton(m_toolbar);
    m_home->setObjectName("workspaceHome");
    header->addWidget(m_home);
    m_save = new QPushButton(m_toolbar);
    m_save->setObjectName("workspaceSave");
    header->addWidget(m_save);
    right->addWidget(m_toolbar);
    m_feedback = new QLabel(this);
    m_feedback->setObjectName("workspaceFeedback");
    m_feedback->setWordWrap(true);
    m_feedback->hide();
    right->addWidget(m_feedback);
    m_stack = new QStackedWidget(this);
    right->addWidget(m_stack, 1);
    body->addLayout(right, 1);
    connect(m_home, &QPushButton::clicked, this, [this] { requestClose(); });
    connect(m_save, &QPushButton::clicked, this, [this] { if (saveAll()) emit settingsApplied(); });
    setFrench(true);
}

void SanctuaryWorkspace::applyAppearance()
{
    SanctuaryAppearance::apply(this, QStringLiteral(R"(
        #sanctuaryWorkspace, #sanctuaryWorkspace QWidget { background: @surface; color: @text; font-size: 13px; }
        #sanctuaryWorkspace QLabel { background: transparent; border: none; }
        #sanctuaryWorkspace QLabel#workspaceTitle { font-size: 23px; font-weight: 600; }
        #sanctuaryWorkspace QLabel#workspaceFeedback { color: @errorText; }
        #sanctuaryWorkspace QPushButton, #sanctuaryWorkspace QToolButton { border: 1px solid @border; border-radius: 4px; padding: 5px 10px; background: @panel; }
        #sanctuaryWorkspace QPushButton:hover, #sanctuaryWorkspace QToolButton:hover { background: @hover; }
        #sanctuaryWorkspace QPushButton:focus { border: 2px solid @link; }
        #sanctuaryWorkspace QPushButton:disabled { color: @disabledText; }
        #sanctuaryWorkspace QPushButton[nav="true"] { border: none; text-align: left; background: transparent; min-height: 23px; }
        #sanctuaryWorkspace QPushButton[nav="true"]:checked { color: @greenText; background: @greenPanel; font-weight: 600; }
        #sanctuaryWorkspace QFrame[navGroup] { border: 1px solid @border; border-radius: 7px; }
        #sanctuaryWorkspace QFrame[navGroup="community"] { border-color: @navCommunityBorder; }
        #sanctuaryWorkspace QFrame[navGroup="play"] { border-color: @navGreenBorder; }
        #sanctuaryWorkspace QPushButton[navGroup] {
            border: 1px solid transparent; border-radius: 4px; min-height: 21px; padding: 4px 8px;
        }
        #sanctuaryWorkspace QPushButton[navGroup]:focus { border-color: @link; }
        #sanctuaryWorkspace QPushButton[navGroup="community"]:checked,
        #sanctuaryWorkspace QPushButton[navGroup="game"]:checked { background: @panel; color: @text; }
        #sanctuaryWorkspace QPushButton[navGroup="community"]:hover { background: @selected; }
        #sanctuaryWorkspace QPushButton[navGroup="game"]:hover { background: @navGameHover; }
        #sanctuaryWorkspace QPushButton[navGroup="play"],
        #sanctuaryWorkspace QPushButton[progress="complete"] { background: @greenPanel; color: @greenText; font-weight: 600; }
        #sanctuaryWorkspace QPushButton[navGroup="play"]:hover,
        #sanctuaryWorkspace QPushButton[progress="complete"]:hover { background: @navGreenHover; }
        #sanctuaryWorkspace QPushButton[navGroup="play"] { min-height: 28px; font-size: 15px; }
        #sanctuaryWorkspace QFrame[navGroup="play"][playReady="false"] { border-color: @border; }
        #sanctuaryWorkspace QPushButton[navGroup="play"][playReady="false"],
        #sanctuaryWorkspace QPushButton[navGroup="play"][playReady="false"]:checked,
        #sanctuaryWorkspace QPushButton[navGroup="play"][playReady="false"]:hover { background: @disabledPanel; color: @muted; }
        #sanctuaryWorkspace QPushButton[navGroup="connection"] { text-align: left; border-radius: 7px; padding: 6px 12px; }
        #sanctuaryWorkspace QPushButton[progress="complete"] { border-color: @navGreenBorder; }
        #sanctuaryWorkspace QPushButton[progress="start"] { border-color: @navErrorBorder; }
        #sanctuaryWorkspace QPushButton[progress="partial"] { border-color: @navPendingBorder; }
        #sanctuaryWorkspace QPushButton[progress="start"] { background: @errorPanel; color: @errorText; font-weight: 600; }
        #sanctuaryWorkspace QPushButton[progress="start"]:hover { background: @navErrorHover; }
        #sanctuaryWorkspace QPushButton[progress="partial"] { background: @alertRow; color: @alertText; font-weight: 600; }
        #sanctuaryWorkspace QPushButton[progress="partial"]:hover { background: @alertPanel; }
        #sanctuaryWorkspace QPushButton[navGroup]:disabled { color: @disabledText; }
        #sanctuaryWorkspace QPushButton[navGroup="connection"]:focus { border-color: @link; }
        #sanctuaryWorkspace QLineEdit, #sanctuaryWorkspace QTextEdit, #sanctuaryWorkspace QPlainTextEdit, #sanctuaryWorkspace QComboBox, #sanctuaryWorkspace QSpinBox { border: 1px solid @border; border-radius: 4px; padding: 4px; background: @surface; color: @text; }
        #sanctuaryWorkspace QTreeView, #sanctuaryWorkspace QListView, #sanctuaryWorkspace QTableView { background: @surface; alternate-background-color: @panel; border: 1px solid @border; selection-background-color: @selected; selection-color: @text; }
        #sanctuaryWorkspace QHeaderView::section { background: @panel; color: @muted; border: none; border-bottom: 1px solid @border; padding: 5px; }
        #sanctuaryWorkspace QScrollArea { border: none; }
        #sanctuaryWorkspace QGroupBox { border: 1px solid @border; border-radius: 4px; margin-top: 12px; padding-top: 12px; }
        #sanctuaryWorkspace QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
    )"));
}

SanctuaryWorkspace::~SanctuaryWorkspace()
{
    if (m_current) m_current->closed();
}

QString SanctuaryWorkspace::name(BasePage* page) const
{
    const QHash<QString, QPair<QString, QString>> names{
        {"home", {"Jouer", "Play"}}, {"news", {"Actualités", "News"}},
        {"console", {"Journaux du jeu", "Game logs"}}, {"version", {"Versions", "Versions"}},
        {"mods", {"Mods", "Mods"}}, {"resourcepacks", {"Packs de ressources", "Resource packs"}},
        {"shaderpacks", {"Packs de shaders", "Shader packs"}}, {"screenshots", {"Captures d'écran", "Screenshots"}},
        {"worlds", {"Mondes", "Worlds"}}, {"servers", {"Serveurs", "Servers"}},
        {"settings", {"Réglages du jeu", "Game settings"}}, {"notes", {"Notes", "Notes"}},
        {"logs", {"Autres journaux", "Other logs"}}, {"Other-Logs", {"Autres journaux", "Other logs"}},
        {"launcher-settings", {"Général", "General"}}, {"language-settings", {"Langue", "Language"}},
        {"appearance-settings", {"Apparence", "Appearance"}}, {"java-settings", {"Java et mémoire", "Java and memory"}},
        {"minecraft-settings", {"Minecraft", "Minecraft"}}, {"accounts", {"Comptes Microsoft", "Microsoft accounts"}},
        {"apis", {"Services de contenu", "Content services"}}, {"external-tools", {"Outils externes", "External tools"}},
        {"proxy-settings", {"Réseau", "Network"}}};
    if (names.contains(page->id())) return m_french ? names[page->id()].first : names[page->id()].second;
    return page->displayName();
}

void SanctuaryWorkspace::setPages(const QList<BasePage*>& pages, bool global)
{
    Q_ASSERT(m_pages.isEmpty()); // Each workspace owns a single fixed set of pages and one instance.
    m_global = global;
    if (!m_global) {
        m_navLayout->setSpacing(10);
        for (const QString& group : {QStringLiteral("play"), QStringLiteral("community"), QStringLiteral("game")}) {
            auto* frame = new QFrame(m_navLayout->parentWidget());
            frame->setObjectName(group + "Group");
            frame->setProperty("navGroup", group);
            auto* layout = new QVBoxLayout(frame);
            layout->setContentsMargins(4, 4, 4, 4);
            layout->setSpacing(1);
            m_navGroups.insert(group, layout);
            m_navLayout->insertWidget(m_navLayout->count() - 1, frame);
        }
    }
    appendPages(pages);
}

void SanctuaryWorkspace::setConnectionButton(QPushButton* button)
{
    button->setProperty("navGroup", "connection");
    m_navLayout->insertWidget(0, button);
}

void SanctuaryWorkspace::setPlayReady(bool ready)
{
    auto* button = m_navigation.value("home");
    if (!button) return;
    for (auto* widget : {static_cast<QWidget*>(button), button->parentWidget()}) {
        if (widget->property("playReady") == QVariant(ready)) continue;
        widget->setProperty("playReady", ready);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
}

void SanctuaryWorkspace::appendPages(const QList<BasePage*>& pages)
{
    for (auto* page : pages) {
        if (getPage(page->id())) continue;
        m_pages.append(page);
        auto* widget = dynamic_cast<QWidget*>(page);
        Q_ASSERT(widget);
        page->setParentContainer(this);
        m_stack->addWidget(widget);
        auto* nav = new QPushButton(name(page), this);
        nav->setObjectName("nav-" + page->id());
        nav->setProperty("nav", true);
        nav->setCheckable(true);
        m_navSelection->addButton(nav);
        m_navigation.insert(page->id(), nav);
        const auto group = navigationGroup(page->id());
        if (auto* layout = m_navGroups.value(group)) {
            nav->setProperty("navGroup", group);
            layout->addWidget(nav);
        } else {
            m_navLayout->insertWidget(m_navLayout->count() - 1, nav);
        }
        connect(nav, &QPushButton::toggled, this, [this, page](bool checked) { if (checked) selectPage(page->id()); });
        page->updateExtraInfo = [this, page](const QString&, const QString& info) {
            if (m_current == page) m_title->setText(name(page) + info);
        };
    }
    refreshContainer();
    setFrench(m_french);
}

void SanctuaryWorkspace::setFrench(bool french)
{
    m_french = french;
    m_home->setText(french ? "Accueil" : "Home");
    m_save->setText(french ? "Enregistrer" : "Save");
    m_home->setVisible(m_global || !getPage("home"));
    for (auto* page : m_pages) {
        if (auto* site = dynamic_cast<SanctuarySitePage*>(page)) site->setFrench(french);
        m_navigation.value(page->id())->setText(name(page));
    }
    if (m_current) m_title->setText(name(m_current));
}

BasePage* SanctuaryWorkspace::getPage(QString id)
{
    for (auto* page : m_pages) if (page->id() == id) return page;
    return nullptr;
}

bool SanctuaryWorkspace::selectPage(QString id)
{
    auto* next = getPage(id);
    if (!next || !next->shouldDisplay()) return false;
    if (m_current == next) return true;
    if (m_current && m_current != next) {
        if (!m_current->apply()) {
            m_navigation.value(next->id())->setChecked(false);
            m_navigation.value(m_current->id())->setChecked(true);
            return false;
        }
        m_current->closed();
    }
    m_current = next;
    m_visited.insert(next);
    m_stack->setCurrentWidget(dynamic_cast<QWidget*>(next));
    for (auto it = m_navigation.begin(); it != m_navigation.end(); ++it) it.value()->setChecked(it.key() == id);
    m_title->setText(name(next));
    m_title->setVisible(id != "home" && id != "setup"); // Home owns the current-step header.
    m_toolbar->setVisible(id != "home" && id != "setup");
    m_save->setVisible(id != "home" && id != "news" && id != "map" && id != "wiki");
    m_feedback->clear();
    m_feedback->hide();
    next->opened();
    return true;
}

void SanctuaryWorkspace::refreshContainer()
{
    BasePage* first = nullptr;
    for (auto* page : m_pages) {
        const bool visible = page->shouldDisplay();
        m_navigation.value(page->id())->setVisible(visible);
        if (!first && visible) first = page;
    }
    for (auto* layout : m_navGroups) {
        bool visible = false;
        for (int i = 0; i < layout->count(); ++i)
            visible |= !layout->itemAt(i)->widget()->isHidden();
        layout->parentWidget()->setVisible(visible);
    }
    if ((!m_current || !m_current->shouldDisplay()) && first) selectPage(first->id());
}

bool SanctuaryWorkspace::saveAll()
{
    // Unvisited native settings pages can contain fallback defaults (e.g. a
    // language catalogue still downloading). Do not overwrite saved preferences.
    for (auto* page : m_pages) if (m_visited.contains(page) && !page->apply()) {
        selectPage(page->id());
        m_feedback->setText(m_french ? "Vérifiez les réglages de cette page." : "Check the settings on this page.");
        m_feedback->show();
        return false;
    }
    m_feedback->clear();
    m_feedback->hide();
    return true;
}

bool SanctuaryWorkspace::requestClose()
{
    if (!saveAll()) return false;
    if (m_global) emit settingsApplied();
    emit homeRequested();
    return true;
}

void SanctuaryWorkspace::setGameBusy(bool synchronizing, bool running)
{
    m_save->setEnabled(!synchronizing && !running);
    for (auto* page : m_pages) {
        if (page->id() != "home" && page->id() != "news" && page->id() != "setup") {
            const bool readOnly = QStringList{"console", "logs", "screenshots"}.contains(page->id());
            const bool enabled = !synchronizing && (!running || readOnly);
            dynamic_cast<QWidget*>(page)->setEnabled(enabled);
            m_navigation.value(page->id())->setEnabled(enabled);
        }
    }
}
