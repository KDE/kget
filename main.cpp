/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QtDBus>

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

#include "core/kget.h"
#include "kgetadaptor.h"
#include "settings.h"
#include "ui/splash.h"
#include "mainwindow.h"

static const char description[] = I18N_NOOP("An advanced download manager for KDE");

static const char version[] = KGETVERSION;

static KCmdLineOptions option[] = {
    { "showDropTarget", I18N_NOOP("Start KGet with drop target"), 0 },
    { "+[URL(s)]", I18N_NOOP("URL(s) to download"), 0},
    KCmdLineLastOption
};

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
        : KUniqueApplication(), kget( 0 ), splash( 0 )
    {
        showSplash();
    }

    ~KGetApp()
    {
        delete splash;
        delete kget;
    }

    void showSplash()
    {
        //determine whether splash-screen is enabled in kgetrc
        if ( !Settings::showSplashscreen() )
            return;

        // getting splash-screen path
        QString path = KStandardDirs::locate( "data", "kget/pics/kget_splash.png" );

        if ( !path.isEmpty() )
            splash = new Splash( path );
    }

    int newInstance()
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (!kget)
        {
            kget = new MainWindow();
            new KgetAdaptor(kget);
            QDBusConnection::sessionBus().registerObject("/KGet", kget);
        }
        KWin::activateWindow(kget->winId());

        if (args->isSet("showDropTarget"))
            Settings::setShowDropTarget( true );

        KUrl::List l;
        for (int i = 0; i < args->count(); i++)
        {
            QString txt(args->arg(i));
            if ( txt.endsWith( ".kgt", Qt::CaseInsensitive ) )
                KGet::load( txt );
            else
                l.push_back(KUrl(args->arg(i)));
        }
        // all the args read from command line are downloads
        if (l.count() >= 1)
            KGet::addTransfer( l );
/*
        // the last arg read (when we have more than 1 arg) is considered
        // as destination dir for the previous downloads
        if (l.count() == 1)
            kget->addTransfersEx( l, KUrl());
        else if (l.count() > 1)
        {
            KUrl last = l.last();
            l.pop_back();
            kget->addTransfersEx(l, last);
        }
*/
        args->clear();
        if ( splash )
            splash->removeSplash();
        return 0;
    }

private:
    MainWindow * kget;
    Splash * splash;
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", I18N_NOOP("KGet"), version, description,
                         KAboutData::License_GPL,
                         "(C) 2005 - 2006, The KGet developers\n"
                         "(C) 2001 - 2002, Patrick Charbonnier\n"
                         "(C) 2002, Carsten Pfeiffer\n"
                         "(C) 1998 - 2000, Matej Koss\n",
                         "kget@kde.org", 0);

    aboutData.addAuthor("Dario Massarin", I18N_NOOP("Maintainer, Core Developer"), "nekkar@libero.it");
    aboutData.addAuthor("Urs Wolfer", I18N_NOOP("Core Developer"), "uwolfer@kde.org");
    aboutData.addAuthor("Manolo Valdes", I18N_NOOP("Multithreaded Plugin Author"), "nolis71cu@gmail.com");
    aboutData.addAuthor("Patrick  Charbonnier", I18N_NOOP("Former KGet Developer"), "pch@freeshell.org");
    aboutData.addAuthor("Carsten Pfeiffer", I18N_NOOP("Former KGet Developer"), "pfeiffer@kde.org");
    aboutData.addAuthor("Matej Koss", I18N_NOOP("Former KGet Developer"));

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(option);

    KGetApp::addCmdLineOptions();

    if (!KGetApp::start()) {
        fprintf(stderr, "kget is already running!\n");
        exit(0);
    }

    KGetApp kApp;

    //setSignalHandler(signalHandler);

    kApp.exec();

    cleanup();
}
