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
#include <ktoolinvocation.h>
#include <dom/html_misc.h>
#include <dom/html_document.h>
#include <kparts/partmanager.h>

#ifdef HAVE_KWEBKITPART
#include <KDE/KWebKitPart>
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebPage>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebElementCollection>
#endif


QStringList getLinks(QObject *obj, bool selectedOnly = false)
{
    Q_ASSERT(obj);

    QStringList linkList;

    if (obj->inherits("KHTMLPart"))
    { 
        KHTMLPart *htmlPart = qobject_cast<KHTMLPart*>(obj);
        KParts::Part *activePart = 0;
        
        if ( htmlPart && htmlPart->partManager() )
        {
            activePart = htmlPart->partManager()->activePart();
            if ( activePart && activePart->inherits( "KHTMLPart" ) )
                htmlPart = qobject_cast<KHTMLPart*>( activePart );
        }
        
        DOM::HTMLDocument doc;
        
        if ( selectedOnly )
        {
            doc.open();
            if (htmlPart)
                doc.write(htmlPart->selectedTextAsHTML());
            doc.close();
        }
        else
        {
            if (htmlPart)
                doc = htmlPart->htmlDocument();
        }
        
        if ( !doc.isNull() )
        {
            DOM::HTMLCollection links = doc.links();
            DOM::HTMLCollection images = doc.images();

            for ( uint i = 0; i < links.length(); i++ )
            {
                DOM::Node link = links.item( i );
                if ( link.isNull() || link.nodeType() != DOM::Node::ELEMENT_NODE )
                    continue;

                LinkItem *item = new LinkItem( (DOM::Element) link );
                if (item->isValid())
                    linkList.append(item->url.url());
                else
                    delete item;
            }

            /* do the same for images */
            for ( uint i = 0; i < images.length(); i++ )
            {
                DOM::Node image  = images.item( i );
                if ( image.isNull() || image.nodeType() != DOM::Node::ELEMENT_NODE )
                    continue;

                LinkItem *item = new LinkItem( (DOM::Element) image );
                if (item->isValid())
                    linkList.append(item->url.url());
                else
                    delete item;
            }
        }
    }
#ifdef HAVE_KWEBKITPART
    else if (obj->inherits("KWebKitPart"))
    {
        KWebKitPart *part = qobject_cast<KWebKitPart*>(obj);
        Q_ASSERT(part);

        QWebView *view = part->view();
        if (view)
        {
            QWebPage *page = view->page();
            Q_ASSERT(page);

            QWebFrame *frame = page->currentFrame();

            // QtWebKit selector language is the same as the CSS3 language selectors. Consult
            // http://www.w3.org/TR/css3-selectors/#selectors for details on CSS3 language
            // selectors. The query below finds all the anchor(<a>) and image (<img>) links
            // in the current frame.
            const QWebElementCollection collection = frame->findAllElements(QLatin1String("a[href], img[src]"));
            QString attrValue;

            if (selectedOnly)
            {
                // TODO: Find a way to find links and images in selected content.
                // No straight forward way to accomplish this in QtWebKit without
                // resorting to javascript based hacks right now!
            }
            else
            {
                Q_FOREACH(const QWebElement &element, collection)
                {
                    if (element.hasAttribute(QLatin1String("href")))
                        attrValue = QLatin1String("href");
                    else
                        attrValue = QLatin1String("src");

                    const QUrl url = frame->baseUrl().resolved(QUrl(element.attribute("href")));
                    if (url.isValid())
                        linkList << url.toString();
                }
            }
        }
    }
#endif

    return linkList;
}

KGet_plug_in::KGet_plug_in( QObject* parent )
             :Plugin(parent)
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

    QAction *showSelectedLinksAction = actionCollection()->addAction("show_selected_links");
    showSelectedLinksAction->setText(i18n("List Selected Links"));
    connect(showSelectedLinksAction, SIGNAL(triggered()), SLOT(slotShowSelectedLinks()));
    menu->addAction(showSelectedLinksAction);

    if (parent && !parent->inherits("KHTMLPart") && !parent->inherits("KWebKitPart"))
        actionCollection()->action("show_selected_links")->setVisible(false);
}


KGet_plug_in::~KGet_plug_in()
{
}


void KGet_plug_in::showPopup()
{
    bool hasDropTarget = false;

    if(QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget"))
    {
        OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        QDBusReply<bool> reply = kgetInterface.dropTargetVisible();
        if (reply.isValid())
            hasDropTarget = reply.value();
    }

    m_dropTargetAction->setChecked(hasDropTarget);

    if ( parent() )
    {
        bool enabled = false;
        if (parent()->inherits("KHTMLPart"))
        {
            const QString selectedHtml = qobject_cast<KHTMLPart*>(parent())->selectedTextAsHTML();
            DOM::HTMLDocument document;
            document.open();
            document.write( selectedHtml );
            document.close();
            enabled = (document.getElementsByTagName("a").length() > 0);
        }
#ifdef HAVE_KWEBKITPART
        else if (parent()->inherits("KWebKitPart"))
        {
          // TODO: Find a way to find links and images in selected content.
          // No straight forward way to accomplish this in QtWebKit without
          // resorting to javascript based hacks right now!
        }
#endif
        actionCollection()->action("show_selected_links")->setEnabled(enabled);
    }
}

void KGet_plug_in::slotShowDrop()
{
    if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
        QWidget *parentWidget = 0;
        if (parent())
        {
            if (parent()->inherits("KHTMLPart"))
                parentWidget = qobject_cast<KHTMLPart*>(parent())->widget();
#ifdef HAVE_KWEBKITPART
            else if (parent()->inherits("KWebKitPart"))
                parentWidget = qobject_cast<KWebKitPart*>(parent())->view();
#endif
            KRun::runCommand("kget --showDropTarget --hideMainWindow", "kget", "kget", parentWidget);
        }
    } else {
        OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        kgetInterface.setDropTargetVisible(m_dropTargetAction->isChecked());
    }
}

void KGet_plug_in::slotShowLinks()
{
    showLinks(false);
}

void KGet_plug_in::slotShowSelectedLinks()
{
    showLinks(true);
}

void KGet_plug_in::showLinks( bool selectedOnly )
{
    QStringList linkList = getLinks(parent(), selectedOnly);

    if ( linkList.isEmpty() )
    {
        KParts::ReadOnlyPart *part = qobject_cast<KParts::ReadOnlyPart *>(parent());
        KMessageBox::sorry(part ? part->widget() : 0,
                           i18n("There are no links in the active frame of the current HTML page."),
                           i18n("No Links"));
        return;
    }

    // Remove any duplicates links from the list...
    linkList.removeDuplicates();

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget") &&
        KToolInvocation::kdeinitExecWait("kget") != 0)
    {
        KParts::ReadOnlyPart *part = qobject_cast<KParts::ReadOnlyPart *>(parent());
        KMessageBox::sorry(part ? part->widget() : 0,
                           i18n("Unable to communicate with the KGet download manager!"),
                           i18n("Communication Error"));
        return;
    }

    OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
    kgetInterface.importLinks(linkList);
}

KGetPluginFactory::KGetPluginFactory( QObject* parent )
                  :KPluginFactory("kget", "kget", parent)
{
}

QObject* KGetPluginFactory::createObject( QObject* parent, const char*, const QStringList & )
{
    QObject *obj = new KGet_plug_in( parent );
    return obj;
}

KGetPluginFactory::~KGetPluginFactory()
{
}

extern "C"
{
    KDE_EXPORT void* init_khtml_kget()
    {
        KGlobal::locale()->insertCatalog("kget");
        return new KGetPluginFactory;
    }
}

#include "kget_plug_in.moc"
