/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "mirrors.h"

#include <KDebug>

#include <QDomElement>

mirror::mirror()
{
    m_search_engine = "http://www.filemirrors.com/search.src?type=begins&file=${filename}&action=Find";
}

void mirror::slotData(KIO::Job *, const QByteArray& data)
{
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void mirror::slotResult( KJob *job )
{
    kDebug(5001) << "mirror::slotResult() " << endl;
    if (job->error())
        return;
    m_job = 0;
    if ( EventLoop.isRunning() )
        EventLoop.quit();
}

void mirror::URLRequest(const KUrl &url)
{
    KUrl search(m_search_engine.replace("${filename}",url.fileName()));
    m_job = KIO::get(search,false,false);
    connect(m_job,SIGNAL(data(KIO::Job*,const QByteArray &)),
               SLOT(slotData(KIO::Job*, const QByteArray& )));
    connect(m_job,SIGNAL(result(KJob *)),
               SLOT(slotResult(KJob * )));
}


QList<KUrl>  mirror::search(const KUrl &url)
{
    kDebug(5001) << "mirror::search()" << endl;
    m_Urls << url;
    URLRequest (url);
    EventLoop.exec();
    QDomDocument doc;
    doc.setContent( m_data );

    kDebug(5001) << "doc has " << doc.childNodes().length()
        << " nodes " << doc.childNodes().item(0).toElement().tagName()
        << endl;

    QDomElement root = doc.documentElement();
    QDomNodeList body = root.elementsByTagName ("body");
    kDebug(5001) << "has " << root.childNodes().length()
        << " nodes " << root.childNodes().item(0).toElement().tagName()
        << endl;
    kDebug(5001) << body.length() << " <body> tags found" << endl;
    if(!body.isEmpty())
//       QList<KUrl> tmpUrlList = Urls (body.item(0).toElement());
        m_Urls << Urls (body.item(0).toElement());

/*   for( uint i=0 ; i < link.length () ; ++i )
   {
      node = link.item(i);
      href = node.toElement ();
      KUrl tUrl( href.attribute("href") );
      if ( tUrl.fileName() == url.fileName() )
         m_Urls << tUrl;
   }*/
    return m_Urls;
}

QList<KUrl> MirrorSearch ( const KUrl &url )
{
    mirror searcher;
    return searcher.search(url);
}

QList<KUrl> Urls( QDomElement root )
{
    QList<KUrl> _Urls;
    QDomNodeList children = root.childNodes ();
    QDomNode child;
    QDomNodeList links;
    QDomNode link;
    QDomElement e;
    for( uint i=0 ; i < children.length () ; ++i )
    {
        child = children.item(i);
        kDebug(5001) << "Processing tag: " << child.toElement().tagName() <<" whith "<< child.childNodes ().length() << " children" << endl;

        links = child.toElement().elementsByTagName ("a");
        for( uint i=0 ; i < links.length () ; ++i )
        {
            e = link.toElement();
            kDebug(5001) << e.attribute("href") << endl;
//          KUrl Url( e.attribute("href") );
//          _Urls << Url;
        }
        _Urls << Urls(child.toElement());
    }
    return _Urls;
}

#include "mirrors.moc"
