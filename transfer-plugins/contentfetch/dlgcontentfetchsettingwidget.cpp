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
    connect(p_parent, SIGNAL(accepted()), this, SLOT(slotAccepted()));
    connect(p_parent, SIGNAL(rejected()), this, SLOT(slotRejected()));
}

DlgContentFetchSettingWidget::~DlgContentFetchSettingWidget()
{
}

void DlgContentFetchSettingWidget::setScriptWidget(QWidget *p_widget)
{
    if (!p_widget)
    {
	return;
    }
    // load the widget into a dialog
    KDialog *p_dialog = new KDialog(this);
    p_dialog->setCaption(i18nc("Configure Script", "Configure Script"));
    p_widget->setParent(p_dialog);
    p_dialog->setMainWidget(p_widget);
    p_dialog->setModal(true);
    p_dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    p_dialog->showButtonSeparator(true);

    connect(p_dialog, SIGNAL(okClicked()),
	    this, SIGNAL(configurationAccepted()));
    p_dialog->show();
}

void DlgContentFetchSettingWidget::slotNewScript()
{
    DlgScriptEditing dialog(this);
    if(dialog.exec())
    {
	addScriptItem(dialog.scriptPath(), dialog.scriptUrlRegexp(),
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
    DlgScriptEditing dialog(this, (QStringList() << item.text(0)
				   << item.text(1) << item.text(2)));
    if(dialog.exec())
    {
	if (item.text(0) != dialog.scriptPath())
	{
	    // check file existance?
	    item.setText(0, dialog.scriptPath());
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
    QString filename = selectedItems[0]->text(0);
    if (m_p_action)
    {
	delete m_p_action;
    }
    m_p_action = new Kross::Action(0, "ContentFetchConfig");
    // TODO add check file
    m_p_action->setFile(filename);
    m_p_action->addObject(this, "kgetscriptconfig",
		     Kross::ChildrenInterface::AutoConnectSignals);
    // TODO: check if configurable
    m_p_action->trigger();
    emit configureScript();
}

void DlgContentFetchSettingWidget::slotRemoveScript()
{
    QList<QTreeWidgetItem *> selectedItems =
	ui.scriptTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
	delete(selectedItem);
}

void DlgContentFetchSettingWidget::addScriptItem(const QString &path, const QString &regexp, const QString &description)
{
    ui.scriptTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << path << regexp << description));
}

void DlgContentFetchSettingWidget::loadContentFetchSetting()
{
    ui.scriptTreeWidget->clear();//Cleanup things first

    QStringList paths = ContentFetchSetting::self()
	->findItem("PathList")->property().toStringList();
    QStringList regexps = ContentFetchSetting::self()
	->findItem("UrlRegexpList")->property().toStringList();
    QStringList descriptions = ContentFetchSetting::self()
	->findItem("DescriptionList")->property().toStringList();
    for (int i = 0; i < paths.size(); ++i)
    {
	addScriptItem(paths[i], regexps[i], descriptions[i]);
    }
}

void DlgContentFetchSettingWidget::saveContentFetchSetting()
{
    kDebug(5002);
    QStringList paths;
    QStringList regexps;
    QStringList descriptions;

    for (int i = 0; i < ui.scriptTreeWidget->topLevelItemCount(); ++i)
    {
	paths.append(ui.scriptTreeWidget->topLevelItem(i)->text(0));
	regexps.append(ui.scriptTreeWidget->topLevelItem(i)->text(1));
	descriptions.append(ui.scriptTreeWidget->topLevelItem(i)->text(2));
    }

    ContentFetchSetting::self()->findItem("PathList")
	->setProperty(QVariant(paths));
    ContentFetchSetting::self()->findItem("UrlRegexpList")
	->setProperty(QVariant(regexps));
    ContentFetchSetting::self()->findItem("DescriptionList")
	->setProperty(QVariant(descriptions));

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
    }
}

void DlgContentFetchSettingWidget::slotRejected()
{
    // clean the last config script
    if (m_p_action)
    {
	delete m_p_action;
    }
}

void DlgContentFetchSettingWidget::slotCheckConfigurable(QTreeWidgetItem *p_item,
							 int column )
{
    if (column == -1)
    {
	return;
    }
    QString filename = p_item->text(0);
    Kross::Action action(0, "CheckConfig");
    // TODO add check file
    action.setFile(filename);
    // NOTICE: Might need further investigation whether we need this object imported here
    action.addObject(this, "kgetscriptconfig");
    action.trigger();
#ifdef DEBUG
    QStringList funcs = action.functionNames();
    for (int i = 0; i < funcs.size() ; ++i)
    {
	kDebug(5002) << funcs[i];
    }
#endif
    if (action.functionNames().contains("configureScript"))
    {
	ui.configureScriptButton->setEnabled(true);
    }
    else
    {
	ui.configureScriptButton->setEnabled(false);
    }
}
