// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryFeed.h"
#include "SanctuaryAppearance.h"
#include "Application.h"
#include "SanctuarySession.h"
#include <QDesktopServices>
#include <QJsonArray>
#include <QImage>
#include <QImageReader>
#include <QBuffer>
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
#include <QScrollArea>
#include <QScrollBar>
#include <QResizeEvent>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QUuid>

static void setPostTitle(QLabel* title, bool expanded)
{
    const auto link = SanctuaryAppearance::colors(QStringLiteral("<a style='color:@text;text-decoration:none' href='toggle'>%1 %2</a>"));
    title->setText(link.arg(expanded ? QStringLiteral("⌄") : QStringLiteral("›"),
                            title->property("heading").toString().toHtmlEscaped()));
}

// Keep the original image in memory and fit it to the reading column.
class SanctuaryPostPhoto : public QLabel {
public:
    explicit SanctuaryPostPhoto(QWidget* parent) : QLabel(parent)
    {
        auto policy = QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        setSizePolicy(policy);
        setMinimumWidth(0);
        setAlignment(Qt::AlignLeft | Qt::AlignTop);
        setWordWrap(true);
    }
    void setImage(const QImage& image) { m_image = QPixmap::fromImage(image); reflow(); updateGeometry(); }
    bool hasHeightForWidth() const override { return !m_image.isNull(); }
    int heightForWidth(int width) const override
    {
        return m_image.isNull() ? QLabel::heightForWidth(width) : qMax(1, qMin(width, m_image.width()) * m_image.height() / m_image.width());
    }
    QSize sizeHint() const override { return m_image.isNull() ? QLabel::sizeHint() : QSize(320,heightForWidth(320)); }
protected:
    void resizeEvent(QResizeEvent* event) override { QLabel::resizeEvent(event); reflow(); }
private:
    void reflow() { if (!m_image.isNull()) setPixmap(m_image.scaledToWidth(qMax(1,qMin(width(),m_image.width())),Qt::SmoothTransformation)); }
    QPixmap m_image;
};

SanctuaryFeed::SanctuaryFeed(const QString& kind, QWidget* parent, QNetworkAccessManager* network) : QWidget(parent), m_kind(kind), m_network(network)
{
    setObjectName("feed-" + kind);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* top = new QHBoxLayout;
    m_status = new QLabel(this);
    m_status->setObjectName("feedStatus");
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
        layout->addWidget(m_content, 1);
        connect(m_content, &QTextBrowser::anchorClicked, this, [this](const QUrl& link) {
            const auto url = QUrl("https://sanctuary-minecraft.net/").resolved(link);
            if (url.scheme() == "https" && url.host() == "sanctuary-minecraft.net") emit linkRequested(url);
        });
    } else {
        auto* scroll = new QScrollArea(this);
        scroll->setObjectName("postScroll");
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setMinimumSize(0,0);
        auto* posts = new QWidget(scroll);
        m_posts = new QVBoxLayout(posts);
        m_posts->setContentsMargins(0, 0, 8, 0);
        m_posts->setSpacing(16);
        m_posts->setAlignment(Qt::AlignTop);
        scroll->setWidget(posts);
        layout->addWidget(scroll, 1);
        m_status->hide();
        m_refresh->hide();
    }
    connect(m_refresh, &QPushButton::clicked, this, &SanctuaryFeed::reload);
    new SanctuaryAppearance::PaletteWatcher(this, [this] { applyAppearance(); });
    setFrench(true);
}

void SanctuaryFeed::applyAppearance()
{
    const auto style = SanctuaryAppearance::colors(QStringLiteral(
        "QLabel#feedStatus, QLabel#postAuthor {color:@muted; font-size:12px;}"
        "QTextBrowser {background:@surface; color:@text;}"
    ));
    if (styleSheet() == style) return;
    setStyleSheet(style);
    for (auto title = m_titles.cbegin(); title != m_titles.cend(); ++title) {
        if (title.value()) setPostTitle(title.value(), title.key() == m_openId);
    }
    if (m_content) {
        m_content->document()->setDefaultStyleSheet(SanctuaryAppearance::colors(QStringLiteral(
            "body {color:@text; font-size:14px;} h2,h3 {font-size:18px;} a {color:@link;} p {margin-bottom:12px;}"
        )));
        if (!m_newsHtml.isEmpty()) {
            const int scroll = m_content->verticalScrollBar()->value();
            m_content->setHtml(m_newsHtml);
            m_content->verticalScrollBar()->setValue(scroll);
        }
    }
}

void SanctuaryFeed::setFrench(bool french)
{
    const bool changed = m_french != french;
    m_french = french;
    m_refresh->setText(french ? "Actualiser" : "Refresh");
    const QHash<QString, QPair<QString,QString>> names{{"bulletin",{"Intendance","Stewardship"}},
        {"article",{"Gazette","Gazette"}},{"notice",{"Tableau d'affichage","Notice board"}}};
    if (names.contains(m_kind)) setAccessibleName(french ? names[m_kind].first : names[m_kind].second);
    if (m_content && m_content->toPlainText().isEmpty()) status(french ? "Contenu du site Sanctuary" : "Content from the Sanctuary website");
    if (changed && !m_entries.isEmpty()) {
        const auto entries = m_entries;
        m_entries = {};
        setEntries(entries);
    }
}

void SanctuaryFeed::status(const QString& message, bool attention)
{
    m_status->setText(message);
    m_status->setVisible(!message.isEmpty());
    m_refresh->setVisible(m_kind == "news" || attention);
}

void SanctuaryFeed::setContent(const QString& content, bool html)
{
    if (m_content) {
        m_newsHtml = html ? content : QString();
        if (html) m_content->setHtml(content);
        else m_content->setPlainText(content);
    } else {
        setEntries({});
        if (!content.isEmpty()) {
            QTextDocument document;
            document.setHtml(html ? content : QString());
            status(html ? document.toPlainText() : content, true);
        }
    }
}

QUrl SanctuaryFeed::playerFaceUrl(const QString& name)
{
    if (!QRegularExpression("^[A-Za-z0-9_]{3,16}$").match(name).hasMatch()) return {};
    return QUrl("https://mc-api.io/render/FACE/" + name + "/JAVA?size=64");
}

void SanctuaryFeed::loadFace(const QString& author, QLabel* face)
{
    if (m_faces.contains(author)) { face->setPixmap(m_faces.value(author)); return; }
    auto* app = qobject_cast<Application*>(QCoreApplication::instance());
    const auto url = playerFaceUrl(author);
    auto* network = m_network ? m_network.data() : app ? app->network() : nullptr;
    if (!network || url.isEmpty()) return;
    QNetworkRequest request(url); // Public skin request: never attach the Sanctuary session.
    request.setTransferTimeout(10000);
    request.setRawHeader("Accept", "image/png,image/webp,image/*");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    auto* reply = network->get(request);
    const int generation = m_generation;
    const QPointer<QLabel> target(face);
    connect(reply, &QNetworkReply::finished, this, [this, reply, target, author, generation] {
        const auto bytes = reply->readAll();
        const auto code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();
        if (generation != m_generation || code != 200 || bytes.size() > 256 * 1024) return;
        QBuffer buffer;
        buffer.setData(bytes);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        const auto format = reader.format().toLower();
        const auto size = reader.size();
        if ((format != "png" && format != "webp") || size.isEmpty() || size.width() > 512 || size.height() > 512) return;
        const auto image = reader.read();
        if (image.isNull()) return;
        const auto pixmap = QPixmap::fromImage(image.scaled(32,32,Qt::KeepAspectRatio,Qt::FastTransformation));
        if (m_faces.size() >= 64) m_faces.clear();
        m_faces.insert(author,pixmap);
        if (target) target->setPixmap(pixmap);
    });
}

void SanctuaryFeed::loadPhoto(const QString& id, QLabel* label)
{
    if (!m_session || label->property("requested").toBool()) return;
    auto* reply = m_session->photo(id);
    if (!reply) return;
    label->setProperty("requested",true);
    label->setText(m_french ? "Chargement de l’image…" : "Loading image…");
    reply->setReadBufferSize(64 * 1024);
    const int generation = m_generation;
    const QPointer<QLabel> target(label);
    connect(reply,&QNetworkReply::readyRead,this,[reply] { if (reply->bytesAvailable() > 49152) reply->abort(); });
    connect(reply,&QNetworkReply::finished,this,[this,reply,target,generation] {
        const auto bytes = reply->readAll();
        const bool success = reply->error() == QNetworkReply::NoError && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
        reply->deleteLater();
        if (generation != m_generation || !target) return;
        QBuffer buffer;
        buffer.setData(bytes);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        const auto size = reader.size();
        QImage image;
        if (success && bytes.size() <= 49152 && reader.format().toLower() == "jpeg" && !size.isEmpty() && size.width() <= 960 && size.height() <= 540)
            image = reader.read();
        if (image.isNull()) {
            target->setText(m_french ? "Image indisponible. Rouvrez la publication pour réessayer." : "Image unavailable. Reopen the post to retry.");
            target->setProperty("requested",false);
        } else static_cast<SanctuaryPostPhoto*>(target.data())->setImage(image);
    });
}

void SanctuaryFeed::setEntries(const QJsonArray& entries)
{
    if (!m_posts || (m_entries == entries && m_posts->count() > 0)) return;
    if (entries.isEmpty()) m_latestBulletin.clear();
    m_entries = entries;
    ++m_generation;
    m_details.clear();
    m_titles.clear();
    while (auto* item = m_posts->takeAt(0)) {
        if (auto* widget = item->widget()) { widget->hide(); widget->deleteLater(); }
        delete item;
    }
    int count = 0;
    const auto previousOpen = m_openId;
    m_openId.clear();
    for (const auto& value : entries) {
        if (count++ >= (m_kind == "bulletin" ? 1 : 3)) break;
        const auto entry = value.toObject();
        const auto id = entry.value("id").toString();
        if (QUuid(id).isNull() || m_details.contains(id)) continue;
        const auto url = QUrl("https://sanctuary-minecraft.net/communaute/" + id);
        auto* post = new QWidget(this);
        post->setObjectName("communityPost");
        auto* column = new QVBoxLayout(post);
        column->setContentsMargins(0,0,0,0);
        column->setSpacing(10);
        auto* row = new QHBoxLayout;
        row->setSpacing(10);
        const auto author = entry.value("author_name").toString();
        if (m_kind != "bulletin") {
            auto* face = new QLabel(post);
            face->setObjectName("playerFace");
            face->setFixedSize(32,32);
            face->setAlignment(Qt::AlignCenter);
            face->setTextFormat(Qt::PlainText);
            face->setText(author.left(1).toUpper());
            face->setToolTip(author);
            face->setAccessibleName((m_french ? "Visage de " : "Face of ") + author);
            row->addWidget(face,0,Qt::AlignTop);
            loadFace(author,face);
        }
        auto* title = new QLabel(post);
        title->setObjectName("postTitle");
        title->setWordWrap(true);
        title->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        title->setStyleSheet("font-size:14px; font-weight:600;");
        title->setTextFormat(Qt::RichText);
        title->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
        title->setFocusPolicy(Qt::StrongFocus);
        title->setCursor(Qt::PointingHandCursor);
        const auto heading = entry.value("title").toString().isEmpty() ? author : entry.value("title").toString();
        title->setProperty("heading",heading);
        title->setAccessibleName(heading);
        setPostTitle(title, false);
        row->addWidget(title,1);
        column->addLayout(row);
        auto* details = new QWidget(post);
        details->setObjectName("postDetails");
        auto* content = new QVBoxLayout(details);
        content->setContentsMargins(m_kind == "bulletin" ? 0 : 42,0,0,0);
        content->setSpacing(10);
        auto* byline = new QLabel(author,details);
        byline->setObjectName("postAuthor");
        byline->setTextFormat(Qt::PlainText);
        content->addWidget(byline);
        QLabel* photo = nullptr;
        if (m_kind == "article" && entry.value("has_photo").toBool()) {
            photo = new SanctuaryPostPhoto(details);
            photo->setObjectName("postPhoto");
            photo->setAccessibleName(m_french ? "Image de la publication" : "Post image");
            content->addWidget(photo);
        }
        auto* body = new QLabel(entry.value("body").toString(),details);
        body->setObjectName("postBody");
        body->setTextFormat(Qt::PlainText);
        body->setWordWrap(true);
        body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        body->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        content->addWidget(body);
        auto* link = new QLabel(details);
        link->setText("<a href='site'>" + QString(m_french ? "Lire sur le site →" : "Read on the website →") + "</a>");
        link->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
        link->setFocusPolicy(Qt::StrongFocus);
        connect(link,&QLabel::linkActivated,this,[this,url] { emit linkRequested(url); });
        content->addWidget(link);
        column->addWidget(details);
        details->hide();
        m_details.insert(id,details);
        m_titles.insert(id,title);
        connect(title,&QLabel::linkActivated,this,[this,id,details,title,photo] {
            const bool expand = details->isHidden();
            for (auto it = m_details.begin(); it != m_details.end(); ++it) {
                if (it.value()) it.value()->hide();
                auto* summary = m_titles.value(it.key()).data();
                if (summary) setPostTitle(summary, false);
            }
            m_openId = expand ? id : QString();
            if (expand) {
                setPostTitle(title, true);
                details->show();
                if (photo) loadPhoto(id,photo);
            }
        });
        m_posts->addWidget(post);
        const bool newBulletin = m_kind == "bulletin" && m_latestBulletin != id;
        if (newBulletin) m_latestBulletin = id;
        if (newBulletin || previousOpen == id) emit title->linkActivated("toggle");
    }
    status(entries.isEmpty() ? (m_french ? "Aucune publication" : "No posts") : QString());
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
        m_faces.clear();
        status(m_french ? "Connectez Discord pour lire les publications." : "Connect Discord to read posts.");
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
        m_faces.clear();
        status(m_french ? "Connectez Discord pour lire les publications." : "Connect Discord to read posts.");
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
    if (!m_posts || m_posts->isEmpty()) status(m_french ? "Chargement…" : "Loading…");
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
                this->status(m_french ? "Reconnectez Discord." : "Reconnect Discord.", true);
                m_session->refresh();
                return;
            }
            this->status(m_french ? "Session Discord du site nécessaire" : "Website Discord session required", true);
            setContent(m_french
                ? "Les publications communautaires restent privées. En mode local, les comptes sont simulés : aucun contenu privé n'est affiché."
                : "Community posts stay private. Accounts are simulated in local mode, so no private content is displayed.");
            return;
        }
        if (error != QNetworkReply::NoError || status != 200 || bytes.size() > 2 * 1024 * 1024) {
            this->status(m_french ? "Contenu indisponible — Réessayez" : "Content unavailable — Retry", true);
            return;
        }
        if (m_session) {
            const auto document = QJsonDocument::fromJson(bytes).object();
            if (document.value("version").toInt() != 1 || !document.value("entries").isArray()) {
                setContent({}); this->status(m_french ? "Contenu indisponible" : "Content unavailable", true); return;
            }
            setEntries(document.value("entries").toArray());
            return;
        }
        QString content = m_kind == "news" ? publicNewsArticles(QString::fromUtf8(bytes))
                                           : QJsonDocument::fromJson(bytes).object().value("html").toString();
        if (content.isEmpty()) {
            this->status(m_french ? "Aucune publication disponible" : "No posts available");
            setContent({});
            return;
        }
        setContent(content, true);
        this->status(m_french ? "Dernières publications du site" : "Latest website posts");
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
