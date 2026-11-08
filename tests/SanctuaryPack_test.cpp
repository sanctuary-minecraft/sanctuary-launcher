// SPDX-License-Identifier: GPL-3.0-only
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>
#include "SanctuaryPack.h"

class SanctuaryPackTest : public QObject {
    Q_OBJECT
    static void write(const QString& path,const QByteArray& data) { QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly)); QCOMPARE(f.write(data),qint64(data.size())); }
    static QJsonObject hash(const QByteArray& data) { return {{"type","sha256"},{"value",QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex())}}; }
private slots:
    void publicImmutableManifestIsRequired()
    {
        QString version,error;
        const QByteArray manifest="name = \"Sanctuary\"\nversion = \"beta.373\"\n";
        const QString path="/distribution/packwiz/channels/"+QString(40,'a')+"/pack.toml";
        QVERIFY(SanctuaryPack::parseManifest(manifest,QUrl("https://sanctuary-minecraft.net"+path),&version,&error));
        QCOMPARE(version,QString("beta.373"));
        QVERIFY(!SanctuaryPack::parseManifest(manifest,QUrl("http://sanctuary-minecraft.net"+path),&version,&error));
        QVERIFY(!SanctuaryPack::parseManifest(manifest,QUrl("https://github.com/private/repo"),&version,&error));
        QVERIFY(!SanctuaryPack::parseManifest(manifest,QUrl("https://sanctuary-minecraft.net/pack.toml"),&version,&error));
    }
    void hashesAndOptionalDisabledFilesAreVerified()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("mods");
        const QByteArray required="managed-mod",optional="optional-mod",manifest="manifest";
        write(dir.filePath("mods/sanctuary.jar"),required);
        write(dir.filePath("mods/optional.jar.disabled"),optional);
        QJsonObject files{
            {"mods/sanctuary.pw.toml",QJsonObject{{"cachedLocation","mods/sanctuary.jar"},{"optionValue",true},{"linkedFileHash",hash(required)}}},
            {"mods/optional.pw.toml",QJsonObject{{"cachedLocation","mods/optional.jar"},{"optionValue",true},{"isOptional",true},{"linkedFileHash",hash(optional)}}}};
        QJsonObject cache{{"packFileHash",hash(manifest)},{"cachedSide","client"},{"cachedFiles",files}};
        write(dir.filePath("packwiz.json"),QJsonDocument(cache).toJson());
        QString error;
        const auto expected=QCryptographicHash::hash(manifest,QCryptographicHash::Sha256).toHex();
        QVERIFY(SanctuaryPack::verifyManagedFiles(dir.path(),expected,&error));
        write(dir.filePath("mods/sanctuary.jar"),"corrupt");
        QVERIFY(!SanctuaryPack::verifyManagedFiles(dir.path(),expected,&error));
        write(dir.filePath("mods/sanctuary.jar"),required);
        files["mods/sanctuary.pw.toml"]=QJsonObject{{"cachedLocation","../outside.jar"},{"optionValue",true},{"linkedFileHash",hash(required)}};
        cache["cachedFiles"]=files;
        write(dir.filePath("packwiz.json"),QJsonDocument(cache).toJson());
        QVERIFY(!SanctuaryPack::verifyManagedFiles(dir.path(),expected,&error));
    }
    void missingSanctuaryEntryCannotBeDeclaredReady()
    {
        QTemporaryDir dir;
        QString error;
        QJsonObject cache{{"packFileHash",QJsonObject{{"type","sha256"},{"value","test"}}},{"cachedSide","client"},
            {"cachedFiles",QJsonObject{{"mods/sanctuary.pw.toml",QJsonObject{}}}}};
        write(dir.filePath("packwiz.json"),QJsonDocument(cache).toJson());
        QVERIFY(!SanctuaryPack::verifyManagedFiles(dir.path(),"test",&error));
    }
};
QTEST_MAIN(SanctuaryPackTest)
#include "SanctuaryPack_test.moc"
