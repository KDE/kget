/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgscriptediting.h"
#include "contentfetchsetting.h"

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent)
    : KDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    setWindowTitle(i18n("Add new script"));
    init();
}

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent,
				   const QStringList &script)
    : KDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    setWindowTitle(i18n("Edit script"));
    ui.scriptPathEdit->setText(script[0]);
    ui.scriptUrlRegexpEdit->setText(script[1]);
    ui.scriptDescriptionEdit->setText(script[2]);

    init();
}

void DlgScriptEditing::init()
{
    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel);
    showButtonSeparator(true);

    connect(ui.scriptPathEdit,SIGNAL(textChanged(const QString &)),
	    this, SLOT(slotChangeText()));
    connect(ui.scriptUrlRegexpEdit,SIGNAL(textChanged(const QString &)),
	    this, SLOT(slotChangeText()));
    connect(ui.scriptDescriptionEdit,SIGNAL(textChanged(const QString &)),
	    this, SLOT(slotChangeText()));
}

DlgScriptEditing::~DlgScriptEditing()
{
}

void DlgScriptEditing::slotChangeText()
{
    enableButton(KDialog::Ok, !(ui.scriptPathEdit->text().isEmpty() ||
				ui.scriptUrlRegexpEdit->text().isEmpty()));
}

QString DlgScriptEditing::scriptPath() const
{
    return ui.scriptPathEdit->text();
}

QString DlgScriptEditing::scriptUrlRegexp() const
{
    return ui.scriptUrlRegexpEdit->text();
}

QString DlgScriptEditing::scriptDescription() const
{
    return ui.scriptDescriptionEdit->text();
}

