/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _VIEWSCONTAINER_H
#define _VIEWSCONTAINER_H

#include <QLabel>
#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>

class TransfersView;
class MainView;
class TransferHandler;

class TitleBar : public QWidget
{
    Q_OBJECT
    public:
        TitleBar(QWidget * parent = 0);

        void setTransfer(TransferHandler * transfer);
        void setDownloadsWindow();
        void setFinishedWindow();

    private:
        QLabel * m_label;
        QHBoxLayout * m_layout;
};

class ButtonBase : public QToolButton
{
    Q_OBJECT
    public:
        ButtonBase(QWidget * parent = 0);

    public slots:
        virtual void slotToggled(bool checked);

    signals:
        void activated();
};

class TransfersButton : public ButtonBase
{
    Q_OBJECT
    public:
        TransfersButton();

    public slots:
        void addTransfer(TransferHandler * transfer);
        void removeTransfer(TransferHandler * transfer);
        void setTransfer(TransferHandler * transfer);

    signals:
        void selectedTransfer(TransferHandler * transfer);

    private slots:
        void slotToggled(bool checked);
        void slotActionTriggered(QAction *);

    private:
        TransferHandler * m_selectedTransfer;
        QMenu * m_menu;
        QMap<QAction *, TransferHandler *> m_transfersMap;
};

class ViewsContainer : public QWidget
{
    Q_OBJECT
    public:
        ViewsContainer(QWidget * parent = 0);

    public slots:
        void showTransferDetails(TransferHandler * transfer);
        void closeTransferDetails(TransferHandler * transfer);

        void showDownloadsWindow();
        void showFinishedWindow();

    private slots:
        void slotTransferSelected(TransferHandler * transfer);

    private:
        QVBoxLayout     * m_VLayout;
        QHBoxLayout     * m_HLayout;
        QStackedLayout  * m_SLayout;

        TitleBar        * m_titleBar;

        TransfersView   * m_transfersView;
        QWidget         * m_finishedView;  //This view has still to be created.

        ButtonBase      * m_downloadsBt;
        ButtonBase      * m_finishedBt;
        TransfersButton * m_transfersBt;

        QMap<TransferHandler *, QWidget *> m_transfersMap;
};

#endif
