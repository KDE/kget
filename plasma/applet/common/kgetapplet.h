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

#include <plasma/popupapplet.h>
#include <plasma/dataengine.h>
#include "transfer_interface.h"

#include <KIO/Job>

class QEvent;
class QDropEvent;
class QGraphicsLinearLayout;
class ErrorWidget;

namespace Plasma
{
    class Theme;
    class IconWidget;
    class Meter;
}

class ProxyWidget : public QGraphicsWidget
{
    Q_OBJECT
    public:
        ProxyWidget(QGraphicsWidget * parent);
        ~ProxyWidget();

        void paint(QPainter * p, const QStyleOptionGraphicsItem * option, QWidget * widget);

        void setDataWidget(QGraphicsWidget *widget);

        QGraphicsWidget * dataWidget();

    private slots:
        void themeChanged();

    private:
        QGraphicsLinearLayout * m_layout;
        QGraphicsWidget * m_dataWidget;
        int m_textWidth;
        int m_textHeight;
        static const int MARGIN;
        static const int TOP_MARGIN;
        static const int LEFT_MARGIN;
        static const int SPACING;
};

class KGetApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    KGetApplet(QObject *parent, const QVariantList &args);
    ~KGetApplet();

    void init();
    void setDataWidget(QGraphicsWidget * widget);

public slots:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);

private slots:
    virtual void slotKgetStarted();
    void slotUpdateTransfer(int transferChange);

signals:
    void transfersAdded(const QList<OrgKdeKgetTransferInterface*> &transfers);
    void transfersRemoved(const QList<OrgKdeKgetTransferInterface*> &transfers);
    void update();

private:
    void updateGlobalProgress();
    void transferAdded(const QVariantMap &transfer);
    void transferRemoved(const QVariantMap &transfer);

protected:
    virtual bool sceneEventFilter(QGraphicsItem * watched, QEvent * event);
    virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
    virtual void dropEvent(QDropEvent * event);
    virtual void constraintsEvent(Plasma::Constraints constraints);

    struct Data
    {
        KIO::filesize_t size;
        KIO::filesize_t downloadedSize;
    };

    ProxyWidget *m_proxyWidget;
    ErrorWidget *m_errorWidget;
    QGraphicsWidget *m_dataWidget;
    Plasma::Meter *m_globalProgress;
    Plasma::IconWidget *m_icon;
    Plasma::DataEngine *m_engine;
    KIO::filesize_t m_totalSize;
    KIO::filesize_t m_downloadedSize;
    QHash<OrgKdeKgetTransferInterface*, Data> m_transfers;
    static const QString KGET_DBUS_SERVICE;
    static const QString KGET_DBUS_PATH;
};

#endif
