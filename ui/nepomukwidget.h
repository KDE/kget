/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef NEPOMUKWIDGET_H
#define NEPOMUKWIDGET_H

#include <QWidget>

class TransferHandler;
class NepomukHandler;
class KLineEdit;

class NepomukWidget : public QWidget
{
    Q_OBJECT
    public:
        NepomukWidget(TransferHandler *transfer, QWidget * parent);
        ~NepomukWidget();

        static QWidget * createNepomukWidget(TransferHandler *transfer);
        bool isValid();

    private slots:
        void showTagContextMenu(const QString& tag);
        void addCurrentTag();
        void removeCurrentTag();
        void addNewTag();

    private:
        TransferHandler *m_transfer;
        NepomukHandler *m_nepHandler;
        QString m_currentTag;
        KLineEdit *m_lineEdit;
};

#endif
