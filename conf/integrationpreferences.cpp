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

#include "integrationpreferences.h"
#include "autopastemodel.h"
#include "settings.h"

#include <KConfigDialog>

IntegrationPreferences::IntegrationPreferences(KConfigDialog *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    ui.setupUi(this);

    //AutoPaste stuff
    ui.type->addItem(KIcon("list-add"), i18n("Include"), AutoPasteModel::Include);
    ui.type->addItem(KIcon("list-remove"), i18n("Exclude"), AutoPasteModel::Exclude);

    ui.patternSyntax->addItem(i18n("Escape sequences"), AutoPasteModel::Wildcard);
    ui.patternSyntax->addItem(i18n("Regular expression"), AutoPasteModel::RegExp);

    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.increase->setIcon(KIcon("arrow-up"));
    ui.decrease->setIcon(KIcon("arrow-down"));

    m_model = new AutoPasteModel(this);
    m_model->load();
    ui.list->setModel(m_model);
    AutoPasteDelegate *delegate = new AutoPasteDelegate(ui.type->model(), ui.patternSyntax->model(), this);
    ui.list->setItemDelegate(delegate);

    QByteArray loadedState = QByteArray::fromBase64(Settings::autoPasteHeaderState().toAscii());
    if (Settings::autoPasteHeaderState().isEmpty()) {
        ui.list->resizeColumnToContents(AutoPasteModel::Type);
    } else if (!loadedState.isNull()) {
        ui.list->header()->restoreState(loadedState);
    }

    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(changed()));
    connect(ui.list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateButtons()));
    connect(ui.pattern, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtons()));
    connect(ui.pattern, SIGNAL(returnPressed(QString)), this, SLOT(slotAddItem()));
    connect(ui.add, SIGNAL(clicked()), this, SLOT(slotAddItem()));
    connect(ui.remove, SIGNAL(clicked()), this, SLOT(slotRemoveItem()));
    connect(ui.increase, SIGNAL(clicked()), this, SLOT(slotIncreasePriority()));
    connect(ui.decrease, SIGNAL(clicked()), this, SLOT(slotDecreasePriority()));
    connect(parent, SIGNAL(rejected()), m_model, SLOT(load()));
    connect(parent, SIGNAL(applyClicked()), m_model, SLOT(save()));
    connect(parent, SIGNAL(okClicked()), m_model, SLOT(save()));
    connect(parent, SIGNAL(defaultClicked()), m_model, SLOT(resetDefaults()));

    slotUpdateButtons();
}

IntegrationPreferences::~IntegrationPreferences()
{
}

void IntegrationPreferences::slotUpdateButtons()
{
    ui.add->setEnabled(!ui.pattern->text().isEmpty());
    ui.remove->setEnabled(ui.list->selectionModel()->hasSelection());

    const QModelIndex index = ui.list->currentIndex();
    const bool indexValid = index.isValid() && (ui.list->selectionModel()->selectedRows().count() == 1);
    ui.increase->setEnabled(indexValid && (index.row() > 0));
    ui.decrease->setEnabled(indexValid && (m_model->rowCount() > (index.row() + 1)));
}

void IntegrationPreferences::slotAddItem()
{
    const QString pattern = ui.pattern->text();
    if (pattern.isEmpty()) {
        return;
    }

    AutoPasteModel::TypeData type = static_cast<AutoPasteModel::TypeData>(ui.type->itemData(ui.type->currentIndex()).toInt());
    AutoPasteModel::PatternSyntaxData syntax = static_cast<AutoPasteModel::PatternSyntaxData>(ui.patternSyntax->itemData(ui.patternSyntax->currentIndex()).toInt());
    m_model->addItem(type, syntax, pattern);

    ui.pattern->clear();
    ui.pattern->setFocus();
    emit changed();
}

void IntegrationPreferences::slotRemoveItem()
{
    QItemSelectionModel *selection = ui.list->selectionModel();
    if (selection->hasSelection()) {
        while (selection->selectedRows().count()) {
            const QModelIndex index = selection->selectedRows().first();
            m_model->removeRow(index.row());
        }
        emit changed();
    }
}

void IntegrationPreferences::slotIncreasePriority()
{
    const int row = ui.list->currentIndex().row();
    m_model->moveItem(row, row - 1);
    slotUpdateButtons();
    emit changed();
}

void IntegrationPreferences::slotDecreasePriority()
{
    const int row = ui.list->currentIndex().row();
    m_model->moveItem(row, row + 2);
    slotUpdateButtons();
    emit changed();
}

