// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryFeed.h"
#include "SanctuarySession.h"

#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QNetworkReply>
#include <QPushButton>
#ifdef Q_OS_WIN
#include <webview/webview.h>
#else
#include <QQuickItem>
#include <QQuickView>
#endif
#include <QRegularExpression>
#include <QVBoxLayout>

SanctuarySitePage::SanctuarySitePage(Section section, SanctuarySession* session, QWidget* parent)
    : QWidget(parent), m_section(section), m_session(session), m_connect(new QPushButton(this)),
      m_reload(new QPushButton(this)), m_back(new QPushButton(this)), m_status(new QLabel(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* actions = new QHBoxLayout;
    actions->addWidget(m_back);
    actions->addWidget(m_connect);
    actions->addWidget(m_reload);
    actions->addStretch();
    layout->addLayout(actions);
    layout->addWidget(m_status);
    layout->addStretch();
    m_status->setWordWrap(true);
    connect(m_connect, &QPushButton::clicked, m_session, &SanctuarySession::start);
    connect(m_reload, &QPushButton::clicked, this, &SanctuarySitePage::reload);
    connect(m_back, &QPushButton::clicked, this, [this] { evaluate(QStringLiteral("history.back()")); });
    connect(m_session, &SanctuarySession::credentialsChanged, this, [this] {
        if (m_section != Section::Map) return;
        m_loaded = false;
        if (m_open || m_browser) reload();
    });
    setFrench(true);
}

SanctuarySitePage::~SanctuarySitePage()
{
#ifdef Q_OS_WIN
    if (m_browser) webview_destroy(m_browser);
#endif
}

QString SanctuarySitePage::text(const char* fr, const char* en) const { return QString::fromUtf8(m_french ? fr : en); }
QString SanctuarySitePage::id() const { return m_section == Section::Map ? QStringLiteral("map") : QStringLiteral("wiki"); }
QString SanctuarySitePage::displayName() const { return m_section == Section::Map ? text("Carte", "Map") : QStringLiteral("Wiki"); }

void SanctuarySitePage::setFrench(bool french)
{
    const bool changed = m_french != french;
    m_french = french;
    m_back->setText(text("Retour", "Back"));
    m_connect->setText(text("Connecter Discord", "Connect Discord"));
    m_reload->setText(text("Actualiser", "Refresh"));
    if (changed && m_open) reload();
}

void SanctuarySitePage::openedImpl()
{
    m_open = true;
    if (!m_loaded) reload();
}

void SanctuarySitePage::setStatus(const QString& message)
{
    m_status->setText(message);
    m_status->setVisible(!message.isEmpty());
}

bool SanctuarySitePage::createBrowser()
{
#ifdef Q_OS_WIN
    if (!m_container) {
        m_container = new QWidget(this);
        m_container->setAttribute(Qt::WA_NativeWindow);
        static_cast<QVBoxLayout*>(layout())->insertWidget(2, m_container, 1);
    }
    m_browser = webview_create(0, reinterpret_cast<void*>(m_container->winId()));
    if (!m_browser) return false;
    webview_bind(m_browser, "sanctuaryPageLoaded", [](const char* ref, const char* args, void* context) {
        auto* self = static_cast<SanctuarySitePage*>(context);
        const auto data = QJsonDocument::fromJson(QByteArray(args)).array();
        if (!data.isEmpty()) self->pageReady(QUrl(data.first().toString()));
        webview_return(self->m_browser, ref, 0, "null");
    }, this);
    webview_init(m_browser,
        "window.addEventListener('DOMContentLoaded', () => {"
        " if (window === top && location.origin === 'https://sanctuary-minecraft.net')"
        "   window.sanctuaryPageLoaded(location.href);"
        "});");
    return true;
#else
    m_browser = new QQuickView;
    m_browser->setResizeMode(QQuickView::SizeRootObjectToView);
    m_browser->setSource(QUrl("qrc:/sanctuary/SanctuaryWebView.qml"));
    m_container = QWidget::createWindowContainer(m_browser, this);
    static_cast<QVBoxLayout*>(layout())->insertWidget(2, m_container, 1);
    auto* view = m_browser->rootObject();
    if (!view) return false;
    connect(view, SIGNAL(pageReady(QUrl)), this, SLOT(pageReady(QUrl)));
    connect(view, SIGNAL(pageFailed()), this, SLOT(pageFailed()));
    return true;
#endif
}

void SanctuarySitePage::navigate(const QUrl& url)
{
    if (!m_browser) return;
    if (m_url == url) { evaluate(QStringLiteral("location.reload()")); return; }
    m_url = url;
#ifdef Q_OS_WIN
    webview_navigate(m_browser, url.toEncoded().constData());
#else
    if (auto* view = m_browser->rootObject()) view->setProperty("url", url);
#endif
}

bool SanctuarySitePage::evaluate(const QString& script)
{
    if (!m_browser) return false;
#ifdef Q_OS_WIN
    return webview_eval(m_browser, script.toUtf8().constData()) == WEBVIEW_ERROR_OK;
#else
    auto* view = m_browser->rootObject();
    return view && QMetaObject::invokeMethod(view, "evaluate", Q_ARG(QVariant, QVariant(script)));
#endif
}

void SanctuarySitePage::reload()
{
    auto* pending = m_ticket.data();
    m_ticket.clear();
    if (pending) pending->abort();

    const bool map = m_section == Section::Map;
    const bool connected = !map || m_session->connected();
    m_connect->setVisible(!connected);
    m_back->setVisible(!map);
    m_reload->setVisible(connected);
    if (!connected) {
        navigate(QUrl(QStringLiteral("about:blank")));
        if (m_container) m_container->hide();
        setStatus(text("Connectez Discord pour afficher la carte en direct.", "Connect Discord to view the live map."));
        return;
    }

    if (!m_browser && !createBrowser()) { pageFailed(); return; }

    m_container->show();
    setStatus(text("Chargement…", "Loading…"));
    const auto url = QUrl(QStringLiteral("https://sanctuary-minecraft.net/launcher/%1?lang=%2")
                            .arg(id(), m_french ? QStringLiteral("fr") : QStringLiteral("en")));
    navigate(url);
    m_loaded = true;
}

void SanctuarySitePage::pageReady(const QUrl& url)
{
    if (url.scheme() != "https" || url.host() != "sanctuary-minecraft.net") return;
    m_url = url;
    if (m_section != Section::Map || url.path().startsWith("/carte/")) { setStatus({}); return; }
    if (url.path() != "/launcher/map" || m_ticket) return;

    auto* reply = m_session->mapTicket();
    if (!reply) return;
    m_ticket = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (m_ticket != reply) return;
        m_ticket.clear();
        if (!m_session->connected() || reply->error() != QNetworkReply::NoError) { pageFailed(); return; }
        const auto ticket = QJsonDocument::fromJson(reply->readAll()).object().value("ticket").toString();
        if (!QRegularExpression(QStringLiteral("^[0-9a-f]{64}$")).match(ticket).hasMatch()) { pageFailed(); return; }
        const auto payload = QString::fromUtf8(QJsonDocument(QJsonObject{{"ticket", ticket}}).toJson(QJsonDocument::Compact));
        const auto script = QStringLiteral(R"(
            if (window.location.origin === 'https://sanctuary-minecraft.net'
                && window.location.pathname === '/launcher/map') {
                fetch('/api/launcher/v1/map/session', {method: 'POST',
                    headers: {'Content-Type': 'application/json'}, credentials: 'same-origin',
                    redirect: 'error', body: JSON.stringify(%1)})
                    .then(r => { if (!r.ok) throw new Error(); window.location.replace('/carte/'); })
                    .catch(() => { document.body.textContent = document.body.dataset.loadError; });
            }
        )").arg(payload);
        if (!evaluate(script)) { pageFailed(); return; }
        setStatus({});
    });
}

void SanctuarySitePage::pageFailed()
{
    setStatus(text("La page n’a pas pu se charger. Réessayez avec Actualiser.",
                   "The page could not load. Try Refresh."));
}
