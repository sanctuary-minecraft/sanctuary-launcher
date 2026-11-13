// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <QJsonObject>
#include <QWidget>
#include <memory>
#include "ui/pages/BasePage.h"

class QTimer;
class SanctuarySession;
class QUrl;

/** Sanctuary's UI; website authorization and Minecraft launching live outside this widget. */
class SanctuaryHome : public QWidget, public BasePage {
    Q_OBJECT
public:
    enum class View { Adaptive, Dashboard, Setup };
    explicit SanctuaryHome(QWidget* parent = nullptr, View view = View::Adaptive);
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
    QString id() const override { return m_view == View::Setup ? "setup" : "home"; }
    QString displayName() const override { return m_view == View::Setup ? (m_french ? "Bien démarrer" : "Getting started") : (m_french ? "Accueil" : "Home"); }
    QIcon icon() const override { return QIcon(":/sanctuary/icon.png"); }
    void openedImpl() override;
    bool communityUnlocked() const;
    void setDiscordSession(SanctuarySession* session);
    void setDiscordState(bool connected, bool busy, const QJsonObject& profile, const QString& message);
    void setMicrosoftTestState(bool enabled, bool verified, const QString& profile = {}, const QString& message = {});
    void setGameBusy(bool busy);
    bool canLaunchGame() const;

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
    void discordRequested();
    void discordDisconnectRequested();
    void discordRefreshRequested();
    void microsoftTestRequested();

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
    View m_view;
    bool m_onlineSession = false, m_discordConnected = false, m_discordBusy = false;
    QJsonObject m_discordProfile;
    QString m_discordMessage;
    bool m_microsoftDevTest = false, m_microsoftVerified = false;
    bool m_gameBusy = false;
    QString m_microsoftProfile, m_microsoftMessage;
    void buildUi();
    void refresh();
    void refreshClock();
    void commitLocalChange();
    void install();
    QString text(const char* french, const char* english) const;
    bool m_instanceReady = false;
    bool m_microsoftAvailable = false;
    bool m_french = true;
};
