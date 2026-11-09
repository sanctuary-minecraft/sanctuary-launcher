// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryWorkspace.h"
#include "SanctuaryWordmark.h"
#include "ui/pages/BasePage.h"
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

static void initializeWorkspaceResources() { Q_INIT_RESOURCE(sanctuary); }

SanctuaryWorkspace::SanctuaryWorkspace(QWidget* parent) : QWidget(parent)
{
    initializeWorkspaceResources();
    setObjectName("sanctuaryWorkspace");
    setStyleSheet(R"(
        #sanctuaryWorkspace, #sanctuaryWorkspace QWidget { background: #fff; color: #1f2328; font-size: 13px; }
        #sanctuaryWorkspace QLabel { background: transparent; border: none; }
        #sanctuaryWorkspace QLabel#workspaceTitle { font-size: 23px; font-weight: 600; }
        #sanctuaryWorkspace QLabel#workspaceFeedback { color: #a40e26; }
        #sanctuaryWorkspace QPushButton, #sanctuaryWorkspace QToolButton { border: 1px solid #d0d7de; border-radius: 4px; padding: 5px 10px; background: #f6f8fa; }
        #sanctuaryWorkspace QPushButton:hover, #sanctuaryWorkspace QToolButton:hover { background: #eef1f4; }
        #sanctuaryWorkspace QPushButton:focus { border: 2px solid #0969da; }
        #sanctuaryWorkspace QPushButton:disabled { color: #8c959f; }
        #sanctuaryWorkspace QPushButton[nav="true"] { border: none; text-align: left; background: transparent; min-height: 23px; }
        #sanctuaryWorkspace QPushButton[nav="true"]:checked { color: #1a7f37; background: #dafbe1; font-weight: 600; }
        #sanctuaryWorkspace QLineEdit, #sanctuaryWorkspace QTextEdit, #sanctuaryWorkspace QPlainTextEdit, #sanctuaryWorkspace QComboBox, #sanctuaryWorkspace QSpinBox { border: 1px solid #d0d7de; border-radius: 4px; padding: 4px; background: white; color: #1f2328; }
        #sanctuaryWorkspace QTreeView, #sanctuaryWorkspace QListView, #sanctuaryWorkspace QTableView { background: white; alternate-background-color: #f6f8fa; border: 1px solid #d0d7de; selection-background-color: #ddf4ff; selection-color: #1f2328; }
        #sanctuaryWorkspace QHeaderView::section { background: #f6f8fa; color: #57606a; border: none; border-bottom: 1px solid #d0d7de; padding: 5px; }
        #sanctuaryWorkspace QScrollArea { border: none; }
        #sanctuaryWorkspace QGroupBox { border: 1px solid #d0d7de; border-radius: 4px; margin-top: 12px; padding-top: 12px; }
        #sanctuaryWorkspace QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }
    )");
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

SanctuaryWorkspace::~SanctuaryWorkspace()
{
    if (m_current) m_current->closed();
}

QString SanctuaryWorkspace::name(BasePage* page) const
{
    const QHash<QString, QPair<QString, QString>> names{
        {"home", {"Accueil", "Home"}}, {"setup", {"Bien démarrer", "Getting started"}}, {"news", {"Actualités", "News"}},
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
    appendPages(pages);
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
        nav->setAutoExclusive(true);
        m_navigation.insert(page->id(), nav);
        m_navLayout->insertWidget(m_navLayout->count() - 1, nav);
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
    for (auto* page : m_pages) m_navigation.value(page->id())->setText(name(page));
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
    m_save->setVisible(id != "home" && id != "news");
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

void SanctuaryWorkspace::setGameBusy(bool busy)
{
    m_save->setEnabled(!busy);
    for (auto* page : m_pages) {
        if (page->id() != "home" && page->id() != "news" && page->id() != "setup") {
            dynamic_cast<QWidget*>(page)->setEnabled(!busy);
            m_navigation.value(page->id())->setEnabled(!busy);
        }
    }
}
