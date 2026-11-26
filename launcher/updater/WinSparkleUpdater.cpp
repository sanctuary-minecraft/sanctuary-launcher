// SPDX-License-Identifier: GPL-3.0-only
#include "WinSparkleUpdater.h"
#include "Application.h"
#include "BuildConfig.h"
#include "settings/SettingsObject.h"

#include <QDateTime>
#include <QSettings>
#include <algorithm>
#include <atomic>
#include <winsparkle.h>

namespace {
std::atomic_bool canClose = false;

int __cdecl canShutdown()
{
    return canClose.load() ? 1 : 0;
}

void __cdecl shutdownRequested()
{
    QMetaObject::invokeMethod(qApp, &QApplication::closeAllWindows, Qt::QueuedConnection);
}
}

WinSparkleUpdater::WinSparkleUpdater()
{
    const auto version = BuildConfig.versionString().toStdWString();
    m_ready = win_sparkle_set_eddsa_public_key(BuildConfig.SANCTUARY_UPDATE_KEY.toLatin1().constData()) != 0;
    if (!m_ready) {
        qWarning() << "WinSparkle rejected the Sanctuary update key";
        return;
    }
    win_sparkle_set_app_details(L"Sanctuary", L"Sanctuary Launcher", version.c_str());
    constexpr auto registry = "Software\\SanctuaryLauncher\\WinSparkle";
    win_sparkle_set_registry_path(registry);
    QSettings preferences(QStringLiteral("HKEY_CURRENT_USER\\Software\\SanctuaryLauncher\\WinSparkle"), QSettings::NativeFormat);
    if (!preferences.contains("CheckForUpdates")) win_sparkle_set_automatic_check_for_updates(1);
    win_sparkle_set_lang(APPLICATION->settings()->get("Language").toString().startsWith("fr") ? "fr" : "en");
    win_sparkle_set_appcast_url("https://sanctuary-minecraft.net/launcher-updates/stable/windows-x64.xml");
    canClose = APPLICATION->updatesAreAllowed();
    connect(APPLICATION, &Application::updateAllowedChanged, this, [](bool allowed) { canClose = allowed; });
    win_sparkle_set_can_shutdown_callback(canShutdown);
    win_sparkle_set_shutdown_request_callback(shutdownRequested);
    const auto lastCheck = win_sparkle_get_last_check_time();
    const auto nextCheck = lastCheck + win_sparkle_get_update_check_interval();
    win_sparkle_init();
    // WinSparkle starts its normal periodic check when the interval has elapsed.
    // Check on relaunch as well when that timer would still be waiting.
    if (nextCheck > QDateTime::currentSecsSinceEpoch() + 3) {
        m_startupCheck.setSingleShot(true);
        connect(&m_startupCheck, &QTimer::timeout, this, [this, lastCheck] {
            if (getAutomaticallyChecksForUpdates() && win_sparkle_get_last_check_time() == lastCheck)
                win_sparkle_check_update_without_ui();
        });
        m_startupCheck.start(3000);
    }
}

WinSparkleUpdater::~WinSparkleUpdater()
{
    canClose = false;
    if (m_ready) win_sparkle_cleanup();
}

void WinSparkleUpdater::checkForUpdates()
{
    if (m_ready && canClose.load()) {
        m_startupCheck.stop();
        win_sparkle_check_update_with_ui();
    }
}

bool WinSparkleUpdater::getAutomaticallyChecksForUpdates()
{
    return m_ready && win_sparkle_get_automatic_check_for_updates();
}

double WinSparkleUpdater::getUpdateCheckInterval()
{
    return m_ready ? win_sparkle_get_update_check_interval() : 86400;
}

void WinSparkleUpdater::setAutomaticallyChecksForUpdates(bool check)
{
    if (m_ready) win_sparkle_set_automatic_check_for_updates(check ? 1 : 0);
}

void WinSparkleUpdater::setUpdateCheckInterval(double seconds)
{
    if (m_ready) win_sparkle_set_update_check_interval(static_cast<int>(std::clamp(seconds, 3600.0, 604800.0)));
}
