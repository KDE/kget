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

#ifndef PLASMA_KGET_H
#define PLASMA_KGET_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>

#include "transfergraph.h"
#include "ui_kgetConfig.h"

namespace Plasma {
    class Svg;
}

class QGraphicsLinearLayout;
class QGraphicsWidget;
class KDialog;

class PlasmaKGet : public Plasma::Applet
{
    Q_OBJECT
public:
    enum TransferGraphType {
        ErrorGraphType = 1,
        BarChartType = 2,
        PieGraphType = 3,
        SpeedGraphType = 4,
        PanelGraphType = 5
    };

    PlasmaKGet(QObject *parent, const QVariantList &args);
    ~PlasmaKGet();

    void init();
    void constraintsEvent(Plasma::Constraints constraints);
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect &contentsRect);

public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
    void createConfigurationInterface(KConfigDialog *parent);

protected slots:
    void configAccepted();

private:
    void loadTransferGraph(uint type);

    Plasma::Svg *m_theme;
    Plasma::DataEngine *m_engine;
    QGraphicsLinearLayout *m_layout;

    TransferGraph *m_transferGraph;
    KDialog *m_dialog;
    QString m_errorMessage;
    bool m_error;
    uint m_graphType;

    Ui::KGetConfig ui;
};

K_EXPORT_PLASMA_APPLET(kget, PlasmaKGet)

#endif
