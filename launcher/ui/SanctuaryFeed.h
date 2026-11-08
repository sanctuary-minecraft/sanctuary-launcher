// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QWidget>
#include <QPointer>
#include <QJsonArray>
#include <QHash>
#include <QPixmap>
#include "ui/pages/BasePage.h"

class QLabel;
class QPushButton;
class QTextBrowser;
class QNetworkReply;
class QNetworkAccessManager;
class QVBoxLayout;
class SanctuarySession;

/** Read-only view of existing site routes. Private community feeds keep their Discord session requirement. */
class SanctuaryFeed : public QWidget {
    Q_OBJECT
public:
    explicit SanctuaryFeed(const QString& kind, QWidget* parent = nullptr, QNetworkAccessManager* network = nullptr);
    void setFrench(bool french);
    void reload();
    void setSession(SanctuarySession* session);
    void setEntries(const QJsonArray& entries);
    static QUrl playerFaceUrl(const QString& name);
    static QString publicNewsArticles(const QString& html);
signals:
    void linkRequested(const QUrl& url);
private:
    QString m_kind;
    bool m_french = true;
    QLabel* m_status;
    QPushButton* m_refresh;
    QTextBrowser* m_content = nullptr;
    QVBoxLayout* m_posts = nullptr;
    QHash<QString, QPixmap> m_faces;
    QJsonArray m_entries;
    QString m_openId;
    QHash<QString, QPointer<QWidget>> m_details;
    QHash<QString, QPointer<QLabel>> m_titles;
    void loadPhoto(const QString& id, QLabel* label);
    void loadFace(const QString& author, QLabel* label);
    void status(const QString& message, bool attention = false);
    void setContent(const QString& content, bool html = false);
    QPointer<QNetworkReply> m_reply;
    QPointer<QNetworkAccessManager> m_network;
    QPointer<SanctuarySession> m_session;
    int m_generation = 0;
};

class SanctuaryNewsPage : public QWidget, public BasePage {
    Q_OBJECT
public:
    explicit SanctuaryNewsPage(QWidget* parent = nullptr);
    QString id() const override { return "news"; }
    QString displayName() const override { return "Actualités"; }
    QIcon icon() const override { return QIcon(":/sanctuary/icon.png"); }
    void openedImpl() override;
    void setFrench(bool french);
signals:
    void linkRequested(const QUrl& url);
private:
    SanctuaryFeed* m_feed;
};
