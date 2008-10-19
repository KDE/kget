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

class QTreeView;
class QModelIndex;
class QAbstractButton;
class QButtonGroup;
class QBoxLayout;
class QProgressBar;
class QSortFilterProxyModel;
class LinkImporter;
class KComboBox;
class KLineEdit;
class KUrlRequester;


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

    enum FilterMode {
        Contain = 0,
        DoesNotContain = 1
    };

    KGetLinkView(QWidget *parent = 0);
    ~KGetLinkView();

    // void setLinks( QList<LinkItem*>& links );
    void setLinks(const QList <QString> &links);
    void setPageUrl( const QString& url );
    void importUrl(const QString &url = QString());

signals:
    void leechUrls( const KUrl::List& urls );

private slots:
    void slotStartLeech();
    void selectionChanged();
    void updateSelectAllText(const QString &text = QString());
    void doFilter(int id, const QString &textFilter = QString());
    void checkAll();
    void uncheckAll();
    void slotShowWebContent(int mode);
    void uncheckItem(const QModelIndex &index);
    void slotCheckSelected();
    void slotInvertSelection();

    // import links slots
    void slotStartImport();
    void slotImportProgress(int progress);
    void slotImportFinished();
    void updateImportButtonStatus(const QString &text);

private:
    void showLinks( const QList<QString>& links );
    QAbstractButton *createFilterButton(const QString &icon, const QString &name,
                            QButtonGroup *group, uint filterType, bool checked = false);

    QList<QString> m_links;

    QTreeView *m_treeWidget;
    QSortFilterProxyModel *m_proxyModel;
    bool m_showWebContent;

    QButtonGroup *filterButtonsGroup;
    KLineEdit *m_searchLine;
    KComboBox *m_filterModeBox;
    QPushButton *downloadCheckedButton;
    QPushButton *checkAllButton;
    QPushButton *uncheckAllButton;
    QPushButton *m_importButton;
    QPushButton *m_checkSelectedButton;
    QPushButton *m_invertSelectionButton;

    // import links widgets
    LinkImporter *m_linkImporter;
    KUrlRequester *m_urlRequester;
    QBoxLayout *m_importerLayout;
    QProgressBar *m_progressBar;
};

// icon, name, regular expression, and default of the filter buttons
static const filterDefinition filters [] = {
    {QString("view-list-icons"), i18nc("filter: show all file types", "All"), KGetLinkView::NoFilter, true},
    {QString("video-x-generic"), i18n("Videos"), KGetLinkView::VideoFiles, false},
    {QString("audio-x-generic"), i18n("Audio"), KGetLinkView::AudioFiles, false},
    {QString("package-x-generic"), i18n("Archives"), KGetLinkView::CompressedFiles, false}
};

#endif // KGET_LINKVIEW_H
