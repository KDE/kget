/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "nepomukwidget.h"

#include "core/nepomukhandler.h"
#include "core/transferhandler.h"
#include <nepomuk/kratingwidget.h>
#include <nepomuk/kmetadatatagcloud.h>
#include <KAction>
#include <KLineEdit>
#include <KMenu>
#include <KLocale>
#include <QHBoxLayout>
#include <QToolButton>
#include <QVBoxLayout>

NepomukWidget::NepomukWidget(TransferHandler *transfer, QWidget *parent)
  : QWidget(parent),
    m_transfer(transfer),
    m_nepHandler(m_transfer->nepomukHandler()),
    m_currentTag(QString())
{
    if (!m_nepHandler)
        return;

    QVBoxLayout *layout = new QVBoxLayout(this);
    KRatingWidget *ratingWidget = new KRatingWidget(this);
    ratingWidget->setRating(m_nepHandler->rating());
    layout->addWidget(ratingWidget);
    Nepomuk::TagCloud *tags = new Nepomuk::TagCloud(this);
    tags->setAutoUpdate(true);
    foreach (const QString &string, m_nepHandler->tags())
        tags->addTag(string, 4);

    layout->addWidget(tags);

    QHBoxLayout *inputLayout = new QHBoxLayout(this);
    m_lineEdit = new KLineEdit(this);
    m_lineEdit->setClickMessage(i18n("Enter a new tag"));
    inputLayout->addWidget(m_lineEdit);
    QToolButton *button = new QToolButton(this);
    button->setIcon(KIcon("list-add"));
    inputLayout->addWidget(button);
    layout->addLayout(inputLayout);


    connect(ratingWidget, SIGNAL(ratingChanged(int)), m_nepHandler, SLOT(setRating(int)));
    connect(tags, SIGNAL(tagClicked(const QString&)), SLOT(showTagContextMenu(const QString&)));
    connect(m_lineEdit, SIGNAL(returnPressed(const QString&)), m_nepHandler, SLOT(addTag(const QString&)));
    connect(m_lineEdit, SIGNAL(returnPressed(const QString&)), m_lineEdit, SLOT(clear()));
    connect(button, SIGNAL(pressed()), this, SLOT(addNewTag()));
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

#include "nepomukwidget.moc"
