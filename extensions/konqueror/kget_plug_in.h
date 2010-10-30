/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@valleeurpe.net>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGETPLUGIN_H
#define KGETPLUGIN_H

#include <kparts/plugin.h>

class KToggleAction;

namespace KParts {
    class HtmlExtension;
    class FileInfoExtension;
}

class KGetPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    KGetPlugin(QObject* parent, const QVariantList&);
    KToggleAction *m_dropTargetAction;
    virtual ~KGetPlugin();

private Q_SLOTS:
    void slotShowDrop();
    void slotShowLinks();
    void slotShowSelectedLinks();
    void slotImportLinks();
    void showPopup();

private:
    void getLinks(bool selectedOnly = false);

    KParts::FileInfoExtension* m_fileinfoExtn;
    KParts::HtmlExtension* m_htmlExtn;
    QStringList m_linkList;
};

#endif
