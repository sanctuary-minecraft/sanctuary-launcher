// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryResidents.h"
#include "SanctuaryAppearance.h"
#include "SanctuaryFeed.h"
#include "SanctuarySession.h"

#include <QHBoxLayout>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScrollArea>
#include <QTimer>
#include <algorithm>

SanctuaryResidents::SanctuaryResidents(SanctuarySession* session, QNetworkAccessManager* network, QWidget* parent)
    : QWidget(parent), m_session(session), m_network(network), m_summary(new QLabel(this))
{
    setObjectName("sanctuaryResidents");
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(22, 5, 22, 3);
    row->setSpacing(12);
    row->addWidget(m_summary);

    auto* scroll = new QScrollArea(this);
    auto* faces = new QWidget(scroll);
    m_facesLayout = new QHBoxLayout(faces);
    m_facesLayout->setContentsMargins(0, 0, 0, 0);
    m_facesLayout->setSpacing(5);
    m_facesLayout->setSizeConstraint(QLayout::SetMinimumSize);
    m_facesLayout->addStretch();
    scroll->setWidget(faces);
    scroll->setWidgetResizable(true);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFixedHeight(43);
    row->addWidget(scroll, 1);

    new SanctuaryAppearance::PaletteWatcher(this, [this] {
        SanctuaryAppearance::apply(this, QStringLiteral(R"(
            #sanctuaryResidents { background: @surface; border-bottom: 1px solid @border; }
            #sanctuaryResidents QLabel { color: @muted; font-size: 12px; }
            #sanctuaryResidents QScrollArea { border: none; background: @surface; }
            #sanctuaryResidents QLabel[residentFace="true"] { background: @panel; border: 1px solid @border; border-radius: 4px; }
            #sanctuaryResidents QLabel[residentFace="true"][online="true"] { border-color: @navGreenBorder; background: @greenPanel; }
        )"));
    });
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SanctuaryResidents::reload);
    timer->start(15000);
    connect(m_session, &SanctuarySession::credentialsChanged, this, [this] {
        auto* pending = m_request.data();
        m_request.clear();
        if (pending) pending->abort();
        m_residents.clear();
        m_status = m_session->connected() ? Status::Loading : Status::Disconnected;
        render();
        reload();
    });
    render();
    reload();
}

QString SanctuaryResidents::text(const char* fr, const char* en) const
{
    return QString::fromUtf8(m_french ? fr : en);
}

void SanctuaryResidents::setFrench(bool french)
{
    if (m_french == french) return;
    m_french = french;
    render();
}

void SanctuaryResidents::reload()
{
    if (m_request || !m_session->connected()) return;
    auto* reply = m_session->residents();
    if (!reply) return;
    m_request = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (m_request != reply) return;
        m_request.clear();
        const auto data = QJsonDocument::fromJson(reply->readAll()).object();
        const auto entries = data.value("residents");
        bool valid = reply->error() == QNetworkReply::NoError && data.value("version").toInt() == 1 && entries.isArray();
        QMap<QString, bool> residents;
        if (valid) {
            for (const auto& value : entries.toArray()) {
                const auto entry = value.toObject();
                const auto name = entry.value("name").toString().toLower();
                if (SanctuaryFeed::playerFaceUrl(name).isEmpty() || !entry.value("online").isBool() || residents.contains(name)) {
                    valid = false;
                    break;
                }
                residents.insert(name, entry.value("online").toBool());
            }
        }
        const auto status = valid ? Status::Ready : Status::Unavailable;
        if (!valid) residents.clear();
        if (m_status != status || m_residents != residents) {
            m_status = status;
            m_residents = residents;
            render();
        } else {
            for (const auto& name : m_residents.keys()) requestFace(name);
        }
    });
}

void SanctuaryResidents::render()
{
    for (auto* label : m_labels) delete label;
    m_labels.clear();
    if (m_status == Status::Disconnected) {
        m_summary->setText(text("Connectez Discord pour voir les habitants.", "Connect Discord to see the residents."));
        return;
    }
    if (m_status == Status::Loading) {
        m_summary->setText(text("Chargement des habitants…", "Loading residents…"));
        return;
    }
    if (m_status == Status::Unavailable) {
        m_summary->setText(text("Présence des habitants indisponible.", "Resident presence unavailable."));
        return;
    }
    const auto online = std::count(m_residents.cbegin(), m_residents.cend(), true);
    m_summary->setText(text("Habitants · %1/%2 en ligne", "Residents · %1/%2 online").arg(online).arg(m_residents.size()));
    for (auto it = m_residents.cbegin(); it != m_residents.cend(); ++it) {
        auto* face = new QLabel(it.key().left(1).toUpper(), this);
        face->setAlignment(Qt::AlignCenter);
        face->setFixedSize(34, 34);
        face->setProperty("residentFace", true);
        face->setProperty("online", it.value());
        const auto description = it.key() + (it.value() ? text(" · En ligne", " · Online") : text(" · Déconnecté", " · Offline"));
        face->setToolTip(description);
        face->setAccessibleName(description);
        m_facesLayout->insertWidget(m_facesLayout->count() - 1, face);
        m_labels.insert(it.key(), face);
        if (m_faces.contains(it.key())) displayFace(face, m_faces.value(it.key()), it.value());
        else requestFace(it.key());
    }
}

void SanctuaryResidents::requestFace(const QString& name)
{
    if (m_faces.contains(name) || m_loadingFaces.contains(name)) return;
    QNetworkRequest request(SanctuaryFeed::playerFaceUrl(name));
    request.setTransferTimeout(10000);
    request.setRawHeader("Accept", "image/png,image/webp,image/*");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    m_loadingFaces.insert(name);
    auto* reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, name] {
        m_loadingFaces.remove(name);
        reply->deleteLater();
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) return;
        auto* label = m_labels.value(name);
        if (!label) return;
        const auto face = SanctuaryFeed::decodePlayerFace(reply->readAll());
        if (face.isNull()) return;
        m_faces.insert(name, face);
        displayFace(label, face, m_residents.value(name));
    });
}

void SanctuaryResidents::displayFace(QLabel* label, const QPixmap& face, bool online) const
{
    if (online) {
        label->setPixmap(face);
        return;
    }
    auto image = face.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        auto* pixels = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const auto pixel = pixels[x];
            const auto grey = qGray(pixel);
            pixels[x] = qRgba(grey, grey, grey, qAlpha(pixel));
        }
    }
    label->setPixmap(QPixmap::fromImage(image));
}
