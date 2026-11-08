// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QJsonObject>
#include <QWidget>
#include <memory>
#include "ui/pages/BasePage.h"

class QTimer;
class QUrl;

/** Sanctuary's dedicated entry point; account secrets and game launching stay in Prism. */
class SanctuaryHome : public QWidget, public BasePage {
    Q_OBJECT
public:
    explicit SanctuaryHome(QWidget* parent = nullptr);
    ~SanctuaryHome() override;
    void setState(bool instanceReady, bool microsoftAvailable, bool french);
    void setLocalMode(bool enabled, const QJsonObject& savedState = {});
    void setLocalScenario(const QString& scenario);
    QJsonObject localState() const;
    int activeStep() const;
    bool isLocalMode() const { return m_localMode; }
    void setPackState(bool verified, bool busy, const QString& version, const QString& message, int progress = -1);
    void showFeedback(const QString& message);
    void setEmbedded(bool embedded);
    QString id() const override { return "home"; }
    QString displayName() const override { return m_french ? "Accueil" : "Home"; }
    QIcon icon() const override { return QIcon(":/sanctuary/icon.png"); }
    void openedImpl() override;
    bool communityUnlocked() const;

signals:
    void installRequested();
    void playRequested();
    void accountsRequested();
    void settingsRequested();
    void advancedRequested();
    void instanceToolRequested(const QString& page);
    void linkRequested(const QUrl& url);
    void localStateChanged(const QJsonObject& state);
    void localPlayRequested();

private:
    enum class Access { None, Pending, Synchronizing, Granted, Rejected, Revoked };
    struct LocalState {
        bool discord = false;
        bool microsoft = false;
        Access access = Access::None;
        bool installed = false;
        bool historical = false;
    };
    struct Ui;
    std::unique_ptr<Ui> m_ui;
    QTimer* m_installTimer = nullptr;
    LocalState m_local;
    bool m_localMode = false;
    bool m_codeVisible = false;
    int m_progress = 0;
    bool m_realPack = false;
    bool m_packVerified = false;
    bool m_packBusy = false;
    int m_packProgress = -1;
    QString m_packVersion, m_packMessage;
    bool m_communityVisible = false;
    bool m_embedded = false;
    void buildUi();
    void refresh();
    void commitLocalChange();
    void install();
    QString text(const char* french, const char* english) const;
    bool m_instanceReady = false;
    bool m_microsoftAvailable = false;
    bool m_french = true;
};
