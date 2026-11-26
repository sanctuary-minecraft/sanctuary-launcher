// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "ExternalUpdater.h"

#include <QCryptographicHash>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>
#include <memory>
#include <optional>
#include <sys/stat.h>

class QLockFile;
class QNetworkAccessManager;
class QNetworkReply;
class QSaveFile;
class QWidget;

struct AppImageRelease {
    QVersionNumber version;
    QByteArray sha256;
    qint64 size;
    QUrl url;

    static std::optional<AppImageRelease> verify(const QByteArray& envelope, const QByteArray& publicKey);
};

class AppImageUpdater final : public ExternalUpdater {
public:
    AppImageUpdater(QWidget* window, QNetworkAccessManager* network, const QString& dataDirectory);
    ~AppImageUpdater() override;

    static QString runningImage();
    void checkForUpdates() override;
    bool getAutomaticallyChecksForUpdates() override;
    double getUpdateCheckInterval() override;
    bool getBetaAllowed() override { return false; }
    void setAutomaticallyChecksForUpdates(bool enabled) override;
    void setUpdateCheckInterval(double seconds) override;
    void setBetaAllowed(bool) override {}

private:
    void schedule();
    void check(bool manual);
    void download(const AppImageRelease& release, bool manual);
    void report(bool manual, const QString& message);
    bool originalImagePresent() const;
    void install();

    QPointer<QWidget> m_window;
    QNetworkAccessManager* m_network;
    const QString m_image;
    struct stat m_original {};
    QSettings m_settings;
    QTimer m_timer;
    QPointer<QNetworkReply> m_reply;
    std::unique_ptr<QLockFile> m_lock;
    std::shared_ptr<QSaveFile> m_pending;
    bool m_pendingManual = false;
};
