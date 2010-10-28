/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@valleeurpe.net>
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2010 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kget_plug_in.h"

#include "kget_interface.h"

#include <KDE/KActionCollection>
#include <KDE/KToggleAction>
#include <KDE/KActionMenu>
#include <KDE/KIconLoader>
#include <KDE/KComponentData>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KMenu>
#include <KDE/KRun>
#include <KDE/KIcon>
#include <KDE/KToolInvocation>
#include <KDE/KGenericFactory>
#include <KDE/KProtocolInfo>
#include <KDE/KParts/Part>
#include <KDE/KParts/PartManager>
#include <KDE/KParts/HtmlExtension>

#include <QtDBus/QDBusConnection>


#define QL1S(x)   QLatin1String(x)

K_PLUGIN_FACTORY(KGetPluginFactory, registerPlugin<KGetPlugin>();)
K_EXPORT_PLUGIN(KGetPluginFactory("kgetplugin"))

static QWidget* partWidget(QObject* obj)
{
    KParts::ReadOnlyPart* part = qobject_cast<KParts::ReadOnlyPart*>(obj);
    return part ? part->widget() : 0;
}

KGetPlugin::KGetPlugin(QObject *parent, const QVariantList&)
           :KParts::Plugin(parent)
{
    KActionMenu *menu = new KActionMenu(KIcon("kget"), i18n("Download Manager"), actionCollection());
    actionCollection()->addAction("kget_menu", menu);

    menu->setDelayed( false );
    connect( menu->menu(), SIGNAL( aboutToShow() ), SLOT( showPopup() ));

    m_dropTargetAction = new KToggleAction(i18n("Show Drop Target"), actionCollection());

    connect(m_dropTargetAction, SIGNAL(triggered()), this, SLOT(slotShowDrop()));
    actionCollection()->addAction(QL1S("show_drop"), m_dropTargetAction);
    menu->addAction(m_dropTargetAction);

    QAction *showLinksAction = actionCollection()->addAction(QL1S("show_links"));
    showLinksAction->setText(i18n("List All Links"));
    connect(showLinksAction, SIGNAL(triggered()), SLOT(slotShowLinks()));
    menu->addAction(showLinksAction);

    QAction *showSelectedLinksAction = actionCollection()->addAction(QL1S("show_selected_links"));
    showSelectedLinksAction->setText(i18n("List Selected Links"));
    connect(showSelectedLinksAction, SIGNAL(triggered()), SLOT(slotShowSelectedLinks()));
    menu->addAction(showSelectedLinksAction);

    m_extension = KParts::HtmlExtension::childObject(parent);
    if (!m_extension)
        menu->setVisible(false); // Hide if parent does not implement KParts::HtmlExtesnion.
}


KGetPlugin::~KGetPlugin()
{
}

void KGetPlugin::showPopup()
{
    if (!m_extension)
        return;   

    KParts::SelectorInterface* selector = qobject_cast<KParts::SelectorInterface*>(m_extension);
    if (selector) {
        bool hasDropTarget = false;
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
            OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
            QDBusReply<bool> reply = kgetInterface.dropTargetVisible();
            if (reply.isValid()) {
                hasDropTarget = reply.value();
            }
        }
        m_dropTargetAction->setChecked(hasDropTarget);
      
        const KParts::SelectorInterface::QueryMethods methods = selector->supportedQueryMethods();
        bool enable = (methods & KParts::SelectorInterface::EntireContent);
        actionCollection()->action(QL1S("show_links"))->setEnabled(enable);
        enable = (m_extension->hasSelection() && (methods & KParts::SelectorInterface::EntireContent));
        actionCollection()->action(QL1S("show_selected_links"))->setEnabled(enable);
        enable = (actionCollection()->action(QL1S("show_links"))->isEnabled() ||
                  actionCollection()->action(QL1S("show_selected_links"))->isEnabled());
        actionCollection()->action(QL1S("show_drop"))->setEnabled(enable);
        return;
    }
    
    actionCollection()->action(QL1S("show_selected_links"))->setEnabled(false);
    actionCollection()->action(QL1S("show_links"))->setEnabled(false);
    if (m_dropTargetAction->isChecked())
        m_dropTargetAction->setChecked(false);
    actionCollection()->action(QL1S("show_drop"))->setEnabled(false);        
}

void KGetPlugin::slotShowDrop()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
        KRun::runCommand("kget --showDropTarget --hideMainWindow", "kget", "kget", partWidget(parent()));
    } else {
        OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        kgetInterface.setDropTargetVisible(m_dropTargetAction->isChecked());
    }
}

void KGetPlugin::slotShowLinks()
{
    getLinks(false);
}

void KGetPlugin::slotShowSelectedLinks()
{
    getLinks(true);
}

void KGetPlugin::slotImportLinks()
{
    if (m_linkList.isEmpty()) {
        KMessageBox::sorry(partWidget(parent()),
                           i18n("No downloadable links were found in the current view."),
                           i18n("No Links"));
        return;
    }

    // Remove any duplicates links from the list...
    m_linkList.removeDuplicates();

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget") &&
        KToolInvocation::kdeinitExecWait("kget") != 0) {
        KMessageBox::sorry(partWidget(parent()),
                           i18n("Unable to communicate with the KGet download manager."),
                           i18n("Communication Error"));
        return;
    }

    OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
    kgetInterface.importLinks(m_linkList);
}

void KGetPlugin::getLinks(bool selectedOnly)
{
    KParts::SelectorInterface* selector = qobject_cast<KParts::SelectorInterface*>(m_extension);
    if (selector) {
        m_linkList.clear();        
        const QUrl baseUrl = m_extension->baseUrl();
        KParts::SelectorInterface::QueryMethod method;
        if (selectedOnly)
            method = KParts::SelectorInterface::SelectedContent;
        else
            method = KParts::SelectorInterface::EntireContent;
        const QList<KParts::SelectorInterface::Element> elements = selector->querySelectorAll(QL1S("a[href], img[src]"), method);
        Q_FOREACH(const KParts::SelectorInterface::Element& element, elements) {
            const QString attr = (element.hasAttribute(QL1S("href")) ? QL1S("href") : QL1S("src"));
            const QUrl resolvedUrl = baseUrl.resolved(element.attribute(attr));
            // Only select valid and non local (e.g. file, man page) links for download...
            if (resolvedUrl.isValid() &&
                KProtocolInfo::protocolClass(resolvedUrl.scheme()).compare(QL1S(":internet"), Qt::CaseInsensitive) == 0)
                m_linkList << resolvedUrl.toString();
        }
        
        slotImportLinks();
    }
}

#include "kget_plug_in.moc"
