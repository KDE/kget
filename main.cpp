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
#include <kdbusservice.h>
#include <kstandarddirs.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "core/kget.h"
#include "dbus/dbuskgetwrapper.h"
#include "mainadaptor.h"
#include "settings.h"
#include "mainwindow.h"
#include "ui/newtransferdialog.h"

#if defined Q_WS_X11
    #include <kstartupinfo.h>
#endif

class KGetApp : public QObject
{
public:
    KGetApp(QCommandLineParser *p)
        : kget( 0 ), parser( p )
    {
    }

    ~KGetApp()
    {
        delete kget;
    }

    int newInstance()
    {
        if (!kget)
        {
#ifdef DEBUG
            kget = new MainWindow(!parser->isSet("showDropTarget"), parser->isSet("startWithoutAnimation"), parser->isSet("test"));
#else
            kget = new MainWindow(!parser->isSet("showDropTarget"), parser->isSet("startWithoutAnimation"), false);
#endif

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

        if (parser->isSet("showDropTarget"))
            Settings::setShowDropTarget( true );

        QList<QUrl> l;
        const QStringList args = parser->positionalArguments();
        for (int i = 0; i < args.count(); i++)
        {
            QString txt(args.at(i));
            if ( txt.endsWith( QLatin1String(".kgt"), Qt::CaseInsensitive ) )
                KGet::load( txt );
            else
                l.push_back(QUrl::fromUserInput(args.at(i)));
        }

        if (!l.isEmpty())
            NewTransferDialogHandler::showNewTransferDialog(l);

        return 0;
    }

public slots:
    void slotActivateRequested (QStringList args, const QString & /*workingDir*/)
    {
        parser->parse(args);
        newInstance();
    }

private:
    MainWindow * kget;
    QCommandLineParser *parser;
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kget");
    KAboutData aboutData(QStringLiteral("kget"), i18n("KGet"),
                         QStringLiteral("2.95.0"),//+ QByteArray::number(KDE_VERSION_MINOR) + '.' + QByteArray::number(KDE_VERSION_RELEASE)),
                         i18n("An advanced download manager by KDE"),
                         KAboutLicense::GPL,
                         i18n("(C) 2005 - 2014, The KGet developers\n"
                         "(C) 2001 - 2002, Patrick Charbonnier\n"
                         "(C) 2002, Carsten Pfeiffer\n"
                         "(C) 1998 - 2000, Matej Koss"),
                         i18n("<a href=\"mailto:kget@kde.org\">kget@kde.org</a>"));

    aboutData.addAuthor(i18n("Lukas Appelhans"), i18n("Maintainer, Core Developer, Torrent Plugin Author"), "l.appelhans@gmx.de");
    aboutData.addAuthor(i18n("Dario Massarin"), i18n("Core Developer"), "nekkar@libero.it");
    aboutData.addAuthor(i18n("Urs Wolfer"), i18n("Core Developer"), "uwolfer@kde.org");
    aboutData.addAuthor(i18n("Manolo Valdes"), i18n("Core Developer, Multithreaded Plugin Author"), "nolis71cu@gmail.com");
    aboutData.addAuthor(i18n("Matthias Fuchs"), i18n("Core Developer"), "mat69@gmx.net");
    aboutData.addAuthor(i18n("Javier Goday"), i18n("Developer"), "jgoday@gmail.com");
    aboutData.addAuthor(i18n("Aish Raj Dahal"), i18n("Google Summer of Code Student"));
    aboutData.addAuthor(i18n("Ernesto Rodriguez Ortiz"), i18n("Mms Plugin Author"), "eortiz@uci.cu");
    aboutData.addAuthor(i18n("Patrick Charbonnier"), i18n("Former Developer"), "pch@freeshell.org");
    aboutData.addAuthor(i18n("Carsten Pfeiffer"), i18n("Former Developer"), "pfeiffer@kde.org");
    aboutData.addAuthor(i18n("Matej Koss"), i18n("Former Developer"));
    aboutData.addCredit(i18n("Joris Guisson"), i18n("BTCore (KTorrent) Developer"), "joris.guisson@gmail.com");
    aboutData.addCredit(i18n("Mensur Zahirovic (Nookie)"), i18n("Design of Web Interface"), "linuxsajten@gmail.com");
    // necessary to make the "Translators" tab appear in the About dialog
    aboutData.setTranslator( i18nc( "NAME OF TRANSLATORS", "Your names" ), i18nc( "EMAIL OF TRANSLATORS", "Your emails" ) );
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("showDropTarget"), i18n("Start KGet with drop target")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("hideMainWindow"), i18n("Start KGet with hidden main window")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("startWithoutAnimation"), i18n("Start KGet without drop target animation")));
#ifdef DEBUG
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("test"), i18n("Execute Unit Testing")));
#endif
    parser.addPositionalArgument(QLatin1String("[URL(s)]"), i18n("URL(s) to download"));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService dbusService(KDBusService::Unique);

    KGetApp kApp(&parser);

    QObject::connect(&dbusService, &KDBusService::activateRequested, &kApp, &KGetApp::slotActivateRequested);

    kApp.newInstance();

    return app.exec();
}
