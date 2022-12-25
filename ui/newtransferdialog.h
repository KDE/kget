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

#include <QDialog>
#include <QUrl>

#include "ui_newtransferwidget.h"

class KJob;
class TransferHandler;

/**
 * Dialog to allow add one or more transfers to kget.
 * If only one transfer is added then the dialog shows a QUrlRequester.
 * If a list of transfers are added then the dialog shows a KListWidget (multiple = true)
 * with the transfers as checkable items.
 * Also display a QUrlComboRequester for the destination file (or folder if multiple = true)
 * And a QComboBox with the groups of transfer in case there are more than one
 *
 * @note this class is private and should be used via NewTransferDialogHandler
 */
class NewTransferDialog : public QDialog
{
    Q_OBJECT

    friend class NewTransferDialogHandler;

public:
    ~NewTransferDialog() override;

public Q_SLOTS:
    /**
     * Called when the transfer group or the urlREquester changed, the dialog sets the default destination
     * for transfers in the new group
     */
    void setDefaultDestination();

private Q_SLOTS:
    void inputTimer();
    void checkInput();
    void slotFinished(int resultCode);

private:
    explicit NewTransferDialog(QWidget *parent = nullptr);

    /**
     * Shows the dialog adding one url list transfers
     */
    void showDialog(QList<QUrl> list, const QString &suggestedFileName = QString());
    void prepareDialog();
    bool isEmpty();

    /**
     * Determines where is a multiple (listwidget) or single (kurlrequester) transfer
     */
    void setMultiple(bool useMultiple);

    /**
     * Set sources to the dialog
     */
    void setSource(const QList<QUrl> &sources);

    void setDestinationFileName(const QString &filename);
    void setDestination();

    void setWarning(const QString &warning);
    void setInformation(const QString &information);

    void dialogAccepted();

    void clear();

private:
    Ui::NewTransferWidget ui;
    QWidget *m_window;
    QTimer *m_timer;
    QList<QUrl> m_sources;

    // points to a folder if m_multiple otherwise to the destination
    QUrl m_destination;

    TransferHandler *m_existingTransfer;

    QBrush m_existingFileBackground;
    QBrush m_normalBackground;

    bool m_multiple;
    bool m_overWriteSingle;
};

class NewTransferDialogHandler : public QObject
{
    Q_OBJECT

public:
    explicit NewTransferDialogHandler(QObject *parent = nullptr);
    ~NewTransferDialogHandler() override;

    /**
     * @see showNewTransferDialog(QList<QUrl>)
     */
    static void showNewTransferDialog(const QUrl &url = QUrl());

    /**
     * This will show a dialog to the user to input needed information.
     * If the last url of the list is a local file or directory, then all files will
     * be downloaded to that destination.
     * If there are matching groups with default folders and the user set the option to
     * use those, then the affected urls will be downloaded without showing them in the dialog
     *
     * @note MainWindow will always be the parent widget
     */
    static void showNewTransferDialog(QList<QUrl> list);

private Q_SLOTS:
    void slotMostLocalUrlResult(KJob *job);

private:
    void handleUrls(const int jobId);
    void createDialog(const QList<QUrl> &urls, const QString &suggestedFileName);

private:
    struct UrlData {
        QList<QUrl> urls;
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
