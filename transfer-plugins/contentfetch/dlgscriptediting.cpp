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
#include <KFileDialog>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent)
    : QDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    setWindowTitle(i18n("Add New Script"));
    init();
}

DlgScriptEditing::DlgScriptEditing(QWidget *p_parent,
                                   const QStringList &script)
    : QDialog(p_parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    setWindowTitle(i18n("Edit Script"));
    ui.scriptPathRequester->setUrl(QUrl::fromLocalFile(script[0]));
    ui.scriptUrlRegexpEdit->setText(script[1]);
    ui.scriptDescriptionEdit->setText(script[2]);

    init();
}

void DlgScriptEditing::init()
{
    ui.scriptPathRequester->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
    ui.scriptPathRequester->fileDialog()->setWindowTitle(i18n("Set Script File"));

    QStringList filter;
    foreach(Kross::InterpreterInfo* info, Kross::Manager::self().interpreterInfos())
        filter << info->mimeTypes().join(" ");
    ui.scriptPathRequester->setFilter(filter.join(" "));

    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgScriptEditing::accept);
    connect(buttonBox, &QSIGNAL(rejected()), this, &DlgScriptEditing::reject);
    mainLayout->addWidget(buttonBox);

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
    okButton->setEnabled(!(ui.scriptPathRequester->url().isEmpty() ||
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

