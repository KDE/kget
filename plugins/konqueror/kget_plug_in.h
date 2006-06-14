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
#include <klibloader.h>
#include <ktoggleaction.h>

class KGet_plug_in : public KParts::Plugin
{
    Q_OBJECT
public:
    KGet_plug_in( QObject* parent = 0 );
    KToggleAction *m_paToggleDropTarget ;
//     DCOPClient* p_dcopServer;
    virtual ~KGet_plug_in();

private slots:
    void slotShowDrop();
    void slotShowLinks();
    void showPopup();
};


class KPluginFactory : public KLibFactory
{
    Q_OBJECT
public:
    KPluginFactory( QObject *parent = 0 );
    ~KPluginFactory() ;

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0,
                                   const QStringList &args = QStringList() );

private:
    static KInstance* s_instance;
};

#endif
