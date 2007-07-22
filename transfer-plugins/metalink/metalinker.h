/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef Metalinker_H
#define Metalinker_H

#include <KUrl>

class MlinkFileData
{
    public:
        MlinkFileData() {}
        QString fileName;
        QString md5;
        QString sha256;
        KUrl::List urls;
};


class Metalinker
{
    public:
        Metalinker();
        static QList<MlinkFileData> parseMetalinkFile(const QByteArray& data);
        static bool verifyMD5(QIODevice& file, const QString& md5);
};

#endif // Metalinker_H
