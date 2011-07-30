/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgscriptediting.h"
#include "contentfetchsetting.h"

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kfiledialog.h>

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent)
    : KDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    setWindowTitle(i18n("Add New Script"));
    init();
}

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent,
                                   const QStringList &script)
    : KDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    setWindowTitle(i18n("Edit Script"));
    ui.scriptPathRequester->setUrl(KUrl::fromPath(script[0]));
    ui.scriptUrlRegexpEdit->setText(script[1]);
    ui.scriptDescriptionEdit->setText(script[2]);

    init();
}

void DlgScriptEditing::init()
{
    ui.scriptPathRequester->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
    ui.scriptPathRequester->fileDialog()->setCaption(i18n("Set Script File"));

    QStringList filter;
    foreach(Kross::InterpreterInfo* info, Kross::Manager::self().interpreterInfos())
        filter << info->mimeTypes().join(" ");
    ui.scriptPathRequester->setFilter(filter.join(" "));

    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel);
    showButtonSeparator(true);

    connect(ui.scriptPathRequester,SIGNAL(textChanged(QString)),
            this, SLOT(slotChangeText()));
    connect(ui.scriptUrlRegexpEdit,SIGNAL(textChanged(QString)),
            this, SLOT(slotChangeText()));
    connect(ui.scriptDescriptionEdit,SIGNAL(textChanged(QString)),
            this, SLOT(slotChangeText()));
}

DlgScriptEditing::~DlgScriptEditing()
{
}

void DlgScriptEditing::slotChangeText()
{
    enableButton(KDialog::Ok, !(ui.scriptPathRequester->url().isEmpty() ||
                                ui.scriptUrlRegexpEdit->text().isEmpty()));
}

QString DlgScriptEditing::scriptPath() const
{
    return ui.scriptPathRequester->url().toLocalFile();
}

QString DlgScriptEditing::scriptUrlRegexp() const
{
    return ui.scriptUrlRegexpEdit->text();
}

QString DlgScriptEditing::scriptDescription() const
{
    return ui.scriptDescriptionEdit->text();
}

