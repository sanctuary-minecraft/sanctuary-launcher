// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "ExternalUpdater.h"
#include <QTimer>

class WinSparkleUpdater final : public ExternalUpdater {
public:
    WinSparkleUpdater();
    ~WinSparkleUpdater() override;
    void checkForUpdates() override;
    bool getAutomaticallyChecksForUpdates() override;
    double getUpdateCheckInterval() override;
    bool getBetaAllowed() override { return false; }
    void setAutomaticallyChecksForUpdates(bool check) override;
    void setUpdateCheckInterval(double seconds) override;
    void setBetaAllowed(bool) override {}

private:
    bool m_ready = false;
    QTimer m_startupCheck;
};
