/***************************************************************************
                          kget_plug_in.cpp  -  description
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

#include <khtml_part.h> // this plugin applies to a khtml part
#include <kdebug.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <krun.h>
#include <kiconloader.h>

#include "kget_plug_in.h"

KGet_plug_in::KGet_plug_in( QObject* parent, const char* name )
        : Plugin( parent, name )
{
     

    m_paToggleDropTarget=new KAction(i18n("Show Kget drop target"),
                                    KGlobal::iconLoader()->loadIcon("khtml_kget", KIcon::MainToolbar) , 0,
                                     this, SLOT(slotShowDrop()),
                                     actionCollection(), "show_drop" );
    p_dcopServer= new DCOPClient();

    p_dcopServer->attach ();


}

KGet_plug_in::~KGet_plug_in()
{
    p_dcopServer->detach();
    delete p_dcopServer;
}


void KGet_plug_in::slotShowDrop()
{
    if (!p_dcopServer->isApplicationRegistered ("kget"))
        KRun::runCommand("kget --showDropTarget");
    else
    {
      QByteArray data;
      QDataStream arg(data, IO_WriteOnly);
      arg << QCString("drop_target");
      p_dcopServer->send("kget", "kget-mainwindow#1", (QCString) "activateAction(QCString)", data);
    }
}

KPluginFactory::KPluginFactory( QObject* parent, const char* name )
        : KLibFactory( parent, name )
{
    s_instance = new KInstance("KPluginFactory");
}

QObject* KPluginFactory::createObject( QObject* parent, const char* name, const char*, const QStringList & )
{
    QObject *obj = new KGet_plug_in( parent, name );
    return obj;
}
KPluginFactory::~KPluginFactory()
{ delete s_instance; }


extern "C"
{
    void* init_khtml_kget()
    {
		KGlobal::locale()->insertCatalogue("kget");
        return new KPluginFactory;
    }

}

KInstance* KPluginFactory::s_instance = 0L;

#include "kget_plug_in.moc"
