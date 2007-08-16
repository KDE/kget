/* This file is part of the KDE project

   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_LINKVIEW_H
#define KGET_LINKVIEW_H

#include <KDialog>

#include "links.h"

class QTreeView;
class QModelIndex;
class QAbstractButton;
class QButtonGroup;
class QSortFilterProxyModel;

static const QString WEB_CONTENT_REGEXP = "(^.(?:(?!(\\.php|\\.html|\\.asp|\\.aspx|\\.jsp)).)*$)";
static const QString MEDIA_FILES_REGEXP = "(.(?:\\.mp3|\\.ogg|\\.avi|\\.mpeg|\\.mpg))";
static const QString COMPRESSED_FILES_REGEXP = "(.(?:\\.zip|\\.tar|\\.tar.bz|\\.tar.gz|\\.rar))";

class KGetLinkView : public KDialog
{
    Q_OBJECT

public:
    enum DownloadFilterType {
        NoFilter = 0,
        MediaFiles=1,
        CompressedFiles=2
    };

    KGetLinkView(QWidget *parent = 0);
    ~KGetLinkView();

    void setLinks( QList<LinkItem*>& links );
    void setPageUrl( const QString& url );

signals:
    void leechUrls( const KUrl::List& urls );

private slots:
    void slotStartLeech();
    void selectionChanged();
    void updateSelectAllText(const QString &text);
    void doFilter(int id, const QString &textFilter = QString());
    void checkAll();
    void slotShowWebContent(int mode);
    void uncheckItem(const QModelIndex &index);

private:
    void showLinks( const QList<LinkItem*>& links );
    QAbstractButton *createFilterButton(const char*, const char*, 
                            QButtonGroup *group, int filterType, bool checked = false);
    
    QList<LinkItem*> m_links;

    QTreeView *m_treeWidget;
    QSortFilterProxyModel *m_proxyModel;
    bool m_showWebContent;

    QButtonGroup *filterButtonsGroup;
    QPushButton *downloadCheckedButton;
    QPushButton *checkAllButton;
};

#endif // KGET_LINKVIEW_H
