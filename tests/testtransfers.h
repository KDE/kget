/* This file is part of the KDE project

   Copyright (C) 2009 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TESTTRANSFERS_H
#define _TESTTRANSFERS_H

#include <QDBusVariant>
#include <QtTest/QtTest>
#include <QtXml/QDomElement>

class KTempDir;
class OrgKdeKgetTransferInterface;
class OrgKdeKgetVerifierInterface;
class TestTransfers;

class Commands : public QObject
{
    Q_OBJECT

    public:
        Commands(const QString &source, QObject *parent);

        QString source() const;
        bool hasCommands() const;
        void setCommands(const QList<QPair<int, QVariant> > &commands);
        void executeCommands();
        void associateTransfer(OrgKdeKgetTransferInterface *transfer);

        static QList<QPair<int, QVariant> > parseCommands(const QDomElement &e, TestTransfers *transfer);

    enum Action {
        Start,
        Stop,
        AddChecksum,        //QStringList() << type << hash
        AddPartialChecksums, //QList<QVariant>() << QString typ << qulonglong length << QStringList checksums
        IsVerifyable,   //bool if verifyable
        Verify,
        FindBrokenPieces,
        Repair,         //bool if it works
        SetDirectory,
        Wait,           //int time; waits until time is over and then proceeds to executeCommands
        RandomAction,  //QList<QVariant> << bool false OR QList<QVariant> << bool true << int timeBetweenActions; chooses automatically periodially Start or Stop
        CustomEvent = 100
    };

    /**
     * Event is a list of Events, events are blocking, so every command after an event
     * is blocked, unless the event happens, that way one can start a Repair when Verified returns false
     */
    enum Event {
        Verified = CustomEvent + 1,         //bool verified
        ChangedEvent = CustomEvent + 2,      //QList<Variant>() << int TransferChange << int triggerValue = optional; when the event and the triggervalue match then executing continues; e.g. QList<Variant>() << Transfer::Tc_Percent << 50; //goes on when more than 50% have been downloaded
        BrokenPieces = CustomEvent + 3
    };

    protected:
        void timerEvent(QTimerEvent *event);

    private slots:
        void slotVerified(bool verified);
        void slotBrokenPieces(const QStringList &offsets, qulonglong length);
        void slotChangedEvent(int event);
        void slotWaitEvent();

    private:
        int m_timerId;
        const QString m_source;
        QList<QPair<int, QVariant> > m_commands;
        OrgKdeKgetTransferInterface *m_transfer;
        OrgKdeKgetVerifierInterface *m_verifier;

        static QHash<QString, int> s_stringCommands;
        static QHash<QString, int> s_transferChangeEvents;
};

class TestTransfers: public QObject
{
    Q_OBJECT

    public:
        TestTransfers();

        QString tempDir() const;

    public slots:
        void createTransfer();

    private slots:
        void simpleTest();

    private:
        void parseFile();

    private:
        QList<OrgKdeKgetTransferInterface *> m_transferIfaces;
        QList<Commands*> m_commands;
        QScopedPointer<KTempDir> m_dir;
};

#endif
