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

#include <KDebug>
#include <KLocale>
#include <KSystemTimeZone>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Variant>
#endif //HAVE_NEPOMUK

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

void KGetMetalink::DateConstruct::setData(const QDateTime &dateT, const QTime &timeZoneOff, bool negOff)
{
    dateTime = dateT;
    timeZoneOffset = timeZoneOff;
    negativeOffset = negOff;
}

void KGetMetalink::DateConstruct::setData(const QString &dateConstruct)
{
    if (dateConstruct.isEmpty())
    {
        return;
    }

    const QString exp = "yyyy-MM-ddThh:mm:ss";
    const int length = exp.length();

    dateTime = QDateTime::fromString(dateConstruct.left(length), exp);
    if (dateTime.isValid())
    {
        int index = dateConstruct.indexOf('+', length - 1);
        if (index > -1)
        {
            timeZoneOffset = QTime::fromString(dateConstruct.mid(index + 1), "hh:mm");
        }
        else
        {
            index = dateConstruct.indexOf('-', length - 1);
            if (index > -1)
            {
                negativeOffset = true;
                timeZoneOffset = QTime::fromString(dateConstruct.mid(index + 1), "hh:mm");
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
    logo = KUrl(e.firstChildElement("logo").text());
    language = e.firstChildElement("language").text();
    copyright = e.firstChildElement("copyright").text();

    const QDomElement publisherElem = e.firstChildElement("publisher");
    publisher.name = publisherElem.attribute("name");
    publisher.url = KUrl(publisherElem.attribute("url"));

    const QDomElement lincenseElem = e.firstChildElement("license");
    license.name = lincenseElem.attribute("name");
    license.url = KUrl(lincenseElem.attribute("url"));
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

        elem.setAttribute("url", license.url.url());
        elem.setAttribute("name", license.name);

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
        elem.setAttribute("url", publisher.url.url());
        elem.setAttribute("name", publisher.name);

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
    for (QDomElement elem = e.firstChildElement("url"); !elem.isNull(); elem = elem.nextSiblingElement("url"))
    {
        Url url;
        url.load(elem);
        if (url.isValid())
        {
            urls.append(url);
        }
    }

    for (QDomElement elem = e.firstChildElement("metaurl"); !elem.isNull(); elem = elem.nextSiblingElement("metaurl"))
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
    foreach (const Metaurl &metaurl, metaurls)
    {
        metaurl.save(e);
    }

    foreach (const Url &url, urls)
    {
        url.save(e);
    }
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

    for (int i = 0; i < hashesList.count(); ++i)
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
    for (QDomElement elem = e.firstChildElement("hash"); !elem.isNull(); elem = elem.nextSiblingElement("hash")) {
        QString type = elem.attribute("type");
        const QString hash = elem.text();
        if (!type.isEmpty() && !hash.isEmpty()) {
            type = addaptHashType(type, true);
            hashes[type] = hash;
        }
    }

    for (QDomElement elem = e.firstChildElement("pieces"); !elem.isNull(); elem = elem.nextSiblingElement("pieces")) {
        Pieces piecesItem;
        piecesItem.load(elem);
        pieces.append(piecesItem);
    }

    for (QDomElement elem = e.firstChildElement("signature"); !elem.isNull(); elem = elem.nextSiblingElement("signature")) {
        const QString type = elem.attribute("type");
        const QString siganture = elem.text();
        if (!type.isEmpty() && !siganture.isEmpty()) {
            signatures[type] = siganture;
        }
    }
}

void KGetMetalink::Verification::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();

    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = hashes.constEnd();
    for (it = hashes.constBegin(); it != itEnd; ++it) {
        QDomElement hash = doc.createElement("hash");
        hash.setAttribute("type", addaptHashType(it.key(), false));
        QDomText text = doc.createTextNode(it.value());
        hash.appendChild(text);
        e.appendChild(hash);
    }

    foreach (const Pieces &item, pieces) {
        item.save(e);
    }

    itEnd = signatures.constEnd();
    for (it = signatures.constBegin(); it != itEnd; ++it) {
        QDomElement hash = doc.createElement("signature");
        hash.setAttribute("type", it.key());
        QDomText text = doc.createTextNode(it.value());
        hash.appendChild(text);
        e.appendChild(hash);
    }
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

        if (size)
        {
            QDomElement elem = doc.createElement("size");
            QDomText text = doc.createTextNode(QString::number(size));
            elem.appendChild(text);
            file.appendChild(elem);
        }

        data.save(file);
        resources.save(file);
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
    for (QDomElement elem = e.firstChildElement("file"); !elem.isNull(); elem = elem.nextSiblingElement("file"))
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

    foreach (const File &file, files)
    {
        file.save(e);
    }
}

void KGetMetalink::Files::clear()
{
    files.clear();
}

bool KGetMetalink::Metalink::isValid() const
{
    return files.isValid();
}

// #ifdef HAVE_NEPOMUK
// QHash<QUrl, Nepomuk::Variant> KGetMetalink::Files::properties() const
// {
//     return data.properties();
// }
// #endif //HAVE_NEPOMUK

void KGetMetalink::Metalink::load(const QDomElement &e)
{
    QDomDocument doc = e.ownerDocument();
    const QDomElement metalink = doc.firstChildElement("metalink");


    dynamic = (metalink.firstChildElement("dynamic").text() == "true");
    xmlns = metalink.attribute("xmlns");
    origin = KUrl(metalink.firstChildElement("origin").text());
    generator = metalink.firstChildElement("generator").text();
    updated.setData(metalink.firstChildElement("updated").text());
    published.setData(metalink.firstChildElement("published").text());
    updated.setData(metalink.firstChildElement("updated").text());


    files.load(e);
}

QDomDocument KGetMetalink::Metalink::save() const
{
    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(header);

    QDomElement metalink = doc.createElement("metalink");
    metalink.setAttribute("xmlns", "urn:ietf:params:xml:ns:metalink"); //the xmlns value is ignored, instead the data format described in the specification is always used

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

    files.save(metalink);

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
    file.close();

    QDomElement root = doc.documentElement();
    if (root.attribute("xmlns") == "urn:ietf:params:xml:ns:metalink")
    {
        metalink->load(root);
        return true;
    }
    else if ((root.attribute("xmlns") == "http://www.metalinker.org/") || (root.attribute("version") == "3.0"))
    {
        parseMetealink_v3_ed2(root, metalink);
        return true;
    }

    return false;
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
    if (root.attribute("xmlns") == "urn:ietf:params:xml:ns:metalink")
    {
        metalink->load(root);
        return true;
    }
    else if ((root.attribute("xmlns") == "http://www.metalinker.org/") || (root.attribute("version") == "3.0"))
    {
        parseMetealink_v3_ed2(root, metalink);
        return true;
    }

    return false;
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

void KGetMetalink::HandleMetalink::parseMetealink_v3_ed2(const QDomElement &e, Metalink *metalink)
{
    if (!metalink)
    {
        return;
    }

    QDomDocument doc = e.ownerDocument();
    const QDomElement metalinkDom = doc.firstChildElement("metalink");

    //here we assume that the CommonData set in metalink is for the whole metalink
    //and not the consisting files; the specification is a little bit unclear in that regard
    CommonData data;
    parseCommonData_v3_ed2(metalinkDom, &data);//TODO use this as general metadata for the whole document if something link that is specified?

    metalink->dynamic = (metalinkDom.attribute("type") == "dynamic");
    metalink->origin = KUrl(metalinkDom.firstChildElement("origin").text());
    metalink->generator = metalinkDom.firstChildElement("generator").text();

    parseDateConstruct_v3_ed2(&metalink->published, metalinkDom.attribute("pubdate"));
    parseDateConstruct_v3_ed2(&metalink->updated, metalinkDom.attribute("refreshdate"));

    paresFiles_v3_ed2(metalinkDom, &metalink->files);
}

void KGetMetalink::HandleMetalink::paresFiles_v3_ed2(const QDomElement &e, KGetMetalink::Files *files)
{
    if (!files) {
        return;
    }

    const QDomElement filesElem = e.firstChildElement("files");
    CommonData filesData;
    parseCommonData_v3_ed2(filesElem, &filesData);

    for (QDomElement elem = filesElem.firstChildElement("file"); !elem.isNull(); elem = elem.nextSiblingElement("file")) {
        File file;
        file.name = elem.attribute("name");
        file.size = elem.firstChildElement("size").text().toULongLong();
        parseCommonData_v3_ed2(elem, &file.data);

        //ensure that inheritance works
        if (file.data.identity.isEmpty()) {
            file.data.identity = filesData.identity;
        }
        if (file.data.version.isEmpty()) {
            file.data.version = filesData.version;
        }
        if (file.data.description.isEmpty()) {
            file.data.description = filesData.description;
        }
        if (file.data.os.isEmpty()) {
            file.data.os = filesData.os;
        }
        if (file.data.logo.isEmpty()) {
            file.data.logo = filesData.logo;
        }
        if (file.data.language.isEmpty()) {
            file.data.language = filesData.language;
        }
        if (file.data.copyright.isEmpty()) {
            file.data.copyright = filesData.copyright;
        }
        if (file.data.publisher.isEmpty()) {
            file.data.publisher = filesData.publisher;
        }
        if (file.data.license.isEmpty()) {
            file.data.license = filesData.license;
        }

        parseResources_v3_ed2(elem, &file.resources);

        //load the verification information
        QDomElement veriE = elem.firstChildElement("verification");

        for (QDomElement elemVer = veriE.firstChildElement("hash"); !elemVer.isNull(); elemVer = elemVer.nextSiblingElement("hash")) {
            QString type = elemVer.attribute("type");
            QString hash = elemVer.text();
            if (!type.isEmpty() && !hash.isEmpty()) {
                type = addaptHashType(type, true);
                file.verification.hashes[type] = hash;
            }
        }

        for (QDomElement elemVer = veriE.firstChildElement("pieces"); !elemVer.isNull(); elemVer = elemVer.nextSiblingElement("pieces")) {
            Pieces piecesItem;
            piecesItem.load(elemVer);
            file.verification.pieces.append(piecesItem);
        }

         for (QDomElement elemVer = veriE.firstChildElement("signature"); !elemVer.isNull(); elemVer = elemVer.nextSiblingElement("signature")) {
            const QString type = elemVer.attribute("type");
            const QString signature = elemVer.text();
            if (!type.isEmpty() && !signature.isEmpty()) {
                file.verification.hashes[type] = signature;
            }
        }

        files->files.append(file);
    }
}

void KGetMetalink::HandleMetalink::parseCommonData_v3_ed2(const QDomElement &e, KGetMetalink::CommonData *data)
{
    if (!data)
    {
        return;
    }

    data->load(e);

    const QDomElement publisherElem = e.firstChildElement("publisher");
    data->publisher.name = publisherElem.firstChildElement("name").text();
    data->publisher.url = KUrl(publisherElem.firstChildElement("url").text());

    const QDomElement lincenseElem = e.firstChildElement("license");
    data->license.name = lincenseElem.firstChildElement("name").text();
    data->license.url = KUrl(lincenseElem.firstChildElement("url").text());
}

void KGetMetalink::HandleMetalink::parseResources_v3_ed2(const QDomElement &e, KGetMetalink::Resources *resources)
{
    if (!resources)
    {
        return;
    }

    QDomElement res = e.firstChildElement("resources");
    for (QDomElement elemRes = res.firstChildElement("url"); !elemRes.isNull(); elemRes = elemRes.nextSiblingElement("url"))
    {
        const QString location = elemRes.attribute("location");
        int preference = elemRes.attribute("preference").toInt();
        if (preference > 100)
        {
            preference = 100;
        }
        const KUrl link = KUrl(elemRes.text());
        QString type;

        if (link.fileName().endsWith(QLatin1String(".torrent")))
        {
            type = "torrent";
        }

        if (type.isEmpty())
        {
            Url url;
            url.location = location;
            url.preference = preference;
            url.url = link;
            url.load(elemRes);
            if (url.isValid())
            {
                resources->urls.append(url);
            }
        }
        else
        {
            //it might be a metaurl
            Metaurl metaurl;
            metaurl.preference = preference;
            metaurl.url = link;
            metaurl.type = type;
            if (metaurl.isValid())
            {
                resources->metaurls.append(metaurl);
            }
        }
    }
}

void KGetMetalink::HandleMetalink::parseDateConstruct_v3_ed2(KGetMetalink::DateConstruct *dateConstruct, const QString &data)
{
    if (!dateConstruct || data.isEmpty())
    {
        return;
    }

    kDebug(5001) << "Parsing" << data;

    QString temp = data;
    QDateTime dateTime;
    QTime timeZoneOffset;

    //Date according to RFC 822, the year with four characters preferred
    //e.g.: "Mon, 15 May 2006 00:00:01 GMT", "Fri, 01 Apr 2009 00:00:01 +1030"
    const QString weekdayExp = "ddd, ";
    const bool weekdayIncluded = (temp.indexOf(',') == 3);
    int startPosition = (weekdayIncluded ? weekdayExp.length() : 0);
    const QString dayMonthExp = "dd MMM ";
    const QString yearExp = "yy";

    QString exp = dayMonthExp + yearExp + yearExp;
    int length = exp.length();

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
    temp = temp.mid(length + 1);//also remove the space
    bool negativeOffset = false;
    //e.g. GMT
    if (temp.length() == 3)
    {
        KTimeZone timeZone = KSystemTimeZones::readZone(temp);
        if (timeZone.isValid())
        {
            int offset = timeZone.currentOffset();
            negativeOffset = (offset < 0);
            timeZoneOffset = QTime(0, 0, 0);
            timeZoneOffset = timeZoneOffset.addSecs(qAbs(offset));
        }
    }
    //e.g. +1030
    else if (temp.length() == 5)
    {
        negativeOffset = (temp[0] == '-');
        timeZoneOffset = QTime::fromString(temp.mid(1,4), "hhmm");
    }

    dateConstruct->setData(dateTime, timeZoneOffset, negativeOffset);
}
