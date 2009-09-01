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

#include "metalinker.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtXml/QDomElement>

#include <KLocale>
#include <KSystemTimeZone>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Variant>
#endif //HAVE_NEPOMUK
#include <KDebug>

namespace KGetMetalink
{
    QString addaptHashType(const QString &type, bool loaded);
}

/**
 * Adapts type to the way the hash is internally stored
 * @param type the hash-type
 * @param load true if the hash has been loaded, false if it should be saved
 * @note metalink wants sha1 in the form "sha-1", though
 * the metalinker uses it internally in the form "sha1", this function
 * transforms it to the correct form, it is only needed internally
*/
QString KGetMetalink::addaptHashType(const QString &type, bool loaded)
{
    QString t = type;
    if (loaded)
    {
        t.replace("sha-", "sha");
    }
    else
    {
        t.replace("sha", "sha-");
    }

    return t;
}

void KGetMetalink::DateConstruct::setData(const QDateTime &dateT, const QTime &timeZoneOff)
{
    dateTime = dateT;
    timeZoneOffset = timeZoneOff;
}

void KGetMetalink::DateConstruct::setData(const QString &dateConstruct)
{
    if (dateConstruct.isEmpty())
    {
        return;
    }

    QString temp = dateConstruct;
    QString exp = "yyyy-MM-ddThh:mm:ss";

    int length = exp.length();

    dateTime = QDateTime::fromString(temp.left(length), exp);
    if (dateTime.isValid())
    {
        int index = dateConstruct.indexOf('+');
        if (index > -1)
        {
            timeZoneOffset = QTime::fromString(temp.mid(index + 1));
        }
        else
        {
            index = dateConstruct.indexOf('-');
            if (index > -1)
            {
                negativeOffset = true;
                timeZoneOffset = QTime::fromString(temp.left(index + 1));
            }
        }
    }


    //NOTE Metalink 3.0 2nd ed compatibility
    //Date according to RFC 822, the year with four characters preferred
    //e.g.: "Mon, 15 May 2006 00:00:01 GMT", "Fri, 01 Apr 2009 00:00:01 +1030"
    if (!dateTime.isValid())
    {
        const QString weekdayExp = "ddd, ";
        bool weekdayIncluded = (temp.at(3) == ',');
        int startPosition = (weekdayIncluded ? weekdayExp.length() : 0);
        const QString dayMonthExp = "dd MMM ";
        const QString yearExp = "yy";

        exp = dayMonthExp + yearExp + yearExp;
        length = exp.length();

//BUG why does ddd that not work?? --> qt bug?
        QDate date = QDate::fromString(temp.mid(startPosition, length), exp);
        if (!date.isValid())
        {
            exp = dayMonthExp + yearExp;
            length = exp.length();
            date = QDate::fromString(temp.mid(startPosition, length), exp);
            if (!date.isValid())
            {
                return;
            }
        }

        dateTime.setDate(date);
        temp = temp.mid(startPosition);
        temp = temp.mid(length + 1);//also remove the space

        const QString hourExp = "hh";
        const QString minuteExp = "mm";
        const QString secondExp = "ss";

        exp = hourExp + ':' + minuteExp + ':' + secondExp;
        length = exp.length();
kDebug() << "***exp: " << exp;
kDebug() << "****: " <<      temp.left(length);
        QTime time = QTime::fromString(temp.left(length), exp);
        if (!time.isValid())
        {
            exp = hourExp + ':' + minuteExp;
            length = exp.length();
            time = QTime::fromString(temp.left(length), exp);
            if (!time.isValid())
            {
                return;
            }
        }

        dateTime.setTime(time);
        kDebug() << "***bevor: " << temp;
        temp = temp.mid(length + 1);//also remove the space
kDebug() << "***temp: " << temp;
        //e.g. GMT
        if (temp.length() == 3)
        {
            KTimeZone timeZone = KSystemTimeZones::readZone(temp);
            if (timeZone.isValid())
            {
                kDebug() << "***: " << timeZone.countryCode();
                kDebug() << "***timezone valid";
                int offset = timeZone.currentOffset();
                kDebug() << "***offest: " << offset;
                negativeOffset = (offset < 0);
                timeZoneOffset = QTime(0, 0, 0);
                timeZoneOffset = timeZoneOffset.addSecs(abs(offset));
            }
        }
    }
}

bool KGetMetalink::DateConstruct::isNull() const
{
    return dateTime.isNull();
}

bool KGetMetalink::DateConstruct::isValid() const
{
    return dateTime.isValid();
}

QString KGetMetalink::DateConstruct::toString() const
{
    QString string;

    if (dateTime.isValid())
    {
        string += dateTime.toString(Qt::ISODate);
    }

    if (timeZoneOffset.isValid())
    {
        string += (negativeOffset ? '-' : '+');
        string += timeZoneOffset.toString("hh:mm");
    }
    else if (!string.isEmpty())
    {
        string += 'Z';
    }

    return string;
}

void KGetMetalink::DateConstruct::clear()
{
    dateTime = QDateTime();
    timeZoneOffset = QTime();
}

void KGetMetalink::UrlText::clear()
{
    name.clear();
    url.clear();
}

void KGetMetalink::CommonData::load(const QDomElement &e)
{
    identity = e.firstChildElement("identity").text();
    version = e.firstChildElement("version").text();
    description = e.firstChildElement("description").text();
    os = e.firstChildElement("os").text();
    logo = KUrl(e.firstChildElement("log").text());
    language = e.firstChildElement("language").text();
    copyright = e.firstChildElement("copyright").text();

    const QDomElement publisherElem = e.firstChildElement("publisher");
    publisher.name = publisherElem.attribute("name");
    publisher.url = KUrl(publisherElem.firstChildElement("url").text());
    //backward compatibility to Metalink 3.0 Second Edition
    if (publisher.name.isEmpty())
    {
        publisher.name = publisherElem.firstChildElement("name").text();
    }
    if (publisher.url.isEmpty())
    {
        publisher.url = KUrl(publisherElem.firstChildElement("url").text());
    }

    const QDomElement lincenseElem = e.firstChildElement("license");
    license.name = lincenseElem.attribute("name");
    license.url = KUrl(lincenseElem.firstChildElement("url").text());
    //backward compatibility to Metalink 3.0 Second Edition
    if (license.name.isEmpty())
    {
        license.name = lincenseElem.firstChildElement("name").text();
    }
    if (license.url.isEmpty())
    {
        license.url = KUrl(lincenseElem.firstChildElement("url").text());
    }
}

void KGetMetalink::CommonData::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();

    if (!copyright.isEmpty())
    {
        QDomElement elem = doc.createElement("copyright");
        QDomText text = doc.createTextNode(copyright);
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!description.isEmpty())
    {
        QDomElement elem = doc.createElement("description");
        QDomText text = doc.createTextNode(description);
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!identity.isEmpty())
    {
        QDomElement elem = doc.createElement("identity");
        QDomText text = doc.createTextNode(identity);
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!language.isEmpty())
    {
        QDomElement elem = doc.createElement("language");
        QDomText text = doc.createTextNode(language);
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!license.isEmpty())
    {
        QDomElement elem = doc.createElement("license");

        QDomElement url = doc.createElement("url");
        QDomText text = doc.createTextNode(license.url.url());
        url.appendChild(text);
        elem.appendChild(url);

        QDomElement name = doc.createElement("name");
        text = doc.createTextNode(license.name);
        name.appendChild(text);
        elem.appendChild(name);

        e.appendChild(elem);
    }
    if (!logo.isEmpty())
    {
        QDomElement elem = doc.createElement("logo");
        QDomText text = doc.createTextNode(logo.url());
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!os.isEmpty())
    {
        QDomElement elem = doc.createElement("os");
        QDomText text = doc.createTextNode(os);
        elem.appendChild(text);
        e.appendChild(elem);
    }
    if (!publisher.isEmpty())
    {
        QDomElement elem = doc.createElement("publisher");

        QDomElement url = doc.createElement("url");
        QDomText text = doc.createTextNode(publisher.url.url());
        url.appendChild(text);
        elem.appendChild(url);

        QDomElement name = doc.createElement("name");
        text = doc.createTextNode(publisher.name);
        name.appendChild(text);
        elem.appendChild(name);

        e.appendChild(elem);
    }
    if (!version.isEmpty())
    {
        QDomElement elem = doc.createElement("version");
        QDomText text = doc.createTextNode(version);
        elem.appendChild(text);
        e.appendChild(elem);
    }
}

void KGetMetalink::CommonData::clear()
{
    identity.clear();
    version.clear();
    description.clear();
    os.clear();
    logo.clear();
    language.clear();
    publisher.clear();
    copyright.clear();
    license.clear();
}

#ifdef HAVE_NEPOMUK
QHash<QUrl, Nepomuk::Variant> KGetMetalink::CommonData::properties() const
{
    //TODO what to do with identity?
    //TODO what uri for logo?
    //TODO what uri for publisher-url?
    //TODO what uri for license-url?
    QHash<QUrl, Nepomuk::Variant> data;

    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/2007/01/19/nie/#version", version);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/2007/01/19/nie/#description", description);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo/#OperatingSystem", os);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/nie/#language", language);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/2007/03/22/nco/#publisher", publisher.name);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/nie/#copyright", copyright);
    HandleMetalink::addProperty(&data, "http://www.semanticdesktop.org/ontologies/nie/#licenseType", license.name);

    return data;
}
#endif //HAVE_NEPOMUK

void KGetMetalink::Metaurl::load(const QDomElement &e)
{
    type = e.attribute("type");
    preference = e.attribute("preference").toInt();
    name = e.attribute("name");
    url = KUrl(e.text());

    //NOTE Metalink 3.0 2nd ed compatibility
    if (type.isEmpty())
    {
        if (url.fileName().endsWith(QLatin1String(".torrent")))
        {
            type = "torrent";
        }
    }
}

void KGetMetalink::Metaurl::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement metaurl = doc.createElement("metaurl");
    if (preference)
    {
        metaurl.setAttribute("preference", preference);
    }
    if (!name.isEmpty())
    {
        metaurl.setAttribute("name", name);
    }
    metaurl.setAttribute("type", type);

    QDomText text = doc.createTextNode(url.url());
    metaurl.appendChild(text);

    e.appendChild(metaurl);
}

bool KGetMetalink::Metaurl::isValid()
{
    return url.isValid() && !type.isEmpty();
}

void KGetMetalink::Metaurl::clear()
{
    type.clear();
    preference = 0;
    name.clear();
    url.clear();
}

void KGetMetalink::Url::load(const QDomElement &e)
{
    location = e.attribute("location");
    preference = e.attribute("preference").toInt();
    url = KUrl(e.text());
}

void KGetMetalink::Url::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement elem = doc.createElement("url");
    if (preference)
    {
        elem.setAttribute("preference", preference);
    }
    if (!location.isEmpty())
    {
        elem.setAttribute("location", location);
    }

    QDomText text = doc.createTextNode(url.url());
    elem.appendChild(text);

    e.appendChild(elem);
}

bool KGetMetalink::Url::isValid()
{
    bool valid = url.isValid();
    if (url.fileName().endsWith(QLatin1String(".torrent")))
    {
        valid = false;
    }
    else if (url.fileName().endsWith(QLatin1String(".metalink")))
    {
        valid = false;
    }

    return valid;
}

void KGetMetalink::Url::clear()
{
    preference = 0;
    location.clear();
    url.clear();
}

void KGetMetalink::Resources::load(const QDomElement &e)
{
    QDomElement res = e.firstChildElement("resources");

    for (QDomElement elem = res.firstChildElement("url"); !elem.isNull(); elem = elem.nextSiblingElement("url"))
    {
        Url url;
        url.load(elem);
        if (url.isValid())
        {
            urls.append(url);
        }
        //NOTE Metalink 3.0 2nd ed compatibility
        else
        {
            //it might be a metaurl
            Metaurl metaurl;
            metaurl.load(elem);
            if (metaurl.isValid())
            {
                metaurls.append(metaurl);
            }
        }
    }

    for (QDomElement elem = res.firstChildElement("metaurl"); !elem.isNull(); elem = elem.nextSiblingElement("metaurl"))
    {
        Metaurl metaurl;
        metaurl.load(elem);
        if (metaurl.isValid())
        {
            metaurls.append(metaurl);
        }
    }
}

void KGetMetalink::Resources::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement resources = doc.createElement("resources");

    foreach (const Metaurl &metaurl, metaurls)
    {
        metaurl.save(resources);
    }

    foreach (const Url &url, urls)
    {
        url.save(resources);
    }

    e.appendChild(resources);
}

void KGetMetalink::Resources::clear()
{
    urls.clear();
    metaurls.clear();
}

void KGetMetalink::Pieces::load(const QDomElement &e)
{
    type = addaptHashType(e.attribute("type"), true);
    length = e.attribute("length").toULongLong();

    QDomNodeList hashesList = e.elementsByTagName("hash");

    for (int i = 0; i < hashesList.count(); ++i)//TODO!!
    {
        QDomElement element = hashesList.at(i).toElement();
        hashes.append(element.text());
    }
}

void KGetMetalink::Pieces::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement pieces = doc.createElement("pieces");
    pieces.setAttribute("type", addaptHashType(type, false));
    pieces.setAttribute("length", length);

    for (int i = 0; i < hashes.size(); ++i)
    {
        QDomElement hash = doc.createElement("hash");
        QDomText text = doc.createTextNode(hashes.at(i));
        hash.appendChild(text);
        pieces.appendChild(hash);
    }

    e.appendChild(pieces);
}

void KGetMetalink::Pieces::clear()
{
    type.clear();
    length = 0;
    hashes.clear();
}

void KGetMetalink::Verification::load(const QDomElement &e)
{
    QDomElement veriE = e.firstChildElement("verification");

    for (QDomElement elem = veriE.firstChildElement("hash"); !elem.isNull(); elem = elem.nextSiblingElement("hash"))
    {
        QString type = elem.attribute("type");
        QString hash = elem.text();
        if (!type.isEmpty() && !hash.isEmpty())
        {
            type = addaptHashType(type, true);
            hashes[type] = hash;
        }
    }

    for (QDomElement elem = veriE.firstChildElement("pieces"); !elem.isNull(); elem = elem.nextSiblingElement("pieces"))
    {
        Pieces piecesItem;
        piecesItem.load(elem);
        pieces.append(piecesItem);
    }
}

void KGetMetalink::Verification::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement verification = doc.createElement("verification");

    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = hashes.constEnd();
    for (it = hashes.constBegin(); it != itEnd; ++it)
    {
        QDomElement hash = doc.createElement("hash");
        hash.setAttribute("type", addaptHashType(it.key(), false));
        QDomText text = doc.createTextNode(it.value());
        hash.appendChild(text);
        verification.appendChild(hash);
    }

    foreach (const Pieces &item, pieces)
    {
        item.save(verification);
    }

    e.appendChild(verification);
}

void KGetMetalink::Verification::clear()
{
    hashes.clear();
    pieces.clear();
}

bool KGetMetalink::File::isValid() const
{
    return !name.isEmpty() && resources.isValid();
}

void KGetMetalink::File::load(const QDomElement &e)
{
    data.load(e);

    name = e.attribute("name");
    size = e.firstChildElement("size").text().toULongLong();

    verification.load(e);
    resources.load(e);
}

void KGetMetalink::File::save(QDomElement &e) const
{
    if (isValid())
    {
        QDomDocument doc = e.ownerDocument();
        QDomElement file = doc.createElement("file");
        file.setAttribute("name", name);

        resources.save(file);

        data.save(file);

        if (size)
        {
            QDomElement elem = doc.createElement("size");
            QDomText text = doc.createTextNode(QString::number(size));
            elem.appendChild(text);
            file.appendChild(elem);
        }

        verification.save(file);

        e.appendChild(file);
    }
}

void KGetMetalink::File::clear()
{
    name.clear();
    verification.clear();
    size = 0;
    data.clear();
    resources.clear();
}

#ifdef HAVE_NEPOMUK
QHash<QUrl, Nepomuk::Variant> KGetMetalink::File::properties() const
{
    return data.properties();
}
#endif //HAVE_NEPOMUK

bool KGetMetalink::Files::isValid() const
{
    bool isValid = !files.empty();
    foreach (const File &file, files)
    {
        isValid &= file.isValid();
    }

    return isValid;
}

void KGetMetalink::Files::load(const QDomElement &e)
{
    const QDomElement filesElem = e.firstChildElement("files");

    data.load(filesElem);

    for (QDomElement elem = filesElem.firstChildElement("file"); !elem.isNull(); elem = elem.nextSiblingElement("file"))
    {
        File file;
        file.load(elem);
        files.append(file);
    }
}

void KGetMetalink::Files::save(QDomElement &e) const
{
    if (e.isNull())
    {
        return;
    }

    QDomDocument doc(e.ownerDocument());
    QDomElement filesE = doc.createElement("files");

    foreach (const File &file, files)
    {
        file.save(filesE);
    }

    data.save(filesE);

    e.appendChild(filesE);
}

void KGetMetalink::Files::clear()
{
    data.clear();
    files.clear();
}

bool KGetMetalink::Metalink::isValid() const
{
    return files.isValid();
}

#ifdef HAVE_NEPOMUK
QHash<QUrl, Nepomuk::Variant> KGetMetalink::Files::properties() const
{
    return data.properties();
}
#endif //HAVE_NEPOMUK

void KGetMetalink::Metalink::load(const QDomElement &e)
{
    QDomDocument doc = e.ownerDocument();
    const QDomElement metalink = doc.firstChildElement("metalink");

    //NOTE Metalink 3.0 2nd ed compatibility
    if (metalink.hasAttribute("type"))
    {
        dynamic = (metalink.attribute("type") == "dynamic");
    }
    else
    {
        dynamic = (metalink.firstChildElement("dynamic").text() == "true");
    }
    xmlns = metalink.attribute("xmlns");
    origin = KUrl(metalink.firstChildElement("origin").text());
    generator = metalink.firstChildElement("generator").text();
    updated.setData(metalink.firstChildElement("updated").text());

    //when the metalink was published
    QString publishedText = metalink.firstChildElement("published").text();
    //NOTE Metalink 3.0 2nd ed compatibility
    if (publishedText.isEmpty())
    {
        publishedText = metalink.attribute("pubdate");
    }
    published.setData(publishedText);

    //when the metalink was updated
    QString updatedText = metalink.firstChildElement("updated").text();
    //NOTE Metalink 3.0 2nd ed compatibility
    if (updatedText.isEmpty())
    {
        updatedText = metalink.attribute("refreshdate");
    }
    updated.setData(updatedText);


    files.load(e);

    //NOTE Metalink 3.0 2nd ed compatibility that is unclear in this regard
    CommonData data;
    data.load(metalink);
    if (files.data.identity.isEmpty())
    {
        files.data.identity = data.identity;
    }
    if (files.data.version.isEmpty())
    {
        files.data.version = data.version;
    }
    if (files.data.description.isEmpty())
    {
        files.data.description = data.description;
    }
    if (files.data.os.isEmpty())
    {
        files.data.os = data.os;
    }
    if (files.data.logo.isEmpty())
    {
        files.data.logo = data.logo;
    }
    if (files.data.language.isEmpty())
    {
        files.data.language = data.language;
    }
    if (files.data.publisher.isEmpty())
    {
        files.data.publisher = data.publisher;
    }
    if (files.data.copyright.isEmpty())
    {
        files.data.copyright = data.copyright;
    }
    if (files.data.license.isEmpty())
    {
        files.data.license = data.license;
    }
}

QDomDocument KGetMetalink::Metalink::save() const
{
    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(header);

    QDomElement metalink = doc.createElement("metalink");
    metalink.setAttribute("xmlns", "urn:ietf:params:xml:ns:metalink"); //the xmlns value is ignored, instead the data format described in the specification is always used

    files.save(metalink);

    if (!generator.isEmpty())
    {
        QDomElement elem = doc.createElement("generator");
        QDomText text = doc.createTextNode(generator);
        elem.appendChild(text);
        metalink.appendChild(elem);
    }
    if (!origin.isEmpty())
    {
        QDomElement elem = doc.createElement("origin");
        QDomText text = doc.createTextNode(origin.url());
        elem.appendChild(text);
        metalink.appendChild(elem);
    }
    if (published.isValid())
    {
        QDomElement elem = doc.createElement("published");
        QDomText text = doc.createTextNode(published.toString());
        elem.appendChild(text);
        metalink.appendChild(elem);
    }
    if (dynamic)
    {
        QDomElement elem = doc.createElement("dynamic");
        QDomText text = doc.createTextNode("true");
        elem.appendChild(text);
        metalink.appendChild(elem);
    }
    if (updated.isValid())
    {
        QDomElement elem = doc.createElement("updated");
        QDomText text = doc.createTextNode(updated.toString());
        elem.appendChild(text);
        metalink.appendChild(elem);
    }

    doc.appendChild(metalink);

    return doc;
}

void KGetMetalink::Metalink::clear()
{
    dynamic = false;
    xmlns.clear();
    published.clear();
    origin.clear();
    generator.clear();
    updated.clear();
    files.clear();
}

bool KGetMetalink::HandleMetalink::load(const KUrl &destination, KGetMetalink::Metalink *metalink)
{
    QFile file(destination.pathOrUrl());
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return false;
    }

    QDomElement root = doc.documentElement();
    metalink->load(root);
    file.close();

    return true;
}

bool KGetMetalink::HandleMetalink::load(const QByteArray &data, KGetMetalink::Metalink *metalink)
{
    if (data.isNull())
    {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(data))
    {
        return false;
    }

    QDomElement root = doc.documentElement();
    metalink->load(root);

    return true;
}

bool KGetMetalink::HandleMetalink::save(const KUrl &destination, KGetMetalink::Metalink *metalink)
{
    QFile file(destination.pathOrUrl());
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDomDocument doc = metalink->save();

    QTextStream stream(&file);
    doc.save(stream, 2);
    file.close();

    return true;
}

#ifdef HAVE_NEPOMUK
void KGetMetalink::HandleMetalink::addProperty(QHash<QUrl, Nepomuk::Variant> *data, const QByteArray &uriBa, const QString &value)
{
    if (data && !uriBa.isEmpty() && !value.isEmpty())
    {
        const QUrl uri = QUrl::fromEncoded(uriBa, QUrl::StrictMode);
        (*data)[uri] = Nepomuk::Variant(value);
    }
}
#endif //HAVE_NEPOMUK
