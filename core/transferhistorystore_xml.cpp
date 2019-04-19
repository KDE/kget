/* This file is part of the KDE project

   Copyright (C) 2007 - 2014 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "core/transferhistorystore_xml_p.h"

#include <QDomDocument>
#include <QFile>
#include <QTextStream>

#include "kget_debug.h"
#include <QDebug>

XmlStore::SaveThread::SaveThread(QObject *parent, const QString &url, const QList<TransferHistoryItem> &list) : QThread(parent),
    m_url(url),
    m_items(list),
    m_item()
{
}

XmlStore::SaveThread::SaveThread(QObject *parent, const QString &url, const TransferHistoryItem &item) : QThread(parent),
    m_url(url),
    m_items(),
    m_item(item)
{
}

void XmlStore::SaveThread::run()
{
    QFile file(m_url);
    QDomDocument *doc;
    QDomElement root;

    if (!file.exists())
    {
        doc = new QDomDocument("Transfers");
        root = doc->createElement("Transfers");
        doc->appendChild(root);
    }
    else
    {
        doc = new QDomDocument();
        doc->setContent(&file);
        file.close();
        root = doc->documentElement();
        doc->appendChild(root);
    }

    QDomElement e = doc->createElement("Transfer");
    root.appendChild(e);

    e.setAttribute("Source", m_item.source());
    e.setAttribute("Dest", m_item.dest());
    e.setAttribute("Time", QDateTime::currentDateTime().toTime_t());
    e.setAttribute("Size", QString::number(m_item.size()));
    e.setAttribute("State", QString::number(m_item.state()));

    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream( &file );
        doc->save( stream, 0 );
        file.close();
    }
    delete doc;
}

XmlStore::DeleteThread::DeleteThread(QObject *parent, const QString &url, const TransferHistoryItem &item) : QThread(parent),
    m_url(url),
    m_item(item),
    m_items()
{
}

void XmlStore::DeleteThread::run()
{
    QDomDocument doc("tempHistory");
    QFile file(m_url);

    QString error;
    int line;
    int column;

    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        qCDebug(KGET_DEBUG) << "Error1" << error << line << column;
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    for (int i = 0 ; i < nItems ; i++) {
        QDomElement element = list.item(i).toElement();

        if(QString::compare(element.attribute("Source"), m_item.source()) == 0) {
            root.removeChild(element);
        }
        else {
            TransferHistoryItem item;
            item.setDest(element.attribute("Dest"));
            item.setSource(element.attribute("Source"));
            item.setSize(element.attribute("Size").toInt());
            item.setDateTime(QDateTime::fromTime_t(element.attribute("Time").toUInt()));
            item.setState(element.attribute("State").toInt());
            m_items << item;
        }
    }

    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream stream( &file );
        doc.save(stream, 0);
        file.close();
        doc.clear();
    }
}

XmlStore::LoadThread::LoadThread(QObject *parent, const QString &url) : QThread(parent),
    m_url(url)
{
}

void XmlStore::LoadThread::run()
{
    qRegisterMetaType<TransferHistoryItem>("TransferHistoryItem");
    QDomDocument doc("tempHistory");
    QFile file(m_url);

    QString error;
    int line;
    int column;
    int total;

    if (!doc.setContent(&file, &error, &line, &column)) 
    {
        qCDebug(KGET_DEBUG) << "Error1" << error << line << column;
        file.close();
        return;
    }

    QDomElement root = doc.documentElement();
    total = root.childNodes().size();

    QDomNodeList list = root.elementsByTagName("Transfer");

    int nItems = list.length();

    for (int i = 0 ; i < nItems ; i++)
    {
        QDomElement dom = list.item(i).toElement();
        
        TransferHistoryItem item;
        item.setDest(dom.attribute("Dest"));
        item.setSource(dom.attribute("Source"));
        item.setSize(dom.attribute("Size").toInt());
        item.setDateTime(QDateTime::fromTime_t(dom.attribute("Time").toUInt()));
        item.setState(dom.attribute("State").toInt());

        emit elementLoaded(i, total, item);
    }
    doc.clear();
    file.close();
}

XmlStore::XmlStore(const QString &url) : TransferHistoryStore(),
    m_storeUrl(url),
    m_loadThread(nullptr),
    m_saveThread(nullptr),
    m_deleteThread(nullptr)
{
}

XmlStore::~XmlStore()
{
    if(m_loadThread && m_loadThread->isRunning()) {
        m_loadThread->terminate();
    }

    if(m_saveThread && m_saveThread->isRunning()) {
        m_saveThread->terminate();
    }

    if(m_deleteThread && m_deleteThread->isRunning()) {
        m_deleteThread->terminate();
    }

    delete m_loadThread;
    delete m_saveThread;
    delete m_deleteThread;
}

void XmlStore::load()
{
    m_items.clear();
    // TODO: only load if necessary
    m_loadThread = new XmlStore::LoadThread(this, m_storeUrl);

    connect(m_loadThread, SIGNAL(finished()), SIGNAL(loadFinished()));
    connect(m_loadThread, SIGNAL(elementLoaded(int,int,TransferHistoryItem)),
                        SIGNAL(elementLoaded(int,int,TransferHistoryItem)));
    connect(m_loadThread, SIGNAL(elementLoaded(int,int,TransferHistoryItem)),
                        SLOT(slotLoadElement(int,int,TransferHistoryItem)));
    m_loadThread->start();
}

void XmlStore::clear()
{
    QFile::remove(m_storeUrl);
}

void XmlStore::saveItem(const TransferHistoryItem &item)
{
    m_saveThread = new XmlStore::SaveThread(this, m_storeUrl, item);

    connect(m_saveThread, SIGNAL(finished()), SIGNAL(saveFinished()));
    connect(m_saveThread, SIGNAL(elementLoaded(int,int,TransferHistoryItem)),
                        SIGNAL(elementLoaded(int,int,TransferHistoryItem)));
    m_saveThread->start();
}

void XmlStore::deleteItem(const TransferHistoryItem &item)
{
    Q_UNUSED(item)

    m_deleteThread = new XmlStore::DeleteThread(this, m_storeUrl, item);

    connect(m_deleteThread, SIGNAL(finished()), SLOT(slotDeleteElement()));

    m_deleteThread->start();
}

void XmlStore::slotLoadElement(int number, int total, const TransferHistoryItem &item)
{
    Q_UNUSED(number)
    Q_UNUSED(total)
    m_items.append(item);
}

void XmlStore::slotDeleteElement()
{
    m_items.clear();
    m_items << m_deleteThread->items();

    emit loadFinished();
}


