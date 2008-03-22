/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERHISTORYITEMWIDGET_H
#define TRANSFERHISTORYITEMWIDGET_H

#include <QFrame>
#include <QModelIndex>

class QContextMenuEvent;
class QAction;
class QLabel;
class QDate;

class TransferHistoryItemWidget : public QFrame
{
Q_OBJECT
public:
    TransferHistoryItemWidget(QWidget *parent = 0);
    ~TransferHistoryItemWidget();

    void setSize(int size);
    void setDate(const QDate &date);
    void setUrl(const QString &transfer);
    void setDest(const QString &dest);
    void setModelIndex(const QModelIndex &index);

signals:
    void deletedTransfer(const QString &url, const QModelIndex &index);

private slots:
    void contextMenuEvent(QContextMenuEvent *event);
    void slotOpenFile();
    void slotDownload();
    void slotDeleteTransfer();

private:
    QLabel *m_size;
    QLabel *m_date;
    QLabel *m_image;
    QLabel *m_name;
    QLabel *m_host;
    QModelIndex m_index;

    QAction *m_actionDelete_Selected;
    QAction *m_actionDownload;
    QAction *m_openFile;

    QString m_url;
    QString m_dest;
};
#endif
