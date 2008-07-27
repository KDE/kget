/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QObject>
#include <QThread>
#include <kross/core/action.h>
#include <KUrl>

class ScriptDownloadEngine;
class TransferGroup;

class Script: public QThread
{
    Q_OBJECT
    public:
        Script(QObject* parent, const KUrl &source);
        ~Script();
        bool setFile(const QString &filename);
    signals:
        void newTransfer(const QString &url);
        void startDownload();
    protected:
        void run();
    private:
        Kross::Action *m_p_action;
        ScriptDownloadEngine *m_p_kgetcore;
        KUrl m_source;
        QString m_fileName;
};
#endif // SCRIPT_H
