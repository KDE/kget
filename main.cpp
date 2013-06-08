/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <kwindowsystem.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <kstandarddirs.h>

#include "core/kget.h"
#include "dbus/dbuskgetwrapper.h"
#include "mainadaptor.h"
#include "settings.h"
#include "mainwindow.h"
#include "ui/newtransferdialog.h"

#if defined Q_WS_X11
    #include <kstartupinfo.h>
#endif

class KGetApp : public KUniqueApplication
{
public:
    KGetApp()
        : KUniqueApplication(), kget( 0 )
    {
    }

    ~KGetApp()
    {
        delete kget;
    }

    int newInstance()
    {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (!kget)
        {
#ifdef DEBUG
            kget = new MainWindow(!args->isSet("showDropTarget"), args->isSet("startWithoutAnimation"), args->isSet("test"));
#else
            kget = new MainWindow(!args->isSet("showDropTarget"), args->isSet("startWithoutAnimation"), false);
#endif

            kget->init();
            DBusKGetWrapper *wrapper = new DBusKGetWrapper(kget);
            new MainAdaptor(wrapper);
            QDBusConnection::sessionBus().registerObject("/KGet", wrapper);
        } else {
            
            //BEGIN taken from "kuniqueapplication.cpp"
#ifdef Q_WS_X11
            // This is the line that handles window activation if necessary,
            // and what's important, it does it properly. If you reimplement newInstance(),
            // and don't call the inherited one, use this (but NOT when newInstance()
            // is called for the first time, like here).
            KStartupInfo::setNewStartupId(kget, startupId());
#endif
#ifdef Q_WS_WIN
            KWindowSystem::forceActiveWindow(kget->winId());
#endif
            //END

        }

        if (args->isSet("showDropTarget"))
            Settings::setShowDropTarget( true );

        KUrl::List l;
        for (int i = 0; i < args->count(); i++)
        {
            QString txt(args->arg(i));
            if ( txt.endsWith( QLatin1String(".kgt"), Qt::CaseInsensitive ) )
                KGet::load( txt );
            else
                l.push_back(KUrl(args->arg(i)));
        }

        args->clear();

        if (!l.isEmpty())
            NewTransferDialogHandler::showNewTransferDialog(l);

        return 0;
    }

private:
    MainWindow * kget;
};


int main(int argc, char *argv[])
{
    KAboutData aboutData("kget", 0, ki18n("KGet"),
                         QByteArray("2." + QByteArray::number(KDE_VERSION_MINOR) + '.' + QByteArray::number(KDE_VERSION_RELEASE)),
                         ki18n("An advanced download manager for KDE"),
                         KAboutData::License_GPL,
                         ki18n("(C) 2005 - 2012, The KGet developers\n"
                         "(C) 2001 - 2002, Patrick Charbonnier\n"
                         "(C) 2002, Carsten Pfeiffer\n"
                         "(C) 1998 - 2000, Matej Koss"),
                         ki18n("<a href=\"mailto:kget@kde.org\">kget@kde.org</a>"));

    aboutData.addAuthor(ki18n("Lukas Appelhans"), ki18n("Maintainer, Core Developer, Torrent Plugin Author"), "l.appelhans@gmx.de");
    aboutData.addAuthor(ki18n("Dario Massarin"), ki18n("Core Developer"), "nekkar@libero.it");
    aboutData.addAuthor(ki18n("Urs Wolfer"), ki18n("Core Developer"), "uwolfer@kde.org");
    aboutData.addAuthor(ki18n("Manolo Valdes"), ki18n("Core Developer, Multithreaded Plugin Author"), "nolis71cu@gmail.com");
    aboutData.addAuthor(ki18n("Matthias Fuchs"), ki18n("Core Developer"), "mat69@gmx.net");
    aboutData.addAuthor(ki18n("Javier Goday"), ki18n("Developer"), "jgoday@gmail.com");
    aboutData.addAuthor(ki18n("Aish Raj Dahal"), ki18n("Google Summer of Code Student"));
    aboutData.addAuthor(ki18n("Ernesto Rodriguez Ortiz"), ki18n("Mms Plugin Author"), "eortiz@uci.cu");
    aboutData.addAuthor(ki18n("Patrick Charbonnier"), ki18n("Former Developer"), "pch@freeshell.org");
    aboutData.addAuthor(ki18n("Carsten Pfeiffer"), ki18n("Former Developer"), "pfeiffer@kde.org");
    aboutData.addAuthor(ki18n("Matej Koss"), ki18n("Former Developer"));
    aboutData.addCredit(ki18n("Joris Guisson"), ki18n("BTCore (KTorrent) Developer"), "joris.guisson@gmail.com");
    aboutData.addCredit(ki18n("Mensur Zahirovic (Nookie)"), ki18n("Design of Web Interface"), "linuxsajten@gmail.com");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions option;
    option.add("showDropTarget", ki18n("Start KGet with drop target"));
    option.add("hideMainWindow", ki18n("Start KGet with hidden main window"));
    option.add("startWithoutAnimation", ki18n("Start KGet without drop target animation"));
#ifdef DEBUG
    option.add("test", ki18n("Execute Unit Testing"));
#endif
    option.add("+[URL(s)]", ki18n("URL(s) to download"));
    KCmdLineArgs::addCmdLineOptions(option);

    KGetApp::addCmdLineOptions();

    if (!KGetApp::start())
    {
        fprintf(stderr, "kget is already running!\n");
        exit(0);
    }

    KGetApp kApp;

    return kApp.exec();
}
