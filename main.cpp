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
#include <qprocess.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "kmainwidget.h"
#include "splash.h"


static const char description[] = I18N_NOOP("An advanced download manager for KDE.");

static const char version[] = KGETVERSION;

static KCmdLineOptions option[] = {
    { "showDropTarget", I18N_NOOP("Start KGet with drop target"), 0 },
    { "+[URL(s)]", I18N_NOOP("URL(s) to download."), 0},
    KCmdLineLastOption
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
//     QString cmd;
}


class KGetApp : public KUniqueApplication
{
private:
    KMainWidget *kmainwidget;
    OSDWidget * osd;
    
public:
    KGetApp()
        : KUniqueApplication(), kmainwidget( 0 ), osd( 0 )
    {
        showSplash();
    }

    ~KGetApp()
    {
        delete kmainwidget;
    }

    void showSplash()
    {
        //determine whether splash-screen is enabled in amarokrc
        QString rcfile( ::getenv( "HOME" ) );
        rcfile += "/.kde/share/config/kgetrc";

        QFile file( rcfile );
        file.open( IO_ReadOnly );
        QString line;

        //TODO read the right property from the 'kgetrc' file
//         while ( file.readLine( line, 2000 ) != -1 ) {
//             if ( line.contains( "Show Splashscreen" ) && line.contains( "false" ) )
//                 return;
//         }

        //get KDE prefix from kde-config
        QProcess * proc = new QProcess( this );
        proc->addArgument( "kde-config" );
        proc->addArgument( "--prefix" );
        proc->start();

        //wait until process has finished
        while ( proc->isRunning() );

        //read output from kde-config
        QString path = proc->readStdout();
        path.remove( "\n" );
        path += "/share/apps/kget/pics/kget_splash.png";

        osd = new OSDWidget( path );
    }

    int newInstance()
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (kmainwidget==0)
        {
            if(args->count()>0)
                kmainwidget=new KMainWidget(true);
            else
                kmainwidget=new KMainWidget();
            setMainWidget(kmain);
        }
        else
            KWin::activateWindow (kmainwidget->winId());

        if (args->isSet("showDropTarget"))
            kmain->activateDropTarget();

        if (args->count()==1)
        {
#ifdef _DEBUG
            sDebug <<"args(0)= "<<args->arg(0) << endl;
#endif
            QString txt(args->arg(0));
            if ( txt.endsWith( ".kgt" ) )
                kmain->readTransfersEx(KURL::fromPathOrURL( txt ));
/* FIXME: the scheduler sould do that
            else
                kmain->addTransferEx( KURL::fromPathOrURL( txt ),
                                      KURL());
*/
        }
/* FIXME: the scheduler sould do that
        else if(args->count()==2)
            kmain->addTransferEx( KURL::fromPathOrURL( args->arg(0) ),
                                  KURL::fromPathOrURL( args->arg(1) ));
*/
        args->clear();
        if ( osd )
            osd->removeOSD();
        return 0;
    }
};


/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", I18N_NOOP("KGet"), version, description, KAboutData::License_GPL, "(C) 2001 - 2002, Patrick Charbonnier \n(C) 2002, Carsten Pfeiffer\n(C) 1998 - 2000, Matej Koss", "http://kget.sourceforge.net");

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
