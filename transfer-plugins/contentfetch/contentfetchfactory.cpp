/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "contentfetchfactory.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "contentfetch.h"
#include "contentfetchsetting.h"

#include <QStringList>
#include <QList>
#include <QRegExp>
#include <QtGlobal>

#include <KDebug>

KGET_EXPORT_PLUGIN( ContentFetchFactory )

ContentFetchFactory::ContentFetchFactory(QObject *parent,
                                         const QVariantList &args)
  : TransferFactory(parent, args)
{
}

ContentFetchFactory::~ContentFetchFactory()
{
}

Transfer * ContentFetchFactory::createTransfer( const KUrl &srcUrl,
                                                const KUrl &destUrl,
                                                TransferGroup * parent,
                                                Scheduler * scheduler,
                                                const QDomElement * e )
{
    QStringList scriptPathList;
    QVector<QRegExp> regexpList;
    QStringList allRegexpList = ContentFetchSetting::self()->urlRegexpList();
    QStringList allScriptPathList = ContentFetchSetting::self()->pathList();
    QList<int> allEnableList = ContentFetchSetting::self()->enableList();

    // TODO: change to notify user without crash
    // Q_ASSERT_X(scriptPathList.size() == regexpList.size(), "kcfg File", "Contentfetch config file corrupted!");
    for (int i = 0; i < allRegexpList.size(); ++i)
    {
        if (allEnableList[i])
        {
            regexpList.push_back(QRegExp(allRegexpList[i]));
            scriptPathList.push_back(allScriptPathList[i]);
        }
    }
    // No user script exists
    if (regexpList.size() == 0)
    {
        return 0;
    }
    QString url = srcUrl.url();
    QStringList::iterator fileIter = scriptPathList.begin();
    for(QVector<QRegExp>::iterator iter = regexpList.begin();
        iter != regexpList.end(); ++iter, ++fileIter)
    {
        if (iter->indexIn(url) != -1)
        {
            kDebug(5001) << url << " match " << iter->pattern();
            return new ContentFetch(parent, this, scheduler, srcUrl, destUrl,
                                    *fileIter, e);
        }
    }
    return 0;
}

TransferHandler * ContentFetchFactory::createTransferHandler(
    Transfer * transfer,
    Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * ContentFetchFactory::createDetailsWidget(TransferHandler *transfer)
{
    Q_UNUSED(transfer)
    return 0;   //Temporary!!
}

const QList<KAction*> ContentFetchFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler)
    return QList<KAction*>();
}
