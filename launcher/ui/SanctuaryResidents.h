// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QHash>
#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QSet>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class SanctuarySession;

class SanctuaryResidents final : public QWidget {
public:
    SanctuaryResidents(SanctuarySession* session, QNetworkAccessManager* network, QWidget* parent = nullptr);
    void setFrench(bool french);

private:
    enum class Status { Disconnected, Loading, Ready, Unavailable };
    void reload();
    void render();
    void requestFace(const QString& name);
    void displayFace(QLabel* label, const QPixmap& face, bool online) const;
    QString text(const char* fr, const char* en) const;

    SanctuarySession* m_session;
    QNetworkAccessManager* m_network;
    QLabel* m_summary;
    QHBoxLayout* m_facesLayout;
    QPointer<QNetworkReply> m_request;
    QMap<QString, bool> m_residents;
    QHash<QString, QLabel*> m_labels;
    QHash<QString, QPixmap> m_faces;
    QSet<QString> m_loadingFaces;
    Status m_status = Status::Disconnected;
    bool m_french = true;
};
