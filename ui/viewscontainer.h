/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _VIEWSCONTAINER_H
#define _VIEWSCONTAINER_H

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MainView;

class ViewsContainer : public QWidget
{
    Q_OBJECT
    public:
        ViewsContainer(QWidget * parent = 0);

    private:
        QVBoxLayout * m_VLayout;
        QHBoxLayout * m_HLayout;

        MainView    * m_mainView;

        QToolButton * m_downloads;
        QToolButton * m_finished;
};

#endif
