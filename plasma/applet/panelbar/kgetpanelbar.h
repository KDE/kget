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
#ifndef KGETPANELBAR_H
#define KGETPANELBAR_H

#include "common/kgetapplet.h"
#include <plasma/dataengine.h>

class QGraphicsProxyWidget;
class QGraphicsLinearLayout;
class QProgressBar;

namespace Plasma {
    class Dialog;
    class IconWidget;
}

class KGetPanelBar : public KGetApplet
{
    Q_OBJECT
public:
    KGetPanelBar(QObject *parent, const QVariantList &args);
    ~KGetPanelBar();

    void init();
    void paintInterface(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);

public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

private slots:
    void showDialog();

private:
    Plasma::DataEngine *m_engine;
    Plasma::IconWidget *m_icon;
    QProgressBar * m_bar;
    QGraphicsLinearLayout *m_layout;

    class Private;
    Private *d;
};

K_EXPORT_PLASMA_APPLET(kgetpanelbar, KGetPanelBar)

#endif
