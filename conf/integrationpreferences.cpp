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
#include <KLocale>

IntegrationPreferences::IntegrationPreferences(KConfigDialog *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
    ui.setupUi(this);

    //AutoPaste stuff
    ui.type->addItem(QIcon::fromTheme("list-add"), i18n("Include"), AutoPasteModel::Include);
    ui.type->addItem(QIcon::fromTheme("list-remove"), i18n("Exclude"), AutoPasteModel::Exclude);

    ui.patternSyntax->addItem(i18n("Escape sequences"), AutoPasteModel::Wildcard);
    ui.patternSyntax->addItem(i18n("Regular expression"), AutoPasteModel::RegExp);

    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.increase->setIcon(QIcon::fromTheme("arrow-up"));
    ui.decrease->setIcon(QIcon::fromTheme("arrow-down"));

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

    connect(m_model, &AutoPasteModel::dataChanged, this, &IntegrationPreferences::changed);
    connect(ui.list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotUpdateButtons()));
    connect(ui.pattern, &KLineEdit::textChanged, this, &IntegrationPreferences::slotUpdateButtons);
    connect(ui.pattern, &KLineEdit::returnPressed, this, &IntegrationPreferences::slotAddItem);
    connect(ui.add, &KPushButton::clicked, this, &IntegrationPreferences::slotAddItem);
    connect(ui.remove, &KPushButton::clicked, this, &IntegrationPreferences::slotRemoveItem);
    connect(ui.increase, &KPushButton::clicked, this, &IntegrationPreferences::slotIncreasePriority);
    connect(ui.decrease, &KPushButton::clicked, this, &IntegrationPreferences::slotDecreasePriority);
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

