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
#include "dlgcontentfetchsettingwidget.h"
#include <kdebug.h>

#include <QtGlobal>

KGET_EXPORT_PLUGIN( ContentFetchFactory )

ContentFetchFactory::ContentFetchFactory(QObject *parent,
					 const QVariantList &args)
  : TransferFactory(parent, args)
{
    QStringList regexpList = ContentFetchSetting::self()->findItem("UrlRegexpList")->property().toStringList();
    m_scriptPathList = ContentFetchSetting::self()->findItem("PathList")->property().toStringList();
    // TODO: change to notify user without crash
    Q_ASSERT_X(m_scriptPathList.size() == regexpList.size(), "kcfg File", "Contentfetch config file corrupted!");
    for (int i = 0; i < regexpList.size(); ++i)
    {
	m_regexpList.push_back(QRegExp(regexpList[i]));
    }
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
    // No user script exists
    if (m_regexpList.size() == 0)
    {
	return 0;
    }
    QString url = srcUrl.url();
    QStringList::iterator fileIter = m_scriptPathList.begin();
    for(QVector<QRegExp>::iterator iter = m_regexpList.begin();
	iter!=m_regexpList.end(); ++iter,++fileIter)
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
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

QWidget * ContentFetchFactory::createSettingsWidget(KDialog *parent)
{
    return new DlgContentFetchSettingWidget(parent);
}

const QList<KAction*> ContentFetchFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler);
    return QList<KAction*>();
}

