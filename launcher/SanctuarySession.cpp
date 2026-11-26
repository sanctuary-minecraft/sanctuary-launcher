// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuarySession.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>
#ifdef Q_OS_MACOS
#include <Security/Security.h>
#elif defined(Q_OS_WIN)
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <windows.h>
#include <dpapi.h>
#elif defined(SANCTUARY_QT_KEYCHAIN)
#include <qt6keychain/keychain.h>
#endif
namespace {
const QString origin = "https://sanctuary-minecraft.net";
const QRegularExpression tokenPattern(QStringLiteral("^[a-f0-9]{64}$"));
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
#elif defined(Q_OS_WIN)
QByteArray readWindowsToken(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() <= 0 || file.size() > 64 * 1024) return {};
    auto encrypted = file.readAll();
    DATA_BLOB input{static_cast<DWORD>(encrypted.size()), reinterpret_cast<BYTE*>(encrypted.data())};
    DATA_BLOB decrypted{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &decrypted)) return {};
    const QByteArray token(reinterpret_cast<const char*>(decrypted.pbData), decrypted.cbData);
    SecureZeroMemory(decrypted.pbData, decrypted.cbData);
    LocalFree(decrypted.pbData);
    return token;
}

bool saveWindowsToken(const QString& path, QByteArray token)
{
    if (token.isEmpty()) return QFile::remove(path) || !QFile::exists(path);
    DATA_BLOB input{static_cast<DWORD>(token.size()), reinterpret_cast<BYTE*>(token.data())};
    DATA_BLOB encrypted{};
    if (!CryptProtectData(&input, L"Sanctuary Launcher", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &encrypted)) return false;
    QSaveFile file(path);
    const bool saved = file.open(QIODevice::WriteOnly)
        && file.write(reinterpret_cast<const char*>(encrypted.pbData), encrypted.cbData) == encrypted.cbData
        && file.commit();
    LocalFree(encrypted.pbData);
    return saved;
}
#elif defined(SANCTUARY_QT_KEYCHAIN)
const QString keychainService = QStringLiteral("net.sanctuary-minecraft.launcher.web");
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
#ifdef Q_OS_WIN
    m_store = QDir(profile).filePath(QStringLiteral("discord-session.dpapi"));
#else
    m_store = QString::fromLatin1(QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Sha256).toHex());
#endif
#if defined(Q_OS_MACOS)
    restoreToken(keychain(m_store));
#elif defined(Q_OS_WIN)
    restoreToken(readWindowsToken(m_store));
#elif defined(SANCTUARY_QT_KEYCHAIN)
    const int generation = m_generation;
    auto* job = new QKeychain::ReadPasswordJob(keychainService, this);
    job->setKey(m_store);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this, [this, job, generation] {
        if (generation == m_generation)
            restoreToken(job->error() == QKeychain::NoError ? job->binaryData() : QByteArray{});
    });
    m_busy = true;
    m_message = text("Vérification de la session Discord…", "Checking your Discord session…");
    emit changed();
    job->start();
#endif
}

void SanctuarySession::restoreToken(const QByteArray& token)
{
    m_busy = false;
    m_message.clear();
    if (tokenPattern.match(QString::fromLatin1(token)).hasMatch()) {
        m_token = QString::fromLatin1(token);
        emit credentialsChanged();
        refresh();
    } else {
        emit changed();
    }
}

void SanctuarySession::saveToken(std::function<void()> finished)
{
    if (m_store.isEmpty()) {
        if (finished) finished();
        return;
    }
#if defined(Q_OS_MACOS)
    const auto token = m_token.toLatin1();
    keychain(m_store, &token);
#elif defined(Q_OS_WIN)
    if (!saveWindowsToken(m_store, m_token.toLatin1()))
        qWarning() << "Couldn't update the protected Sanctuary session on Windows";
#elif defined(SANCTUARY_QT_KEYCHAIN)
    auto* job = [&]() -> QKeychain::Job* {
        if (m_token.isEmpty()) return new QKeychain::DeletePasswordJob(keychainService, QCoreApplication::instance());
        auto* write = new QKeychain::WritePasswordJob(keychainService, QCoreApplication::instance());
        write->setBinaryData(m_token.toLatin1());
        return write;
    }();
    job->setKey(m_store);
    job->setInsecureFallback(false);
    connect(job, &QKeychain::Job::finished, this, [job, finished = std::move(finished)] {
        if (job->error() != QKeychain::NoError && job->error() != QKeychain::EntryNotFound)
            qWarning() << "Couldn't update the Sanctuary session in the desktop keyring";
        if (finished) finished();
    });
    job->start();
    return;
#endif
    if (finished) finished();
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
        if (status == 200 && tokenPattern.match(token).hasMatch()) {
            m_token = token;
            m_device.clear();
            saveToken([this, generation] {
                if (generation != m_generation) return;
                m_busy = false;
                emit credentialsChanged();
                refresh();
            });
            return;
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

QNetworkReply* SanctuarySession::mapTicket()
{
    return connected() ? request("map/ticket", {}, true) : nullptr;
}
