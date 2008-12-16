/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTDETAILSWIDGET_H
#define BTDETAILSWIDGET_H

#include "ui_btdetailswidgetfrm.h"

#include <QWidget>

#include "core/observer.h"

class BTTransferHandler;

class QShowEvent;
class QHideEvent;

class BTDetailsWidget : public QWidget, public TransferObserver, public Ui::BTDetailsWidgetFrm
{
    Q_OBJECT
    public:
        BTDetailsWidget(BTTransferHandler * transfer);
        ~BTDetailsWidget();

        void transferChangedEvent(TransferHandler * transfer);

    protected:
        virtual void showEvent(QShowEvent * event);
        virtual void hideEvent(QHideEvent * event);

    private:
        BTTransferHandler * m_transfer;
};

#endif
