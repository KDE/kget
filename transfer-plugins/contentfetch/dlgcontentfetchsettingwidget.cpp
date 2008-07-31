/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgcontentfetchsettingwidget.h"
#include "dlgscriptediting.h"
#include "contentfetchsetting.h"

#include <QSize>

#include <kdialog.h>
#include <kdebug.h>

DlgContentFetchSettingWidget::DlgContentFetchSettingWidget(KDialog *p_parent)
    : QWidget(p_parent),
      m_p_parent(p_parent),m_p_action(0)
{
    ui.setupUi(this);
    ui.newScriptButton->setIcon(KIcon("list-add"));
    ui.removeScriptButton->setIcon(KIcon("list-remove"));

    loadContentFetchSetting();
    m_changed = false;

    connect(ui.newScriptButton, SIGNAL(clicked()), this, SLOT(slotNewScript()));
    connect(ui.editScriptButton, SIGNAL(clicked()), this, SLOT(slotEditScript()));
    connect(ui.configureScriptButton, SIGNAL(clicked()), this, SLOT(slotConfigureScript()));
    connect(ui.removeScriptButton, SIGNAL(clicked()), this, SLOT(slotRemoveScript()));
    connect(ui.scriptTreeWidget,
            SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotCheckConfigurable(QTreeWidgetItem*, int)));
    connect(ui.scriptTreeWidget,
            SIGNAL(itemChanged(QTreeWidgetItem* , int)),
            this, SLOT(slotEnableChanged(QTreeWidgetItem*, int)));
    connect(p_parent, SIGNAL(accepted()), this, SLOT(slotAccepted()));
    connect(p_parent, SIGNAL(rejected()), this, SLOT(slotRejected()));
}

DlgContentFetchSettingWidget::~DlgContentFetchSettingWidget()
{
}

void DlgContentFetchSettingWidget::slotNewScript()
{
    DlgScriptEditing dialog(this);
    if(dialog.exec())
    {
        addScriptItem(true, dialog.scriptPath(), dialog.scriptUrlRegexp(),
                      dialog.scriptDescription());
    }
    m_changed = true;
}

void DlgContentFetchSettingWidget::slotEditScript()
{
    QList<QTreeWidgetItem *> selectedItems =
        ui.scriptTreeWidget->selectedItems();
    // only edit one item at one time
    if (selectedItems.size()!=1)
    {
        return;
    }
    QTreeWidgetItem &item = *(selectedItems[0]);
    DlgScriptEditing dialog(this, (QStringList() << item.toolTip(0)
                                   << item.text(1) << item.text(2)));
    if(dialog.exec())
    {
        if (item.toolTip(0) != dialog.scriptPath())
        {
            item.setText(0, QFileInfo(dialog.scriptPath()).fileName());
            item.setToolTip(0, dialog.scriptPath());
            m_changed = true;
        }
        if (item.text(1) != dialog.scriptUrlRegexp())
        {
            item.setText(1, dialog.scriptUrlRegexp());
            m_changed = true;
        }
        if (item.text(2) != dialog.scriptDescription())
        {
            item.setText(2, dialog.scriptDescription());
            m_changed = true;
        }
    }
}

void DlgContentFetchSettingWidget::slotConfigureScript()
{
    QList<QTreeWidgetItem *> selectedItems =
        ui.scriptTreeWidget->selectedItems();
    // only configure one item at one time
    if (selectedItems.size()!=1)
    {
        return;
    }
    QString filename = selectedItems[0]->toolTip(0);
    if (m_p_action)
    {
        delete m_p_action;
    }
    m_p_action = new Kross::Action(this, QString("%1_ContentFetchConfig").arg(filename));
    // TODO add check file
    m_p_action->setFile(filename);
    m_p_action->addObject(this, "kgetscriptconfig",
                          Kross::ChildrenInterface::AutoConnectSignals);
    m_p_action->trigger();

    KDialog *dialog = new KDialog(this);
    dialog->setObjectName("configure_script");
    dialog->setCaption(i18nc("Configure script", "Configure script"));
    dialog->enableButtonOk(false);
    dialog->setModal(true);

    SettingWidgetAdaptor *widget = new SettingWidgetAdaptor(dialog);
    ScriptConfigAdaptor config(selectedItems[0]->text(0));
    emit configureScript(widget, &config);

    if (widget->findChild<QWidget*>())
    {
        dialog->enableButtonOk(true);
    }

    dialog->setMainWidget(widget);
    dialog->showButtonSeparator(true);
    // dirty hack, add the ok/canel button size manually
    dialog->resize(widget->size()+QSize(0,30));
    dialog->show();

    if (dialog->exec() == QDialog::Accepted)
    {
        emit configurationAccepted(widget, &config);
    }

    dialog->deleteLater();
}

void DlgContentFetchSettingWidget::slotRemoveScript()
{
    QList<QTreeWidgetItem *> selectedItems =
        ui.scriptTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
        delete(selectedItem);
    m_changed = true;
}

void DlgContentFetchSettingWidget::addScriptItem(bool enabled, const QString &path, const QString &regexp, const QString &description)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << QFileInfo(path).fileName() << regexp << description);
    item->setToolTip(0, path);
    item->setCheckState(0, enabled ? Qt::Checked : Qt::Unchecked);
    ui.scriptTreeWidget->addTopLevelItem(item);
}

void DlgContentFetchSettingWidget::loadContentFetchSetting()
{
    ui.scriptTreeWidget->clear();//Cleanup things first

    QStringList paths = ContentFetchSetting::self()->pathList();
    QStringList regexps = ContentFetchSetting::self()->urlRegexpList();
    QStringList descriptions = ContentFetchSetting::self()->descriptionList();
    QList<int> enables = ContentFetchSetting::self()->enableList();
    for (int i = 0; i < paths.size(); ++i)
    {
        addScriptItem(bool(enables[i]), paths[i], regexps[i], descriptions[i]);
    }
}

void DlgContentFetchSettingWidget::saveContentFetchSetting()
{
    kDebug(5002);
    QStringList paths;
    QStringList regexps;
    QStringList descriptions;
    QList<int> enables;

    for (int i = 0; i < ui.scriptTreeWidget->topLevelItemCount(); ++i)
    {
        paths.append(ui.scriptTreeWidget->topLevelItem(i)->toolTip(0));
        regexps.append(ui.scriptTreeWidget->topLevelItem(i)->text(1));
        descriptions.append(ui.scriptTreeWidget->topLevelItem(i)->text(2));
        if (ui.scriptTreeWidget->topLevelItem(i)->checkState(0) == Qt::Unchecked)
        {
            enables.append(0);
        }
        else
        {
            enables.append(1);
        }
    }

    ContentFetchSetting::self()->setPathList(paths);
    ContentFetchSetting::self()->setUrlRegexpList(regexps);
    ContentFetchSetting::self()->setDescriptionList(descriptions);
    ContentFetchSetting::self()->setEnableList(enables);

    ContentFetchSetting::self()->writeConfig();
}

void DlgContentFetchSettingWidget::slotSave()
{
    if (m_changed)
    {
        saveContentFetchSetting();
        ContentFetchSetting::self()->writeConfig();
        m_changed = false;
    }
}

void DlgContentFetchSettingWidget::slotAccepted()
{
    slotSave();
    // NOTICE: clean the last config script, might change in the furture
    if (m_p_action)
    {
        delete m_p_action;
        m_p_action = 0;
    }
}

void DlgContentFetchSettingWidget::slotRejected()
{
    // clean the last config script
    if (m_p_action)
    {
        delete m_p_action;
        m_p_action = 0;
    }
}

void DlgContentFetchSettingWidget::slotCheckConfigurable(QTreeWidgetItem *p_item,
                                                         int column )
{
    if (column == -1)
    {
        return;
    }
    QString filename = p_item->toolTip(0);
    Kross::Action action(this, QString("%1_CheckConfig").arg(filename));
    // TODO add check file
    action.setFile(filename);
    action.trigger();
    if (action.functionNames().contains("configureScript"))
    {
        ui.configureScriptButton->setEnabled(true);
    }
    else
    {
        ui.configureScriptButton->setEnabled(false);
    }
}

void DlgContentFetchSettingWidget::slotEnableChanged(QTreeWidgetItem* p_item,
                                                     int column)
{
    Q_UNUSED(p_item);
    if (column != 0)
    {
        return;
    }
    m_changed = true;
}
