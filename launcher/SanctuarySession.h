// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QObject>
#include <QJsonObject>
#include <QPointer>
#include <QTimer>
#include <QUrl>
#include <QElapsedTimer>
#include <functional>
class QNetworkAccessManager;
class QNetworkReply;

/** Read-only Sanctuary session. Discord credentials stay in the system browser/site. */
class SanctuarySession : public QObject {
    Q_OBJECT
public:
    explicit SanctuarySession(QNetworkAccessManager* network, QObject* parent = nullptr);
    void restore(const QString& profile);
    void start();
    void refresh();
    void disconnectAccount();
    void setFrench(bool french) { m_french = french; }
    bool connected() const { return !m_token.isEmpty(); }
    bool busy() const { return m_busy; }
    QString message() const { return m_message; }
    QJsonObject profile() const { return m_profile; }
    QNetworkReply* feed(const QString& kind, bool french);
    QNetworkReply* photo(const QString& id);
    QNetworkReply* mapTicket();
    static bool validProfile(const QJsonObject& value);
signals:
    void changed();
    void credentialsChanged();
    void browserRequested(const QUrl& url);
private:
    QNetworkReply* request(const QString& path, const QJsonObject& body = {}, bool post = false);
    void poll();
    void finish(const QString& message);
    void clearCredentials();
    void restoreToken(const QByteArray& token);
    void saveToken(std::function<void()> finished = {});
    QString text(const char* fr, const char* en) const;
    QNetworkAccessManager* m_network;
    QPointer<QNetworkReply> m_reply;
    QTimer m_poll, m_refresh;
    QElapsedTimer m_elapsed;
    QString m_token, m_device, m_message, m_store;
    QJsonObject m_profile;
    int m_generation = 0, m_interval = 5000;
    bool m_busy = false, m_french = true;
};
