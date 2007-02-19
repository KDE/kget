/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_LINKVIEW_H
#define KGET_LINKVIEW_H

#include <kmainwindow.h>

#include "links.h"

class QTreeWidget;

class KGetLinkView : public KMainWindow
{
    Q_OBJECT

public:
    KGetLinkView( QWidget *parent = 0L );
    ~KGetLinkView();

    void setLinks( QList<LinkItem*>& links );
    void setPageUrl( const QString& url );

signals:
    void leechUrls( const KUrl::List& urls );

private slots:
    void slotStartLeech();
    void slotSelectAll();

private:
    void showLinks( const QList<LinkItem*>& links );

    QList<LinkItem*> m_links;

    QTreeWidget *m_treeWidget;
};

#endif // KGET_LINKVIEW_H
