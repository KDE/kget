/* This file is part of the KDE project
   
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __svr4__
#define map BULLSHIT            // on Solaris it conflicts with STL ?
#include <net/if.h>
#undef map
#include <sys/sockio.h>         // needed for SIOCGIFFLAGS
#else
#include <net/if.h>
#endif

#include "connection.h"
#include "conf/settings.h"

QString ConnectionDevices[6] = {
    "",
    "eth",
    "plip",
    "slip",
    "ppp",
    "isdn"
};

Connection::Connection( QObject * parent, const char * name )
    : QObject( parent, name )
{
 /*
    // Setup connection timer
//    connectionTimer = new QTimer(this);
    sDebug << "ddd2" << endl;
//    connect(connectionTimer, SIGNAL(timeout()), SLOT(slotCheckOnline()));

    sDebug << "ddd3" << endl;
    // setup socket for checking connection
    if ((_sock = sockets_open()) < 0) {
        log(i18n("Could not create valid socket"), false);
    } else {
        //connectionTimer->start(5000);   // 5 second interval for checking connection
    }
    sDebug << "ddd4" << endl;

    checkOnline();
    if (!b_online) {
        log(i18n("Starting offline"));
    }
*/

}

Connection::~Connection()
{
}

//BEGIN stuff for connection checking 

void Connection::slotCheckConnections()
{

}

/*
void KMainWidget::onlineDisconnect()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if (!b_online)
        return;

    if (!Settings::expertMode()) {
        if (KMessageBox::questionYesNo(this, i18n("Do you really want to disconnect?"),
                                       i18n("Question"),
                                       KStdGuiItem::yes(), KStdGuiItem::no(),
                                       "kget_AutoOnlineDisconnect")
            != KMessageBox::Yes) {
            return;
        }
    }
    log(i18n("Disconnecting..."));
    system(QFile::encodeName(Settings::disconnectCommand()));

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void KMainWidget::slotCheckOnline()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    bool old = b_online;

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifreq));

    // setup the device name according to the type of connection and link number
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", ConnectionDevices[Settings::connectionType()].ascii(), Settings::linkNumber());

    bool flag = false;

    if (Settings::connectionType() != Settings::Permanent) {
        // get the flags for particular device
        if (ioctl(_sock, SIOCGIFFLAGS, &ifr) < 0) {
            flag = true;
            b_online = false;
        } else if (ifr.ifr_flags == 0) {
#ifdef _DEBUG
            sDebug << "Can't get flags from interface " << ifr.ifr_name << endl;
#endif
            b_online = false;
        } else if (ifr.ifr_flags & IFF_UP) {    // if (ifr.ifr_flags & IFF_RUNNING)
            b_online = true;
        } else {
            b_online = false;
        }
    } else {
        b_online = true;        // PERMANENT connection
    }

    m_paOfflineMode->setEnabled(b_online);

    if (b_online != old) {
        if (flag) {             // so that we write this only once when connection is changed
#ifdef _DEBUG
            sDebug << "Unknown interface " << ifr.ifr_name << endl;
#endif
        }

        if (b_online) {
            log(i18n("We are online."));
            //checkQueue();
        } else {
            log(i18n("We are offline."));
//            pauseAll();
        }
    }


#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}


// Helper method for opening device socket


// socket constants
static int sockets_open();
int ipx_sock = -1;              // IPX socket
int ax25_sock = -1;             // AX.25 socket
int inet_sock = -1;             // INET socket
int ddp_sock = -1;              // Appletalk DDP socket

static int sockets_open()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef AF_IPX
    ipx_sock = socket(AF_IPX, SOCK_DGRAM, 0);
#else
    ipx_sock = -1;
#endif

#ifdef AF_AX25
    ax25_sock = socket(AF_AX25, SOCK_DGRAM, 0);
#else
    ax25_sock = -1;
#endif

    ddp_sock = socket(AF_APPLETALK, SOCK_DGRAM, 0);

    // Now pick any (exisiting) useful socket family for generic queries

    sDebug << "<<<<Leaving -> sockets_open () " << endl;
    if (inet_sock != -1)
        return inet_sock;
    if (ipx_sock != -1)
        return ipx_sock;
    if (ax25_sock != -1)
        return ax25_sock;
    // If this is -1 we have no known network layers and its time to jump.

#ifdef _DEBUG
    sDebugOut << endl;
#endif

    return ddp_sock;
}
*/

// END stuff for connection checking 
