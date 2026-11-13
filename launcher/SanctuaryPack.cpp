// SPDX-License-Identifier: GPL-3.0-only
#include "SanctuaryPack.h"
#include "Application.h"
#include "BaseInstance.h"
#include "InstanceImportTask.h"
#include "InstanceList.h"
#include "settings/SettingsObject.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <QWidget>

namespace {
QString packText(const char* fr, const char* en)
{
    const auto* app = qobject_cast<Application*>(QCoreApplication::instance());
    const bool french = !app || app->settings()->get("Language").toString().startsWith("fr");
    return QString::fromUtf8(french ? fr : en);
}
QJsonObject objectFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.readAll()).object();
}
bool digestMatches(const QString& fileName, const QJsonObject& expected)
{
    const auto kind = expected.value("type").toString();
    if (kind != "sha256" && kind != "sha512") return false;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QCryptographicHash hash(kind == "sha256" ? QCryptographicHash::Sha256 : QCryptographicHash::Sha512);
    return hash.addData(&file) && hash.result().toHex() == expected.value("value").toString().toLatin1();
}
QString managedPath(const QString& root, const QString& relative)
{
    if (relative.isEmpty() || QDir::isAbsolutePath(relative)) return {};
    const QString cleaned = QDir::cleanPath(relative);
    if (cleaned == ".." || cleaned.startsWith("../")) return {};
    const auto candidate = QDir(root).absoluteFilePath(cleaned);
    const auto canonical = QFileInfo(candidate).canonicalFilePath();
    if (!canonical.isEmpty() && !canonical.startsWith(QDir(root).canonicalPath() + "/")) return {};
    return candidate;
}
const QUrl archiveUrl("https://sanctuary-minecraft.net/downloads/sanctuary-prism.zip");
const QUrl channelUrl("https://git.botsu.net/koka/sanctuary-beta/raw/branch/packwiz/pack.toml");
}

SanctuaryPack::SanctuaryPack(QWidget* owner) : QObject(owner), m_owner(owner)
{
    m_version = APPLICATION->settings()->get("SanctuaryPackVersion").toString();
    m_manifestHash = APPLICATION->settings()->get("SanctuaryPackHash").toString().toLatin1();
    QString error;
    if (auto* inst = instance(); inst && !m_manifestHash.isEmpty())
        m_verified = verifyTools(inst->gameRoot(), &error) && verifyManagedFiles(inst->gameRoot(), m_manifestHash, &error);
    m_message = m_verified ? packText("Pack vérifié localement", "Pack verified locally") : packText("Installer le pack officiel depuis le site Sanctuary", "Install the official pack from the Sanctuary website");
    if (!error.isEmpty()) m_message = error;
    connect(APPLICATION->instances(), &InstanceList::instanceSelectRequest, this, [this](const QString& id) {
        if (m_importing) APPLICATION->settings()->set("SanctuaryInstanceId", id);
    });
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        const auto lines = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed().split('\n');
        if (!lines.isEmpty() && !lines.last().isEmpty()) { m_message = lines.last().left(300); emit changed(); }
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) { restoreDisabledMods(); fail(packText("Impossible de démarrer Java pour préparer le pack.", "Java could not start to prepare the pack.")); }
    });
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus status) {
        if (!restoreDisabledMods()) {
            fail(packText("Impossible de rétablir les mods facultatifs désactivés. Vérifiez l'onglet Mods avant de jouer.",
                          "Could not restore disabled optional mods. Check the Mods tab before playing."));
            return;
        }
        QString error;
        auto* inst = instance();
        if (m_cancelled) { fail(packText("Synchronisation annulée. Réessayez pour vérifier le pack.", "Synchronization cancelled. Retry to verify the pack.")); return; }
        if (code != 0 || status != QProcess::NormalExit || !inst) { fail(packText("La synchronisation du pack a échoué. Réessayez.", "Pack synchronization failed. Please retry.")); return; }
        if (!verifyManagedFiles(inst->gameRoot(), m_manifestHash, &error)) { fail(error); return; }
        APPLICATION->settings()->set("SanctuaryPackVersion", m_version);
        APPLICATION->settings()->set("SanctuaryPackHash", QString::fromLatin1(m_manifestHash));
        m_busy = false;
        m_verified = true;
        m_progress = 100;
        m_message = packText("Pack officiel vérifié · %1", "Official pack verified · %1").arg(m_version);
        emit changed();
        emit instanceReady();
    });
}

SanctuaryPack::~SanctuaryPack()
{
    if (m_process->state() != QProcess::NotRunning) { m_process->terminate(); m_process->waitForFinished(3000); }
    restoreDisabledMods();
}
BaseInstance* SanctuaryPack::instance() const
{
    return APPLICATION->instances()->getInstanceById(APPLICATION->settings()->get("SanctuaryInstanceId").toString());
}
void SanctuaryPack::fail(const QString& reason)
{
    m_busy = false;
    m_importing = false;
    m_verified = false;
    m_progress = -1;
    m_message = reason;
    emit changed();
}

bool SanctuaryPack::verifyTools(const QString& root, QString* error)
{
    const auto installer = objectFile(":/sanctuary/packwiz-installer.json").value("installer_sha256").toString();
    const QList<QPair<QString, QString>> tools{
        {"packwiz-installer-bootstrap.jar", "a8fbb24dc604278e97f4688e82d3d91a318b98efc08d5dbfcbcbcab6443d116c"},
        {"packwiz-installer.jar", installer}};
    for (const auto& tool : tools) if (tool.second.isEmpty() || !digestMatches(QDir(root).filePath(tool.first), {{"type","sha256"},{"value",tool.second}})) {
        *error = packText("L'installateur officiel ne correspond pas aux empreintes attendues : %1", "The official installer does not match the expected hashes: %1").arg(tool.first);
        return false;
    }
    return true;
}

bool SanctuaryPack::parseManifest(const QByteArray& bytes, const QUrl& resolved, QString* version, QString* error)
{
    const QRegularExpression immutable("^/distribution/packwiz/channels/[0-9a-f]{40}/pack\\.toml$");
    const auto match = QRegularExpression("(?m)^version\\s*=\\s*\"(beta\\.[0-9]{3,})\"\\s*$").match(QString::fromUtf8(bytes));
    if (resolved.scheme() != "https" || resolved.host() != "sanctuary-minecraft.net" || !immutable.match(resolved.path()).hasMatch()
        || resolved.hasQuery() || resolved.hasFragment() || bytes.size() > 65536 || !match.hasMatch()) {
        *error = packText("Le canal Sanctuary n'a pas fourni un manifeste Beta immuable reconnu.", "The Sanctuary channel did not return a recognized immutable Beta manifest.");
        return false;
    }
    *version = match.captured(1);
    return true;
}

bool SanctuaryPack::verifyManagedFiles(const QString& root, const QByteArray& packHash, QString* error)
{
    const auto state = objectFile(QDir(root).filePath("packwiz.json"));
    const auto savedHash = state.value("packFileHash").toObject();
    if (packHash.isEmpty() || savedHash.value("type") != "sha256" || savedHash.value("value").toString().toLatin1() != packHash
        || state.value("cachedSide") != "client") {
        *error = packText("Le pack n'a pas encore été vérifié pour le profil client.", "The pack has not yet been verified for the client profile."); return false;
    }
    const auto files = state.value("cachedFiles").toObject();
    const auto required = files.value("mods/sanctuary.pw.toml").toObject();
    if (required.value("cachedLocation").toString().isEmpty() || required.value("isOptional").toBool() || !required.value("optionValue").toBool()) {
        *error = packText("Le mod Sanctuary manque dans le profil vérifié.", "The verified profile is missing the Sanctuary mod."); return false;
    }
    for (auto it = files.begin(); it != files.end(); ++it) {
        const auto file = it.value().toObject();
        const auto relative = file.value("cachedLocation").toString();
        if (relative.isEmpty() || (file.value("isOptional").toBool() && !file.value("optionValue").toBool())) continue;
        auto path = managedPath(root, relative);
        if (path.isEmpty()) { *error = packText("Chemin de fichier géré incorrect.", "Invalid managed file path."); return false; }
        if (!QFile::exists(path) && file.value("isOptional").toBool() && QFile::exists(path + ".disabled")) path += ".disabled";
        auto hash = file.value("linkedFileHash").toObject();
        if (hash.isEmpty()) hash = file.value("hash").toObject();
        if (!digestMatches(path, hash)) { *error = packText("Fichier géré absent ou modifié : %1", "Managed file missing or modified: %1").arg(relative); return false; }
    }
    return true;
}

void SanctuaryPack::installOrUpdate()
{
    if (m_busy) return;
    if (!APPLICATION->updatesAreAllowed() || (instance() && instance()->isRunning())) { m_message = packText("Fermez le jeu avant de mettre à jour le pack.", "Close the game before updating the pack."); emit changed(); return; }
    m_busy = true;
    m_cancelled = false;
    m_verified = false;
    m_progress = -1;
    m_message = packText("Téléchargement du pack officiel…", "Downloading the official pack…");
    emit changed();
    if (instance()) { synchronize(); return; }
    // Recover a previously committed official profile before creating another instance.
    for (int i = 0; i < APPLICATION->instances()->count(); ++i) {
        auto* existing = APPLICATION->instances()->at(i);
        if (existing->name() == "Sanctuary Beta" && QFile::exists(QDir(existing->gameRoot()).filePath("packwiz-installer-bootstrap.jar"))) {
            APPLICATION->settings()->set("SanctuaryInstanceId", existing->id());
            synchronize(); return;
        }
    }
    m_importing = true;
    auto* child = new InstanceImportTask(archiveUrl, true, m_owner);
    child->setName("Sanctuary Beta");
    child->setGroup("Sanctuary");
    child->setIcon("default");
    auto* task = APPLICATION->instances()->wrapInstanceTask(child);
    task->setParent(this);
    m_task = task;
    connect(task, &Task::progress, this, [this](qint64 current, qint64 total) {
        m_progress = total > 0 ? int(current * 100 / total) : -1;
        emit changed();
    });
    connect(task, &Task::failed, this, [this, task](const QString& why) { task->deleteLater(); fail(why); });
    connect(task, &Task::aborted, this, [this, task] { task->deleteLater(); fail(packText("Installation annulée.", "Installation cancelled.")); });
    connect(task, &Task::succeeded, this, [this, task] {
        m_importing = false;
        task->deleteLater();
        if (m_cancelled) { fail(packText("Installation annulée.", "Installation cancelled.")); return; }
        QTimer::singleShot(0, this, [this] {
            if (!instance()) APPLICATION->instances()->loadList();
            if (!instance()) { fail(packText("Le profil importé n'a pas été retrouvé.", "The imported profile could not be found.")); return; }
            emit instanceReady();
            synchronize();
        });
    });
    task->start();
}

void SanctuaryPack::synchronize()
{
    auto* inst = instance();
    QString error;
    if (!inst || !verifyTools(inst->gameRoot(), &error)) { fail(error); return; }
    m_message = packText("Vérification du canal Sanctuary Update…", "Checking the Sanctuary Update channel…");
    m_progress = -1;
    emit changed();
    QNetworkRequest request(channelUrl);
    request.setTransferTimeout(30000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto* reply = APPLICATION->network()->get(request);
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto bytes = reply->readAll();
        const auto resolved = reply->url();
        const auto networkError = reply->error();
        reply->deleteLater();
        if (m_cancelled) { fail(packText("Synchronisation annulée.", "Synchronization cancelled.")); return; }
        if (networkError != QNetworkReply::NoError) { fail(packText("Le canal de mise à jour est indisponible. Réessayez.", "The update channel is unavailable. Please retry.")); return; }
        QString error;
        if (!parseManifest(bytes, resolved, &m_version, &error)) { fail(error); return; }
        runInstaller(bytes, resolved);
    });
}

void SanctuaryPack::runInstaller(const QByteArray& manifest, const QUrl& resolved)
{
    auto* inst = instance();
    if (!inst) { fail(packText("Profil Sanctuary absent.", "Sanctuary profile missing.")); return; }
    QString java = APPLICATION->settings()->get("JavaPath").toString();
    if (!QFileInfo(java).isExecutable()) java = QStandardPaths::findExecutable("java");
    if (java.isEmpty()) { fail(packText("Choisissez un exécutable Java dans les réglages du launcher.", "Choose a Java executable in launcher settings.")); return; }
    m_manifestHash = QCryptographicHash::hash(manifest, QCryptographicHash::Sha256).toHex();
    const auto root = inst->gameRoot();
    const auto files = objectFile(QDir(root).filePath("packwiz.json")).value("cachedFiles").toObject();
    m_disabledMods = {};
    for (auto it = files.begin(); it != files.end(); ++it) {
        const auto record = it.value().toObject();
        if (!record.value("isOptional").toBool()) continue;
        const auto path = managedPath(root, record.value("cachedLocation").toString());
        if (!path.isEmpty() && !QFile::exists(path) && QFile::exists(path + ".disabled")) {
            if (!digestMatches(path + ".disabled", record.value("linkedFileHash").toObject())) {
                restoreDisabledMods(); fail(packText("Un mod facultatif désactivé a été modifié : %1", "A disabled optional mod has been modified: %1").arg(record.value("cachedLocation").toString())); return;
            }
            if (!QFile::rename(path + ".disabled", path)) { restoreDisabledMods(); fail(packText("Impossible de conserver le choix d'un mod facultatif.", "Could not preserve an optional mod selection.")); return; }
            m_disabledMods.insert(it.key(), record.value("cachedLocation"));
        }
    }
    m_message = packText("Synchronisation du pack · %1", "Synchronizing pack · %1").arg(m_version);
    emit changed();
    m_process->setWorkingDirectory(root);
    m_process->start(java, {"-jar", QDir(root).filePath("packwiz-installer-bootstrap.jar"),
        "--bootstrap-main-jar", QDir(root).filePath("packwiz-installer.jar"), "--bootstrap-no-update", "-g", "-s", "client",
        "--pack-folder", root, "--multimc-folder", inst->instanceRoot(), resolved.toString()});
}

bool SanctuaryPack::restoreDisabledMods()
{
    auto* inst = instance();
    if (!inst) return m_disabledMods.isEmpty();
    const auto root = inst->gameRoot();
    const auto files = objectFile(QDir(root).filePath("packwiz.json")).value("cachedFiles").toObject();
    QJsonObject failed;
    for (auto it = m_disabledMods.begin(); it != m_disabledMods.end(); ++it) {
        const auto relative = files.value(it.key()).toObject().value("cachedLocation").toString(it.value().toString());
        const auto path = managedPath(root, relative);
        if (!path.isEmpty() && QFile::exists(path)) {
            if (QFile::exists(path + ".disabled") || !QFile::rename(path, path + ".disabled")) failed.insert(it.key(), it.value());
        }
    }
    m_disabledMods = failed;
    return failed.isEmpty();
}

void SanctuaryPack::cancel()
{
    if (!m_busy) return;
    m_cancelled = true;
    if (m_importing && m_task) { m_task->abort(); return; }
    if (m_reply && !m_reply->isFinished()) { m_reply->abort(); return; }
    if (m_process->state() != QProcess::NotRunning) { m_process->terminate(); return; }
    fail(packText("Opération annulée.", "Operation cancelled."));
}
