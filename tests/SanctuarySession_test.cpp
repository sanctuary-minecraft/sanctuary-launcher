// SPDX-License-Identifier: GPL-3.0-only
#include <QTest>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include "SanctuarySession.h"

class MemoryReply : public QNetworkReply {
public:
    MemoryReply(const QNetworkRequest& request, const QJsonObject& data, QObject* parent) : QNetworkReply(parent), m_data(QJsonDocument(data).toJson())
    {
        setRequest(request); setUrl(request.url()); setAttribute(QNetworkRequest::HttpStatusCodeAttribute,200);
        open(QIODevice::ReadOnly);
        QTimer::singleShot(0,this,[this] { if (!isFinished()) { setFinished(true); emit readyRead(); emit finished(); } });
    }
    void abort() override { if (!isFinished()) { setFinished(true); setError(OperationCanceledError,"cancelled"); emit finished(); } }
    qint64 bytesAvailable() const override { return m_data.size()-m_position+QIODevice::bytesAvailable(); }
protected:
    qint64 readData(char* data,qint64 max) override
    {
        const auto size=qMin(max,qint64(m_data.size()-m_position));
        if (size<=0) return -1;
        memcpy(data,m_data.constData()+m_position,size); m_position+=size; return size;
    }
private:
    QByteArray m_data; qint64 m_position=0;
};
class FakeWebsite : public QNetworkAccessManager {
public:
    bool invalidBrowser=false;
    QList<QNetworkRequest> requests;
protected:
    QNetworkReply* createRequest(Operation op,const QNetworkRequest& req,QIODevice* body) override
    {
        Q_UNUSED(op); Q_UNUSED(body);
        requests.append(req);
        QJsonObject response;
        const auto path=req.url().path();
        if (path.endsWith("/device")) response={{"device_code",QString(64,'a')},{"user_code","AB12CD34EF"},{"verification_uri_complete",invalidBrowser ? "https://untrusted.example/launcher/connect?code=AB12CD34EF" : "https://sanctuary-minecraft.net/launcher/connect?code=AB12CD34EF"}};
        else if (path.endsWith("/token")) response={{"access_token",QString(64,'b')}};
        else if (path.endsWith("/profile")) response={{"version",1},{"user",QJsonObject{{"display_name","Example player"}}},{"access",QJsonObject{{"status","granted"}}}};
        else if (path.endsWith("/revoke")) response={{"revoked",true}};
        else response={{"version",1},{"entries",QJsonArray{}}};
        return new MemoryReply(req,response,this);
    }
};
class SanctuarySessionTest : public QObject {
    Q_OBJECT
private slots:
    void browserApprovalProducesReadOnlyRequestsAndLogoutClearsIdentity()
    {
        FakeWebsite website;
        SanctuarySession session(&website);
        QSignalSpy browser(&session,&SanctuarySession::browserRequested);
        session.start();
        QTRY_COMPARE(browser.count(),1);
        QVERIFY(!session.connected());
        QTRY_VERIFY_WITH_TIMEOUT(!session.profile().isEmpty(),7000);
        QVERIFY(session.connected());
        QCOMPARE(website.requests[0].rawHeader("Authorization"),QByteArray());
        QCOMPARE(website.requests[1].rawHeader("Authorization"),QByteArray());
        QCOMPARE(website.requests[2].rawHeader("Authorization"),QByteArray("Bearer ")+QByteArray(64,'b'));
        auto* feed=session.feed("article",true);
        connect(feed,&QNetworkReply::finished,feed,&QObject::deleteLater);
        QCOMPARE(website.requests.last().url().host(),QString("sanctuary-minecraft.net"));
        auto* photo=session.photo("11111111-1111-4111-8111-111111111111");
        QVERIFY(photo);
        connect(photo,&QNetworkReply::finished,photo,&QObject::deleteLater);
        QCOMPARE(website.requests.last().url(),QUrl("https://sanctuary-minecraft.net/api/launcher/v1/posts/11111111-1111-4111-8111-111111111111/photo"));
        QCOMPARE(website.requests.last().rawHeader("Authorization"),QByteArray("Bearer ")+QByteArray(64,'b'));
        QVERIFY(!session.photo("../../outside?token=test"));
        session.disconnectAccount();
        QVERIFY(!session.connected());
        QVERIFY(!session.photo("11111111-1111-4111-8111-111111111111"));
        QVERIFY(session.profile().isEmpty());
        QVERIFY(!session.busy());
        QVERIFY(website.requests.last().url().path().endsWith("/revoke"));
    }
    void untrustedVerificationPagesAreNeverOpened()
    {
        FakeWebsite website; website.invalidBrowser=true;
        SanctuarySession session(&website);
        QSignalSpy browser(&session,&SanctuarySession::browserRequested);
        session.start();
        QTRY_VERIFY(!session.busy());
        QCOMPARE(browser.count(),0);
        QVERIFY(!session.connected());
    }
};
QTEST_MAIN(SanctuarySessionTest)
#include "SanctuarySession_test.moc"
