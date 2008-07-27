/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef CONTENT_FETCH_H
#define CONTENT_FETCH_H

#include "core/transfer.h"
#include "script.h"
#include <QString>

class ContentFetch : public QObject, public Transfer
{
    Q_OBJECT

    public:
        ContentFetch(TransferGroup * parent, TransferFactory * factory,
                     Scheduler * scheduler, const KUrl & src,
                     const KUrl & dest, const QString &scriptFile,
                     const QDomElement * e = 0);

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        bool isResumable() const;
        void postDeleteEvent();

        void save(const QDomElement &e);

    protected:
        void load(const QDomElement &e);

    private:
        Script *m_p_script;
        TransferGroup *m_p_group;
        QString m_scriptFile;
        QString m_destDir;
    private slots:
        void slotFinish();
        void slotAddTransfer(const QString &url);
//    private slots:
//        void slotResult( KJob * kioJob );
//        void slotInfoMessage( KJob * kioJob, const QString & msg );
//        void slotPercent( KJob * kioJob, unsigned long percent );
//        void slotTotalSize( KJob * kioJob, qulonglong size );
//        void slotProcessedSize( KJob * kioJob, qulonglong size );
//        void slotSpeed( KJob * kioJob, unsigned long bytes_per_second );
};

#endif // CONTENT_FETCH_H
