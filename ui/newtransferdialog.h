/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef NEW_TRANSFER_DIALOG_H
#define NEW_TRANSFER_DIALOG_H

#include <KDialog>
#include <KUrl>

#include "ui_newtransferwidget.h"

class KListWidget;

class NewTransferDialog : public KDialog, Ui::NewTransferWidget
{
    Q_OBJECT

public:
    NewTransferDialog(QWidget *parent = 0);

    static void showNewTransferDialog(const QString &srcUrl = QString());
    static void showNewTransferDialog(const KUrl::List &list);

    void setSource(const QString &srcUrl = QString());
    void setSource(const KUrl::List &list);
    KUrl::List source() const;
    void setDestination(QStringList list);
    QString destination() const;
    QString transferGroup() const;

    void setMultiple(bool multiple);
    bool multiple() const;

public slots:
    void setDefaultDestination();

private:
    static void showNewTransferDialog(NewTransferDialog *dialog);
    void prepareGui();

    bool m_multiple;

    KListWidget *listWidget;
    KLineEdit *urlRequester;
    QGridLayout *m_gridLayout;
    KTitleWidget *m_titleWidget;
    QGridLayout *m_gridLayout1;
    KUrlComboRequester *m_destRequester;
    QComboBox *m_groupComboBox;
    QCheckBox *m_defaultFolderButton;
    QLabel *m_groupLabel;
};

#endif
