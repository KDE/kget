/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferfactory.h"

#include "kget.h"

#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>

TransferFactory::TransferFactory(QObject *parent, const QVariantList &args)
  : KGetPlugin(parent, args)
{

}

