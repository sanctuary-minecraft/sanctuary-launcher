// SPDX-License-Identifier: GPL-3.0-only
#include <QMainWindow>
#include <QLabel>
#include <QJsonArray>
#include <QJsonObject>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>
#include <QScrollArea>
#include <QScrollBar>
#include <QBuffer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
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

class FaceReply : public QNetworkReply {
public:
    FaceReply(const QNetworkRequest& request,const QByteArray& image,QObject* parent) : QNetworkReply(parent), m_image(image)
    {
        setRequest(request);
        setUrl(request.url());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute,200);
        open(QIODevice::ReadOnly);
        QTimer::singleShot(0,this,[this] { setFinished(true); emit readyRead(); emit finished(); });
    }
    void abort() override {}
protected:
    qint64 readData(char* target,qint64 maximum) override
    {
        const auto size=qMin(maximum,qint64(m_image.size()-m_position));
        if (size<=0) return -1;
        memcpy(target,m_image.constData()+m_position,size);
        m_position+=size;
        return size;
    }
private:
    QByteArray m_image;
    qint64 m_position=0;
};
class FaceProvider : public QNetworkAccessManager {
public:
    QByteArray image;
    QList<QNetworkRequest> requests;
protected:
    QNetworkReply* createRequest(Operation,const QNetworkRequest& request,QIODevice*) override
    {
        requests.append(request);
        return new FaceReply(request,image,this);
    }
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
    void communityPostsKeepTextSafeAndUsePublicMinecraftFaces()
    {
        SanctuaryFeed feed("article");
        feed.resize(360,240);
        feed.show();
        const QJsonObject post{{"id","11111111-1111-4111-8111-111111111111"},
            {"author_name","Notch"},{"title","<script>title</script>"},{"body","Body <b>as plain text</b>"}};
        feed.setEntries({post});
        QApplication::processEvents();
        auto* title=feed.findChild<QLabel*>("postTitle");
        auto* details=feed.findChild<QWidget*>("postDetails");
        QVERIFY(details->isHidden());
        QSignalSpy external(&feed,&SanctuaryFeed::linkRequested);
        QVERIFY(QMetaObject::invokeMethod(title,"linkActivated",Q_ARG(QString,"toggle")));
        QVERIFY(details->isVisible());
        QCOMPARE(external.count(),0);
        QCOMPARE(feed.findChild<QLabel*>("postAuthor")->text(),QString("Notch"));
        QVERIFY(feed.findChild<QLabel*>("postTitle")->text().contains("&lt;script&gt;"));
        QCOMPARE(feed.findChild<QLabel*>("postBody")->textFormat(),Qt::PlainText);
        QCOMPARE(feed.findChild<QLabel*>("playerFace")->size(),QSize(32,32));
        QCOMPARE(SanctuaryFeed::playerFaceUrl("Notch"),QUrl("https://mc-api.io/render/FACE/Notch/JAVA?size=64"));
        QVERIFY(SanctuaryFeed::playerFaceUrl("../unsafe?token=test").isEmpty());
        QVERIFY(SanctuaryFeed::playerFaceUrl("A display name").isEmpty());
        feed.setEntries({post}); // A periodic refresh must not close the current post.
        QVERIFY(details->isVisible());
        QVERIFY(QMetaObject::invokeMethod(title,"linkActivated",Q_ARG(QString,"toggle")));
        QVERIFY(details->isHidden());
        auto second=post;
        second.insert("id","22222222-2222-4222-8222-222222222222");
        second.insert("body",QString("Long text for scrolling. ").repeated(400));
        feed.setEntries({post,second});
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        const auto titles=feed.findChildren<QLabel*>("postTitle");
        const auto panels=feed.findChildren<QWidget*>("postDetails");
        QVERIFY(QMetaObject::invokeMethod(titles[0],"linkActivated",Q_ARG(QString,"toggle")));
        QVERIFY(QMetaObject::invokeMethod(titles[1],"linkActivated",Q_ARG(QString,"toggle")));
        QApplication::processEvents();
        QVERIFY(panels[0]->isHidden());
        QVERIFY(panels[1]->isVisible());
        QTRY_VERIFY(feed.findChild<QScrollArea*>("postScroll")->verticalScrollBar()->maximum()>0);
        feed.setEntries({});
        for (auto* entry : feed.findChildren<QWidget*>("communityPost")) QVERIFY(!entry->isVisible());
    }
    void playerFacesRenderFromThePublicProviderAndStayCachedInMemory()
    {
        FaceProvider provider;
        QImage skinFace(64,64,QImage::Format_RGB32);
        skinFace.fill(Qt::green);
        QBuffer buffer(&provider.image);
        QVERIFY(buffer.open(QIODevice::WriteOnly));
        QVERIFY(skinFace.save(&buffer,"PNG"));
        SanctuaryFeed feed("article",nullptr,&provider);
        const QJsonObject first{{"id","11111111-1111-4111-8111-111111111111"},{"author_name","PlayerOne"},{"title","First"}};
        auto second=first;
        second.insert("id","22222222-2222-4222-8222-222222222222");
        second.insert("author_name","PlayerTwo");
        feed.setEntries({first,second});
        const auto faces=feed.findChildren<QLabel*>("playerFace");
        QCOMPARE(faces.size(),2);
        QTRY_VERIFY(!faces[0]->pixmap().isNull());
        QTRY_VERIFY(!faces[1]->pixmap().isNull());
        QCOMPARE(faces[0]->pixmap().size(),QSize(32,32));
        QCOMPARE(provider.requests.size(),2);
        for (const auto& request : provider.requests) {
            QCOMPARE(request.url().host(),QString("mc-api.io"));
            QVERIFY(request.rawHeader("Authorization").isEmpty());
            QVERIFY(request.rawHeader("Cookie").isEmpty());
        }
        auto updated=first;
        updated.insert("title","Updated title");
        feed.setEntries({updated});
        QCOMPARE(provider.requests.size(),2);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(!feed.findChild<QLabel*>("playerFace")->pixmap().isNull());
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
