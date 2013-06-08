/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>            *
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
#include <QtCore/QEventLoop>
#include <QtCore/QString>
#include <QtCore/QObject>

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
 * This class contains a url and the name, it can be used to e.g. describe a publisher
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
        QList<QPair<QUrl, Nepomuk::Variant> > properties() const;
#endif //HAVE_NEPOMUK

        QString identity;
        QString version;
        QString description;
        QStringList oses;
        KUrl logo;
        QStringList languages;
        UrlText publisher;
        QString copyright;
};

class Metaurl
{
    public:
        Metaurl()
          : priority(0)
        {
        }

        /**
         * "smaller" urls are less important than larger, larger urls should be preffered
         */
        bool operator<(const Metaurl &other) const;

        void load(const QDomElement &e);
        void save(QDomElement &e) const;

        bool isValid();

        void clear();

        QString type;

        /**
         * the priority of the urls, 1 is highest priority, 999999 lowest
         * default is 0 as in not set and thus is ranked even behind 999999
         */
        uint priority;

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
          : priority(0)
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
         * the priority of the urls, 1 is highest priority, 999999 lowest
         * default is 0 as in not set and thus is ranked even behind 999999
         */
        uint priority;

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
        QStringList hashes;
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

        /**
         * Controls if the name attribute is valid, i.e. it is not empty and
         * does not contain any directory traversal directives or information,
         * as described in the Metalink 4.0 specification 4.1.2.1.
         */
        bool isValidNameAttribute() const;

#ifdef HAVE_NEPOMUK
        /**
         * Return Nepomuk-properties that can be extracted of file, only including data
         */
        QList<QPair<QUrl, Nepomuk::Variant> > properties() const;
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

        static const QString KGET_DESCRIPTION;
        static const uint MAX_URL_PRIORITY; //maximum pirority a Metalink 4.0 Url or Metaurl can have, not to be mixed up with the highest priority
};

/**
 * @class Metalink_v3
 * Metalink version 3.0 2nd ed
 * Used only for loading and saving, uses itself Metalink internally
 */
class Metalink_v3
{
    public:
        Metalink_v3();

        Metalink metalink();
        void setMetalink(const Metalink &metalink);

        void load(const QDomElement &e);

        /**
         * Save the metalink
         * @return the QDomDocument containing the metalink
         */
        QDomDocument save() const;

    private:
        void parseFiles(const QDomElement &e);
        Resources parseResources(const QDomElement &e);
        DateConstruct parseDateConstruct(const QString &data);
        CommonData parseCommonData(const QDomElement &e);

        /**
         * Inherits CommonData, the inheritor inherits every settings
         * from the ancestor it has not set itself
         */
        void inheritCommonData(const CommonData &ancestor, CommonData *inheritor);

        void saveFiles(QDomElement &e) const;
        void saveResources(const Resources &resources, QDomElement &e) const;
        void saveVerification(const Verification &verification, QDomElement &e) const;
        void saveCommonData(const CommonData &data, QDomElement &e) const;
        QString dateConstructToString(const DateConstruct &date) const;

    private:
        Metalink m_metalink;
        static const uint MAX_PREFERENCE;
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
         * @param destination the place where the metlink will be saved, the ending defines
         * what version should be used: *.meta4 --> Metalink 4.0; *.metalink --> Metalink 3.0
         * @param metalink the instance of metalink that will be written to the filesystem
         * @return return true if it worked
         */
        static bool save(const KUrl &destination, Metalink *metalink);

#ifdef HAVE_NEPOMUK
        /**
         * Convenience method to add Strings to the data
         */
        static void addProperty(QList<QPair<QUrl, Nepomuk::Variant> > *data, const QByteArray &uriBa, const QString &value);
        static void addProperty(QList<QPair<QUrl, Nepomuk::Variant> > *data, const QUrl &uri, const QString &value);
#endif //HAVE_NEPOMUK
};

class MetalinkHttpParser : public QObject
{
    Q_OBJECT
    public:
        MetalinkHttpParser(const KUrl& Url)
            : m_Url(Url), m_MetalinkHSatus(false) , m_EtagValue(QString(""))
        {
            checkMetalinkHttp();
        }

        ~MetalinkHttpParser();

        /**
         * @return true if m_Url is a metalink/http supported URL.
         */

        bool isMetalinkHttp();

        /**
         * @return the Url m_Url which is being tested for metalink
         */

        KUrl getUrl();
        QMultiMap<QString, QString>* getHeaderInfo();

        /**
         * @return Returns the ETag if present in the HTTP headers
         */

        QString* getEtag();

    private slots:
        void slotHeaderResult(KJob* kjob);
        void checkMetalinkHttp();
        void detectMime(KIO::Job *  job, const QString &  type);
        void slotRedirection(KIO::Job*, const KUrl&);


    private:
        KUrl m_Url;
        KUrl m_redirectionUrl;
        bool m_MetalinkHSatus;
        QEventLoop m_loop;
        QMultiMap<QString, QString> m_headerInfo;
        QString m_EtagValue ;

        /**
         * Parsees the Metalink values from QString to the Map
         * @param Value of the QString ie raw HTTP headers
         */
        void parseHeaders(const QString&);

        /**
         * Sets the status of m_MetalinkHStatus to true if the URL is a Metalink
         */
        void setMetalinkHSatus();

};

class httpLinkHeader : public Metaurl
{
    public:
    httpLinkHeader()
            : m_pref(false)
        {
        }

        QString m_reltype;
        bool m_pref;
        int m_depth;
        QString m_geo;

        /**
         * Loads information from a header value into metalink header structure.
         * @param Value of the "link" HTTP header response.
         */
        void headerBuilder(const QString &);

        bool operator<(const httpLinkHeader &) const;

};

}

#endif // Metalinker_H
