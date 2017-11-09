/**************************************************************************
*   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef URL_CHECKER_P_H
#define URL_CHECKER_P_H

#include <QDialog>

class QCheckBox;

/**
 * Asks if existing transfers should be overwritten
 */
class ExistingTransferDialog : public QDialog
{
    Q_OBJECT

    public:
        ExistingTransferDialog(const QString &text, const QString &caption, QWidget *parent = nullptr);
        
        enum ExistingDialogReturn {
            Cancel = 0,

            //old stuff is overwritten/deleted
            Yes,
            YesAll,

            //new stuff is not used
            No,
            NoAll
        };
        
        ExistingDialogReturn result();

    private slots:
        void slotYesClicked();
        void slotNoClicked();
        void slotCancelClicked();

    private:
        QCheckBox *m_applyAll;
        ExistingDialogReturn m_result;
};

#endif

