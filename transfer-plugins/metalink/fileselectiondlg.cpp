/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
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

#include "fileselectiondlg.h"

#include "../../core/filemodel.h"

#include <QtGui/QSortFilterProxyModel>

#include <KInputDialog>

FileSelectionDlg::FileSelectionDlg(FileModel *model, QWidget *parent)
  : KDialog(parent)
{
    setCaption(i18n("File Selection"));
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    ui.treeView->setModel(proxy);
    ui.treeView->sortByColumn(0, Qt::AscendingOrder);
    ui.treeView->hideColumn(FileItem::Status);
    ui.treeView->hideColumn(FileItem::ChecksumVerified);
    ui.treeView->hideColumn(FileItem::SignatureVerified);

    setButtons(KDialog::Ok | KDialog::Cancel);
}
