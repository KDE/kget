/* This file is part of the KDE project

   Copyright (C) 2009 by Fabian Henze <flyser42 AT gmx DOT de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRAY_H
#define TRAY_H

#include <kstatusnotifieritem.h>

class MainWindow;
class KGet;

/**
  * This class implements the main tray icon for kget. It has a popup
  * from which the user can open a new transfer, configure kget, 
  * minimize/restore or quit the app (default behavior).
  *
  * @short KGet's system tray icon.
  **/

class Tray : public KStatusNotifierItem
{
Q_OBJECT
public:
    Tray( MainWindow * parent );

    void setDownloading(bool downloading);
    bool isDownloading();

private slots:
    void slotActivated();
};

#endif
