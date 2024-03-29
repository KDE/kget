/***************************************************************************
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

#ifndef KGET_VERIFIER_TEST_H
#define KGET_VERIFIER_TEST_H

#include <QObject>
#include <QStringList>
#include <QUrl>

class QTemporaryDir;

class VerfierTest : public QObject
{
    Q_OBJECT

public:
    explicit VerfierTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testChecksum();
    void testChecksum_data();
    void testPartialChecksums();
    void testPartialChecksums_data();
    void testIsChecksum();
    void testIsChecksum_data();
    void testAvailableChecksum();
    void testAvailableChecksum_data();
    void testAvailablePartialChecksum();
    void testAvailablePartialChecksum_data();
    void testVerify();
    void testVerify_data();
    void testBrokenPieces();
    void testBrokenPieces_data();

private:
    /**
     * @returns (expected && Verifier::supportedVerficationTypes().contains(type))
     */
    bool expectedResult(bool expected, const QString &type);

private:
    QScopedPointer<QTemporaryDir> m_tempDir;
    QUrl m_file;
    const QStringList m_supported;
};

#endif
