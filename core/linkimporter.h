/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday @ gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef LINKIMPORTER_H
#define LINKIMPORTER_H

#include "kget_export.h"

#include <QList>
#include <QThread>

#include <QUrl>

class KLocalizedString;

/**
 * Import a list of urls from a file (local or remote)
 */
class KGET_EXPORT LinkImporter : public QThread
{
    Q_OBJECT
public:
    LinkImporter(const QUrl &source, QObject *parent);
    LinkImporter(QObject *parent);
    ~LinkImporter() override;

    /**
     * Check for urls in clipboard
     */
    void checkClipboard(const QString &clipboardContent);

    /**
     * Start reading the url contents
     */
    void run() override;

    /**
     * copy the remote file out of the thread
     */
    void copyRemoteFile();

    /**
     * Returns a list with the links of the selected url m_url
     */
    QList<QString> links()
    {
        return m_transfers;
    };

Q_SIGNALS:
    void error(const KLocalizedString &);
    void progress(int progress);

private Q_SLOTS:
    void slotReadFile(const QUrl &url);

private:
    /**
     * Checks if an url is valid and adds it to the transfers lists
     */
    void addTransfer(QString &link);

private:
    QUrl m_url;
    QList<QString> m_transfers;
    QString m_tempFile;
};
#endif
