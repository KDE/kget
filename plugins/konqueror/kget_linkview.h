/****************************************************************************
** $Id$
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KGET_LINKVIEW_H
#define KGET_LINKVIEW_H

#include <q3ptrlist.h>

#include <klistview.h>
#include <kmainwindow.h>
#include <kurl.h>

#include "links.h"

class LinkViewItem : public Q3ListViewItem
{
public:
    LinkViewItem( Q3ListView *parent, const LinkItem * lnk );
    const LinkItem *link;
};


class KGetLinkView : public KMainWindow
{
    Q_OBJECT

public:
    KGetLinkView( QWidget *parent = 0L, const char *name = 0L );
    ~KGetLinkView();

    void setLinks( Q3PtrList<LinkItem>& links );
    void setPageURL( const QString& url );

signals:
    void leechURLs( const KURL::List& urls );

private slots:
    void slotStartLeech();
    void slotSelectAll();

private:
    void showLinks( const Q3PtrList<LinkItem>& links );

    Q3PtrList<LinkItem> m_links;

    KListView *m_view;

};

#endif // KGET_LINKVIEW_H
