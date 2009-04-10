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
class KGetSortFilterProxyModel;


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
    void setTextFilter(const QString &text = QString());
    void checkAll();
    void uncheckAll();
    void uncheckItem(const QModelIndex &index);
    void slotCheckSelected();
    void slotInvertSelection();
    void updateSelectionButtons();

    // import links slots
    void slotStartImport();
    void slotImportProgress(int progress);
    void slotImportFinished();
    void updateImportButtonStatus(const QString &text);

private:
    void checkClipboard();
    void showLinks( const QList<QString>& links );
    QAbstractButton *createFilterButton(const QString &icon, const QString &name,
                            QButtonGroup *group, uint filterType, bool checked = false);

    QList<QString> m_links;

    QTreeView *m_treeWidget;
    KGetSortFilterProxyModel *m_proxyModel;

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

#endif // KGET_LINKVIEW_H
