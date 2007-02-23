/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRAYTRANSFER_H
#define TRAYTRANSFER_H

#include <KSystemTray>

class KMenu;

class TrayTransfer : public KSystemTray  {
    Q_OBJECT
public:
    TrayTransfer(QWidget *parent=0);
    ~TrayTransfer();
    int nPic;
    void setTip(const QString &);
    void setValue(int value);
    virtual void contextMenuAboutToShow ( KMenu* menu );
};

#endif
