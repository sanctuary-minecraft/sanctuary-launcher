// SPDX-License-Identifier: GPL-3.0-only
#include <QMainWindow>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include "ui/SanctuaryWorkspace.h"
#include "ui/SanctuaryFeed.h"
#include "ui/pages/BasePage.h"

class SamplePage : public QWidget, public BasePage {
public:
    explicit SamplePage(QString key) : m_id(key) {}
    QString id() const override { return m_id; }
    QString displayName() const override { return m_id; }
    QIcon icon() const override { return {}; }
    bool shouldDisplay() const override { return visible; }
    bool apply() override { ++saves; return valid; }
    void openedImpl() override { ++opens; }
    void closedImpl() override { ++closes; }
    bool visible = true, valid = true;
    int saves = 0, opens = 0, closes = 0;
private:
    QString m_id;
};

class SanctuaryWorkspaceTest : public QObject {
    Q_OBJECT
private slots:
    void pagesStayInsideTheSingleWindow()
    {
        QMainWindow window;
        auto* workspace = new SanctuaryWorkspace(&window);
        window.setCentralWidget(workspace);
        auto* mods = new SamplePage("mods");
        auto* captures = new SamplePage("screenshots");
        workspace->setPages({mods,captures});
        window.resize(1120,760);
        window.show();
        QVERIFY(workspace->selectPage("mods"));
        QTest::mouseClick(workspace->findChild<QPushButton*>("nav-screenshots"),Qt::LeftButton);
        QCOMPARE(workspace->selectedPage(), static_cast<BasePage*>(captures));
        QCOMPARE(captures->window(), static_cast<QWidget*>(&window));
        QCOMPARE(mods->window(), static_cast<QWidget*>(&window));
        QVERIFY(mods->saves > 0);
        QVERIFY(mods->closes > 0);
        QSignalSpy home(workspace,&SanctuaryWorkspace::homeRequested);
        QTest::mouseClick(workspace->findChild<QPushButton*>("workspaceHome"),Qt::LeftButton);
        QCOMPARE(home.count(),1);
    }
    void invalidSettingsKeepTheCurrentPageAndItsData()
    {
        SanctuaryWorkspace workspace;
        auto* settings = new SamplePage("settings");
        auto* mods = new SamplePage("mods");
        workspace.setPages({settings,mods});
        workspace.show();
        settings->valid=false;
        QTest::mouseClick(workspace.findChild<QPushButton*>("nav-mods"),Qt::LeftButton);
        QCOMPARE(workspace.selectedPage(),static_cast<BasePage*>(settings));
        QVERIFY(workspace.findChild<QPushButton*>("nav-settings")->isChecked());
        QVERIFY(!workspace.findChild<QPushButton*>("nav-mods")->isChecked());
        QSignalSpy home(&workspace,&SanctuaryWorkspace::homeRequested);
        QVERIFY(!workspace.requestClose());
        QCOMPARE(home.count(),0);
    }
    void nonApplicablePagesAreNotAdvertised()
    {
        SanctuaryWorkspace workspace;
        auto* mods = new SamplePage("mods");
        auto* legacy = new SamplePage("coremods");
        legacy->visible=false;
        workspace.setPages({mods,legacy});
        QVERIFY(!workspace.selectPage("coremods"));
        QVERIFY(workspace.findChild<QPushButton*>("nav-coremods")->isHidden());
        workspace.setFrench(false);
        QCOMPARE(workspace.findChild<QPushButton*>("workspaceHome")->text(),QString("Home"));
    }
    void homeIsAboveTheGameLogsAndNavigationIsAccessible()
    {
        SanctuaryWorkspace workspace;
        auto* home = new SamplePage("home");
        auto* news = new SamplePage("news");
        auto* logs = new SamplePage("console");
        workspace.setPages({home,news});
        workspace.appendPages({logs});
        workspace.show();
        QApplication::processEvents();
        auto* homeNav=workspace.findChild<QPushButton*>("nav-home");
        auto* logsNav=workspace.findChild<QPushButton*>("nav-console");
        QVERIFY(homeNav->y() < logsNav->y());
        logsNav->setChecked(true); // Accessibility toggles the selection without synthesizing a mouse click.
        QCOMPARE(workspace.selectedPage(),static_cast<BasePage*>(logs));
        homeNav->setChecked(true);
        QCOMPARE(workspace.selectedPage(),static_cast<BasePage*>(home));
    }
    void newsUsesPublishedArticlesRatherThanTheWholeLoginPage()
    {
        const auto content=SanctuaryFeed::publicNewsArticles("<form>login</form><article class=\"release-entry\"><h2>Beta 373</h2><p>Publication</p><script>bad()</script></article>");
        QVERIFY(content.contains("Beta 373"));
        QVERIFY(!content.contains("login"));
        QVERIFY(!content.contains("bad()"));
        const auto table=SanctuaryFeed::publicNewsArticles("<nav>login</nav><table><tr id=\"version-beta_373\" data-infinite-id=\"beta.373\"><th scope=\"row\"><span>beta.373</span><time>8 Nov 2026</time></th><td><strong>Published title</strong><p>Published summary</p></td><td>downloads</td></tr></table>");
        QVERIFY(table.contains("beta.373"));
        QVERIFY(table.contains("Published summary"));
        QVERIFY(!table.contains("downloads"));
        QVERIFY(!table.contains("login"));
    }
    void unopenedSettingsDoNotOverwritePreferences()
    {
        SanctuaryWorkspace workspace;
        auto* launcher = new SamplePage("launcher");
        auto* language = new SamplePage("language");
        language->valid = false; // Catalogue not ready; this page has never been opened.
        workspace.setPages({launcher, language}, true);
        QVERIFY(workspace.saveAll());
        QCOMPARE(language->saves, 0);
        QVERIFY(workspace.selectPage("language"));
        QVERIFY(!workspace.saveAll());
    }
};
QTEST_MAIN(SanctuaryWorkspaceTest)
#include "SanctuaryWorkspace_test.moc"
