/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef METALINKCREATOR_H
#define METALINKCREATOR_H

#include <KAssistantDialog>

#include "metalinker.h"
#include "filehandler.h"

#include "ui_introduction.h"
#include "ui_files.h"

namespace KGetMetalink
{
    class Metalink;
}

class CountryModel;
class GeneralWidget;
class LanguageModel;
class QDragEnterEvent;
class QShowEvent;
class QSortFilterProxyModel;
class QStandardItemModel;

class FileWidget : public QWidget
{
    Q_OBJECT

    public:
        FileWidget(QWidget *parent = 0);

    signals:
        void urlsDropped(const QList<KUrl> &files);

    protected:
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
};

class MetalinkCreator : public KAssistantDialog
{
    Q_OBJECT

    public:
        MetalinkCreator(QWidget *parent = 0);
        ~MetalinkCreator();

    public slots:
        /**
         * Adds m_tempFile to metalink and clears it for future reuse, also adds the
         * filename to the filemodel
         */
        void slotAddFile();

        void slotAddFile(const KGetMetalink::File &file);

        /**
         * This slot is used to update the filename in the filemodel if needed
         */
        void slotFileEdited(const QString &oldFileName, const QString &newFileName);

        /**
         * Handles the dropped files, calls dialogs if needed etc
         * @param types the types for which checksums should be calculated
         * @param createPartial whether partial checksums should be created or not
         */
        void slotHandleDropped(const QStringList &types, bool createPartial);

    private slots:
        /**
         * Creates the parts that take longer
         */
        void slotDelayedCreation();

        /**
         * Deactivates the Next/Finish-Button when the metalink is not valid i.e. data is missing
         */
        void slotUpdateAssistantButtons(KPageWidgetItem *to = 0, KPageWidgetItem *from = 0);

        void slotUpdateIntroductionNextButton();

        void slotUpdateFilesButtons();

        /**
         * Removes the selected files
         */
        void slotRemoveFile();

        /**
         * Shows the properties of a selected files, the properties can be edited
         */
        void slotFileProperties();

        /**
         * Shows a dialog where local files can be selected
         */
        void slotAddLocalFilesClicked();

        /**
         * Shows an empty file properties dialog to add a file
         */
        void slotAddClicked();

        /**
         * Saves a metalink to the destination specified in the gui, calls the other save-methods
         */
        void slotSave();

        void slotThreadFinished();//TODO description

        void slotOpenDragDlg();

    private:
        /**
         * Opens the dialog to enter data for a file
         * @param file the file that should be modifed
         * @param edit whether an existing file is being edited or a new one being added
         */
        void fileDlg(KGetMetalink::File *file, bool edit = false);

        /**
         * Creates the GUI parts and the needed models, calls the other create-methods
         */
        void create();

        /**
         * Loads a metalink of the destination specified in the gui, calls the other load-methods
         */
        void load();

        void createIntroduction();

        void createGeneral();
        void loadGeneral();
        void saveGeneral();

        void createFiles();
        void loadFiles();

    private:
        FileHandlerThread m_thread;
        DirectoryHandler *m_handler;
        KGetMetalink::Metalink metalink;
        int m_needUrlCount;

        KGetMetalink::File m_tempFile;
        KGetMetalink::Resources m_tempResources;
        KGetMetalink::CommonData m_tempCommonData;
        bool m_createPartial;

        QSortFilterProxyModel *m_countrySort;
        LanguageModel *m_languageModel;
        QSortFilterProxyModel *m_languageSort;

        Ui::Introduction uiIntroduction;
        KPageWidgetItem *m_introduction;

        GeneralWidget *m_general;
        KPageWidgetItem *m_generalPage;

        Ui::Files uiFiles;
        KPageWidgetItem *m_files;

        QStandardItemModel *m_filesModel;
};

#endif
