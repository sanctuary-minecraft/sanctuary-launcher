// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QObject>
#include <QJsonObject>
#include <QPointer>
#include <QUrl>

class BaseInstance;
class QNetworkReply;
class QProcess;
class Task;
class QWidget;

/** Consumer of the existing Sanctuary Update channel; never publishes or starts Minecraft. */
class SanctuaryPack : public QObject {
    Q_OBJECT
public:
    explicit SanctuaryPack(QWidget* owner);
    ~SanctuaryPack() override;
    BaseInstance* instance() const;
    bool busy() const { return m_busy; }
    bool verified() const { return m_verified; }
    QString version() const { return m_version; }
    QString message() const { return m_message; }
    int progress() const { return m_progress; }
    void installOrUpdate();
    void cancel();
    static bool parseManifest(const QByteArray& bytes, const QUrl& resolved, QString* version, QString* error);
    static bool verifyManagedFiles(const QString& gameRoot, const QByteArray& packHash, QString* error);
    static bool verifyTools(const QString& gameRoot, QString* error);
signals:
    void changed();
    void instanceReady();
private:
    void synchronize();
    void runInstaller(const QByteArray& manifest, const QUrl& resolved);
    void fail(const QString& reason);
    bool restoreDisabledMods();
    bool m_busy = false;
    bool m_verified = false;
    bool m_importing = false;
    bool m_cancelled = false;
    int m_progress = -1;
    QString m_version, m_message;
    QByteArray m_manifestHash;
    QJsonObject m_disabledMods;
    QPointer<Task> m_task;
    QPointer<QNetworkReply> m_reply;
    QProcess* m_process = nullptr;
    QWidget* m_owner;
};
