/***************************************************************************
*                                  main.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#include <kwin.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmainwidget.h"
#include "version.h"
#include "main.h"



static const char *description = I18N_NOOP("An advanced download manager for KDE.");

static const char *version = KGETVERSION;


static KCmdLineOptions option[] = {
                                      {"+[URL(s)]", I18N_NOOP("URL(s) to download."), 0},
                                      {0, 0, 0}
                                  };

static void cleanup(void);
static void setSignalHandler(void (*handler) (int));

//static msg_handler oldMsgHandler = 0L;

//-----------------------------------------------------------------------------
// Crash recovery signal handler
static void signalHandler(int sigId)
{
    fprintf(stderr, "*** KGet got signal %d\n", sigId);

    if (sigId != SIGSEGV && kmain) {
        fprintf(stderr, "*** KGet saving data\n");
        delete kmain;
    }
    // If Kget crashes again below this line we consider the data lost :-|
    // Otherwise Kget will end in an infinite loop.
    setSignalHandler(SIG_DFL);
    cleanup();
    exit(1);
}


//-----------------------------------------------------------------------------
static void setSignalHandler(void (*handler) (int))
{
    signal(SIGSEGV, handler);
    signal(SIGKILL, handler);
    signal(SIGTERM, handler);
    signal(SIGHUP, handler);
    signal(SIGFPE, handler);
    signal(SIGABRT, handler);

    // catch also the keyboard interrupt
    signal(SIGINT, handler);
}


static void cleanup(void)
{
    qInstallMsgHandler(0L /*oldMsgHandler*/);
    QString cmd;
}


KGetApp::KGetApp ():KUniqueApplication ()
{

#ifdef _DEBUG
    sDebugIn << endl;
#endif

    kmainwidget=0;



#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


int KGetApp::newInstance()
{
#ifdef _DEBUG
    sDebugIn <<"kmainwidget="<<kmainwidget << endl;
#endif


    if (kmainwidget==0)
    {

        kmainwidget=new KMainWidget();
        kmainwidget->show();
    }

    else
    {

        KWin::setActiveWindow (kmainwidget->winId());
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count()==1)
        kmain->addTransferEx(args->arg(0), QString::null, true);
    else if(args->count()==2)
        kmain->addTransferEx(args->arg(0),args->arg(1), true);

    args->clear();

#ifdef _DEBUG
    sDebugOut << endl;
#endif

    return 0;
}


/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", I18N_NOOP("kget"), version, description, KAboutData::License_GPL, "(c) 2001 - 2002, Patrick Charbonnier \n(c) 1998 - 2000, Matej Koss", "http://kget.sourceforge.net");

    aboutData.addAuthor("Patrick  Charbonnier", 0, "pch@freeshell.org");
    aboutData.addAuthor("Matej Koss", 0, "koss@miesto.sk");


    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(option);

    KGetApp::addCmdLineOptions();

    if (!KGetApp::start()) {
        fprintf(stderr, "kget is already running!\n");
        exit(0);
    }

    KGetApp kApp;

    setSignalHandler(signalHandler);
    kApp.exec();

    cleanup();



}
