/****************************************************************************
** $Id$
**
** Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef LINKS_H
#define LINKS_H

#include <dom/dom_element.h>

#include <kurl.h>

class LinkItem
{
public:
    LinkItem( DOM::Element link );

    KURL url;
    QString icon;
    QString text;
    QString mimeType;
};


#endif // LINKS_H
