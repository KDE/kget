/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "script.h"
#include "scriptdownloadengine.h"
#include "scriptconfigadaptor.h"
#include <QVariant>
#include <KDebug>

Script::Script(QObject* parent, const KUrl &source)
    :QThread(parent), m_p_kgetcore(0), m_source(source)
{
    kDebug(5002) << "One Script Newed.";
    /*
    Use 0 as parent here because of threading issue, comply to qt manual.
    Adapted below:
    " The child of a QObject must always be created in the thread where the
    parent was created. This implies, among other things, that you should
    never pass the QThread object (this) as the parent of an object created
    in the thread (since the QThread object itself was created in another thread)."

     See http://doc.trolltech.com/4.4/threads.html#qobject-reentrancy
    */
    m_p_kgetcore = new ScriptDownloadEngine(0, m_source);
}

Script::~Script()
{
    delete m_p_kgetcore;
    delete m_p_action;
    kDebug(5002) << "m_p_kgetcore & m_p_action is deleted!";
}

bool Script::setFile(const QString &filename)
{
    m_fileName = filename;
    return true;
}

void Script::run()
{
    setPriority(QThread::LowPriority);
    // use 0 as parent, see Constructor.
    m_p_action = new Kross::Action(0, m_fileName); //"ContentFetchScript");
    connect(m_p_action, SIGNAL(finished(Kross::Action *)), this, SLOT(quit()));
    connect(m_p_kgetcore, SIGNAL(newTransfer(const QString&, const QString&)),
            this, SIGNAL(newTransfer(const QString&, const QString&)));
    connect(m_p_kgetcore, SIGNAL(percentUpdated(int)),
            this, SIGNAL(percentUpdated(int)));
    connect(this, SIGNAL(startDownload(QObject*)),
            m_p_kgetcore, SIGNAL(startDownload(QObject*)));
    m_p_action->setFile(m_fileName);
    // TODO add check
    kDebug(5002) << "KGetCore Added to script at ThreadId " << QThread::currentThreadId();
    m_p_action->addObject(m_p_kgetcore, "kgetcore",
                          Kross::ChildrenInterface::AutoConnectSignals);
    m_p_action->trigger();
    ScriptConfigAdaptor config;
    emit startDownload(&config);

    //m_p_action->callFunction("startDownload", QVariantList());
    kDebug(5002) << "Script Finished!" << QThread::currentThreadId();
    //delete m_p_kgetcore;
    //delete m_p_action;
    if (m_p_action->hadError())
    {
        kDebug(5002) << "Error:" << m_p_action->errorMessage() << m_p_action->errorTrace();
    }
    else
    {
        exec();
    }
}
