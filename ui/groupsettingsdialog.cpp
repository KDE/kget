/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "groupsettingsdialog.h"

#include "core/transfergrouphandler.h"
#include <KFileDialog>

#ifdef HAVE_NEPOMUK
    #include <QtGui/QSortFilterProxyModel>
    #include <KMenu>
#endif

GroupSettingsDialog::GroupSettingsDialog(QWidget *parent, TransferGroupHandler *group)
  : KDialog(parent),
    m_group(group)
{
    setCaption(i18n("Group Settings for %1", group->name()));
    showButtonSeparator(true);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);

    setMainWidget(widget);

    ui.downloadBox->setValue(group->downloadLimit(Transfer::VisibleSpeedLimit));
    ui.uploadBox->setValue(group->uploadLimit(Transfer::VisibleSpeedLimit));

    if (ui.downloadBox->value() != 0)
        ui.downloadCheck->setChecked(true);

    if (ui.uploadBox->value() != 0)
        ui.uploadCheck->setChecked(true);

    ui.defaultFolderRequester->setMode(KFile::Directory);
    QString path = group->defaultFolder();
    ui.defaultFolderRequester->setUrl(path);
    ui.defaultFolderRequester->setStartDir(KUrl(KGet::generalDestDir(true)));

    ui.regExpEdit->setText(group->regExp().pattern());

#ifdef HAVE_NEPOMUK
    m_tags = group->tags();
    m_tags.removeAll(QString());//remove empty strings if they are there
    updateUsedTagsLineEdit();

    //BEGIN the tag-dialog
    m_usedTagsModel = new QStringListModel(m_tags);
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_usedTagsModel);
    proxyModel->sort(0);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QWidget *tagWidget = new QWidget(this);
    tagsUi.setupUi(tagWidget);
    tagsUi.usedTags->setModel(m_usedTagsModel);
    tagsUi.addButton->setIcon(KIcon("list-add"));
    tagsUi.tagCloud->setAutoUpdate(true);

    KDialog *tagDialog = new KDialog(this);
    tagDialog->setButtons(KDialog::Ok);
    tagDialog->setCaption(i18n("Tag Settings for the groups"));
    tagDialog->setMainWidget(tagWidget);
    tagDialog->setInitialSize(QSize(530, 370));
    tagDialog->setMinimumSize(300, 200);//FIXME this minimumSize should not be needed, but wihout it you can resize the dialog to (0,0) but only if the tagCloud has been added

    connect(tagsUi.tagCloud, SIGNAL(tagClicked(const QString&)), this, SLOT(tagClicked(const QString&)));
    connect(tagsUi.usedTags, SIGNAL(clicked(QModelIndex)), this, SLOT(modelClicked(QModelIndex)));
    connect(tagsUi.newTagsEdit, SIGNAL(userTextChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(tagsUi.addButton, SIGNAL(clicked(bool)), this, SLOT(addNewTag()));
    connect(ui.tagButton, SIGNAL(clicked(bool)), tagDialog, SLOT(show()));
    //END the tag-dialog

    m_popup = new KMenu(this);
    m_removeAction = m_popup->addAction(KIcon("list-remove"), i18n("Remove Tag"));
    connect(m_removeAction, SIGNAL(triggered()), this, SLOT(removeCurrentTag()));
    m_addAction = m_popup->addAction(KIcon("list-add"), i18n("Add Tag"));
    connect(m_addAction, SIGNAL(triggered()), this, SLOT(addCurrentTag()));
#else
    ui.nepomukWidget->hide();
#endif

    connect(this, SIGNAL(accepted()), SLOT(save()));
}

GroupSettingsDialog::~GroupSettingsDialog()
{
}

#ifdef HAVE_NEPOMUK
void GroupSettingsDialog::modelClicked(QModelIndex index)
{
    tagClicked(index.data(Qt::DisplayRole).toString());
}

void GroupSettingsDialog::tagClicked(const QString& tag)
{
    m_currentTag = tag;
    bool showAdd = !m_tags.contains(m_currentTag);
    m_addAction->setVisible(showAdd);
    m_removeAction->setVisible(!showAdd);

    m_popup->popup(QCursor::pos());
}

void GroupSettingsDialog::addCurrentTag()
{
    if(!m_currentTag.isEmpty() && !m_tags.contains(m_currentTag))
    {
        m_tags << m_currentTag;
        updateUsedTagsLineEdit();
        m_usedTagsModel->setStringList(m_tags);
    }
}

void GroupSettingsDialog::removeCurrentTag()
{
    m_tags.removeAll(m_currentTag);
    updateUsedTagsLineEdit();
    m_usedTagsModel->setStringList(m_tags);
}

void GroupSettingsDialog::addNewTag()
{
    QString newTag = tagsUi.newTagsEdit->text();
    if(!newTag.isEmpty() && !m_tags.contains(newTag))
    {
        tagsUi.tagCloud->addTag(newTag, 0);
        m_currentTag = newTag;
        addCurrentTag();
        tagsUi.newTagsEdit->clear();
    }
}

void GroupSettingsDialog::textChanged(const QString &text)
{
    tagsUi.addButton->setEnabled(!text.isEmpty());
}

void GroupSettingsDialog::updateUsedTagsLineEdit()
{
    m_tags.sort();
    ui.usedTags->setText(m_tags.join(", "));
}
#endif //HAVE_NEPOMUK

void GroupSettingsDialog::save()
{
    m_group->setDefaultFolder(ui.defaultFolderRequester->url().toLocalFile(KUrl::AddTrailingSlash));

    if (ui.downloadCheck->isChecked())
        m_group->setDownloadLimit(ui.downloadBox->value(), Transfer::VisibleSpeedLimit);
    else
        m_group->setDownloadLimit(0, Transfer::VisibleSpeedLimit);

    if (ui.uploadCheck->isChecked())
        m_group->setUploadLimit(ui.uploadBox->value(), Transfer::VisibleSpeedLimit);
    else
        m_group->setUploadLimit(0, Transfer::VisibleSpeedLimit);

    QRegExp regExp;
    regExp.setPattern(ui.regExpEdit->text());
    m_group->setRegExp(regExp);

#ifdef HAVE_NEPOMUK
    m_group->setTags(m_tags);
#endif
}

#include "groupsettingsdialog.moc"
