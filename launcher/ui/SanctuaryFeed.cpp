// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryFeed.h"
#include "Application.h"
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
#include <QUrlQuery>
#include <QVBoxLayout>

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
    m_content = new QTextBrowser(this);
    m_content->setObjectName("feedContent-" + kind);
    m_content->setOpenLinks(false);
    m_content->setMinimumHeight(kind == "news" ? 260 : 110);
    m_content->document()->setBaseUrl(QUrl("https://sanctuary-minecraft.net/"));
    m_content->document()->setDefaultStyleSheet("body {color:#1f2328; font-size:14px;} h2,h3 {font-size:18px;} a {color:#0969da;} p {margin-bottom:12px;}");
    layout->addWidget(m_content, 1);
    connect(m_refresh, &QPushButton::clicked, this, &SanctuaryFeed::reload);
    connect(m_content, &QTextBrowser::anchorClicked, this, [this](const QUrl& link) {
        const auto url = QUrl("https://sanctuary-minecraft.net/").resolved(link);
        if (url.scheme() == "https" && url.host() == "sanctuary-minecraft.net") emit linkRequested(url);
    });
    setFrench(true);
}

void SanctuaryFeed::setFrench(bool french)
{
    m_french = french;
    m_refresh->setText(french ? "Actualiser" : "Refresh");
    if (m_content->toPlainText().isEmpty()) m_status->setText(french ? "Contenu du site Sanctuary" : "Content from the Sanctuary website");
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

void SanctuaryFeed::reload()
{
    auto* app = qobject_cast<Application*>(QCoreApplication::instance());
    if (!app || m_reply) return;
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
    auto* reply = app->network()->get(request);
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto bytes = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto error = reply->error();
        reply->deleteLater();
        m_reply = nullptr;
        if (m_kind != "news" && (status == 401 || status == 403 || (status >= 300 && status < 400))) {
            m_status->setText(m_french ? "Session Discord du site nécessaire" : "Website Discord session required");
            m_content->setPlainText(m_french
                ? "Les publications communautaires restent privées. En mode local, les comptes sont simulés : aucun contenu privé n'est affiché."
                : "Community posts stay private. Accounts are simulated in local mode, so no private content is displayed.");
            return;
        }
        if (error != QNetworkReply::NoError || status != 200 || bytes.size() > 2 * 1024 * 1024) {
            m_status->setText(m_french ? "Contenu indisponible — Réessayez" : "Content unavailable — Retry");
            return;
        }
        QString content = m_kind == "news" ? publicNewsArticles(QString::fromUtf8(bytes))
                                           : QJsonDocument::fromJson(bytes).object().value("html").toString();
        if (content.isEmpty()) {
            m_status->setText(m_french ? "Aucune publication disponible" : "No posts available");
            m_content->clear();
            return;
        }
        m_content->setHtml(content);
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
