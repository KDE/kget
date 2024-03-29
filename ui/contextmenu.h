/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERCONTEXTMENU_H
#define TRANSFERCONTEXTMENU_H

#include <QList>

class TransferHandler;
class TransferGroupHandler;
class QMenu;
class QWidget;

namespace ContextMenu
{
QMenu *createTransferContextMenu(QList<TransferHandler *> transfer, QWidget *parent);
QMenu *createTransferContextMenu(TransferHandler *handler, QWidget *parent);
QMenu *createTransferGroupContextMenu(TransferGroupHandler *handler, QWidget *parent);
}

#endif
