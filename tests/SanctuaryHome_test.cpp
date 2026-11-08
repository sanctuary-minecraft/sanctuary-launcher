// SPDX-License-Identifier: GPL-3.0-only
#include <QLabel>
#include <QDir>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QUrl>

#include "ui/SanctuaryHome.h"

class SanctuaryHomeTest : public QObject {
    Q_OBJECT
private slots:
    void layoutFitsBothLanguages()
    {
        for (bool french : {true, false}) {
            SanctuaryHome home;
            home.setState(false, false, french);
            home.resize(1280, 800);
            home.show();
            QApplication::processEvents();
            QVERIFY(home.width() <= 1280);
            QVERIFY(home.height() <= 800);
            auto* primary = home.findChild<QPushButton*>("primary");
            const QRect action(primary->mapTo(&home, QPoint(0, 0)), primary->size());
            QVERIFY(home.rect().contains(action));
            const auto capture = qEnvironmentVariable("SANCTUARY_CAPTURE_DIR");
            if (!capture.isEmpty()) {
                QVERIFY(QDir().mkpath(capture));
                QVERIFY(home.grab().save(capture + (french ? "/sanctuary-home-fr.png" : "/sanctuary-home-en.png")));
            }
        }
    }

    void freshInstallationWorksWithoutMicrosoft()
    {
        SanctuaryHome home;
        home.resize(1280, 800);
        home.show();
        auto* primary = home.findChild<QPushButton*>("primary");
        QVERIFY(primary);
        QVERIFY(primary->isEnabled());
        QCOMPARE(primary->text(), QString("INSTALLER"));
        QSignalSpy installation(&home, &SanctuaryHome::installRequested);
        QSignalSpy play(&home, &SanctuaryHome::playRequested);
        QTest::mouseClick(primary, Qt::LeftButton);
        QCOMPARE(installation.count(), 1);
        QCOMPARE(play.count(), 0);
    }

    void readyProfileCannotLaunchWithMissingMicrosoftApplication()
    {
        SanctuaryHome home;
        home.setState(true, false, true);
        home.show();
        auto* primary = home.findChild<QPushButton*>("primary");
        QCOMPARE(primary->text(), QString("JOUER"));
        QVERIFY(!primary->isEnabled());
        QVERIFY(!home.findChild<QPushButton*>("account")->isEnabled());
        QSignalSpy play(&home, &SanctuaryHome::playRequested);
        QTest::mouseClick(primary, Qt::LeftButton);
        QCOMPARE(play.count(), 0);
    }

    void englishSettingsAndAdvancedActionsStayConnectedAfterStateChange()
    {
        SanctuaryHome home;
        home.setState(false, false, false);
        home.resize(1280, 800);
        home.show();
        QCOMPARE(home.findChild<QPushButton*>("primary")->text(), QString("INSTALL"));
        QSignalSpy settings(&home, &SanctuaryHome::settingsRequested);
        QSignalSpy advanced(&home, &SanctuaryHome::advancedRequested);
        QTest::mouseClick(home.findChild<QPushButton*>("settings"), Qt::LeftButton);
        QTest::mouseClick(home.findChild<QPushButton*>("advanced"), Qt::LeftButton);
        QCOMPARE(settings.count(), 1);
        QCOMPARE(advanced.count(), 1);
    }

    void newsOpensExistingPublicArticleAndNeverClaimsServerOnline()
    {
        SanctuaryHome home;
        home.resize(1280, 800);
        home.show();
        QSignalSpy links(&home, &SanctuaryHome::linkRequested);
        const auto articles = home.findChildren<QPushButton*>("articleLink");
        QCOMPARE(articles.count(), 3);
        QTest::mouseClick(articles.first(), Qt::LeftButton);
        QCOMPARE(links.count(), 1);
        QCOMPARE(links.at(0).at(0).toUrl(), QUrl("https://sanctuary-minecraft.net/actualites#version-beta_372"));
        bool foundUnknown = false;
        for (auto* label : home.findChildren<QLabel*>()) {
            QVERIFY(!label->text().contains("Serveur en ligne"));
            QVERIFY(!label->text().contains("Pack à jour"));
            foundUnknown |= label->text().contains("non vérifié");
        }
        QVERIFY(foundUnknown);
    }
};

QTEST_MAIN(SanctuaryHomeTest)
#include "SanctuaryHome_test.moc"
