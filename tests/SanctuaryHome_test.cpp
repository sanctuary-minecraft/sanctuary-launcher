// SPDX-License-Identifier: GPL-3.0-only
#include <QApplication>
#include <QDir>
#include <QComboBox>
#include <QJsonObject>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>
#include "ui/SanctuaryHome.h"

class SanctuaryHomeTest : public QObject {
    Q_OBJECT
    static QPushButton* action(SanctuaryHome& home, const char* name) { return home.findChild<QPushButton*>(name); }
    static QLabel* label(SanctuaryHome& home, const char* name) { return home.findChild<QLabel*>(name); }
private slots:
    void localJourneyWithoutAccountsOrNetwork()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.resize(1280, 800);
        home.show();
        QSignalSpy realPlay(&home, &SanctuaryHome::playRequested);
        QSignalSpy realInstall(&home, &SanctuaryHome::installRequested);
        QSignalSpy accounts(&home, &SanctuaryHome::accountsRequested);
        QSignalSpy links(&home, &SanctuaryHome::linkRequested);
        QSignalSpy persistence(&home, &SanctuaryHome::localStateChanged);
        QSignalSpy previewPlay(&home, &SanctuaryHome::localPlayRequested);
        QCOMPARE(home.activeStep(), 1);
        QTest::mouseClick(action(home,"discord"), Qt::LeftButton);
        QCOMPARE(home.activeStep(), 2);
        QTest::mouseClick(action(home,"microsoft"), Qt::LeftButton);
        QCOMPARE(home.activeStep(), 3);
        QCOMPARE(label(home,"heading")->text(), QString("Demander à rejoindre Sanctuary"));
        QTest::mouseClick(action(home,"access"), Qt::LeftButton);
        QVERIFY(label(home,"heading")->text().contains("En attente"));
        QVERIFY(action(home,"install")->isEnabled());
        QVERIFY(!action(home,"play")->isEnabled());
        QVERIFY(action(home,"codeToggle")->isHidden());
        QTest::mouseClick(action(home,"install"), Qt::LeftButton);
        QVERIFY(!action(home,"install")->isEnabled());
        QVERIFY(!home.findChild<QProgressBar*>("installationProgress")->isHidden());
        QTRY_VERIFY_WITH_TIMEOUT(home.localState().value("installed").toBool(), 6000);
        QVERIFY(!action(home,"play")->isEnabled());
        home.setLocalScenario("accepted");
        QVERIFY(home.localState().value("installed").toBool());
        QCOMPARE(home.activeStep(), 5);
        QVERIFY(action(home,"play")->isEnabled());
        QVERIFY(label(home,"codeValue")->isHidden() || !label(home,"codeValue")->isVisible());
        QTest::mouseClick(action(home,"play"), Qt::LeftButton);
        QCOMPARE(previewPlay.count(), 1);
        QCOMPARE(realPlay.count(), 0);
        QCOMPARE(realInstall.count(), 0);
        QCOMPARE(accounts.count(), 0);
        QCOMPARE(links.count(), 0);
        QVERIFY(persistence.count() >= 5);
    }
    void codeIsOptionalAndNeverBecomesAnAdmissionRequirement()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.setLocalScenario("ready");
        home.show();
        QApplication::processEvents();
        QVERIFY(action(home,"play")->isEnabled());
        QVERIFY(!label(home,"codeValue")->isVisible());
        const auto before = home.localState();
        QApplication::processEvents();
        QTest::mouseClick(action(home,"readyCodeToggle"), Qt::LeftButton);
        QCOMPARE(home.activeStep(), 4);
        QVERIFY(label(home,"readyCodeValue")->isVisible());
        QCOMPARE(label(home,"readyCodeValue")->font().family(), QString("Sga"));
        QVERIFY(action(home,"play")->isEnabled());
        QCOMPARE(home.localState(), before);
        QTest::mouseClick(action(home,"readyCodeToggle"), Qt::LeftButton);
        QCOMPARE(home.activeStep(), 5);
        QVERIFY(action(home,"play")->isEnabled());
        QCOMPARE(home.localState(), before);
    }
    void blockedAdmissionNeverAllowsPlayEvenWithPreparedGame()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.show();
        for (const auto& scenario : {"pending-ready", "sync", "rejected", "revoked"}) {
            home.setLocalScenario(scenario);
            QVERIFY(home.localState().value("installed").toBool());
            QCOMPARE(home.activeStep(), 3);
            QVERIFY(!action(home,"play")->isEnabled());
            QVERIFY(action(home,"advanced")->isEnabled());
            QVERIFY(action(home,"settings")->isEnabled());
            QVERIFY(action(home,"codeToggle")->isHidden());
        }
    }
    void scenarioChooserWorksThroughNativeButtons()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.show();
        auto* choice = home.findChild<QComboBox*>("scenarios");
        QVERIFY(choice);
        choice->setCurrentIndex(choice->findData("pending-ready"));
        QCOMPARE(home.activeStep(), 3);
        QVERIFY(home.localState().value("installed").toBool());
        QVERIFY(!action(home,"play")->isEnabled());
    }
    void localProgressRestoresButCannotLeakIntoNormalMode()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.setLocalScenario("ready");
        const auto saved = home.localState();
        SanctuaryHome restored;
        restored.setLocalMode(true, saved);
        QCOMPARE(restored.localState(), saved);
        QVERIFY(action(restored,"play")->isEnabled());
        restored.setLocalMode(false, saved);
        restored.setState(true, true, true);
        restored.setLocalScenario("ready"); // scenario controls are inert outside preview
        QVERIFY(!action(restored,"play")->isEnabled());
        QVERIFY(label(restored,"historicalBadge")->isHidden());
        QVERIFY(label(restored,"previewNotice")->parentWidget()->isHidden());
        QVERIFY(action(restored,"codeToggle")->isHidden());
        QVERIFY(!restored.localState().value("discord").toBool());
    }
    void corruptOrOutdatedPreviewStateDoesNotInventAccess()
    {
        SanctuaryHome home;
        home.setLocalMode(true, {{"version",1},{"discord",false},{"microsoft",false},{"access",3},{"installed",true}});
        QCOMPARE(home.activeStep(), 1);
        QCOMPARE(home.localState().value("access").toInt(), 0);
        QVERIFY(!action(home,"play")->isEnabled());
        home.setLocalMode(true, {{"version",17},{"discord",true},{"microsoft",true},{"access",3},{"installed",true}});
        QCOMPARE(home.activeStep(), 1);
        QVERIFY(!action(home,"play")->isEnabled());
    }
    void nativeToolsAndPublicLinksRemainUsableAfterLanguageChange()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.setLocalScenario("pending-ready");
        home.setState(false, false, false);
        home.show();
        QSignalSpy tools(&home, &SanctuaryHome::advancedRequested);
        QSignalSpy pages(&home, &SanctuaryHome::instanceToolRequested);
        QSignalSpy settings(&home, &SanctuaryHome::settingsRequested);
        QSignalSpy links(&home, &SanctuaryHome::linkRequested);
        QTest::mouseClick(action(home,"advanced"), Qt::LeftButton);
        QTest::mouseClick(action(home,"mods"), Qt::LeftButton);
        QTest::mouseClick(action(home,"shaders"), Qt::LeftButton);
        QTest::mouseClick(action(home,"instanceSettings"), Qt::LeftButton);
        QTest::mouseClick(action(home,"settings"), Qt::LeftButton);
        QTest::mouseClick(action(home,"news"), Qt::LeftButton);
        QTest::mouseClick(action(home,"site"), Qt::LeftButton);
        QCOMPARE(tools.count(), 1);
        QCOMPARE(pages.count(), 4);
        QCOMPARE(pages[0][0].toString(), QString("mods"));
        QCOMPARE(pages[1][0].toString(), QString("shaderpacks"));
        QCOMPARE(pages[2][0].toString(), QString("settings"));
        QCOMPARE(settings.count(), 1);
        QCOMPARE(pages[3][0].toString(), QString("news"));
        QCOMPARE(links[0][0].toUrl(), QUrl("https://sanctuary-minecraft.net/"));
        QVERIFY(label(home,"heading")->text().contains("Pending"));
    }
    void normalImportRemainsSeparateFromSimulatedInstallation()
    {
        SanctuaryHome home;
        QSignalSpy installs(&home, &SanctuaryHome::installRequested);
        QSignalSpy play(&home, &SanctuaryHome::playRequested);
        home.show();
        QVERIFY(!action(home,"microsoft")->isEnabled());
        QTest::mouseClick(action(home,"install"), Qt::LeftButton);
        QCOMPARE(installs.count(), 1);
        QCOMPARE(play.count(), 0);
        QVERIFY(!action(home,"play")->isEnabled());
        QVERIFY(label(home,"serverStatus")->text().contains("non vérifié"));
    }
    void realPackCannotBeReplacedBySimulatedInstallationState()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.setLocalScenario("ready");
        home.setPackState(false, false, {}, "Pack officiel à télécharger");
        home.show();
        QSignalSpy install(&home, &SanctuaryHome::installRequested);
        QVERIFY(!action(home,"play")->isEnabled());
        QTest::mouseClick(action(home,"install"), Qt::LeftButton);
        QCOMPARE(install.count(), 1);
        home.setPackState(true, false, "beta.373", "Pack vérifié");
        QVERIFY(action(home,"play")->isEnabled());
        home.setPackState(false, true, "beta.373", "Synchronisation");
        QVERIFY(!action(home,"play")->isEnabled());
        QVERIFY(home.findChild<QLabel*>("packHeading")->text() == "Sanctuary Beta");
        QVERIFY(home.findChild<QWidget*>("inlineAccounts")->isVisible());
    }
    void layoutAndRenderedStatesInBothLanguages()
    {
        const auto capture = qEnvironmentVariable("SANCTUARY_CAPTURE_DIR");
        if (!capture.isEmpty()) QVERIFY(QDir().mkpath(capture));
        for (bool french : {true, false}) {
            SanctuaryHome home;
            home.setLocalMode(true);
            home.setState(false, false, french);
            home.resize(1280, 800);
            home.show();
            for (const auto& scenario : {"start", "microsoft", "apply", "pending", "pending-ready", "sync", "accepted", "ready", "rejected", "revoked"}) {
                home.setLocalScenario(scenario);
                QApplication::processEvents();
                QVERIFY(home.width() <= 1280);
                auto* scroll = home.findChild<QScrollArea*>();
                QVERIFY(scroll->widget()->width() <= scroll->viewport()->width());
                const auto heading = label(home,"packHeading");
                QVERIFY(heading->height() >= heading->fontMetrics().height());
                if (!capture.isEmpty() && QStringList{"start","pending","ready"}.contains(scenario))
                    QVERIFY(home.grab().save(capture + "/" + scenario + (french ? "-fr.png" : "-en.png")));
            }
        }
    }
    void homeSwitchesFromStepsToCommunityWithoutRequiringTheCode()
    {
        SanctuaryHome home;
        home.setLocalMode(true);
        home.setEmbedded(true);
        home.setPackState(false, false, {}, "Pack à installer");
        home.show();
        QVERIFY(home.findChild<QWidget*>("inlineAccounts")->isVisible());
        QVERIFY(home.findChild<QWidget*>("communityHome")->isHidden());
        home.setLocalScenario("ready");
        QVERIFY(!home.communityUnlocked());
        home.setPackState(true, false, "beta.373", "Pack vérifié");
        QVERIFY(home.communityUnlocked());
        QVERIFY(home.findChild<QWidget*>("inlineAccounts")->isHidden());
        QVERIFY(home.findChild<QWidget*>("communityHome")->isVisible());
        QVERIFY(!label(home,"readyCodeValue")->isVisible());
        home.setLocalScenario("pending-ready");
        QVERIFY(home.findChild<QWidget*>("communityHome")->isHidden());
        QVERIFY(home.findChild<QWidget*>("inlineAccounts")->isVisible());
    }
};
QTEST_MAIN(SanctuaryHomeTest)
#include "SanctuaryHome_test.moc"
