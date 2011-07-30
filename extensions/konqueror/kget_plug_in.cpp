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
#include <KDE/KFileItem>
#include <KDE/KParts/Part>
#include <KDE/KParts/PartManager>
#include <KDE/KParts/HtmlExtension>
#include <KDE/KParts/FileInfoExtension>

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
    connect( menu->menu(), SIGNAL(aboutToShow()), SLOT(showPopup()));

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

    // Hide this plugin if the parent part does not support either
    // The FileInfo or Html extensions...
    if (!KParts::HtmlExtension::childObject(parent) && !KParts::FileInfoExtension::childObject(parent))
        menu->setVisible(false);
}


KGetPlugin::~KGetPlugin()
{
}

static bool hasDropTarget()
{
    bool found = false;

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kget")) {
        OrgKdeKgetMainInterface kgetInterface("org.kde.kget", "/KGet", QDBusConnection::sessionBus());
        QDBusReply<bool> reply = kgetInterface.dropTargetVisible();
        if (reply.isValid()) {
            found = reply.value();
        }
    }

    return found;
}

void KGetPlugin::showPopup()
{
    // Check for HtmlExtension support...
    KParts::HtmlExtension* htmlExtn = KParts::HtmlExtension::childObject(parent());
    if (htmlExtn) {
        KParts::SelectorInterface* selector = qobject_cast<KParts::SelectorInterface*>(htmlExtn);
        if (selector) {
            m_dropTargetAction->setChecked(hasDropTarget());
            const KParts::SelectorInterface::QueryMethods methods = selector->supportedQueryMethods();
            bool enable = (methods & KParts::SelectorInterface::EntireContent);
            actionCollection()->action(QL1S("show_links"))->setEnabled(enable);
            enable = (htmlExtn->hasSelection() && (methods & KParts::SelectorInterface::SelectedContent));
            actionCollection()->action(QL1S("show_selected_links"))->setEnabled(enable);
            enable = (actionCollection()->action(QL1S("show_links"))->isEnabled() ||
                      actionCollection()->action(QL1S("show_selected_links"))->isEnabled());
            actionCollection()->action(QL1S("show_drop"))->setEnabled(enable);
            return;
        }
    }

    // Check for FileInfoExtension support...
    KParts::FileInfoExtension* fileinfoExtn = KParts::FileInfoExtension::childObject(parent());
    if (fileinfoExtn) {
        m_dropTargetAction->setChecked(hasDropTarget());
        const KParts::FileInfoExtension::QueryModes modes = fileinfoExtn->supportedQueryModes();
        bool enable = (modes & KParts::FileInfoExtension::AllItems);
        actionCollection()->action(QL1S("show_links"))->setEnabled(enable);
        enable = (fileinfoExtn->hasSelection() && (modes & KParts::FileInfoExtension::SelectedItems));
        actionCollection()->action(QL1S("show_selected_links"))->setEnabled(enable);
        enable = (actionCollection()->action(QL1S("show_links"))->isEnabled() ||
                  actionCollection()->action(QL1S("show_selected_links"))->isEnabled());
        actionCollection()->action(QL1S("show_drop"))->setEnabled(enable);
        return;
    }

    actionCollection()->action(QL1S("show_selected_links"))->setEnabled(false);
    actionCollection()->action(QL1S("show_links"))->setEnabled(false);
    actionCollection()->action(QL1S("show_drop"))->setEnabled(false);
    if (m_dropTargetAction->isChecked())
        m_dropTargetAction->setChecked(false);
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
                           i18n("No downloadable links were found."),
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
    KParts::HtmlExtension* htmlExtn = KParts::HtmlExtension::childObject(parent());
    if (htmlExtn) {
        KParts::SelectorInterface* selector = qobject_cast<KParts::SelectorInterface*>(htmlExtn);
        if (selector) {
            m_linkList.clear();
            const QUrl baseUrl = htmlExtn->baseUrl();
            const QString query = QL1S("a[href], img[src], audio[src], video[src], embed[src], object[data]");
            const KParts::SelectorInterface::QueryMethod method = (selectedOnly ? KParts::SelectorInterface::SelectedContent:
                                                                                  KParts::SelectorInterface::EntireContent);
            const QList<KParts::SelectorInterface::Element> elements = selector->querySelectorAll(query, method);
            QString attr;
            Q_FOREACH(const KParts::SelectorInterface::Element& element, elements) {
                if (element.hasAttribute(QL1S("href")))
                    attr = QL1S("href");
                else if (element.hasAttribute(QL1S("src")))
                    attr = QL1S("src");
                else if (element.hasAttribute(QL1S("data")))
                    attr = QL1S("data");
                const KUrl resolvedUrl (baseUrl.resolved(element.attribute(attr)));
                // Only select valid and non-local links for download...
                if (resolvedUrl.isValid() && !resolvedUrl.isLocalFile() && !resolvedUrl.host().isEmpty()) {
                    if (element.hasAttribute(QL1S("type")))
                        m_linkList << QString(QL1S("url ") + resolvedUrl.url() + QL1S(" type ") + element.attribute(QL1S("type")));
                    else
                        m_linkList << resolvedUrl.url();

                }
            }
        }
        slotImportLinks();
    }

    KParts::FileInfoExtension* fileinfoExtn = KParts::FileInfoExtension::childObject(parent());
    if (fileinfoExtn) {
        m_linkList.clear();
        const KParts::FileInfoExtension::QueryMode mode = (selectedOnly ? KParts::FileInfoExtension::SelectedItems:
                                                                          KParts::FileInfoExtension::AllItems);
        const KFileItemList items = fileinfoExtn->queryFor(mode);
        Q_FOREACH(const KFileItem& item, items) {
            const KUrl url = item.url();
            // Only select valid and non local links for download...
            if (item.isReadable() && item.isFile() && !item.isLocalFile() && !url.host().isEmpty()) {
                if (item.mimetype().isEmpty())
                    m_linkList << url.url();
                else
                    m_linkList << QString(QL1S("url ") + url.url() + QL1S(" type ") + item.mimetype());
            }
        }
        slotImportLinks();
    }
}

#include "kget_plug_in.moc"
