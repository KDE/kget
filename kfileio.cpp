/***************************************************************************
*                                kfileio.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*
*    email        : pch@freeshell.org
*
***************************************************************************/

// Author: Stefan Taferner <taferner@kde.org>

#include <qstring.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmainwidget.h>

#include "kfileio.h"

//-----------------------------------------------------------------------------
QString kFileToString(const QString & aFileName, bool aEnsureNL, bool aVerbose)
{
    QCString result;

    QFileInfo info(aFileName);
    unsigned int readLen;
    unsigned int len = info.size();
    QFile file(aFileName);

    // assert(aFileName!=NULL);
    if (aFileName == NULL)
        return "";

    if (!info.exists()) {
        if (aVerbose)
            KMessageBox::error(kmain, i18n("The specified file does not exist:\n%1").arg(aFileName));
        return QString::null;
    }
    if (info.isDir()) {
        if (aVerbose)
            KMessageBox::error(kmain, i18n("This is a directory and not a file:\n%1").arg(aFileName));
        return QString::null;
    }
    if (!info.isReadable()) {
        if (aVerbose)
            KMessageBox::error(kmain, i18n("You do not have read permissions to the file:\n%1").arg(aFileName));
        return QString::null;
    }
    if (len <= 0)
        return QString::null;

    if (!file.open(IO_Raw | IO_ReadOnly)) {
        if (aVerbose)
            switch (file.status()) {
            case IO_ReadError:
                KMessageBox::error(kmain, i18n("Could not read file:\n%1").arg(aFileName));
                break;
            case IO_OpenError:
                KMessageBox::error(kmain, i18n("Could not open file:\n%1").arg(aFileName));
                break;
            default:
                KMessageBox::error(kmain, i18n("Error while reading file:\n%1").arg(aFileName));
            }
        return QString::null;
    }

    result.resize(len + (int) aEnsureNL + 1);
    readLen = file.readBlock(result.data(), len);
    if (aEnsureNL && result[len - 1] != '\n') {
        result[len++] = '\n';
        readLen++;
    }
    result[len] = '\0';

    if (readLen < len) {
        QString msg = i18n("Could only read %1 bytes of %2.").arg(KGlobal::locale()->formatNumber(readLen,
                      0)).arg(KGlobal::locale()->formatNumber(len, 0));

        KMessageBox::error(kmain, msg);
        return QString::null;
    }

    kdDebug() << "kFileToString: " << readLen << " bytes read" << endl;
    return result;
}


//-----------------------------------------------------------------------------
static bool kBytesToFile(const char *aBuffer, int len, const QString & aFileName, bool aAskIfExists, bool aBackup, bool aVerbose)
{
    QFile file(aFileName);
    QFileInfo info(aFileName);
    int writeLen, rc;

    // assert(aFileName!=NULL);
    if (aFileName.isNull())
        return false;

    if (info.exists()) {
        if (aAskIfExists) {
            QString str = i18n("File %1 exists.\nDo you want to replace it?").arg(aFileName);

            rc = KMessageBox::questionYesNo(kmain, str);
            if (rc != KMessageBox::Yes)
                return FALSE;
        }
        if (aBackup) {
            // make a backup copy
            QString bakName = aFileName;

            bakName += '~';
            unlink(QFile::encodeName(bakName));
            rc = rename(QFile::encodeName(aFileName), QFile::encodeName(bakName));
            if (rc) {
                // failed to rename file
                if (!aVerbose)
                    return FALSE;
                rc = KMessageBox::warningYesNo(kmain, i18n("Failed to make a backup copy of %1.\nContinue anyway?").arg(aFileName));
                if (rc != KMessageBox::Yes)
                    return FALSE;
            }
        }
    }

    if (!file.open(IO_Raw | IO_WriteOnly)) {
        if (aVerbose)
            switch (file.status()) {
            case IO_WriteError:
                KMessageBox::error(kmain, i18n("Could not write to file:\n%1").arg(aFileName));
                break;
            case IO_OpenError:
                KMessageBox::error(kmain, i18n("Could not open file for writing:\n%1").arg(aFileName));
                break;
            default:
                KMessageBox::error(kmain, i18n("Error while writing file:\n%1").arg(aFileName));
            }
        return FALSE;
    }

    writeLen = file.writeBlock(aBuffer, len);

    if (writeLen < 0) {
        KMessageBox::error(kmain, i18n("Could not write to file:\n%1").arg(aFileName));
        return FALSE;
    } else if (writeLen < len) {
        QString msg = i18n("Could only write %1 bytes of %2.").arg(KGlobal::locale()->formatNumber(writeLen,
                      0)).arg(KGlobal::locale()->formatNumber(len,
                                                              0));

        KMessageBox::error(kmain, msg);
        return FALSE;
    }

    return TRUE;
}

bool kCStringToFile(const QCString & aBuffer, const QString & aFileName, bool aAskIfExists, bool aBackup, bool aVerbose)
{
    return kBytesToFile(aBuffer, aBuffer.length(), aFileName, aAskIfExists, aBackup, aVerbose);
}

bool kByteArrayToFile(const QByteArray & aBuffer, const QString & aFileName, bool aAskIfExists, bool aBackup, bool aVerbose)
{
    return kBytesToFile(aBuffer, aBuffer.size(), aFileName, aAskIfExists, aBackup, aVerbose);
}
