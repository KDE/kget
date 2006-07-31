/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QTreeView>

class TransfersView : public QTreeView
{
    public:
        TransfersView(QWidget * parent = 0);
        ~TransfersView();
};
