/***************************************************************************
                          kget_plug_in.h  -  description
                             -------------------
    begin                : Wed Jul  3 22:09:28 CEST 2002
    copyright            : (C) 2002 by Patrick
    email                : pch@valleeurpe.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __plugin_kget_plug_in_h
#define __plugin_kget_plug_in_h

#include <kaction.h>
#include <kparts/plugin.h>
#include <klibloader.h>

class KURL;
class KInstance;

class KGet_plug_in : public KParts::Plugin
{
    Q_OBJECT
public:
    KGet_plug_in( QObject* parent = 0, const char* name = 0 );
    KToggleAction *m_paToggleDropTarget ;
    virtual ~KGet_plug_in();

public slots:
    void slotShowDrop();
};


class KPluginFactory : public KLibFactory
{
    Q_OBJECT
public:
    KPluginFactory( QObject *parent = 0, const char *name = 0 );
    ~KPluginFactory() ;

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0,
                                   const char* name = "QObject",
                                   const QStringList &args = QStringList() );

private:
    static KInstance* s_instance;
};

#endif
