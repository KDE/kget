/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "nepomukwidget.h"
#include "ui_nepomukwidget.h"

#include "core/nepomukhandler.h"
#include "core/transferhandler.h"
#include <KAction>
#include <KLineEdit>
#include <KMenu>
#include <KLocale>

NepomukWidget::NepomukWidget(TransferHandler *transfer, QWidget *parent)
  : QWidget(parent),
    m_transfer(transfer),
    m_nepHandler(m_transfer->nepomukHandler()),
    m_currentTag(QString())
{
    if (!m_nepHandler)
        return;

    Ui::NepomukWidget ui;
    ui.setupUi(this);
    ui.ratingWidget->setRating(m_nepHandler->rating());
    ui.tags->setAutoUpdate(true);
    foreach (const QString &string, m_nepHandler->tags())
        ui.tags->addTag(string, 4);
    ui.button->setIcon(KIcon("list-add"));
    m_lineEdit = ui.lineEdit;

    connect(ui.ratingWidget, SIGNAL(ratingChanged(int)), m_nepHandler, SLOT(setRating(int)));
    connect(ui.tags, SIGNAL(tagClicked(const QString&)), SLOT(showTagContextMenu(const QString&)));
    connect(m_lineEdit, SIGNAL(returnPressed(const QString&)), m_nepHandler, SLOT(addTag(const QString&)));
    connect(m_lineEdit, SIGNAL(returnPressed(const QString&)), m_lineEdit, SLOT(clear()));
    connect(ui.button, SIGNAL(pressed()), this, SLOT(addNewTag()));
}

NepomukWidget::~NepomukWidget()
{
}

QWidget * NepomukWidget::createNepomukWidget(TransferHandler *transfer)
{
    return new NepomukWidget(transfer, 0);
}

void NepomukWidget::showTagContextMenu(const QString& tag)
{
    m_currentTag = tag;
    KMenu *popup = new KMenu(this);
    if (m_nepHandler->tags().contains(tag))
    {
        QAction *removeAction = popup->addAction(KIcon("list-remove"), i18n("Remove Tag"));
        connect(removeAction, SIGNAL(triggered()), SLOT(removeCurrentTag()));
        popup->popup(QCursor::pos());
    }
    else
    {
        QAction *addAction = popup->addAction(KIcon("list-add"), i18n("Add Tag"));
        connect(addAction, SIGNAL(triggered()), SLOT(addCurrentTag()));
        popup->popup(QCursor::pos());
    }
}

void NepomukWidget::removeCurrentTag()
{
    m_nepHandler->removeTag(m_currentTag);
}

void NepomukWidget::addCurrentTag()
{
    m_nepHandler->addTag(m_currentTag);
}

void NepomukWidget::addNewTag()
{
    QString tag = m_lineEdit->text();
    if (!tag.isEmpty())
    {
        m_nepHandler->addTag(tag);
        m_lineEdit->clear();
    }
}

bool NepomukWidget::isValid()
{
    return m_nepHandler->isValid();
}

#include "nepomukwidget.moc"
