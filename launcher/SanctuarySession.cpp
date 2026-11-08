// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuarySession.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>
#ifdef Q_OS_MACOS
#include <Security/Security.h>
#endif
namespace {
const QString origin = "https://sanctuary-minecraft.net";
#ifdef Q_OS_MACOS
QByteArray keychain(const QString& account, const QByteArray* value = nullptr)
{
    const auto utf8 = account.toUtf8();
    auto key = CFStringCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(utf8.constData()), utf8.size(), kCFStringEncodingUTF8, false);
    const void* keys[] = {kSecClass, kSecAttrService, kSecAttrAccount};
    const void* values[] = {kSecClassGenericPassword, CFSTR("net.sanctuary-minecraft.launcher.web"), key};
    auto query = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    for (int i = 0; i < 3; ++i) CFDictionarySetValue(query, keys[i], values[i]);
    QByteArray result;
    if (value) {
        if (value->isEmpty()) SecItemDelete(query);
        else {
            auto data = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(value->constData()), value->size());
            auto update = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(update, kSecValueData, data);
            auto status = SecItemUpdate(query, update);
            if (status == errSecItemNotFound) {
                CFDictionarySetValue(query, kSecValueData, data);
                CFDictionarySetValue(query, kSecAttrAccessible, kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly);
                SecItemAdd(query, nullptr);
            }
            CFRelease(update); CFRelease(data);
        }
    } else {
        CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
        CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
        CFTypeRef data = nullptr;
        if (SecItemCopyMatching(query, &data) == errSecSuccess && data && CFGetTypeID(data) == CFDataGetTypeID())
            result = QByteArray(reinterpret_cast<const char*>(CFDataGetBytePtr(static_cast<CFDataRef>(data))), CFDataGetLength(static_cast<CFDataRef>(data)));
        if (data) CFRelease(data);
    }
    CFRelease(query); CFRelease(key);
    return result;
}
#endif
QJsonObject json(QNetworkReply* reply)
{
    const auto bytes = reply->readAll();
    return bytes.size() <= 2 * 1024 * 1024 ? QJsonDocument::fromJson(bytes).object() : QJsonObject{};
}
}
SanctuarySession::SanctuarySession(QNetworkAccessManager* network, QObject* parent) : QObject(parent), m_network(network)
{
    m_poll.setSingleShot(true);
    connect(&m_poll, &QTimer::timeout, this, &SanctuarySession::poll);
    m_refresh.setInterval(60000);
    connect(&m_refresh, &QTimer::timeout, this, &SanctuarySession::refresh);
}
QString SanctuarySession::text(const char* fr, const char* en) const { return QString::fromUtf8(m_french ? fr : en); }
void SanctuarySession::restore(const QString& profile)
{
    m_store = QString::fromLatin1(QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Sha256).toHex());
#ifdef Q_OS_MACOS
    const auto saved = keychain(m_store);
    if (QRegularExpression("^[a-f0-9]{64}$").match(QString::fromLatin1(saved)).hasMatch()) m_token = QString::fromLatin1(saved);
#endif
    if (connected()) { emit credentialsChanged(); refresh(); }
}
void SanctuarySession::saveToken()
{
#ifdef Q_OS_MACOS
    if (!m_store.isEmpty()) { const auto token = m_token.toLatin1(); keychain(m_store, &token); }
#endif
}
QNetworkReply* SanctuarySession::request(const QString& path, const QJsonObject& body, bool post)
{
    QNetworkRequest request(QUrl(origin + "/api/launcher/v1/" + path));
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setTransferTimeout(20000);
    if (connected() && path != "device" && path != "token") request.setRawHeader("Authorization", "Bearer " + m_token.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return post ? m_network->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact)) : m_network->get(request);
}
void SanctuarySession::finish(const QString& message) { m_busy = false; m_message = message; emit changed(); }
void SanctuarySession::start()
{
    if (m_busy || connected()) return;
    const int generation = ++m_generation;
    m_busy = true; m_profile = {}; m_device.clear(); m_interval = 5000;
    m_message = text("Préparation de la connexion Discord…", "Preparing Discord sign-in…"); emit changed();
    auto* reply = request("device", {{"client_id", "sanctuary-launcher"}}, true);
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        const auto data = json(reply); const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); reply->deleteLater();
        if (generation != m_generation) return;
        const auto device = data.value("device_code").toString();
        const auto code = data.value("user_code").toString();
        const QUrl url(data.value("verification_uri_complete").toString());
        if (status != 200 || !QRegularExpression("^[a-f0-9]{64}$").match(device).hasMatch()
            || !QRegularExpression("^[A-F0-9]{10}$").match(code).hasMatch() || url.scheme() != "https"
            || url.host() != "sanctuary-minecraft.net" || url.port(-1) != -1 || url.hasFragment() || url.path() != "/launcher/connect" || !url.userInfo().isEmpty()
            || QUrlQuery(url).queryItemValue("code") != code) {
            finish(text("Connexion indisponible. Réessayez.", "Sign-in unavailable. Please retry.")); return;
        }
        m_device = device; m_elapsed.start();
        m_message = text("Validez dans le navigateur · code %1", "Confirm in your browser · code %1").arg(code);
        emit changed(); emit browserRequested(url); m_poll.start(m_interval);
    });
}
void SanctuarySession::poll()
{
    if (m_device.isEmpty()) return;
    if (m_elapsed.elapsed() > 600000) { m_device.clear(); finish(text("Connexion expirée. Réessayez.", "Sign-in expired. Please retry.")); return; }
    const int generation = m_generation;
    auto* reply = request("token", {{"device_code", m_device}}, true); m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        const auto data = json(reply); const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); reply->deleteLater();
        if (generation != m_generation) return;
        const auto error = data.value("error").toString();
        if (error == "authorization_pending" || error == "slow_down") {
            if (error == "slow_down") m_interval = qMin(30000, m_interval + 5000);
            m_poll.start(m_interval); return;
        }
        const auto token = data.value("access_token").toString();
        if (status == 200 && QRegularExpression("^[a-f0-9]{64}$").match(token).hasMatch()) {
            m_token = token; m_device.clear(); m_busy = false; saveToken(); emit credentialsChanged(); refresh(); return;
        }
        if (status == 0 || status >= 500 || status == 429) { m_poll.start(qMax(10000, m_interval)); return; }
        m_device.clear(); finish(error == "access_denied" ? text("Connexion annulée dans le navigateur.", "Sign-in cancelled in the browser.")
                                                         : text("Connexion expirée ou interrompue. Réessayez.", "Sign-in expired or interrupted. Please retry."));
    });
}
bool SanctuarySession::validProfile(const QJsonObject& data)
{
    return data.value("version").toInt() == 1 && data.value("user").isObject()
        && !data.value("user").toObject().value("display_name").toString().isEmpty()
        && QStringList{"none","pending","granted","synchronizing","rejected","revoked","guild_required"}.contains(data.value("access").toObject().value("status").toString());
}
void SanctuarySession::refresh()
{
    if (!connected() || m_busy) return;
    const int generation = m_generation;
    m_busy = true; m_message = text("Lecture de votre accès Sanctuary…", "Reading your Sanctuary access…"); emit changed();
    auto* reply = request("profile"); m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); const auto data = json(reply); reply->deleteLater();
        if (generation != m_generation) return;
        if (status == 401 || status == 403) { clearCredentials(); finish(text("Reconnectez Discord.", "Reconnect Discord.")); return; }
        m_profile = status == 200 && validProfile(data) ? data : QJsonObject{};
        m_refresh.start();
        finish(m_profile.isEmpty() ? text("Accès non vérifié. Actualisez pour réessayer.", "Access not verified. Refresh to retry.")
                                   : text("Discord connecté · dossier Sanctuary retrouvé", "Discord connected · Sanctuary profile found"));
    });
}
void SanctuarySession::clearCredentials()
{
    ++m_generation; m_poll.stop(); m_refresh.stop(); m_token.clear(); m_device.clear(); m_profile = {}; saveToken(); emit credentialsChanged();
}
void SanctuarySession::disconnectAccount()
{
    if (connected()) { auto* reply = request("revoke", {}, true); connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater); }
    clearCredentials(); if (m_reply) m_reply->abort(); finish(text("Discord déconnecté du launcher.", "Discord disconnected from the launcher."));
}
QNetworkReply* SanctuarySession::feed(const QString& kind, bool french)
{
    return request("feed?kind=" + kind + "&lang=" + (french ? "fr" : "en"));
}

QNetworkReply* SanctuarySession::photo(const QString& id)
{
    if (!connected() || !QRegularExpression("^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$").match(id).hasMatch()) return nullptr;
    return request("posts/" + id + "/photo");
}
