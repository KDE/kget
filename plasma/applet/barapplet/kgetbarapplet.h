/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
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
#ifndef KGETBARAPPLET_H
#define KGETBARAPPLET_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>

namespace Plasma {
    class Svg;
}

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;

class KGetBarApplet : public Plasma::Applet
{
    Q_OBJECT
public:
    KGetBarApplet(QObject *parent, const QVariantList &args);
    ~KGetBarApplet();

    void init();
    void paintInterface(QPainter *painter, 
                            const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);

public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

private:
    Plasma::Svg *m_theme;
    Plasma::DataEngine *m_engine;

    QGraphicsLinearLayout *m_layout;
    QGraphicsWidget *m_errorWidget;

    class Private;
    Private *d;
};

K_EXPORT_PLASMA_APPLET(barapplet, KGetBarApplet)

#endif
