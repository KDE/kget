/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <kdebug.h>
#include "scriptdownloadengine.h"
#include "script.h"

Script::Script(QObject* parent, const KUrl &source)
    :QThread(parent)
{
    kDebug(5002) << "One Script Newed.";
    m_p_action = new Kross::Action(0, "ContentFetchScript");
    m_p_kgetcore = new ScriptDownloadEngine(0, source);
    connect(m_p_kgetcore, SIGNAL(newTransfer(const QString&)),
	    this, SIGNAL(newTransfer(const QString&)));
    connect(this, SIGNAL(startDownload()),
	    m_p_kgetcore, SIGNAL(startDownload()));
    //connect(m_p_action, SIGNAL(finished(Kross::Action *)), this, SLOT(quit()));
}

Script::~Script()
{
    delete m_p_kgetcore;
    delete m_p_action;
    kDebug(5002) << "m_p_kgetcore & m_p_action is deleted!";
}

bool Script::setFile(const QString &filename)
{
    return m_p_action->setFile(filename);
}

void Script::run()
{
    // TODO add check
    kDebug(5002) << "KGetCore Added to script at ThreadId " << QThread::currentThreadId();
    m_p_action->addObject(m_p_kgetcore, "kgetcore",
			  Kross::ChildrenInterface::AutoConnectSignals);
    m_p_action->trigger();
    emit startDownload();
    kDebug(5002) << "Script Finished!" << QThread::currentThreadId();
    //delete m_p_kgetcore;
    //delete m_p_action;
    //exec();
}
