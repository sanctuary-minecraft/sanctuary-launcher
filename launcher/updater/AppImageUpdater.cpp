// SPDX-License-Identifier: GPL-3.0-only
#include "AppImageUpdater.h"
#include "Application.h"
#include "BuildConfig.h"
#include "settings/SettingsObject.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <openssl/evp.h>
#include <algorithm>

namespace {
const QString origin = QStringLiteral("https://sanctuary-minecraft.net/launcher-updates/");
const QString asset = QStringLiteral("Sanctuary-Launcher-linux-x64.AppImage");
constexpr int maxManifest = 8192;
constexpr int maxImage = 256 * 1024 * 1024;

bool verifySignature(const QByteArray& key, const QByteArray& message, const QByteArray& signature)
{
    const auto bytes = [](const QByteArray& value) { return reinterpret_cast<const unsigned char*>(value.constData()); };
    if (key.size() != 32 || signature.size() != 64) return false;
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> publicKey(
        EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, bytes(key), key.size()), EVP_PKEY_free);
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    return publicKey && context && EVP_DigestVerifyInit(context.get(), nullptr, nullptr, nullptr, publicKey.get()) == 1
        && EVP_DigestVerify(context.get(), bytes(signature), signature.size(), bytes(message), message.size()) == 1;
}

QNetworkRequest requestFor(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    request.setTransferTimeout(30000);
    return request;
}

bool ok(const QNetworkReply* reply)
{
    return reply->error() == QNetworkReply::NoError && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
}

QString message(const char* fr, const char* en)
{
    return QString::fromUtf8(APPLICATION->settings()->get("Language").toString().startsWith("fr") ? fr : en);
}
}

std::optional<AppImageRelease> AppImageRelease::verify(const QByteArray& envelope, const QByteArray& publicKey)
{
    if (envelope.size() > maxManifest) return {};
    const auto wrapper = QJsonDocument::fromJson(envelope).object();
    const auto payload = QByteArray::fromBase64(wrapper.value("payload").toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    const auto signature = QByteArray::fromBase64(wrapper.value("signature").toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    if (!verifySignature(publicKey, payload, signature)) return {};
    const auto data = QJsonDocument::fromJson(payload).object();
    const QString versionString = data.value("version").toString();
    const auto version = QVersionNumber::fromString(versionString);
    const auto shaHex = data.value("sha256").toString().toLatin1();
    const auto sha = QByteArray::fromHex(shaHex);
    const int size = data.value("size").toInt();
    if (data.value("schema").toInt() != 1 || data.value("platform").toString() != "linux-x64"
        || version.segmentCount() != 3 || version.toString() != versionString
        || sha.size() != 32 || sha.toHex() != shaHex || size <= 0 || size > maxImage)
        return {};
    return AppImageRelease{version, sha, size, QUrl(origin + "releases/" + versionString + '/' + asset)};
}

QString AppImageUpdater::runningImage()
{
    const QFileInfo image(qEnvironmentVariable("APPIMAGE"));
    if (!image.isAbsolute() || !image.isFile() || image.isSymLink() || image.suffix() != "AppImage"
        || !image.isExecutable() || !QFileInfo(image.absolutePath()).isWritable())
        return {};
    return image.absoluteFilePath();
}

AppImageUpdater::AppImageUpdater(QWidget* window, QNetworkAccessManager* network, const QString& dataDirectory)
    : m_window(window), m_network(network), m_image(runningImage()),
      m_settings(QDir(dataDirectory).filePath("launcher-updates.ini"), QSettings::IniFormat)
{
    if (!m_image.isEmpty()) ::lstat(QFile::encodeName(m_image).constData(), &m_original);
    connect(&m_timer, &QTimer::timeout, this, [this] { check(false); });
    connect(qApp, &QCoreApplication::aboutToQuit, this, &AppImageUpdater::install);
    schedule();
    if (getAutomaticallyChecksForUpdates())
        QTimer::singleShot(5000, this, [this] { if (getAutomaticallyChecksForUpdates()) check(false); });
}

AppImageUpdater::~AppImageUpdater()
{
    if (m_reply) { disconnect(m_reply, nullptr, this, nullptr); m_reply->abort(); m_reply->deleteLater(); }
}

void AppImageUpdater::checkForUpdates() { check(true); }
bool AppImageUpdater::getAutomaticallyChecksForUpdates() { return m_settings.value("automatic", true).toBool(); }
double AppImageUpdater::getUpdateCheckInterval() { return m_settings.value("interval", 86400).toInt(); }

void AppImageUpdater::setAutomaticallyChecksForUpdates(bool enabled)
{
    m_settings.setValue("automatic", enabled);
    if (!enabled && m_pending && !m_pendingManual) {
        m_pending.reset();
        m_lock.reset();
    }
    schedule();
}

void AppImageUpdater::setUpdateCheckInterval(double seconds)
{
    m_settings.setValue("interval", static_cast<int>(std::clamp(seconds, 0.0, 604800.0)));
    schedule();
}

void AppImageUpdater::schedule()
{
    m_timer.stop();
    if (getAutomaticallyChecksForUpdates() && getUpdateCheckInterval() > 0)
        m_timer.start(static_cast<int>(std::clamp(getUpdateCheckInterval(), 3600.0, 604800.0) * 1000));
}

void AppImageUpdater::report(bool manual, const QString& text)
{
    if (!manual) { qInfo().noquote() << text; return; }
    auto* box = new QMessageBox(QMessageBox::Information, QStringLiteral("Sanctuary Launcher"), text, QMessageBox::Ok, m_window);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->open();
}

bool AppImageUpdater::originalImagePresent() const
{
    struct stat current {};
    return !m_image.isEmpty() && ::lstat(QFile::encodeName(m_image).constData(), &current) == 0
        && S_ISREG(current.st_mode) && m_original.st_dev == current.st_dev && m_original.st_ino == current.st_ino
        && m_original.st_size == current.st_size && m_original.st_mtime == current.st_mtime;
}

void AppImageUpdater::check(bool manual)
{
    if (m_pending) {
        if (manual) report(true, message("Mise à jour prête. Elle sera installée à la fermeture.", "Update ready. It will install on quit."));
        return;
    }
    if (m_reply || !originalImagePresent()) return;

    auto* reply = m_network->get(requestFor(QUrl(origin + "stable/linux-x64.json")));
    m_reply = reply;
    reply->setReadBufferSize(maxManifest + 1);
    connect(reply, &QIODevice::readyRead, this, [reply] {
        if (reply->bytesAvailable() > maxManifest) reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, manual] {
        m_reply.clear();
        reply->deleteLater();
        if (!manual && !getAutomaticallyChecksForUpdates()) return;
        const auto key = QByteArray::fromBase64(BuildConfig.SANCTUARY_UPDATE_KEY.toLatin1());
        const auto release = ok(reply) ? AppImageRelease::verify(reply->readAll(), key) : std::nullopt;
        if (!release) {
            report(manual, message("Le canal de mise à jour n’a pas pu être vérifié.", "The update channel could not be verified."));
            return;
        }
        if (release->version <= QVersionNumber(BuildConfig.VERSION_MAJOR, BuildConfig.VERSION_MINOR, BuildConfig.VERSION_PATCH)) {
            if (manual) report(true, message("Le launcher est à jour.", "The launcher is up to date."));
            return;
        }
        download(*release, manual);
    });
}

void AppImageUpdater::download(const AppImageRelease& release, bool manual)
{
    auto lock = std::make_unique<QLockFile>(m_image + ".update.lock");
    lock->setStaleLockTime(0);
    if (!lock->tryLock() || !originalImagePresent()) return;
    auto staged = std::make_shared<QSaveFile>(m_image);
    if (!staged->open(QIODevice::WriteOnly) || !staged->setPermissions(QFileInfo(m_image).permissions())) {
        report(manual, message("Le fichier AppImage ne peut pas être mis à jour.", "The AppImage file cannot be updated."));
        return;
    }
    m_lock = std::move(lock);
    auto* reply = m_network->get(requestFor(release.url));
    m_reply = reply;
    reply->setReadBufferSize(256 * 1024);
    auto hash = std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
    auto consume = [reply, staged, hash, release] {
        const auto chunk = reply->readAll();
        if (staged->pos() + chunk.size() > release.size || staged->write(chunk) != chunk.size()) {
            staged->cancelWriting();
            return false;
        }
        hash->addData(chunk);
        return true;
    };
    connect(reply, &QIODevice::readyRead, this, [reply, consume] { if (!consume()) reply->abort(); });
    connect(reply, &QNetworkReply::finished, this, [this, reply, staged, hash, release, manual, consume] {
        m_reply.clear();
        reply->deleteLater();
        if (!manual && !getAutomaticallyChecksForUpdates()) {
            m_lock.reset();
            return;
        }
        if (!ok(reply) || !consume() || staged->pos() != release.size || hash->result() != release.sha256) {
            staged->cancelWriting();
            m_lock.reset();
            report(manual, message("La mise à jour a échoué à la vérification.", "The update failed verification."));
            return;
        }
        m_pending = staged;
        m_pendingManual = manual;
        if (manual) report(true, message("Mise à jour prête. Elle sera installée à la fermeture.", "Update ready. It will install on quit."));
    });
}

void AppImageUpdater::install()
{
    if (!m_pending || !originalImagePresent() || !APPLICATION->updatesAreAllowed()) return;
    if (!m_pending->commit()) qWarning() << "Could not replace the AppImage";
    m_pending.reset();
    m_lock.reset();
}
