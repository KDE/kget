/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@valleeurpe.net>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_plug_in.h"

#include "links.h"
#include "kget_linkview.h"
#include "kget_interface.h"

#include <KActionCollection>
#include <KToggleAction>
#include <kactionmenu.h>
#include <khtml_part.h>
#include <kiconloader.h>
#include <KComponentData>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <krun.h>
#include <kicon.h>
#include <dom/html_misc.h>
#include <dom/html_document.h>
#include <kparts/partmanager.h>

#include <QtDBus>

#include <set>

KGet_plug_in::KGet_plug_in( QObject* parent )
  : Plugin(parent),
    view(0)
{
    KActionMenu *menu = new KActionMenu(KIcon("kget"), i18n("Download Manager"),
                                        actionCollection());
    actionCollection()->addAction("kget_menu", menu);

    menu->setDelayed( false );
    connect( menu->menu(), SIGNAL( aboutToShow() ), SLOT( showPopup() ));

    m_dropTargetAction = new KToggleAction(i18n("Show Drop Target"), actionCollection());

    connect(m_dropTargetAction, SIGNAL(triggered()), this, SLOT(slotShowDrop()));
    actionCollection()->addAction("show_drop", m_dropTargetAction);
    menu->addAction(m_dropTargetAction);

    QAction *showLinksAction = actionCollection()->addAction("show_links");
    showLinksAction->setText(i18n("List All Links"));
    connect(showLinksAction, SIGNAL(triggered()), SLOT(slotShowLinks()));
    menu->addAction(showLinksAction);
}


KGet_plug_in::~KGet_plug_in()
{
}


void KGet_plug_in::showPopup()
{
    bool hasDropTarget = false;

    if(QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
	OrgKdeKgetInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        QDBusReply<bool> reply = kgetInterface.dropTargetVisible();
        if (reply.isValid())
            hasDropTarget = reply.value();
    }

    m_dropTargetAction->setChecked(hasDropTarget);
}

void KGet_plug_in::slotShowDrop()
{
    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
        KRun::runCommand("kget --showDropTarget --hideMainWindow", "kget", "kget", parent() && parent()->inherits("KHTMLPart")
            ? static_cast<KHTMLPart*>(parent())->widget() : NULL );
    else
    {
	OrgKdeKgetInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
	kgetInterface.setDropTargetVisible(m_dropTargetAction->isChecked());
    }
}

void KGet_plug_in::slotShowLinks()
{
    if ( !parent() || !parent()->inherits( "KHTMLPart" ) )
        return;

    KHTMLPart *htmlPart = static_cast<KHTMLPart*>( parent() );
    KParts::Part *activePart = 0L;
    if ( htmlPart->partManager() )
    {
        activePart = htmlPart->partManager()->activePart();
        if ( activePart && activePart->inherits( "KHTMLPart" ) )
            htmlPart = static_cast<KHTMLPart*>( activePart );
    }

    DOM::HTMLDocument doc = htmlPart->htmlDocument();
    if ( doc.isNull() )
        return;

    DOM::HTMLCollection links = doc.links();

    QList<LinkItem*> linkList;
    std::set<QString> dupeCheck;
    for ( uint i = 0; i < links.length(); i++ )
    {
        DOM::Node link = links.item( i );
        if ( link.isNull() || link.nodeType() != DOM::Node::ELEMENT_NODE )
            continue;

        LinkItem *item = new LinkItem( (DOM::Element) link );
        if ( item->isValid() &&
             dupeCheck.find( item->url.url() ) == dupeCheck.end() )
        {
            linkList.append( item );
            dupeCheck.insert( item->url.url() );
        }
        else
            delete item;
    }

    if ( linkList.isEmpty() )
    {
        KMessageBox::sorry( htmlPart->widget(),
            i18n("There are no links in the active frame of the current HTML page."),
            i18n("No Links") );
        return;
    }

    if (!view)
        view = new KGetLinkView();

    QString url = doc.URL().string();
    view->setPageUrl( url );

    view->setLinks( linkList );
    view->show();
    view->raise();
}

KGetPluginFactory::KGetPluginFactory( QObject* parent )
        : KLibFactory( parent )
{
    s_instance = new KComponentData("KPluginFactory");
}

QObject* KGetPluginFactory::createObject( QObject* parent, const char*, const QStringList & )
{
    QObject *obj = new KGet_plug_in( parent );
    return obj;
}

KGetPluginFactory::~KGetPluginFactory()
{
    delete s_instance;
}

extern "C"
{
    KDE_EXPORT void* init_khtml_kget()
    {
        KGlobal::locale()->insertCatalog("kget");
        return new KGetPluginFactory;
    }
}

KComponentData* KGetPluginFactory::s_instance = 0L;

#include "kget_plug_in.moc"
