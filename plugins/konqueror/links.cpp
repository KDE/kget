#include "links.h"

#include <kmimetype.h>
#include <kprotocolinfo.h>

#include <dom/html_misc.h>
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

    url = KURL::fromPathOrURL( urlString );
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
    icon = mt->icon( QString::null, false ); // dummy parameters
    mimeType = mt->comment();
    
    m_valid = true;
}
