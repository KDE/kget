/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef KGET_URL_CHECKER_H
#define KGET_URL_CHECKER_H

#include "../kget_export.h"

#include <KUrl>

class Transfer;
class TransferHandler;

/**
 * This class provides static methods to check if urls are valid, and if not
 * provides enums to see what is not valid and provides translated error messages
 */
class KGET_EXPORT UrlChecker
{
    public:
        enum UrlType {
            Source,
            Destination,
            Folder
        };

        /**
         * Before using UrlChecker you have to specify a type
         * You can either do that in the constructor and later
         * via setType
         *
         * @see setType
         */
        explicit UrlChecker(UrlType type);
        ~UrlChecker();

        /**
         * Removes duplicates of a list of urls
         */
        static void removeDuplicates(KUrl::List &urls);

        //NOTE only first found error is reported, i.e. NotWriteable before ExistingFile if both are the case
        enum UrlError {
            //General
            NoError = 0,
            Empty,
            Invalid,

            //Source only
            NoProtocol,
            NoHost,

            //Destination and Folder only
            NotWriteable
        };

        enum UrlWarning {
            NoWarning = 0,
            //Source and Destination only
            ExistingFinishedTransfer,
            ExistingTransfer,

            //Destination only
            ExistingFile
        };

        /**
         * Folder:
         * Checks if the destination url points to is a folder that is writeable and existent.
         *
         * Destination:
         * Checks if url points to a file (can be non-existent) and if the
         * directory this file is (would) be in is existent and writeable
         *
         * @param type all types supported here
         * @param showNotification true shows a notification if an error is found
         * @note checkExisting is not done within this method
         * @note you can use the convenience methods checkSource, checkDestination, checkFolder
         * directly
         * @see checkExisting
         */
        static UrlError checkUrl(const KUrl &url, const UrlType type, bool showNotification = false);

        /**
         * Convenience method of checkUrl
         * @param showNotification true shows a notification if an error is found
         */
        static UrlError checkSource(const KUrl &source, bool showNotification = false);

        /**
         * Convenience method of checkUrl
         * @param showNotification true shows a notification if an error is found
         */
        static UrlError checkDestination(const KUrl &destination, bool showNotification = false);

        /**
         * Convenience method of checkUrl
         * @param showNotification true shows a notification if an error is found
         */
        static UrlError checkFolder(const KUrl &folder, bool showNotification = false);

        /**
         * Checks if source is local and exists already
         * @note If both dest and source are the same and local, then false will be returned
         * since it is assumed, that local files are either not handled by any
         * transfer plugin or are e.g. metalink or torrent files otherwise and thus
         * can have the same source/dest.
         * Also keep in mind that false will be returned if dest is being removed
         * at the moment. Avoid to ask a user twice in worst case.
         */
        static bool wouldOverwrite(const KUrl &source, const KUrl &dest);

        /**
         * Checks if there is an existing transfer for url with type
         * @param type *Source checks if there is a transfer with the same source
         * *Destination checks if there is a transfer with the same destination
         * @return if an existing transfer is found it will be returned,
         * otherwise 0 will be returned
         * @note checkUrl check is not done and UrlType Folder is not supported
         * Keep in mind, that the same transfers could be found via Source and Destination!
         * @see checkUrl
         */
        static TransferHandler *existingTransfer(const KUrl &url, const UrlType type, UrlWarning *warning = 0);

        /**
         * @note UrlType folder is not supported, the result then is undefined!
         */
        static KUrl::List hasExistingTransferMessages(const KUrl::List &urls, const UrlType type);

        /**
         * Get a describing message for UrlError
         * @param url is only needed here to include it in some error messages,
         * if url is empty, then it won't be used.
         * @note this method does no checks, it only creates messages based on the error code
         * @see checkUrl
         */
        static QString message(const KUrl &url, const UrlType type, const UrlError error);

        /**
         * Get a describing message for UrlWarning
         * @param url is only needed here to include it in some error messages,
         * if url is empty, then it won't be used.
         * @note this method does no checks, it only creates messages based on the warning code
         * @see existingTransfer fileExists
         */
        static QString message(const KUrl &url, const UrlType type, const UrlWarning warning);

        /**
         * Convenience method for multiple urls (urls can be empty)
         * @see message
         */
        static QString message(const KUrl::List &urls, const UrlType type, const UrlError error);

        /**
         * Convenience method for multiple urls (urls can be empty)
         * @see message
         */
        static QString message(const KUrl::List &urls, const UrlType type, const UrlWarning warning);

        /**
         * Takes an url to a source and an url to either the destination or a folder
         *  and returns a destination url.
         *
         * @param destOrFolder *is an existing folder then a file name derived from source
         * will be appended to it and returned
         * *otherwise no modification will be done and destOrFolder will be returned
         * @note no checkUrl check happens!
         * @see checkUrl
         */
        static KUrl destUrl(const KUrl &destOrFolder, const KUrl &source, const QString &fileName = QString());

        ///Non static methods following

        UrlType type() const;

        /**
         * Sets the type for the following operations to type
         * @note calls clear
         * @see clear
         */
        void setType(UrlType type);

        /**
         * Clears all data, like correctUrls, errorUrls, responses from checkExistingFile ...
         */
        void clear();

        /**
         * Checks url of type()
         * @return UrlError, None if no error has been found
         * @note checkExisting is not done within this method
         * @note both this method and checkUrls() will store all correct
         * urls in correctUrls() and the others in errorUrls() and
         * splitErrorUrls()
         */
        UrlError addUrl(const KUrl &url);

        /**
         * Does checkUrl for a list of urls.
         * @return true if all urls are valid, false, if at least one url is invalid
         * @note checkExisting is not done within this method
         * @note both this method and checkUrl() will store all correct
         * urls in correctUrls() and the others in errorUrls() and
         * splitErrorUrls()
         * @see addUrl
         */
        bool addUrls(const KUrl::List &urls);

        /**
         * Displays error messages for the collected urls if any are needed
         */
        void displayErrorMessages();

        /**
         * Returns the correct urls collected with the last call to urlCollectErrors
         * @see urlCollectErrors
         */
        KUrl::List correctUrls() const;

        /**
         * Checks all correctUrls() if there are existing Transfers
         * for the specified UrlType and asks the user how to proceed.
         *
         * After this urls where the user decided that they should not be downloaded
         * won't be in correctUrls anymore.
         * @note UrlType folder is not supported, the result then is undefined!
         * @return urls to download, after user interaction
         * @see correctUrls errorUrls splitErrorUrls
         */
        void existingTransfers();

        /**
         * Checks if the file at destination exists already, source needs to be defined
         * to have a nice dialog and to make sure that both source and destination aren't
         * the same
         * @note This method does _not_ affect other methods like correctUrls or existingTransfers
         * etc., though responses like overwrite all files are stored in case you call this method
         * again you can clear that with clear.
         * @return returns the destination to download the file to, if empty then the file
         * should not be downloaded
         * @see clear
         */
        KUrl checkExistingFile(const KUrl &source, const KUrl &destination);

        /**
         * Returns all wrong urls 
         * @note the oder of the urls is not guaranteed to be the same as it initially was
         * @see urlCollectErrors splitErrorUrls existingTransfers
         */
        KUrl::List errorUrls()const;

        /**
         * Returns all wrong urls collected with the last call to urlCollectErrors
         * or existingTransfers categorized by their errors
         * @note urls without an error (UrlError == None) are not included
         * @see urlCollectErrors correctUrls errorUrls existingTransfers
         */
        QHash<UrlError, KUrl::List> splitErrorUrls() const;

    private:
        static TransferHandler *existingSource(const KUrl &url, UrlWarning &warning);
        static TransferHandler *existingDestination(const KUrl &url, UrlWarning &warning);
        static int hasExistingDialog(const KUrl &url, const UrlChecker::UrlType type, const UrlWarning warning);//TODO description --> returncode etc.!
        static void removeTransfers(const QList<TransferHandler*> &toRemove);

        enum ExistingDialogReturn {
            Cancel = 0,

            //old stuff is overwritten/deleted
            Yes,
            YesAll,

            //new stuff is not used
            No,
            NoAll
        };

    private:
        UrlType m_type;
        KUrl::List m_correctUrls;
        QHash<UrlError, KUrl::List> m_splitErrorUrls;

        QHash<UrlWarning, QPair<KUrl, Transfer*> > m_existingTransfers;
        KUrl::List m_nonExistingUrls;

        //Existing files stuff
        bool m_cancle;
        bool m_overwriteAll;
        bool m_autoRenameAll;
        bool m_skipAll;
};


#endif
