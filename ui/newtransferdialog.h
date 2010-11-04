/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef NEW_TRANSFER_DIALOG_H
#define NEW_TRANSFER_DIALOG_H

#include <KDialog>
#include <KUrl>

#include "ui_newtransferwidget.h"

class KJob;

/**
* Dialog to allow add one or more transfers to kget.
* If only one transfer is added then the dialog shows a KUrlRequester.
* If a list of transfers are added then the dialog shows a KListWidget (multiple = true)
* with the transfers as checkable items.
* Also display a KUrlComboRequester for the destination file (or folder if multiple = true)
* And a QComboBox with the groups of transfer in case there are more than one
* 
* @note this class is private and should be used via NewTransferDialogHandler
*/
class NewTransferDialog : public KDialog, Ui::NewTransferWidget
{
    Q_OBJECT

    friend class NewTransferDialogHandler;

public:
    ~NewTransferDialog();

public slots:
    /**
    * Called when the transfer group or the urlREquester changed, the dialog sets the default destination 
    * for transfers in the new group
    */
    void setDefaultDestination();
    
private slots:
    void urlChanged(const QString &text);

private:
    explicit NewTransferDialog(QWidget *parent = 0);//TODO make explicit and private?

    /**
     * Shows the dialog adding one url list transfers
     */
    void showDialog(const KUrl::List &list, const QString &suggestedFileName = QString());
    void prepareDialog();
    void resizeDialog();
    bool isEmpty();

private:
    class Private;
    NewTransferDialog::Private *d;
    QWidget *m_window;
    KUrl::List m_sources;
};

class NewTransferDialogHandler : public QObject
{
    Q_OBJECT

    public:
        explicit NewTransferDialogHandler(QObject *parent = 0);
        ~NewTransferDialogHandler();

        /**
         * @see showNewTransferDialog(KUrl::List, QWidget)
         */
        static void showNewTransferDialog(const KUrl &url = KUrl(), QWidget * parent = 0);

        /**
         * This will show a dialog to the user to input needed information.
         * If the last url of the list is a local file or directory, then all files will
         * be downloaded to that destination.
         * If there are matching groups with default folders and the user set the option to
         * use those, then the affected urls will be downloaded without showing them in the dialog
         */
        static void showNewTransferDialog(KUrl::List list, QWidget *parent = 0);

    private slots:
        void slotMostLocalUrlResult(KJob *job);

    private:
        void handleUrls(const int jobId);
        void createDialog(const KUrl::List &urls, const QString &suggestedFileName, QWidget *parent);

    private:
        struct UrlData {
            KUrl::List urls;
            QString folder;
            QString suggestedFileName;
            QWidget *parent;
        };

        /**
         * Always points to the next unused jobId
         */
        int m_nextJobId;

        /**
         * QHash<jobId, numJobsForId>
         * Calling addUrls will create jobs for each url with the same id
         */
        QHash<int, int> m_numJobs;

        /**
         * QHash<jobId, UrlData>
         * Urls for which mosLocalUrl has finished already,
         * folder and suggestedFileName can be an empty string if there are none
         */
        QHash<int, UrlData> m_urls;

        QPointer<NewTransferDialog> m_dialog;
};

#endif
