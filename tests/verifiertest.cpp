
#include "verifiertest.h"
#include "../settings.h"
#include "../core/verifier.h"

#include <QtTest/QtTest>

#include <KDebug>
#include <KStandardDirs>
#include <KTempDir>

typedef QPair<QString, QString> Checksum;
typedef QPair<QString, PartialChecksums> Checksums;
Q_DECLARE_METATYPE(Checksum)
Q_DECLARE_METATYPE(QList<Checksum>)
Q_DECLARE_METATYPE(Checksums)
Q_DECLARE_METATYPE(QList<Checksums>)
Q_DECLARE_METATYPE(Verifier::ChecksumStrength)
Q_DECLARE_METATYPE(QList<KIO::fileoffset_t>)

VerfierTest::VerfierTest(QObject *parent)
  : QObject(parent),
    m_supported(Verifier::supportedVerficationTypes())
{
    //create a file which will used in the test
     m_tempDir.reset(new KTempDir(KStandardDirs::locateLocal("tmp", "kget_test")));
     QString path = m_tempDir->name();
     path.append("test.txt");
     QFile file(path);
     if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
         kError(5001) << "Creating file failed:" << path;
         abort();
     }
     m_file = KUrl(path);

    const QByteArray data("This is a test for the KGet Verifier class.\n");
    const qint64 size = data.size();
    for (int i = 0; i < 50000; ++i) {
        if (file.write(data) != size) {
            kError(5001) << "Creating file failed:" << path;
            abort();
        }
    }

    kDebug(5001) << "Supported types:" << m_supported;

    //Otherwise testVerify fails
    qRegisterMetaType<KUrl>("KUrl");
}

void VerfierTest::testChecksum()
{
    QFETCH(QString, type);
    QFETCH(QString, checksum);

    if (!m_supported.contains(type)) {
        return;
    }

    QCOMPARE(Verifier::checksum(m_file, type, 0), checksum);
}

void VerfierTest::testChecksum_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("checksum");

    QTest::newRow("md5 checksum") << "md5" << "1c3b1b627e4f236fdac8f6ab3ee160d1";
    QTest::newRow("sha1 checksum") << "sha1" << "0a4886f5c6e45c5a879b0093a43cb06fc5568bc5";
    QTest::newRow("sha256 checksum") << "sha256" << "9cf2cfe941569d8627dc3ede2897dea1c3949eb6bd124d3bca06a888492f955b";
    QTest::newRow("sha384 checksum") << "sha384" << "a27d95917b3d3ff9145fd9a66cc9852f3a15e7ebac9e7112d4260e2747d45c89a949fe53d110ecc260e246f959ca30e9";
    QTest::newRow("sha512 checksum") << "sha512" << "4c6d2020482ddb1c6f7275f40fb35084805f06a23e433420e46b794ef7ad9770d527c4a7f5dc50bdf7acbd3f2e0a34f00a1b7283a1b222da6835f93505db6641";
}

void VerfierTest::testPartialChecksums()
{
    QFETCH(QString, type);
    QFETCH(KIO::filesize_t, length);
    QFETCH(QStringList, checksums);
    QFETCH(bool, result);

    const PartialChecksums partial = Verifier::partialChecksums(m_file, type, length, 0);
    QCOMPARE((partial.checksums() == checksums), result);
}

void VerfierTest::testPartialChecksums_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<KIO::filesize_t>("length");
    QTest::addColumn<QStringList>("checksums");
    QTest::addColumn<bool>("result");

    QTest::newRow("md5, empty checksums") << "md5" << KIO::filesize_t(512 * 1000) << QStringList() << expectedResult(false, "md5");
    QTest::newRow("md5, one checksum is too long, 500 KiB") << "md5" << KIO::filesize_t(500 * 1024) << (QStringList() << "AAAAA8c53d0109e5222e2c57d187eec33ab71" << "f13b7c9978ac7edac8b1c2b9be55abb0" <<"643086ed0d7e733629c848e94e29cdc4" << "6c349ee55737d9bc45ef1b1e75ec7bdf" << "40363ab59bbfa39f6faa4aa18ee75a6c") << expectedResult(false, "md5");
    QTest::newRow("valid md5 checksums, 500 KiB") << "md5" << KIO::filesize_t(500 * 1024) << (QStringList() << "8c53d0109e5222e2c57d187eec33ab71" << "f13b7c9978ac7edac8b1c2b9be55abb0" <<"643086ed0d7e733629c848e94e29cdc4" << "6c349ee55737d9bc45ef1b1e75ec7bdf" << "40363ab59bbfa39f6faa4aa18ee75a6c") << expectedResult(true, "md5");
    QTest::newRow("valid md5 checksums, 400 KiB") << "md5" << KIO::filesize_t(400 * 1024) << (QStringList() << "ce7d795bd0b1499f18d2ba8f338302d3" << "e6681cc0049c6cae347039578eaf1117" << "e66499f43f930fb013092a2d66ecdfaf" << "b2930c55fea65c11813dff09447fbe92" << "77b79bd53b62accec6367c7d7b2fc85b" << "40363ab59bbfa39f6faa4aa18ee75a6c") << expectedResult(true, "md5");
    QTest::newRow("valid md5 checksums, one part with file size") << "md5" << KIO::filesize_t(2200000) << (QStringList() << "1c3b1b627e4f236fdac8f6ab3ee160d1") << expectedResult(true, "md5");
    QTest::newRow("valid sha1 checksums, 1000 KiB") << "sha1" << KIO::filesize_t(1000 * 1024) << (QStringList() << "5d76447e7655fd1d4dfba458c33340757d81eb95" << "0bc9428e3b39fc34ab457d58d62f1973a1183ac2" << "48a313a958ea1c55eb9527b0141ae30742fedfdb") << expectedResult(true, "sha1");
}

void VerfierTest::testIsChecksum()
{
    QFETCH(QString, type);
    QFETCH(QString, checksum);
    QFETCH(bool, result);

    QCOMPARE(Verifier::isChecksum(type, checksum), result);
}

void VerfierTest::testIsChecksum_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("checksum");
    QTest::addColumn<bool>("result");

    const QStringList supported = Verifier::supportedVerficationTypes();

    QTest::newRow("md5, too many chars") << "md5" << "1c3b1b627e4f236fdac8f6ab3ee160d1a" << expectedResult(false, "md5");
    QTest::newRow("md5, correct length, one wrong char") << "md5" << "1c b1b627e4f236fdac8f6ab3ee160d1" << expectedResult(false, "md5");
    QTest::newRow("md5, correct length, many wrong chars") << "md5" << "1c[b1b627e4f236fd)c8f6ab3_e160d1" << expectedResult(false, "md5");
    QTest::newRow("valid md5 checksum") << "md5" << "1C3B1b627e4F236fdac8f6ab3ee160d1" << expectedResult(true, "md5");
    QTest::newRow("valid sha1 checksum") << "sha1" << "0a4886f5c6e45c5a879b0093a43cb06fc5568bc5" << expectedResult(true, "sha1");
    QTest::newRow("valid sha256 checksum") << "sha256" << "9cf2cfe941569d8627dc3ede2897dea1c3949eb6bd124d3bca06a888492f955b" << expectedResult(true, "sha256");
    QTest::newRow("valid sha384 checksum") << "sha384" << "a27d95917b3d3ff9145fd9a66cc9852f3a15e7ebac9e7112d4260e2747d45c89a949fe53d110ecc260e246f959ca30e9" << expectedResult(true, "sha384");
    QTest::newRow("valid sha512 checksum") << "sha512" << "4c6d2020482ddb1c6f7275f40fb35084805f06a23e433420e46b794ef7ad9770d527c4a7f5dc50bdf7acbd3f2e0a34f00a1b7283a1b222da6835f93505db6641" << expectedResult(true, "sha512");
}

void VerfierTest::testAvailableChecksum()
{
    QFETCH(QList<Checksum>, checksums);
    QFETCH(Verifier::ChecksumStrength, strength);
    QFETCH(Checksum, result);

    Verifier verifier(m_file);
    for (int i = 0; i < checksums.size(); ++i) {
        verifier.addChecksum(checksums[i].first, checksums[i].second);
    }

    const Checksum returned = verifier.availableChecksum(strength);
    QCOMPARE(returned, result);
}

void VerfierTest::testAvailableChecksum_data()
{
    QTest::addColumn<QList<Checksum> >("checksums");
    QTest::addColumn<Verifier::ChecksumStrength>("strength");
    QTest::addColumn<Checksum>("result");

    const Checksum md5sum("md5", "1c3b1b627e4f236fdac8f6ab3ee160d1");
    const Checksum md5sumInvalid("md5", "1c b1b627e4f236fdac8f6ab3ee160d1");
    const Checksum sha1sum("sha1", "0a4886f5c6e45c5a879b0093a43cb06fc5568bc5");
    const Checksum sha1sumInvalid("sha1", "0a4886f5c6e45c5a879b0093a43cb06fc5568bc");
    const Checksum emptysum;

    QTest::newRow("weak (md5), md5 should be returned") << (QList<Checksum>() << md5sum) << Verifier::Weak << md5sum;
    QTest::newRow("strong (md5), md5 should be returned") << (QList<Checksum>() << md5sum) << Verifier::Strong << md5sum;
    QTest::newRow("strongest (md5), md5 should be returned") << (QList<Checksum>() << md5sum) << Verifier::Strongest << md5sum;
    QTest::newRow("weak (md5), nothing should be returned (md5 invalid)") << (QList<Checksum>() << md5sumInvalid) << Verifier::Weak << emptysum;
    QTest::newRow("strong (md5), nothing should be returned (md5 invalid)") << (QList<Checksum>() << md5sumInvalid) << Verifier::Strong << emptysum;
    QTest::newRow("strongest (md5), nothing should be returned (md5 invalid)") << (QList<Checksum>() << md5sumInvalid) << Verifier::Strongest << emptysum;
    if (m_supported.contains("sha1")) {
        QTest::newRow("weak (md5, sha1), sha1 should be returned (md5 invalid)") << (QList<Checksum>() << md5sumInvalid << sha1sum) << Verifier::Weak << sha1sum;
        QTest::newRow("strong (md5, sha1), md5 should be returned (sha1 invalid)") << (QList<Checksum>() << md5sum << sha1sumInvalid) << Verifier::Strong << md5sum;
        QTest::newRow("strongest (md5, sha1), md5 should be returned (sha1 invalid)") << (QList<Checksum>() << md5sum << sha1sumInvalid) << Verifier::Strongest << md5sum;
        QTest::newRow("strong (md5, sha1), sha1 should be returned") << (QList<Checksum>() << md5sum << sha1sum) << Verifier::Strong << sha1sum;
        QTest::newRow("strong (md5, sha1), sha1 should be returned") << (QList<Checksum>() << md5sum << sha1sum) << Verifier::Strongest << sha1sum;
        QTest::newRow("strongest (md5, sha1), nothing should be returned (md5, sha1 invalid)") << (QList<Checksum>() << md5sumInvalid << sha1sumInvalid) << Verifier::Strongest << emptysum;
    }
}

void VerfierTest::testAvailablePartialChecksum()
{
    QFETCH(QList<Checksums>, data);
    QFETCH(Verifier::ChecksumStrength, strength);
    QFETCH(Checksums, result);

    Verifier verifier(m_file);
    for (int i = 0; i < data.size(); ++i) {
        verifier.addPartialChecksums(data[i].first, data[i].second.length(), data[i].second.checksums());
    }

    const QPair<QString, PartialChecksums*> returned = verifier.availablePartialChecksum(strength);
    QVERIFY(!returned.first.isEmpty());
    QVERIFY(returned.second);

    if (returned.second) {
        QCOMPARE(returned.first, result.first);
        QCOMPARE(returned.second->checksums(), result.second.checksums());
        QCOMPARE(returned.second->length(), result.second.length());
    }
}

void VerfierTest::testAvailablePartialChecksum_data()
{
    QTest::addColumn<QList<Checksums> >("data");
    QTest::addColumn<Verifier::ChecksumStrength>("strength");
    QTest::addColumn<Checksums>("result");

    const Checksums md5Sums("md5", PartialChecksums(400 * 1024, (QStringList() << "ce7d795bd0b1499f18d2ba8f338302d3" << "e6681cc0049c6cae347039578eaf1117" << "e66499f43f930fb013092a2d66ecdfaf" << "b2930c55fea65c11813dff09447fbe92" << "77b79bd53b62accec6367c7d7b2fc85b" << "40363ab59bbfa39f6faa4aa18ee75a6c")));
    const Checksums sha1Sums("sha1", PartialChecksums(1000 * 1024, (QStringList() << "5d76447e7655fd1d4dfba458c33340757d81eb95" << "0bc9428e3b39fc34ab457d58d62f1973a1183ac2" << "48a313a958ea1c55eb9527b0141ae30742fedfdb")));
    const Checksums sha512Sums("sha512", PartialChecksums(2000 * 1024, (QStringList() << "ba9c09d26d7ec7ff60671bc72ec9fed10dee851ae951fbb9e41061490fc10019076982b4c25723870bb5ff17401fdd6d21db43f6018a0604177197db384122d3" << "a63af512c40951216f20c01f5d5623af4c24b907f9a78ded98c14ab550e23764cd131961cbf45df7dfcb17b3cca7443db12de1e0540ed421579d15ccfc7863d0")));

    QTest::newRow("weak (md5), md5 should be returned") << (QList<Checksums>() << md5Sums) << Verifier::Weak << md5Sums;
    QTest::newRow("strong (md5), md5 should be returned") << (QList<Checksums>() << md5Sums) << Verifier::Strong << md5Sums;
    QTest::newRow("strongest (md5), md5 should be returned") << (QList<Checksums>() << md5Sums) << Verifier::Strongest << md5Sums;
    if (m_supported.contains("sha1")) {
        QTest::newRow("weak (md5, sha1), md5 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums) << Verifier::Weak << md5Sums;
        QTest::newRow("strong (md5, sha1), sha1 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums) << Verifier::Strong << sha1Sums;
        QTest::newRow("strongest (md5, sha1), sha1 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums) << Verifier::Strongest << sha1Sums;
        if (m_supported.contains("sha512")) {
            QTest::newRow("weak (md5, sha1, sha512), md5 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums << sha512Sums) << Verifier::Weak << md5Sums;
            QTest::newRow("strong (md5, sha1, sha512), sha1 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums << sha512Sums) << Verifier::Strong << sha1Sums;
            QTest::newRow("strongest (md5, sha1, sha512), sha512 should be returned") << (QList<Checksums>() << md5Sums << sha1Sums << sha512Sums) << Verifier::Strongest << sha512Sums;
        }
    }
}

bool VerfierTest::expectedResult(bool expected, const QString& type)
{
    return (expected && m_supported.contains(type));
}

void VerfierTest::testVerify()
{
    QFETCH(QList<Checksum>, checksums);
    QFETCH(Verifier::ChecksumStrength, strength);
    QFETCH(bool, result);

    const Verifier::ChecksumStrength tempStrength = static_cast<Verifier::ChecksumStrength>(Settings::checksumStrength());
    Settings::setChecksumStrength(strength);
    Verifier verifier(m_file);
    for (int i = 0; i < checksums.size(); ++i) {
        verifier.addChecksum(checksums[i].first, checksums[i].second);
    }

    QSignalSpy stateSpy(&verifier, SIGNAL(verified(bool)));
    QVERIFY(stateSpy.isValid());
    QCOMPARE(stateSpy.count(), 0);

    verifier.verify();

    //wait for a maximum of 5 seconds, the slowest computer should be done by then
    for (int i = 0; !stateSpy.count() && (i < 50); ++i) {
        QTest::qWait(100);
    }

    QCOMPARE(stateSpy.count(), 1);
    const QList<QVariant> argument = stateSpy.takeFirst();
    QCOMPARE(argument.count(), 1);
    QCOMPARE(argument.first().toBool(), result);

    Settings::setChecksumStrength(tempStrength);
}

void VerfierTest::testVerify_data()
{
    QTest::addColumn<QList<Checksum> >("checksums");
    QTest::addColumn<Verifier::ChecksumStrength>("strength");
    QTest::addColumn<bool>("result");

    const Checksum md5sumCorrect("md5", "1c3b1b627e4f236fdac8f6ab3ee160d1");
    const Checksum md5sumWrong("md5", "ac3b1b627e4f236fdac8f6ab3ee160d1");
    const Checksum md5sumInvalid("md5", "1c b1b627e4f236fdac8f6ab3ee160d1");
    const Checksum sha1sumCorrect("sha1", "0a4886f5c6e45c5a879b0093a43cb06fc5568bc5");
    const Checksum sha1sumWrong("sha1", "aa4886f5c6e45c5a879b0093a43cb06fc5568bc5");
    const Checksum sha1sumInvalid("sha1", "0a4886f5c6e45c5a879b0093a43cb0");

    QTest::newRow("weak (md5), should be verified") << (QList<Checksum>() << md5sumCorrect) << Verifier::Weak << true;
    QTest::newRow("strong (md5), should be verified") << (QList<Checksum>() << md5sumCorrect) << Verifier::Strong << true;
    QTest::newRow("strongest (md5), should be verified") << (QList<Checksum>() << md5sumCorrect) << Verifier::Strongest << true;
    QTest::newRow("weak (md5), should not be verified (md5 wrong)") << (QList<Checksum>() << md5sumWrong) << Verifier::Weak << false;
    if (m_supported.contains("sha1")) {
        QTest::newRow("weak (md5, sha1), should not be verified (md5 wrong)") << (QList<Checksum>() << md5sumWrong << sha1sumCorrect) << Verifier::Weak << false;
        QTest::newRow("weak (md5, sha1), should be verified (md5 invalid)") << (QList<Checksum>() << md5sumInvalid << sha1sumCorrect) << Verifier::Weak << true;
        QTest::newRow("strong (md5, sha1), should not be verified (sha1 wrong)") << (QList<Checksum>() << md5sumCorrect << sha1sumWrong) << Verifier::Strong << false;
        QTest::newRow("strong (md5, sha1), should be verified (sha1 invalid)") << (QList<Checksum>() << md5sumCorrect << sha1sumInvalid) << Verifier::Strong << true;
        QTest::newRow("strong (md5, sha1), should be verified") << (QList<Checksum>() << md5sumCorrect << sha1sumCorrect) << Verifier::Strong << true;
        QTest::newRow("strongest (md5, sha1), should be verified (sha1 invalid)") << (QList<Checksum>() << md5sumCorrect << sha1sumInvalid) << Verifier::Strongest << true;
    }
}

void VerfierTest::testBrokenPieces()
{
    QFETCH(QList<Checksums>, data);
    QFETCH(Verifier::ChecksumStrength, strength);
    QFETCH(QList<KIO::fileoffset_t>, offsets);
    QFETCH(KIO::filesize_t, length);

    const Verifier::ChecksumStrength tempStrength = static_cast<Verifier::ChecksumStrength>(Settings::checksumStrength());
    Settings::setChecksumStrength(strength);
    Verifier verifier(m_file);
    for (int i = 0; i < data.size(); ++i) {
        verifier.addPartialChecksums(data[i].first, data[i].second.length(), data[i].second.checksums());
    }

    QSignalSpy stateSpy(&verifier, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)));
    QVERIFY(stateSpy.isValid());
    QCOMPARE(stateSpy.count(), 0);

    verifier.brokenPieces();

    //wait for a maximum of 5 seconds, the slowest computer should be done by then
    for (int i = 0; !stateSpy.count() && (i < 50); ++i) {
        QTest::qWait(100);
    }

    QCOMPARE(stateSpy.count(), 1);
    const QList<QVariant> argument = stateSpy.takeFirst();
    QCOMPARE(argument.count(), 2);
    const QList<KIO::fileoffset_t> returnedOffsets = qvariant_cast<QList<KIO::fileoffset_t> >(argument[0]);
    QCOMPARE(returnedOffsets, offsets);
    const KIO::filesize_t returnedLength = qvariant_cast<KIO::filesize_t>(argument[1]);
    QCOMPARE(returnedLength, length);

    Settings::setChecksumStrength(tempStrength);
}

void VerfierTest::testBrokenPieces_data()
{
    QTest::addColumn<QList<Checksums> >("data");
    QTest::addColumn<Verifier::ChecksumStrength>("strength");
    QTest::addColumn<QList<KIO::fileoffset_t> >("offsets");
    QTest::addColumn<KIO::filesize_t>("length");

    const QList<KIO::fileoffset_t> emptyOffsets;

    const KIO::filesize_t md5Length = 400 * 1024; 
    const Checksums md5sumsCorrect("md5", PartialChecksums(md5Length, (QStringList() << "ce7d795bd0b1499f18d2ba8f338302d3" << "e6681cc0049c6cae347039578eaf1117" << "e66499f43f930fb013092a2d66ecdfaf" << "b2930c55fea65c11813dff09447fbe92" << "77b79bd53b62accec6367c7d7b2fc85b" << "40363ab59bbfa39f6faa4aa18ee75a6c")));
    const Checksums md5sumsWrong1("md5", PartialChecksums(md5Length, (QStringList() << "ae7d795bd0b1499f18d2ba8f338302d3" << "e6681cc0049c6cae347039578eaf1117" << "e66499f43f930fb013092a2d66ecdfaf" << "b2930c55fea65c11813dff09447fbe92" << "77b79bd53b62accec6367c7d7b2fc85b" << "40363ab59bbfa39f6faa4aa18ee75a6c")));
    QList<KIO::fileoffset_t> md5sumsWrong1Offsets1 = (QList<KIO::fileoffset_t>() << 0);
    const Checksums md5sumsWrong2("md5", PartialChecksums(md5Length, (QStringList() << "ce7d795bd0b1499f18d2ba8f338302d3" << "e6681cc0049c6cae347039578eaf1117" << "d66499f43f930fb013092a2d66ecdfaf" << "b2930c55fea65c11813dff09447fbe92" << "77b79bd53b62accec6367c7d7b2fc85b" << "d0363ab59bbfa39f6faa4aa18ee75a6c")));
    QList<KIO::fileoffset_t> md5sumsWrong1Offsets2 = (QList<KIO::fileoffset_t>() << 2 * md5Length << 5 * md5Length);

    const KIO::filesize_t sha1Length = 1000 * 1024;
    const Checksums sha1sumsCorrect("sha1", PartialChecksums(sha1Length, (QStringList() << "5d76447e7655fd1d4dfba458c33340757d81eb95" << "0bc9428e3b39fc34ab457d58d62f1973a1183ac2" << "48a313a958ea1c55eb9527b0141ae30742fedfdb")));
    const Checksums sha1sumsWrong1("sha1", PartialChecksums(sha1Length, (QStringList() << "5d76447e7655fd1d4dfba458c33340757d81eb95" << "abc9428e3b39fc34ab457d58d62f1973a1183ac2" << "48a313a958ea1c55eb9527b0141ae30742fedfdb")));
    QList<KIO::fileoffset_t> sha1sumsWrong1Offsets1 = (QList<KIO::fileoffset_t>() << sha1Length);
    const Checksums sha1sumsWrong2("sha1", PartialChecksums(sha1Length, (QStringList() << "ad76447e7655fd1d4dfba458c33340757d81eb95" << "abc9428e3b39fc34ab457d58d62f1973a1183ac2" << "a8a313a958ea1c55eb9527b0141ae30742fedfdb")));
    QList<KIO::fileoffset_t> sha1sumsWrong1Offsets2 = (QList<KIO::fileoffset_t>() << 0 << sha1Length << 2 * sha1Length);

    QTest::newRow("weak (md5), no broken pieces") << (QList<Checksums>() << md5sumsCorrect) << Verifier::Weak << emptyOffsets << md5Length;
    QTest::newRow("weak (md5), 1 broken piece (first)") << (QList<Checksums>() << md5sumsWrong1) << Verifier::Weak << md5sumsWrong1Offsets1 << md5Length;
    QTest::newRow("weak (md5), 2 broken piece (third, sixth)") << (QList<Checksums>() << md5sumsWrong2) << Verifier::Weak << md5sumsWrong1Offsets2 << md5Length;
    if (m_supported.contains("sha1")) {
        QTest::newRow("weak (md5, sha1), no broken pieces (sha1 wrong)") << (QList<Checksums>() << md5sumsCorrect << sha1sumsWrong1) << Verifier::Weak << emptyOffsets << md5Length;
        QTest::newRow("strong (md5, sha1), no broken pieces (md5 wrong)") << (QList<Checksums>() << md5sumsWrong1 << sha1sumsCorrect) << Verifier::Strong << emptyOffsets << sha1Length;
        QTest::newRow("strong (md5, sha1), 3 broken pieces (first, second, third)") << (QList<Checksums>() << md5sumsCorrect << sha1sumsWrong2) << Verifier::Strong << sha1sumsWrong1Offsets2 << sha1Length;
        QTest::newRow("strongest (md5, sha1), 1 broken piece (second)") << (QList<Checksums>() << md5sumsCorrect << sha1sumsWrong1) << Verifier::Strongest << sha1sumsWrong1Offsets1 << sha1Length;
    }
}

QTEST_MAIN(VerfierTest)

#include "verifiertest.moc"
