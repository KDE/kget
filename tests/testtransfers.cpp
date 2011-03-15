/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "testtransfers.h"
#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "core/kget.h"

#include "kget_interface.h"
#include "transfer_interface.h"
#include "verifier_interface.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingReply>
#include <QList>

#include <KRun>
#include <KTempDir>
#include <qtest_kde.h>

//FIXME not working fine if files or transfers are existing already
QHash<QString, int> Commands::s_stringCommands;
QHash<QString, int> Commands::s_transferChangeEvents;

Commands::Commands(const QString &source, QObject *parent)
  : QObject(parent),
    m_timerId(-1),
    m_source(source),
    m_transfer(0),
    m_verifier(0)
{
    static int instance = 0;
    if (!instance++) {
        s_stringCommands["start"] = Start;
        s_stringCommands["stop"] = Stop;
        s_stringCommands["addchecksum"] = AddChecksum;
        s_stringCommands["addpartialchecksums"] = AddPartialChecksums;
        s_stringCommands["isverifyable"] = IsVerifyable;
        s_stringCommands["verify"] = Verify;
        s_stringCommands["findbrokenpieces"] = FindBrokenPieces;
        s_stringCommands["repair"] = Repair;
        s_stringCommands["setdirectory"] = SetDirectory;
        s_stringCommands["wait"] = Wait;
        s_stringCommands["randomaction"] = RandomAction;
        s_stringCommands["verified"] = Verified;
        s_stringCommands["changedevent"] = ChangedEvent;
        s_stringCommands["brokenpieces"] = BrokenPieces;

        s_transferChangeEvents["tc_filename"] = Transfer::Tc_FileName;
        s_transferChangeEvents["tc_status"] = Transfer::Tc_Status;
        s_transferChangeEvents["tc_totalsize"] = Transfer::Tc_TotalSize;
        s_transferChangeEvents["tc_percent"] = Transfer::Tc_Percent;
        s_transferChangeEvents["tc_downloadspeed"] = Transfer::Tc_DownloadSpeed;
        s_transferChangeEvents["tc_remainingtime"] = Transfer::Tc_RemainingTime;
        s_transferChangeEvents["tc_uploadspeed"] = Transfer::Tc_UploadSpeed;
        s_transferChangeEvents["tc_uploadlimit"] = Transfer::Tc_UploadLimit;
        s_transferChangeEvents["tc_downloadlimit"] = Transfer::Tc_DownloadLimit;
        s_transferChangeEvents["tc_canresume"] = Transfer::Tc_CanResume;
        s_transferChangeEvents["tc_downloadedsize"] = Transfer::Tc_DownloadedSize;
        s_transferChangeEvents["tc_uploadedsize"] = Transfer::Tc_UploadedSize;
        s_transferChangeEvents["tc_log"] = Transfer::Tc_Log;
        s_transferChangeEvents["tc_group"] = Transfer::Tc_Group;
        s_transferChangeEvents["tc_selection"] = Transfer::Tc_Selection;
    }
}

QList<QPair<int, QVariant> > Commands::parseCommands(const QDomElement& e, TestTransfers *transfer)
{
    QList<QPair<int, QVariant> > commands;

    for (QDomElement elem = e.firstChildElement("command"); !elem.isNull(); elem = elem.nextSiblingElement("command")) {
        const QString typeString = elem.attribute("type").toLower();

        if (!s_stringCommands.contains(typeString)) {
            kDebug(5001) << "Error while parsing, type" << typeString << "not supported.";
            QWARN("Problem while parsing.");
            return commands;
        }
        const int type = s_stringCommands[typeString];

        QVariant data;
        QStringList args;
        for (QDomElement arg = elem.firstChildElement("arg"); !arg.isNull(); arg = arg.nextSiblingElement("arg")) {
            args << arg.text();
        }

        switch (type) {
            case Verify:
            case FindBrokenPieces:
            case Start:
            case Stop:
                commands.append(QPair<int, QVariant>(type, data));
                break;
            case IsVerifyable:
            case Verified:
            case Repair:
                if (args.count() == 1) {
                    data = args.first();
                    if (data.canConvert(QVariant::Bool)) {
                        commands.append(QPair<int, QVariant>(type, data));
                        break;
                    }
                }
                kDebug(5001) << "Parsing IsVerifyable/Verified/Repair failed.";
                QWARN("Problem while parsing.");
                break;
            case AddChecksum:
                if (args.count() == 2) {
                    data = args;
                    commands.append(QPair<int, QVariant>(type, data));
                } else {
                    kDebug(5001) << "Parsing setHash failed.";
                    QWARN("Problem while parsing.");
                }
                break;
            case AddPartialChecksums: {
                if (args.count() >= 3) {
                    bool worked;
                    QList<QVariant> list;
                    list << args[0];//Type
                    const qulonglong length = args[1].toULongLong(&worked);
                    if (worked) {
                        list << length;
                        for (int i = 2; i < args.count(); ++i) {
                            list << args[i];
                        }
                        data = list;
                        commands.append(QPair<int, QVariant>(type, data));
                        break;
                    }
                }
                kDebug(5001) << "Parsing AddPartialChecksums failed.";
                QWARN("Problem while parsing.");
                break;
            }
            case BrokenPieces: {
                QList<QVariant> list;
                bool worked = true;
                foreach (const QString &arg, args) {
                    const int value = arg.toInt(&worked);
                    if (!worked) {
                        break;
                    }
                    list << value;
                }
                if (worked) {
                    data = list;
                    commands.append(QPair<int, QVariant>(type, data));
                    break;
                }
                kDebug(5001) << "Parsing BrokenPieces failed.";
                QWARN("Problem while parsing.");
                break;
            }
            case RandomAction: {
                QList<QVariant> list;
                if (args.count() == 1) {
                    data = args.takeFirst();
                    if (data.canConvert(QVariant::Bool) && !data.toBool()) {
                        list << data.toBool();
                        data = list;
                        commands.append(QPair<int, QVariant>(type, data));
                        break;
                    }
                } else if (args.count() == 2) {
                    data = args.takeFirst();
                    if (data.canConvert(QVariant::Bool) && data.toBool()) {
                        list << data.toBool();
                        bool worked;
                        list << args.takeFirst().toInt(&worked);
                        if (worked) {
                            data = list;
                            commands.append(QPair<int, QVariant>(type, data));
                            break;
                        }
                    }
                }
                kDebug(5001) << "Parsing RandomAction failed.";
                QWARN("Problem while parsing.");
                break;
            }
            case SetDirectory:
                if (args.count() == 2) {
                    data = args.last();
                    QString newDirectory = args.first();
                    if (transfer) {
                        newDirectory.replace("${DIR}/", transfer->tempDir());
                    }
                    if (!newDirectory.isEmpty() && data.canConvert(QVariant::Bool)) {
                        const bool shouldWork = data.toBool();
                        QList<QVariant> list;
                        list << newDirectory << shouldWork;
                        data = list;
                        commands.append(QPair<int, QVariant>(type, data));
                        break;
                    }
                }
                kDebug(5001) << "Parsing SetDirectory failed.";
                QWARN("Problem while parsing.");
                break;
            case Wait:
                if (args.count() == 1) {
                    bool worked;
                    data = args.first().toInt(&worked);
                    if (worked) {
                        commands.append(QPair<int, QVariant>(type, data));
                        break;
                    }
                }
                kDebug(5001) << "Parsing Wait failed.";
                QWARN("Problem while parsing.");
                break;
            case ChangedEvent:
                if (!args.isEmpty() && (args.count() <= 2)) {
                    QList<QVariant> list;
                    const QString typeString = args.first().toLower();
                    if (s_transferChangeEvents.contains(typeString)) {
                        int value = s_transferChangeEvents[typeString];
                        list << value;
                        if (args.count() == 2) {
                            bool worked;
                            value = args.last().toInt(&worked);
                            if (worked) {
                                list << value;
                                data = list;
                                commands.append(QPair<int, QVariant>(type, data));
                                break;
                            }
                        } else {
                            data = list;
                            commands.append(QPair<int, QVariant>(type, data));
                            break;
                        }
                    }
                }
                kDebug(5001) << "Parsing ChangedEvent failed" << args;
                QWARN("Problem while parsing.");
                break;
            default:
                QWARN("Default should never happen.");
                break;
        }
    }

    return commands;
}

QString Commands::source() const
{
    return m_source;
}

void Commands::associateTransfer(OrgKdeKgetTransferInterface *transfer)
{
    if (m_transfer) {
        disconnect(m_transfer, 0, this, 0);
    }
    if (m_verifier) {
        disconnect(m_verifier, 0, this, 0);
        delete m_verifier;
    }

    m_transfer = transfer;
    kDebug(5001) << this << "associated with" << m_transfer << m_source;

    QDBusPendingReply<QString> reply = m_transfer->dest();
    const QString dest = reply.value();

    reply = m_transfer->verifier(dest);
    m_verifier = new OrgKdeKgetVerifierInterface("org.kde.kget", reply.value(), QDBusConnection::sessionBus(), this);

    connect(m_verifier, SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    connect(m_verifier, SIGNAL(brokenPieces(QStringList,qulonglong)), this, SLOT(slotBrokenPieces(QStringList,qulonglong)));
    connect(m_transfer, SIGNAL(transferChangedEvent(int)), this, SLOT(slotChangedEvent(int)));
}

bool Commands::hasCommands() const
{
    return !m_commands.isEmpty();
}

void Commands::setCommands(const QList<QPair<int, QVariant> > &commands)
{
    m_commands = commands;
}

void Commands::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    const int value = qrand() % 10;
    //70% of the cases start, in 30% stop
    if (value > 2) {
        kDebug(5001) << this << "is randomly started.";
        m_transfer->start();
    } else {
        kDebug(5001) << this << "is randomly stopped";
        m_transfer->stop();
    }
}

void Commands::slotWaitEvent()
{
    //wait is over so continue with executing commands
    if (!m_commands.isEmpty()) {
        m_commands.takeFirst();
        executeCommands();
    }
}

void Commands::executeCommands()
{
    if (!m_transfer) {
        QFAIL("Calling executeCommands when no transfer is set.");
    }

    //NOTE no checks are being done if the commands are correct, this is to ensure that KGet crashes
    //on faulty commands making sure that those are found
    while (!m_commands.isEmpty()) {
        const QPair<int, QVariant> operation = m_commands.first();
        const int type = operation.first;

        if (type >= CustomEvent) {
            return;
        }
        const QVariant command = operation.second;

        switch (type) {
            case Start:
                m_transfer->start();
                kDebug(5001) << this << "is started.";
                break;
            case Stop:
                m_transfer->stop();
                kDebug(5001) << this << "is stopped.";
                break;
            case AddChecksum: {
                QStringList hash = command.toStringList();
                kDebug(5001) << this << "adding hash" << hash;
                QDBusPendingReply<void> reply = m_verifier->addChecksum(hash.takeFirst(), hash.takeLast());
                break;
            }
            case AddPartialChecksums: {
                QList<QVariant> list = command.toList();
                kDebug(5001) << this << "adding partial hash" << list;
                const QString type = list.takeFirst().toString();
                const qulonglong length = list.takeFirst().toULongLong();
                QStringList checksums;
                foreach (const QVariant &value, list) {
                    checksums << value.toString();
                }
                m_verifier->addPartialChecksums(type, length, checksums);
                break;
            }
            case IsVerifyable: {
                const bool shouldWork = command.toBool();
                QDBusPendingReply<bool> reply = m_verifier->isVerifyable();
                kDebug(5001) << this << "isVerifyable" << reply.value();
                QVERIFY(reply.value() == shouldWork);
                break;
            }
            case Verify: {
                kDebug(5001) << this << "verification started.";
                m_verifier->verify();
                break;
            }
            case FindBrokenPieces:
                kDebug(5001) << this << "find broken pieces.";
                m_verifier->brokenPieces();
                break;
            case Repair: {
                const bool shouldWork = command.toBool();
                QDBusPendingReply<QString> dest = m_transfer->dest();
                QDBusPendingReply<bool> reply = m_transfer->repair(dest.value());

                const bool isRepairable = reply.value();
                kDebug(5001) << this << "repair started" << isRepairable;
                QVERIFY(isRepairable == shouldWork);
                break;
            }
            case SetDirectory: {
                QList<QVariant> commands = command.toList();
                const QString newDirectory = commands.takeFirst().toString();
                const bool shouldWork = commands.takeFirst().toBool();
                QDBusPendingReply<bool> reply = m_transfer->setDirectory(newDirectory);

                const bool moveStarted = reply.value();
                kDebug(5001) << this << "set changing directory started" << moveStarted;
                QVERIFY(moveStarted == shouldWork);
                break;
            }
            case Wait: {
                const int time = command.toInt();
                kDebug(5001) << this << "waiting for" << time << "msecs" << m_transfer;
                QTimer::singleShot(time, this, SLOT(slotWaitEvent()));
                return;
                break;
            }
            case RandomAction: {
                QList<QVariant> commands = command.toList();
                const bool turnOn = commands.takeFirst().toBool();
                if (m_timerId == -1) {
                    if (turnOn) {
                        kDebug(5001) << this << "starting random timer.";
                        m_timerId = startTimer(commands.takeFirst().toInt());
                    }
                } else {
                    kDebug(5001) << this << "killing random timer.";
                    killTimer(m_timerId);
                    m_timerId = -1;
                    if (turnOn) {
                        kDebug(5001) << this << "starting random timer.";
                        m_timerId = startTimer(commands.takeFirst().toInt());
                    }
                }
                break;
            }
        }

        m_commands.takeFirst();
    }
}
void Commands::slotChangedEvent(int event)
{
    if (!m_transfer || m_commands.isEmpty()) {
        return;
    }

    const QPair<int, QVariant> operation = m_commands.first();
    const int type = operation.first;

    if (type != ChangedEvent) {
        return;
    }

    const QList<QVariant> commands = operation.second.toList();

    //at least one and maximum two commands allowed;
    if (commands.isEmpty() || commands.count() > 2) {
        return;
    }

    bool worked;
    const int eventType = commands.first().toInt(&worked);
    if (!worked || !(eventType & event)) {
        return;
    }

    if (commands.count() == 2) {
        const int compareValue = commands.last().toInt(&worked);
        if (!worked) {
            return;
        }

        switch (eventType) {
            case Transfer::Tc_Status: {
                QDBusPendingReply<int> reply = m_transfer->status();
                if (reply.value() == compareValue) {
                    m_commands.takeFirst();
                    executeCommands();
                }
                break;
            }
            case Transfer::Tc_Percent: {
                QDBusPendingReply<int> reply = m_transfer->percent();
                if (reply.value() >= compareValue) {
                    kDebug(5001) << this << "ChangedEvent percent.";
                    m_commands.takeFirst();
                    executeCommands();
                }
                break;
            }
            default:
                break;
        }
    } else {
        m_commands.takeFirst();
        executeCommands();
    }
}

void Commands::slotVerified(bool verified)
{
    if (!m_commands.isEmpty()) {
        const QPair<int, QVariant> operation = m_commands.first();
        const int type = operation.first;

        if (type == Verified) {
            const QVariant command = operation.second;
            m_commands.takeFirst();
            if (command.canConvert(QVariant::Bool)) {
                const bool shouldWork = command.toBool();
                kDebug(5001) << this << "is verified" << verified;
                QVERIFY(verified == shouldWork);
            }

            executeCommands();
            return;
        }
    }

    kDebug(5001) << this << "is verified" << verified;
    QVERIFY(verified);
}

void Commands::slotBrokenPieces(const QStringList &offsets, qulonglong length)
{
    if (m_commands.isEmpty()) {
        return;
    }

    const QPair<int, QVariant> operation = m_commands.first();
    const int type = operation.first;
    if (type != BrokenPieces) {
        return;
    }

//     QList<QVariant> list = brokenPieces.variant().toList();//FIXME
    QList<QVariant> compareList = operation.second.toList();
    if (offsets.count() != compareList.count()) {
        QFAIL("Emitted brokenPieces list does not match.");
    }
    for (int i = 0; i < offsets.count(); ++i) {
        QVERIFY(static_cast<int>(offsets[i].toULongLong() / length) == compareList[i].toInt());
    }

    m_commands.takeFirst();
    executeCommands();
}

TestTransfers::TestTransfers()
{
    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
        KRun::runCommand("kget --showDropTarget --hideMainWindow", "kget", "kget", 0);
    }

    m_dir.reset(new KTempDir());
    kDebug(5001) << "Using temp dir:" << tempDir();

//TODO add a signal to check if the move worked!!

//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdeedu-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdegames-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdegraphics-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdelibs-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdelibs-experimental-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdemultimedia-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdenetwork-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepim-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepim-runtime-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdepimlibs-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdeplasma-addons-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdesdk-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdetoys-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/kdewebdev-4.3.1.tar.bz2"
//                << "http://mirrors.isc.org/pub/kde/stable/4.3.1/src/oxygen-icons-4.3.1.tar.bz2";

//                << "6326cff7779dfadc1b18a3a6bbe7b0750fb7ceaf"
//                << "576255ce66a0c089e0840bd90ea89d5705872bc8"
//                << "d57d9007b95607c0ee925cc963d7e14798fc69f9"
//                << "511532852caca9302c643fded4013ef1f57d5433"
//                << "7d560817a186c4b7099d321ee4a58705962a59d3"
//                << "ef50f869f1a6cdf91fe7808f095fccbd9463a7dd"
//                << "57194b5f89b37329e562951c491fa18029c0b431"
//                << "6fd0309dbd911e2667ec1c08d1f10f2626a54534"
//                << "c39b0fc1d3721fb8c6074ba6a174ad8716c6c604"
//                << "f4b04b21a6aa3accc530bc6c32cf0d820c611265"
//                << "83421181dd3a80c4ac0ff5bab111b7f71f6a1192"
//                << "ded236a12002b824f97856ce5dc882161ed437d2"
//                << "31a60deafef34a02fb7de5339eed1c750a456d3b"
//                << "28580c6f283fa7a6405f6a4415ebe9a4167f0992"
//                << "75a82d2e80d946333f63e32db56767c3ed17ba33";
}

QString TestTransfers::tempDir() const
{
    return m_dir->name();
}

void TestTransfers::parseFile()
{
    QFile file(QString::fromLocal8Bit(KDESRCDIR) + "/kget-test.xml");
    if (!file.open(QIODevice::ReadOnly)) {
        QFAIL("XML file not found.");
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        QFAIL("Not able to set content.");
    }
    file.close();

    for (QDomElement elem = doc.firstChildElement("tests").firstChildElement("transfer"); !elem.isNull(); elem = elem.nextSiblingElement("transfer")) {
        const QString source = elem.attribute("source");
        if (source.isEmpty()) {
            QFAIL("One transfer does not have an source attribute.");
            return;
        }
        Commands *transfer = new Commands(source, this);
        transfer->setCommands(Commands::parseCommands(elem, this));
        m_commands.append(transfer);
    }
}

void TestTransfers::createTransfer()
{
    if (m_commands.isEmpty()) {
        QFAIL("No commands set.");
    }


    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
        kDebug(5001) << "Service not registered yet, retrying.";
        QTimer::singleShot(500, this, SLOT(createTransfer()));
        return;
    }
    QVERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"));

    OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());

    foreach (Commands *command, m_commands) {
        QDBusPendingReply<QStringList> reply = kgetInterface.addTransfer(command->source(), tempDir() + KUrl(command->source()).fileName(), false);
        reply.waitForFinished();

        if (reply.value().size()) {
            kDebug(5001) << "TestTransfers::createTransfer -> transfer = " << reply.value();
            OrgKdeKgetTransferInterface *transfer = new OrgKdeKgetTransferInterface("org.kde.kget", reply.value().first(), QDBusConnection::sessionBus(), this);

            command->associateTransfer(transfer);
            command->executeCommands();
        } else {
            QFAIL("Reply NOT VALID!!!");
        }
    }
}

void TestTransfers::simpleTest()
{
    QVERIFY(1 == 1);

    parseFile();
    createTransfer();

    //execute as long as there are still commands left
    forever {
        bool abort = true;
        foreach (Commands *commands, m_commands) {
            if (commands->hasCommands()) {
                abort = false;
                break;
            }
        }

        if (abort) {
            break;
        }
        QTest::qWait(3000);
    }
}

QTEST_KDEMAIN(TestTransfers, GUI)

#include "testtransfers.moc"

