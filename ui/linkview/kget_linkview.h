/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_LINKVIEW_H
#define KGET_LINKVIEW_H

#include "../../core/basedialog.h"
#include <KLocale>

#include "ui_importlinkdialog.h"

class QAction;
class QModelIndex;
class LinkImporter;
class KGetSortFilterProxyModel;


class KGetLinkView : public KGetSaveSizeDialog
{
    Q_OBJECT

public:
    KGetLinkView(QWidget *parent = 0);
    ~KGetLinkView();

    // void setLinks( QList<LinkItem*>& links );
    void setLinks(const QStringList &links);
    void setPageUrl( const QString& url );
    void importUrl(const QString &url = QString());

signals:
    void leechUrls( const KUrl::List& urls );

private slots:
    void slotMimeTypeChanged(int index);
    void slotFilterModeChanged(int index);
    void slotFilterColumn(QAction*);
    void slotStartLeech();
    void selectionChanged();
    void setTextFilter(const QString &text = QString());
    void checkAll();
    void uncheckAll();
    void uncheckItem(const QModelIndex &index);
    void slotCheckSelected();
    void slotInvertSelection();
    void updateSelectionButtons();
    void contextMenuDisplayed(QMenu *menu);
    void wildcardPatternToggled(bool enabled);

    // import links slots
    void slotStartImport();
    void slotImportProgress(int progress);
    void slotImportFinished();
    void updateImportButtonStatus(const QString &text);

private:
    void checkClipboard();
    void showLinks(const QStringList &links, bool urlRequestVisible);

private:
    enum PatternSyntax {
        Wildcard = 0,
        RegExp
    };
    Ui::ImportLinkDialog ui;
    KGetSortFilterProxyModel *m_proxyModel;
    QStringList m_links;
    LinkImporter *m_linkImporter;
    QMenu *m_patternSyntaxMenu;
    QAction *m_nameAction;
    QAction *m_urlAction;
};

#endif // KGET_LINKVIEW_H
