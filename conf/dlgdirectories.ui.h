/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qfont.h>
#include <klocale.h>
#include <qstringlist.h>
#include "settings.h"

void DlgDirectories::init()
{
    QFont font = txtLabel->font();
    font.setPointSize( font.pointSize() + 2 );
    txtLabel->setFont( font );
    font.setBold( true );
    bhLabel->setFont( font );
    
    kcfg_DefaultDirectory->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    folderRequest->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    
    checkBehavior();
    
    loadData();
    lv_entries->setSorting(0);
    lv_entries->sort();
    updateButtons();
}

void DlgDirectories::checkBehavior()
{
    if ( kcfg_UseDefaultDirectory->isChecked() )
	bhLabel->setText( i18n("Place files into default folders") );
    else if ( kcfg_AlwaysAskDirectory->isChecked() ) 
	bhLabel->setText( i18n("Ask for folder on each download") );
    else
	bhLabel->setText( i18n("Place files into the last directory") );
}

void DlgDirectories::kcfg_AlwaysAskDirectory_toggled( bool e )
{
    if ( e && kcfg_UseDefaultDirectory->isChecked() )
	kcfg_UseDefaultDirectory->setChecked( false );
    checkBehavior();
}

void DlgDirectories::kcfg_UseDefaultDirectory_toggled( bool e )
{
    if ( e && kcfg_AlwaysAskDirectory->isChecked() )
	kcfg_AlwaysAskDirectory->setChecked( false );
    checkBehavior();
}


// BEGIN FoldersMatchingExtension block

void DlgDirectories::updateButtons()
{
    bool noItem = lv_entries->selectedItem() == 0;
    le_ext->setEnabled( !noItem );
    folderRequest->setEnabled( !noItem );
    if ( noItem )
    {
        le_ext->clear();
        folderRequest->clear();
    }
    pb_delete->setEnabled( !noItem );
    pb_add->setEnabled( true );
}

void DlgDirectories::le_ext_textChanged( const QString & ext )
{
    QListViewItem * item = lv_entries->selectedItem();
    if ( !item )
    {
	updateButtons();
	return;
    }
    item->setText( 0, ext );
    lv_entries->sort();
    saveData();
}

void DlgDirectories::folderRequest_textChanged( const QString & dir )
{
    QListViewItem * item = lv_entries->selectedItem();
    if ( !item )
    {
	updateButtons();
	return;
    }
    item->setText( 1, dir );
    saveData();
}

void DlgDirectories::lv_entries_currentChanged( QListViewItem * lvi )
{
    updateButtons();
    if ( lvi )
    {
	le_ext->setText( lvi->text(0) );
	folderRequest->setURL( lvi->text(1) );
    }
}

void DlgDirectories::pb_add_clicked()
{
    // default extension text
    QString ext = i18n("extension");
    // default directory text
    QString dir = i18n("select folder");
    QString text = kcfg_DefaultDirectory->url();
    if ( text.length() > 0 )
	dir = text;
    //check if already added an item
    QListViewItem * item = lv_entries->selectedItem();
    if ( item && item->text(0) == ext && item->text(1) == dir )
	return;
    
    item = new QListViewItem(lv_entries, ext, dir);
    lv_entries->setSelected( item, true );
    lv_entries_currentChanged( item );
}

void DlgDirectories::pb_delete_clicked()
{
    QListViewItem *item = lv_entries->selectedItem();
    if ( item )
	delete item;
    lv_entries->sort();
    updateButtons();
    saveData();
}

/* CHECKs 
        QString ext = le_ext->text();
        QString dir = le_dir->text();

        if (ext.contains(",") || dir.contains(",") || ext.isEmpty() || dir.isEmpty()) {
            KMessageBox::error(this, i18n("Each row consists of exactly one\nextension type and one folder"), i18n("Error"));
            return;
        }

        QDir f(dir);

        if (!f.exists()) {
            KMessageBox::error(this, i18n("Folder does not exist:\n%1").arg(dir), i18n("Error"));
            return;
        }
*/

void DlgDirectories::loadData()
{    
    lv_entries->clear();
    
    QStringList list = Settings::mimeDirList();
    QStringList::Iterator it = list.begin();
    QStringList::Iterator end = list.end();
    while ( it != end )
    {
	// odd list items are regular expressions
	QString rexp = *it;
	++it;
	QString path = *it;
	++it;
	new QListViewItem(lv_entries, rexp, path);
    }
}

void DlgDirectories::saveData()
{
    QStringList list;
    QListViewItemIterator it(lv_entries);

    for (; it.current(); ++it) {
        QListViewItem *item = it.current();
	QString rexp = item->text(0);
        if (rexp.contains(",") || rexp.isEmpty())
	    continue;
        list.append( rexp );
        list.append( item->text(1) );
    }
    Settings::setMimeDirList( list );
}

// END FoldersMatchingExtension block
