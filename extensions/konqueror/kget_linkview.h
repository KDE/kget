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
#include <KLocale>

#include "links.h"

class QTreeView;
class QModelIndex;
class QAbstractButton;
class QButtonGroup;
class QSortFilterProxyModel;

static const QString WEB_CONTENT_REGEXP = "(^.(?:(?!(\\.php|\\.html|\\.asp|\\.aspx|\\.jsp)).)*$)";
static const QString VIDEO_FILES_REGEXP = "(.(?:\\.avi|\\.mpeg|\\.mpg))";
static const QString AUDIO_FILES_REGEXP = "(.(?:\\.mp3|\\.ogg))";
static const QString COMPRESSED_FILES_REGEXP = "(.(?:\\.zip|\\.tar|\\.tar.bz|\\.tar.gz|\\.rar))";

struct filterDefinition {
    QString icon;
    QString name;
    uint type;
    bool defaultFilter;
};

class KGetLinkView : public KDialog
{
    Q_OBJECT

public:
    enum DownloadFilterType {
        NoFilter = 0,
        VideoFiles = 1,
        AudioFiles = 2,
        CompressedFiles = 3
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
    QAbstractButton *createFilterButton(const QString &icon, const QString &name,
                            QButtonGroup *group, uint filterType, bool checked = false);

    QList<LinkItem*> m_links;

    QTreeView *m_treeWidget;
    QSortFilterProxyModel *m_proxyModel;
    bool m_showWebContent;

    QButtonGroup *filterButtonsGroup;
    QPushButton *downloadCheckedButton;
    QPushButton *checkAllButton;
};

// icon, name, regular expression, and default of the filter buttons
static const filterDefinition filters [] = {
    {QString("view-list-icons"), i18nc("filter: show all file types", "All"), KGetLinkView::NoFilter, true},
    {QString("video-x-generic"), i18n("Videos"), KGetLinkView::VideoFiles, false},
    {QString("audio-x-generic"), i18n("Audio"), KGetLinkView::AudioFiles, false},
    {QString("package-x-generic"), i18n("Archives"), KGetLinkView::CompressedFiles, false},
    {QString(""), QString(""), 0, false}
};

#endif // KGET_LINKVIEW_H
