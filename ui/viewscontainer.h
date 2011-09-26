/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef VIEWSCONTAINER_H
#define VIEWSCONTAINER_H

#include <QWidget>

class TransfersView;
class TransferHandler;

class ViewsContainer : public QWidget
{
    Q_OBJECT
    public:
        ViewsContainer(QWidget * parent = 0);

    public slots:
        void showTransferDetails(TransferHandler * transfer);
        void closeTransferDetails(TransferHandler * transfer);
        void selectAll();

    private:
        TransfersView   * m_transfersView;
};

#endif
