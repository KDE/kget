/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TRAYTRANSFER_H
#define _TRAYTRANSFER_H

#include <ksystemtray.h>

#include "core/globals.h"

class KMenu;

class TrayTransfer : public KSystemTray  {
    Q_OBJECT
public:
    TrayTransfer(QWidget *parent=0);
    ~TrayTransfer();
    int nPic;
    void setTip(const QString &);
    void setValue(int value);
    /** No descriptions */
    virtual void contextMenuAboutToShow ( KMenu* menu );
};

#endif
