// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryFeed.h"
#include "Application.h"
#include "SanctuarySession.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextLayout>
#include <QResizeEvent>
#include <QUrlQuery>
#include <QVBoxLayout>

// Bounded text preview: no inner scroll area, even for a long community post.
class SanctuaryExcerpt : public QLabel {
public:
    explicit SanctuaryExcerpt(QWidget* parent) : QLabel(parent)
    {
        setTextFormat(Qt::PlainText);
        setAlignment(Qt::AlignTop | Qt::AlignLeft);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        setMinimumSize(0, 0);
    }
    void setExcerpt(const QString& text) { m_text = text.simplified(); reflow(); }
protected:
    void resizeEvent(QResizeEvent* event) override { QLabel::resizeEvent(event); reflow(); }
private:
    void reflow()
    {
        const auto metrics = fontMetrics();
        const int lines = qBound(1, height() / qMax(1, metrics.lineSpacing()), 8);
        QTextLayout layout(m_text, font());
        layout.beginLayout();
        QStringList visible;
        for (int i = 0; i < lines; ++i) {
            auto line = layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(qMax(1, width()));
            const auto remaining = m_text.mid(line.textStart());
            visible.append(i == lines - 1 ? metrics.elidedText(remaining, Qt::ElideRight, width())
                                          : m_text.mid(line.textStart(), line.textLength()).trimmed());
        }
        layout.endLayout();
        QLabel::setText(visible.join('\n'));
        setAccessibleName(m_text);
    }
    QString m_text;
};

SanctuaryFeed::SanctuaryFeed(const QString& kind, QWidget* parent) : QWidget(parent), m_kind(kind)
{
    setObjectName("feed-" + kind);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* top = new QHBoxLayout;
    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    top->addWidget(m_status, 1);
    m_refresh = new QPushButton(this);
    m_refresh->setObjectName("refresh-" + kind);
    top->addWidget(m_refresh);
    layout->addLayout(top);
    if (kind == "news") {
        m_content = new QTextBrowser(this);
        m_content->setObjectName("feedContent-" + kind);
        m_content->setOpenLinks(false);
        m_content->setMinimumHeight(260);
        m_content->document()->setBaseUrl(QUrl("https://sanctuary-minecraft.net/"));
        m_content->document()->setDefaultStyleSheet("body {color:#1f2328; font-size:14px;} h2,h3 {font-size:18px;} a {color:#0969da;} p {margin-bottom:12px;}");
        layout->addWidget(m_content, 1);
        connect(m_content, &QTextBrowser::anchorClicked, this, [this](const QUrl& link) {
            const auto url = QUrl("https://sanctuary-minecraft.net/").resolved(link);
            if (url.scheme() == "https" && url.host() == "sanctuary-minecraft.net") emit linkRequested(url);
        });
    } else {
        m_excerpt = new SanctuaryExcerpt(this);
        m_excerpt->setObjectName("feedExcerpt-" + kind);
        layout->addWidget(m_excerpt, 1);
        m_read = new QPushButton(this);
        m_read->setObjectName("read-" + kind);
        layout->addWidget(m_read, 0, Qt::AlignLeft);
        connect(m_read, &QPushButton::clicked, this, [this] {
            QUrl url("https://sanctuary-minecraft.net/communaute");
            QUrlQuery query;
            query.addQueryItem("kind", m_kind);
            query.addQueryItem("lang", m_french ? "fr" : "en");
            url.setQuery(query);
            emit linkRequested(url);
        });
    }
    connect(m_refresh, &QPushButton::clicked, this, &SanctuaryFeed::reload);
    setFrench(true);
}

void SanctuaryFeed::setFrench(bool french)
{
    m_french = french;
    m_refresh->setText(french ? "Actualiser" : "Refresh");
    if (m_read) m_read->setText(french ? "Lire sur le site" : "Read on the website");
    if ((m_content && m_content->toPlainText().isEmpty()) || (m_excerpt && m_excerpt->text().isEmpty()))
        m_status->setText(french ? "Contenu du site Sanctuary" : "Content from the Sanctuary website");
}

void SanctuaryFeed::setContent(const QString& content, bool html)
{
    if (m_content) {
        if (html) m_content->setHtml(content);
        else m_content->setPlainText(content);
    } else {
        QTextDocument document;
        document.setHtml(html ? content : QString());
        m_excerpt->setExcerpt(html ? document.toPlainText() : content);
    }
}

QString SanctuaryFeed::publicNewsArticles(const QString& html)
{
    // /actualites uses a version table; the public home uses release-entry articles.
    const auto flags = QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption;
    auto rows = QRegularExpression("<tr\\b[^>]*id=\"version-[^\"]+\"[^>]*>.*?</tr>", flags).globalMatch(html);
    const QRegularExpression cells("<th\\b[^>]*>(.*?)</th>\\s*<td\\b[^>]*>(.*?)</td>", flags);
    QString result;
    int count = 0;
    while (rows.hasNext() && count++ < 20) {
        const auto cell = cells.match(rows.next().captured(0));
        if (cell.hasMatch()) {
            auto heading = cell.captured(1);
            heading.replace(QRegularExpression("<time\\b", QRegularExpression::CaseInsensitiveOption), " · <time");
            result += "<h3>" + heading + "</h3>" + cell.captured(2) + "<hr>";
        }
    }
    const QRegularExpression article("<article\\b[^>]*class=\"[^\"]*\\brelease-entry\\b[^\"]*\"[^>]*>.*?</article>",
                                      flags);
    if (result.isEmpty()) {
        auto matches = article.globalMatch(html);
        count = 0;
        while (matches.hasNext() && count++ < 20) result += matches.next().captured(0) + "<hr>";
    }
    result.remove(QRegularExpression("<(script|style|form)\\b[^>]*>.*?</\\1>", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption));
    return result;
}

void SanctuaryFeed::setSession(SanctuarySession* session)
{
    m_session = session;
    connect(session, &SanctuarySession::credentialsChanged, this, [this] {
        ++m_generation;
        if (m_reply) m_reply->abort();
        setContent({});
        m_status->setText(m_french ? "Connectez Discord pour lire les publications." : "Connect Discord to read posts.");
    });
    connect(session, &SanctuarySession::changed, this, [this] {
        if (!m_session || m_session->profile().value("access").toObject().value("status") != "granted") {
            ++m_generation;
            if (m_reply) m_reply->abort();
            setContent({});
        } else if (isVisible() && !m_session->busy()) reload();
    });
}

void SanctuaryFeed::reload()
{
    auto* app = qobject_cast<Application*>(QCoreApplication::instance());
    if (!app || m_reply) return;
    if (m_session && (!m_session->connected() || m_session->profile().value("access").toObject().value("status") != "granted")) {
        setContent({});
        m_status->setText(m_french ? "Connectez Discord pour lire les publications." : "Connect Discord to read posts.");
        return;
    }
    QUrl url("https://sanctuary-minecraft.net" + QString(m_kind == "news" ? "/actualites" : "/communaute/flux"));
    QUrlQuery query;
    if (m_kind != "news") query.addQueryItem("kind", m_kind);
    query.addQueryItem("lang", m_french ? "fr" : "en");
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", m_kind == "news" ? "text/html" : "application/json");
    request.setTransferTimeout(20000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    m_status->setText(m_french ? "Chargement…" : "Loading…");
    const int generation = m_generation;
    auto* reply = m_session ? m_session->feed(m_kind, m_french) : app->network()->get(request);
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        const auto bytes = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto error = reply->error();
        reply->deleteLater();
        if (m_reply == reply) m_reply = nullptr;
        if (generation != m_generation) return;
        if (m_kind != "news" && (status == 401 || status == 403 || (status >= 300 && status < 400))) {
            if (m_session) {
                setContent({});
                m_status->setText(m_french ? "Reconnectez Discord." : "Reconnect Discord.");
                m_session->refresh();
                return;
            }
            m_status->setText(m_french ? "Session Discord du site nécessaire" : "Website Discord session required");
            setContent(m_french
                ? "Les publications communautaires restent privées. En mode local, les comptes sont simulés : aucun contenu privé n'est affiché."
                : "Community posts stay private. Accounts are simulated in local mode, so no private content is displayed.");
            return;
        }
        if (error != QNetworkReply::NoError || status != 200 || bytes.size() > 2 * 1024 * 1024) {
            m_status->setText(m_french ? "Contenu indisponible — Réessayez" : "Content unavailable — Retry");
            return;
        }
        if (m_session) {
            const auto document = QJsonDocument::fromJson(bytes).object();
            if (document.value("version").toInt() != 1 || !document.value("entries").isArray()) {
                setContent({}); m_status->setText(m_french ? "Contenu indisponible" : "Content unavailable"); return;
            }
            QStringList posts;
            for (const auto& entry : document.value("entries").toArray()) {
                const auto post = entry.toObject();
                posts.append(post.value("title").toString() + " — " + post.value("body").toString());
            }
            setContent(posts.join(" · "));
            m_status->setText(posts.isEmpty() ? (m_french ? "Aucune publication" : "No posts")
                                             : (m_french ? "Publications du site Sanctuary" : "Sanctuary website posts"));
            return;
        }
        QString content = m_kind == "news" ? publicNewsArticles(QString::fromUtf8(bytes))
                                           : QJsonDocument::fromJson(bytes).object().value("html").toString();
        if (content.isEmpty()) {
            m_status->setText(m_french ? "Aucune publication disponible" : "No posts available");
            setContent({});
            return;
        }
        setContent(content, true);
        m_status->setText(m_french ? "Dernières publications du site" : "Latest website posts");
    });
}

SanctuaryNewsPage::SanctuaryNewsPage(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_feed = new SanctuaryFeed("news", this);
    layout->addWidget(m_feed);
    connect(m_feed, &SanctuaryFeed::linkRequested, this, &SanctuaryNewsPage::linkRequested);
}
void SanctuaryNewsPage::openedImpl() { m_feed->reload(); }
void SanctuaryNewsPage::setFrench(bool french) { m_feed->setFrench(french); }
