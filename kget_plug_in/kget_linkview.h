/****************************************************************************
** $Id$
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KGET_LINKVIEW_H
#define KGET_LINKVIEW_H

#include <qptrlist.h>

#include <klistview.h>
#include <kmainwindow.h>
#include <kurl.h>

#include "links.h"

class LinkViewItem : public QListViewItem
{
public:
    LinkViewItem( QListView *parent, const LinkItem * lnk );
    const LinkItem *link;
};


class KGetLinkView : public KMainWindow
{
    Q_OBJECT

public:
    KGetLinkView( QWidget *parent = 0L, const char *name = 0L );
    ~KGetLinkView();

    void setLinks( QPtrList<LinkItem>& links );
    void setPageURL( const QString& url );

signals:
    void leechURLs( const KURL::List& urls );

private slots:
    void slotStartLeech();

private:
    void initView();
    void showLinks( const QPtrList<LinkItem>& links );

    QPtrList<LinkItem> m_links;

    KListView *m_view;

};

#endif // KGET_LINKVIEW_H
