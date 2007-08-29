/* This file is part of the KDE project

   Copyright (C) 2002 Patrick Charbonnier <pch@valleeurpe.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef PLUGIN_KGET_PLUG_IN_H
#define PLUGIN_KGET_PLUG_IN_H

#include <kparts/plugin.h>
#include <KPluginFactory>

class KToggleAction;

class KGetLinkView;

class KGet_plug_in : public KParts::Plugin
{
    Q_OBJECT
public:
    KGet_plug_in( QObject* parent = 0 );
    KToggleAction *m_dropTargetAction;
    virtual ~KGet_plug_in();

private:
    KGetLinkView *view;

private slots:
    void slotShowDrop();
    void slotShowLinks();
    void showPopup();
};


class KGetPluginFactory : public KPluginFactory
{
    Q_OBJECT
public:
    KGetPluginFactory( QObject *parent = 0 );
    ~KGetPluginFactory() ;

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0,
                                   const QStringList &args = QStringList() );
};

#endif
