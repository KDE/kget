/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef LINKS_H
#define LINKS_H

#include <dom/dom_element.h>

#include <kurl.h>

class LinkItem
{
public:
    LinkItem( DOM::Element link );

    KUrl url;
    QString icon;
    QString text;
    QString mimeType;

    bool isValid() const { return m_valid; }

private:
    bool m_valid : 1;
};

#endif // LINKS_H
