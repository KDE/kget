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

#ifndef KGETAPPLETUTILS_H
#define KGETAPPLETUTILS_H

#include <QGraphicsProxyWidget>

class QGraphicsLinearLayout;
class QGraphicsWidget;
class QPainter;
class QRect;

namespace Plasma {
    class Svg;
    class Label;
    class Icon;
    class PushButton;
}

class KGetAppletUtils
{
public:
    static void paintTitle(QPainter *p, Plasma::Svg *svg, const QRect &rect);
    static QGraphicsWidget *createErrorWidget(const QString &message, QGraphicsWidget *parent = 0);
};

class ErrorWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
public:
    explicit ErrorWidget(const QString &message, QGraphicsWidget *parent = 0);
    ~ErrorWidget();

private slots:
    void launchKGet();

private:
    QGraphicsLinearLayout *m_layout;
    Plasma::Label *m_errorLabel;
    Plasma::Icon *m_icon;
    Plasma::PushButton *m_launchButton;
};

#endif
