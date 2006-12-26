/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERSVIEW_H
#define _TRANSFERSVIEW_H

#include <QTreeView>

class TransfersView : public QTreeView
{
    Q_OBJECT

    public:
        TransfersView(QWidget * parent = 0);
        ~TransfersView();

        int sizeHintForRow ( int row ) const;

    private:
};

#endif
