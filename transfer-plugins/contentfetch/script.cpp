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
#include <QDebug>

Script::Script(QObject* parent, const KUrl &source)
    :QThread(parent), m_p_kgetcore(nullptr), m_source(source)
{
    qCDebug(KGET_DEBUG) << "One Script Newed.";
    /*
    Use nullptr as parent here because of threading issue, comply to qt manual.
    Adapted below:
    " The child of a QObject must always be created in the thread where the
    parent was created. This implies, among other things, that you should
    never pass the QThread object (this) as the parent of an object created
    in the thread (since the QThread object itself was created in another thread)."

     See https://doc.qt.io/qt-5/threads-qobject.html
    */
    m_p_kgetcore = new ScriptDownloadEngine(nullptr, m_source);
}

Script::~Script()
{
    delete m_p_kgetcore;
    delete m_p_action;
    qCDebug(KGET_DEBUG) << "m_p_kgetcore & m_p_action is deleted!";
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
    m_p_action = new Kross::Action(nullptr, m_fileName); //"ContentFetchScript");
    // quit the exec() loop after get finish/abort signal from script
    connect(m_p_kgetcore, SIGNAL(finished()), this, SLOT(quit()));
    connect(m_p_kgetcore, SIGNAL(aborted(QString)), this, SLOT(quit()));
    // add transfer
    connect(m_p_kgetcore, SIGNAL(newTransfer(QString,QString)),
            this, SIGNAL(newTransfer(QString,QString)));
    // update status signal/slot
    connect(m_p_kgetcore, SIGNAL(percentUpdated(int)),
            this, SIGNAL(percentUpdated(int)));
    connect(m_p_kgetcore, SIGNAL(textStatusUpdated(QString)),
            this, SIGNAL(textStatusUpdated(QString)));
    connect(m_p_kgetcore, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(m_p_kgetcore, SIGNAL(aborted(QString)), this, SIGNAL(aborted(QString)));
    // main entry point
    connect(this, SIGNAL(startDownload(QObject*)),
            m_p_kgetcore, SIGNAL(startDownload(QObject*)));
    m_p_action->setFile(m_fileName);
    // TODO add check
    qCDebug(KGET_DEBUG) << "KGetCore Added to script at ThreadId " << QThread::currentThreadId();
    m_p_action->addObject(m_p_kgetcore, "kgetcore",
                          Kross::ChildrenInterface::AutoConnectSignals);
    m_p_action->trigger();
    ScriptConfigAdaptor config;
    emit startDownload(&config);

    //m_p_action->callFunction("startDownload", QVariantList());
    qCDebug(KGET_DEBUG) << "Script Finished!" << QThread::currentThreadId();
    //delete m_p_kgetcore;
    //delete m_p_action;
    if (m_p_action->hadError())
    {
        qCDebug(KGET_DEBUG) << "Error:" << m_p_action->errorMessage() << m_p_action->errorTrace();
    }
    else
    {
        exec();
    }
}
