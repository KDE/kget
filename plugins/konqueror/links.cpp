/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "links.h"

#include <kmimetype.h>
#include <kprotocolinfo.h>

#include <dom/html_document.h>

LinkItem::LinkItem( DOM::Element link )
    : m_valid( false )
{
    DOM::NamedNodeMap attrs = link.attributes();
    DOM::Node href = attrs.getNamedItem( "href" );

    // qDebug("*** href: %s", href.nodeValue().string().latin1() );

    QString urlString = link.ownerDocument().completeURL( href.nodeValue() ).string();
    if ( urlString.isEmpty() )
        return;

    url = KUrl::fromPathOrURL( urlString );
    if ( !KProtocolInfo::supportsReading( url ) )
        return;

    // somehow getElementsByTagName("#text") doesn't work :(
    DOM::NodeList children = link.childNodes();
    for ( uint i = 0; i < children.length(); i++ )
    {
        DOM::Node node = children.item( i );
        if ( node.nodeType() == DOM::Node::TEXT_NODE )
            text.append( node.nodeValue().string() );
    }

    // force "local file" mimetype determination
    KMimeType::Ptr mt = KMimeType::findByURL( url, 0, true, true);
    icon = mt->icon( QString(), false ); // dummy parameters
    mimeType = mt->comment();

    m_valid = true;
}
