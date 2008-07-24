/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef KGETAPPLET_H
#define KGETAPPLET_H

#include <plasma/applet.h>

namespace Plasma
{
    class DataEngine;
};

class QEvent;
class QDropEvent;

static const QString KGET_DBUS_SERVICE = "org.kde.kget";
static const QString KGET_DBUS_PATH = "/KGet";

class KGetApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    KGetApplet(QObject *parent, const QVariantList &args);
    ~KGetApplet();

    void init();

protected:
    virtual bool sceneEventFilter(QGraphicsItem * watched, QEvent * event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
    virtual void dropEvent(QDropEvent * event);

    Plasma::DataEngine *m_engine;
};

#endif
