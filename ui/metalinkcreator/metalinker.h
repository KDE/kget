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


#ifndef Metalinker_H
#define Metalinker_H

#include <KIO/Job>
#include <KUrl>
#include <QDate>
#include <QDomElement>

#ifdef HAVE_NEPOMUK
namespace Nepomuk
{
    class Variant;
}
#endif //HAVE_NEPOMUK

/**
 * The following classes try to resemble the structure of a Metalink-document, they partially support
 * the Metalink specification version 3.0 2nd ed and Draft 09
 * It is possible to load and save metalinks and to edit them inbetween, you could also create a metalink
 * from scratch
 */

namespace KGetMetalink
{

class DateConstruct
{
    public:
        DateConstruct()
          : negativeOffset(false)
        {
        }

        void setData(const QDateTime &dateTime, const QTime &timeZoneOffset = QTime(), bool negativeOffset = false);
        void setData(const QString &dateConstruct);

        void clear();

        bool isNull() const;
        bool isValid() const;

        QString toString() const;

        QDateTime dateTime;
        QTime timeZoneOffset;
        bool negativeOffset;
};

/**
 * This class contains a url and the name, it can be used to e.g. describe a license
 */
class UrlText
{
    public:
        UrlText() {};

        bool isEmpty() const {return name.isEmpty() && url.isEmpty();}

        void clear();

        QString name;
        KUrl url;
};

/**
* Files, File and Metadata contain this
* Metadata not as member and only for compatibility
*/
class CommonData
{
    public:
        CommonData() {}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

#ifdef HAVE_NEPOMUK
        /**
         * Return Nepomuk-properties that can be extracted
         */
        QHash<QUrl, Nepomuk::Variant> properties() const;
#endif //HAVE_NEPOMUK

        QString identity;
        QString version;
        QString description;
        QString os;
        KUrl logo;
        QString language;
        UrlText publisher;
        QString copyright;
        UrlText license;
};

class Metaurl
{
    public:
        Metaurl() {}

        /**
         * "smaller" urls are less important than larger, larger urls should be preffered
         */
        bool operator<(const Metaurl &other) const {return (this->preference < other.preference);}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        bool isValid();

        void clear();

        QString type;

        /**
         * the preference of the urls, 100 is highest priority, 1 lowest
         * default is 0 as in not set
         */
        int preference;

        /**
         * Optional the name of a file that should be get of that metaurl
         */
        QString name;

        KUrl url;
};

class Url
{
    public:
        Url()
          : preference(0)
        {
        }

        /**
         * "smaller" urls are less important than larger, larger urls should be preffered
         */
        bool operator<(const Url &other) const;

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        bool isValid();

        void clear();

        /**
         * the preference of the urls, 100 is highest priority, 1 lowest
         * default is 0 as in not set
         */
        int preference;

        /**
         * the location of the server eg. "uk"
         */
        QString location;

        KUrl url;
};

class Resources
{
    public:
        Resources() {}

        bool isValid() const {return !urls.isEmpty() || !metaurls.isEmpty();}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        QList<Url> urls;
        QList<Metaurl> metaurls;
};

class Pieces
{
    public:
        Pieces()
          : length(0)
        {
        }

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        QString type;
        KIO::filesize_t length;
        QList<QString> hashes;
};

class Verification
{
    public:
        Verification() {}

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        QHash<QString, QString> hashes;
        QList<Pieces> pieces;
        QHash<QString, QString> signatures;
};

class File
{
    public:
        File()
          : size(0)
        {
        }

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        void clear();

        bool isValid() const;

#ifdef HAVE_NEPOMUK
        /**
         * Return Nepomuk-properties that can be extracted of file, only including data
         */
        QHash<QUrl, Nepomuk::Variant> properties() const;
#endif //HAVE_NEPOMUK

        QString name;
        Verification verification;
        KIO::filesize_t size;
        CommonData data;
        Resources resources;
};

class Files
{
    public:
        Files() {}

        bool isValid() const;

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

// #ifdef HAVE_NEPOMUK//TODO wha this now?
//         /**
//          * Return all Nepomuk-properties that can be extracted of Files
//          * @Note only Files is being looked at, not each File it contains, so
//          * you only get the general metadata for all Files
//         */
//         QHash<QUrl, Nepomuk::Variant> properties() const;
// #endif //HAVE_NEPOMUK

        void clear();

        QList<File> files;
};

class Metalink
{
    public:
        Metalink()
          : dynamic(false)
        {
        }

        /**
         * checks if the minimum requirements of a metalink are met
         * @return true if the minimum requirements are met
         */
        bool isValid() const;

        void load(const QDomElement &e);

        /**
         * Save the metalink
         * @return the QDomDocument containing the metalink
         */
        QDomDocument save() const;

        void clear();

        bool dynamic;
        QString xmlns; //the xmlns value is ignored when saving, instead the data format described in the specification is always used
        DateConstruct published; //when the metalink was published
        KUrl origin;
        QString generator;
        DateConstruct updated; //when the metalink was updated
        Files files;
};

/**
 * This class can handle the loading and saving of metalinks on the filesystem
 */
class HandleMetalink
{
    public:
        /**
         * Loads destination into metalink
         * @param destination the place of the metalink in the filesystem
         * @param metalink the instance of Metalink where the metalink will be stored
         * @return return true if it worked
         */
        static bool load(const KUrl &destination, Metalink *metalink);

        /**
         * Loads data into metalink
         * @param data the contents of a metalink
         * @param metalink the instance of Metalink where the metalink will be stored
         * @return return true if it worked
         */
        static bool load(const QByteArray &data, Metalink *metalink);

        /**
         * Saves metalink to destination
         * @param destination the place where the metlink will be saved
         * @param metalink the instance of metalink that will be written to the filesystem
         * @return return true if it worked
         */
        static bool save(const KUrl &destination, Metalink *metalink);

#ifdef HAVE_NEPOMUK
        /**
         * Convenience method to add Strings to the data
         */
        static void addProperty(QHash<QUrl, Nepomuk::Variant> *data, const QByteArray &uriBa, const QString &value);
#endif //HAVE_NEPOMUK

    private:
        static void parseMetealink_v3_ed2(const QDomElement &e, Metalink *metalink);
        static void paresFiles_v3_ed2(const QDomElement &e, Files *files);
        static void parseCommonData_v3_ed2(const QDomElement &e, KGetMetalink::CommonData *data);
        static void parseResources_v3_ed2(const QDomElement &e, KGetMetalink::Resources *resources);
        static void parseDateConstruct_v3_ed2(DateConstruct *dateConstruct, const QString &data);
};

}

#endif // Metalinker_H
