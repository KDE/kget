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
#include <kurl.h>
#include <kuniqueapplication.h>
#include <kstandarddirs.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "conf/settings.h"
#include "ui/splash.h"
#include "kget.h"

static const char description[] = I18N_NOOP("An advanced download manager for KDE.");

static const char version[] = KGETVERSION;

static KCmdLineOptions option[] = {
    { "showDropTarget", I18N_NOOP("Start KGet with drop target"), 0 },
    { "+[URL(s)]", I18N_NOOP("URL(s) to download."), 0},
    KCmdLineLastOption
};

//-----------------------------------------------------------------------------
// Crash recovery signal handling routines

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
}

static void signalHandler(int sigId)
{
    fprintf(stderr, "*** KGet got signal %d\n", sigId);

    if (sigId != SIGSEGV) {
        fprintf(stderr, "*** KGet saving data\n");
        //FIXME delete kmain;
    }
    // If Kget crashes again below this line we consider the data lost :-|
    // Otherwise Kget will end in an infinite loop.
    setSignalHandler(SIG_DFL);
    cleanup();
    exit(1);
}


class KGetApp : public KUniqueApplication
{
public:
    KGetApp()
        : KUniqueApplication(), mainwidget( 0 ), osd( 0 )
    {
        showSplash();
    }

    ~KGetApp()
    {
        delete osd;
        delete mainwidget;
    }

    void showSplash()
    {
        //determine whether splash-screen is enabled in kgetrc
        if ( !Settings::showSplashscreen() )
            return;

        // getting splash-screen path
        QString path = locate( "data", "kget/pics/kget_splash.png" );

        if ( !path.isEmpty() )
            osd = new OSDWidget( path );
    }

    int newInstance()
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (!mainwidget)
        {
            mainwidget = new KMainWidget();
            setMainWidget(mainwidget);
        }
        KWin::activateWindow(mainwidget->winId());

        if (args->isSet("showDropTarget"))
        { /*FIXME mainwidget->activateDropTarget();*/ }

        if (args->count()>=1)
        {
            QString txt(args->arg(0));
            if ( txt.endsWith( ".kgt", false ) )
                mainwidget->readTransfersEx(KURL::fromPathOrURL( txt ));
/* FIXME: the scheduler sould do that
            else
                mainwidget->addTransferEx( KURL::fromPathOrURL( txt ),
                                      KURL());
*/
        }
/* FIXME: the scheduler sould do that
        else if(args->count()==2)
            mainwidget->addTransferEx( KURL::fromPathOrURL( args->arg(0) ),
                                  KURL::fromPathOrURL( args->arg(1) ));
*/
        args->clear();
        if ( osd )
            osd->removeOSD();
        return 0;
    }

private:
    KMainWidget * mainwidget;
    OSDWidget * osd;
};


/////////////////////////////////////////////////////////////////
 
int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", I18N_NOOP("KGet"), version, description, KAboutData::License_GPL, "(C) 2001 - 2002, Patrick Charbonnier \n(C) 2002, Carsten Pfeiffer\n(C) 1998 - 2000, Matej Koss", 0, "http://kget.sourceforge.net");

    aboutData.addAuthor("Patrick  Charbonnier", 0, "pch@freeshell.org");
    aboutData.addAuthor("Carsten Pfeiffer", 0, "pfeiffer@kde.org");
    aboutData.addAuthor("Matej Koss", 0);

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
