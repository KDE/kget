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

#include "metalinker.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtXml/QDomElement>

#include <kdeversion.h>
#include <KDebug>
#include <KLocale>
#include <KSystemTimeZone>

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Variant>
#include <Nepomuk/Vocabulary/NCO>
#include <Nepomuk/Vocabulary/NIE>
#include <Nepomuk/Vocabulary/NFO>
#include <Soprano/Vocabulary/NAO>

using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;
#endif //HAVE_NEPOMUK

const QString KGetMetalink::Metalink::KGET_DESCRIPTION = QString(QString("KGet/") + "2." + QString::number(KDE_VERSION_MINOR) + '.' + QString::number(KDE_VERSION_RELEASE));
const uint KGetMetalink::Metalink::MAX_URL_PRIORITY = 999999;
const uint KGetMetalink::Metalink_v3::MAX_PREFERENCE = 100;//as defined in Metalink specification 3.0 2nd edition

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
    logo = KUrl(e.firstChildElement("logo").text());
    copyright = e.firstChildElement("copyright").text();

    const QDomElement publisherElem = e.firstChildElement("publisher");
    publisher.name = publisherElem.attribute("name");
    publisher.url = KUrl(publisherElem.attribute("url"));

    for (QDomElement elemRes = e.firstChildElement("language"); !elemRes.isNull(); elemRes = elemRes.nextSiblingElement("language")) {
        languages << elemRes.text();
    }

    for (QDomElement elemRes = e.firstChildElement("os"); !elemRes.isNull(); elemRes = elemRes.nextSiblingElement("os")) {
        oses << elemRes.text();
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
    if (!logo.isEmpty())
    {
        QDomElement elem = doc.createElement("logo");
        QDomText text = doc.createTextNode(logo.url());
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

    foreach (const QString &language, languages) {
        QDomElement elem = doc.createElement("language");
        QDomText text = doc.createTextNode(language);
        elem.appendChild(text);
        e.appendChild(elem);
    }

    foreach (const QString &os, oses) {
        QDomElement elem = doc.createElement("os");
        QDomText text = doc.createTextNode(os);
        elem.appendChild(text);
        e.appendChild(elem);
    }
}

void KGetMetalink::CommonData::clear()
{
    identity.clear();
    version.clear();
    description.clear();
    oses.clear();
    logo.clear();
    languages.clear();
    publisher.clear();
    copyright.clear();
}

#ifdef HAVE_NEPOMUK
QList<QPair<QUrl, Nepomuk::Variant> > KGetMetalink::CommonData::properties() const
{
    //TODO what to do with identity?
    QList<QPair<QUrl, Nepomuk::Variant> > data;

    HandleMetalink::addProperty(&data, NIE::version(), version);
    HandleMetalink::addProperty(&data, NIE::description(), description);

    QList<Nepomuk::Resource> osResources;
    foreach (const QString &os, oses) {
        Nepomuk::Resource osRes(os, NFO::OperatingSystem());
        osRes.setProperty(NIE::title(), os);
        osResources << osRes;
    }
    if (!osResources.isEmpty()) {
        data << qMakePair(NAO::isRelated(), Nepomuk::Variant(osResources));
    }

    if (!logo.isEmpty()) {
        Nepomuk::Resource logoRes(logo, NFO::RemoteDataObject());
        logoRes.addType(NAO::Symbol());
        data << qMakePair(NAO::hasSymbol(), Nepomuk::Variant(logoRes));
    }

    QList<Nepomuk::Variant> langVariants;
    foreach (const QString &language, languages) {
        langVariants << language;
    }
    if (langVariants.count()) {
        data << qMakePair(NIE::language(), Nepomuk::Variant(langVariants));
    }

    if (!publisher.name.isEmpty()) {
        Nepomuk::Resource res(publisher.name, NCO::OrganizationContact());
        res.setLabel(publisher.name);
        res.addProperty(NCO::fullname(), publisher.name);
        if (!publisher.url.isEmpty()) {
            Nepomuk::Resource website(publisher.url, NFO::Website());
            website.addProperty(NIE::url(), publisher.url);
            res.addProperty(NCO::websiteUrl(), website);
        }
        data << qMakePair(NCO::publisher(), Nepomuk::Variant(res));
    }

    HandleMetalink::addProperty(&data, NIE::copyright(), copyright);

    return data;
}
#endif //HAVE_NEPOMUK


bool KGetMetalink::Metaurl::operator<(const KGetMetalink::Metaurl &other) const
{
     return (this->priority > other.priority) || (this->priority == 0);
}

void KGetMetalink::Metaurl::load(const QDomElement &e)
{
    type = e.attribute("mediatype").toLower();
    priority = e.attribute("priority").toUInt();
    if (priority > Metalink::MAX_URL_PRIORITY) {
        priority = Metalink::MAX_URL_PRIORITY;
    }
    name = e.attribute("name");
    url = KUrl(e.text());
}

void KGetMetalink::Metaurl::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement metaurl = doc.createElement("metaurl");
    if (priority)
    {
        metaurl.setAttribute("priority", priority);
    }
    if (!name.isEmpty())
    {
        metaurl.setAttribute("name", name);
    }
    metaurl.setAttribute("mediatype", type);

    QDomText text = doc.createTextNode(url.url());
    metaurl.appendChild(text);

    e.appendChild(metaurl);
}

bool KGetMetalink::Metaurl::isValid()
{
    return url.isValid() && url.hasHost() && !url.protocol().isEmpty() && !type.isEmpty();
}

void KGetMetalink::Metaurl::clear()
{
    type.clear();
    priority = 0;
    name.clear();
    url.clear();
}

bool KGetMetalink::Url::operator<(const KGetMetalink::Url &other) const
{
    bool smaller = (this->priority > other.priority) || ((this->priority == 0) && (other.priority != 0));

    if (!smaller && (this->priority == other.priority)) {
        QString countryCode = KGlobal::locale()->country();
        if (!countryCode.isEmpty()) {
            smaller = (other.location.toLower() == countryCode.toLower());
        }
    }
     return smaller;
}

void KGetMetalink::Url::load(const QDomElement &e)
{
    location = e.attribute("location").toLower();
    priority = e.attribute("priority").toUInt();
    if (priority > Metalink::MAX_URL_PRIORITY) {
        priority = Metalink::MAX_URL_PRIORITY;
    }
    url = KUrl(e.text());
}

void KGetMetalink::Url::save(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement elem = doc.createElement("url");
    if (priority)
    {
        elem.setAttribute("priority", priority);
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
    return url.isValid() && url.hasHost() && !url.protocol().isEmpty();
}

void KGetMetalink::Url::clear()
{
    priority = 0;
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
        QString type = elem.attribute("mediatype");
        if (type == "application/pgp-signature") {//FIXME with 4.5 make it handle signatures by default with mime-type
            type = "pgp";
        }
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
        QString type = it.key();
        if (type == "pgp") {//FIXME with 4.5 make it handle signatures by default with mime-type
            type = "application/pgp-signature";
        }
        QDomElement hash = doc.createElement("signature");
        hash.setAttribute("mediatype", type);
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
    return isValidNameAttribute() && resources.isValid();
}

void KGetMetalink::File::load(const QDomElement &e)
{
    data.load(e);

    name = QUrl::fromPercentEncoding(e.attribute("name").toAscii());
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


bool KGetMetalink::File::isValidNameAttribute() const
{
    if (name.isEmpty()) {
        kError(5001) << "Name attribute of Metalink::File is empty.";
        return false;
    }

    if (name.endsWith('/')) {
        kError(5001) << "Name attribute of Metalink::File does not contain a file name:" << name;
        return false;
    }

    const QStringList components = name.split('/');
    if (name.startsWith('/') || components.contains("..") || components.contains(".")) {
        kError(5001) << "Name attribute of Metalink::File contains directory traversal directives:" << name;
        return false;
    }

    return true;
}

#ifdef HAVE_NEPOMUK
QList<QPair<QUrl, Nepomuk::Variant> > KGetMetalink::File::properties() const
{
    return data.properties();
}
#endif //HAVE_NEPOMUK

bool KGetMetalink::Files::isValid() const
{
    if (files.isEmpty()) {
        return false;
    }

    QStringList fileNames;
    foreach (const File &file, files) {
        fileNames << file.name;
        if (!file.isValid()) {
            return false;
        }
    }

    //The value of name must be unique for each file
    while (!fileNames.isEmpty()) {
        const QString fileName = fileNames.takeFirst();
        if (fileNames.contains(fileName)) {
            kError(5001) << "Metalink::File name" << fileName << "exists multiple times.";
            return false;
        }
    }

    return true;
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

    xmlns = metalink.attribute("xmlns");
    generator = metalink.firstChildElement("generator").text();
    updated.setData(metalink.firstChildElement("updated").text());
    published.setData(metalink.firstChildElement("published").text());
    updated.setData(metalink.firstChildElement("updated").text());
    const QDomElement originElem = metalink.firstChildElement("origin");
    origin = KUrl(metalink.firstChildElement("origin").text());
    if (originElem.hasAttribute("dynamic")) {
        bool worked = false;
        dynamic = originElem.attribute("dynamic").toInt(&worked);
        if (!worked) {
            dynamic = (originElem.attribute("dynamic") == "true");
        }
    }

    files.load(e);
}

QDomDocument KGetMetalink::Metalink::save() const
{
    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(header);

    QDomElement metalink = doc.createElement("metalink");
    metalink.setAttribute("xmlns", "urn:ietf:params:xml:ns:metalink"); //the xmlns value is ignored, instead the data format described in the specification is always used

    QDomElement elem = doc.createElement("generator");
    QDomText text = doc.createTextNode(Metalink::KGET_DESCRIPTION); //the set generator is ignored, instead when saving KGET is always used
    elem.appendChild(text);
    metalink.appendChild(elem);

    if (!origin.isEmpty()) {
        QDomElement elem = doc.createElement("origin");
        QDomText text = doc.createTextNode(origin.url());
        elem.appendChild(text);
        if (dynamic) {
            elem.setAttribute("dynamic", "true");
        }
        metalink.appendChild(elem);
    }
    if (published.isValid()) {
        QDomElement elem = doc.createElement("published");
        QDomText text = doc.createTextNode(published.toString());
        elem.appendChild(text);
        metalink.appendChild(elem);
    }
    if (updated.isValid()) {
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

KGetMetalink::Metalink_v3::Metalink_v3()
{
}

KGetMetalink::Metalink KGetMetalink::Metalink_v3::metalink()
{
    return m_metalink;
}

void KGetMetalink::Metalink_v3::setMetalink(const KGetMetalink::Metalink &metalink)
{
    m_metalink = metalink;
}

void KGetMetalink::Metalink_v3::load(const QDomElement &e)
{
    QDomDocument doc = e.ownerDocument();
    const QDomElement metalinkDom = doc.firstChildElement("metalink");

    m_metalink.dynamic = (metalinkDom.attribute("type") == "dynamic");
    m_metalink.origin = KUrl(metalinkDom.attribute("origin"));
    m_metalink.generator = metalinkDom.attribute("generator");
    m_metalink.published = parseDateConstruct(metalinkDom.attribute("pubdate"));
    m_metalink.updated = parseDateConstruct(metalinkDom.attribute("refreshdate"));

    parseFiles(metalinkDom);
}

void KGetMetalink::Metalink_v3::parseFiles(const QDomElement &e)
{
    //here we assume that the CommonData set in metalink is for every file in the metalink
    CommonData data;
    data = parseCommonData(e);

    const QDomElement filesElem = e.firstChildElement("files");
    CommonData filesData = parseCommonData(filesElem);

    inheritCommonData(data, &filesData);

    for (QDomElement elem = filesElem.firstChildElement("file"); !elem.isNull(); elem = elem.nextSiblingElement("file")) {
        File file;
        file.name = QUrl::fromPercentEncoding(elem.attribute("name").toAscii());
        file.size = elem.firstChildElement("size").text().toULongLong();

        file.data = parseCommonData(elem);
        inheritCommonData(filesData, &file.data);

        file.resources = parseResources(elem);

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
                file.verification.signatures[type] = signature;
            }
        }

        m_metalink.files.files.append(file);
    }
}

KGetMetalink::CommonData KGetMetalink::Metalink_v3::parseCommonData(const QDomElement &e)
{
    CommonData data;

    data.load(e);

    const QDomElement publisherElem = e.firstChildElement("publisher");
    data.publisher.name = publisherElem.firstChildElement("name").text();
    data.publisher.url = KUrl(publisherElem.firstChildElement("url").text());

    return data;
}

void KGetMetalink::Metalink_v3::inheritCommonData(const KGetMetalink::CommonData &ancestor, KGetMetalink::CommonData *inheritor)
{
    if (!inheritor) {
        return;
    }

    //ensure that inheritance works
    if (inheritor->identity.isEmpty()) {
        inheritor->identity = ancestor.identity;
    }
    if (inheritor->version.isEmpty()) {
        inheritor->version = ancestor.version;
    }
    if (inheritor->description.isEmpty()) {
        inheritor->description = ancestor.description;
    }
    if (inheritor->oses.isEmpty()) {
        inheritor->oses = ancestor.oses;
    }
    if (inheritor->logo.isEmpty()) {
        inheritor->logo = ancestor.logo;
    }
    if (inheritor->languages.isEmpty()) {
        inheritor->languages = ancestor.languages;
    }
    if (inheritor->copyright.isEmpty()) {
        inheritor->copyright = ancestor.copyright;
    }
    if (inheritor->publisher.isEmpty()) {
        inheritor->publisher = ancestor.publisher;
    }
}

KGetMetalink::Resources KGetMetalink::Metalink_v3::parseResources(const QDomElement &e)
{
    Resources resources;

    QDomElement res = e.firstChildElement("resources");
    for (QDomElement elemRes = res.firstChildElement("url"); !elemRes.isNull(); elemRes = elemRes.nextSiblingElement("url")) {
        const QString location = elemRes.attribute("location").toLower();

        uint preference = elemRes.attribute("preference").toUInt();
        //the maximum preference we use is MAX_PREFERENCE
        if (preference > MAX_PREFERENCE) {
            preference = MAX_PREFERENCE;
        }
        const int priority = MAX_PREFERENCE - preference + 1;//convert old preference to new priority

        const KUrl link = KUrl(elemRes.text());
        QString type;

        if (link.fileName().endsWith(QLatin1String(".torrent"))) {
            type = "torrent";
        }

        if (type.isEmpty()) {
            Url url;
            if (preference) {
                url.priority = priority;
            }
            url.location = location;
            url.url = link;
            if (url.isValid()) {
                resources.urls.append(url);
            }
        } else {
            //it might be a metaurl
            Metaurl metaurl;
            if (preference) {
                metaurl.priority = priority;
            }
            metaurl.url = link;
            metaurl.type = type;
            if (metaurl.isValid()) {
                resources.metaurls.append(metaurl);
            }
        }
    }

    return resources;
}

KGetMetalink::DateConstruct KGetMetalink::Metalink_v3::parseDateConstruct(const QString &data)
{
    DateConstruct dateConstruct;

    if (data.isEmpty()){
        return dateConstruct;
    }

    kDebug(5001) << "Parsing" << data;

    QString temp = data;
    QDateTime dateTime;
    QTime timeZoneOffset;

    //Date according to RFC 822, the year with four characters preferred
    //e.g.: "Mon, 15 May 2006 00:00:01 GMT", "Fri, 01 Apr 2009 00:00:01 +1030"

    //find the date
    const QString weekdayExp = "ddd, ";
    const bool weekdayIncluded = (temp.indexOf(',') == 3);
    int startPosition = (weekdayIncluded ? weekdayExp.length() : 0);
    const QString dayMonthExp = "dd MMM ";
    const QString yearExp = "yy";

    QString exp = dayMonthExp + yearExp + yearExp;
    int length = exp.length();

    QLocale locale = QLocale::c();
    QDate date = locale.toDate(temp.mid(startPosition, length), exp);
    if (!date.isValid()) {
        exp = dayMonthExp + yearExp;
        length = exp.length();
        date = locale.toDate(temp.mid(startPosition, length), exp);
        if (!date.isValid()) {
            return dateConstruct;
        }
    }

    //find the time
    dateTime.setDate(date);
    temp = temp.mid(startPosition);
    temp = temp.mid(length + 1);//also remove the space

    const QString hourExp = "hh";
    const QString minuteExp = "mm";
    const QString secondExp = "ss";

    exp = hourExp + ':' + minuteExp + ':' + secondExp;
    length = exp.length();
    QTime time = QTime::fromString(temp.left(length), exp);
    if (!time.isValid()) {
        exp = hourExp + ':' + minuteExp;
        length = exp.length();
        time = QTime::fromString(temp.left(length), exp);
        if (!time.isValid()) {
            return dateConstruct;
        }
    }
    dateTime.setTime(time);

    //find the offset
    temp = temp.mid(length + 1);//also remove the space
    bool negativeOffset = false;

    if (temp.length() == 3) { //e.g. GMT
        KTimeZone timeZone = KSystemTimeZones::readZone(temp);
        if (timeZone.isValid()) {
            int offset = timeZone.currentOffset();
            negativeOffset = (offset < 0);
            timeZoneOffset = QTime(0, 0, 0);
            timeZoneOffset = timeZoneOffset.addSecs(qAbs(offset));
        }
    } else if (temp.length() == 5) { //e.g. +1030
        negativeOffset = (temp[0] == '-');
        timeZoneOffset = QTime::fromString(temp.mid(1,4), "hhmm");
    }

    dateConstruct.setData(dateTime, timeZoneOffset, negativeOffset);

    return dateConstruct;
}

QDomDocument KGetMetalink::Metalink_v3::save() const
{
    QDomDocument doc;
    QDomProcessingInstruction header = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(header);

    QDomElement metalink = doc.createElement("metalink");
    metalink.setAttribute("xmlns", "http://www.metalinker.org/");
    metalink.setAttribute("version", "3.0");
    metalink.setAttribute("type", (m_metalink.dynamic ? "dynamic" : "static"));
    metalink.setAttribute("generator", Metalink::KGET_DESCRIPTION); //the set generator is ignored, instead when saving KGET is always used

    if (m_metalink.published.isValid()) {
        metalink.setAttribute("pubdate", dateConstructToString(m_metalink.published));
    }
    if (m_metalink.updated.isValid()) {
        metalink.setAttribute("refreshdate", dateConstructToString(m_metalink.updated));
    }
    if (!m_metalink.origin.isEmpty()) {
        metalink.setAttribute("origin", m_metalink.origin.url());
    }

    saveFiles(metalink);

    doc.appendChild(metalink);

    return doc;
}

void KGetMetalink::Metalink_v3::saveFiles(QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement filesElem = doc.createElement("files");

    foreach (const File &file, m_metalink.files.files) {
        QDomElement elem = doc.createElement("file");
        elem.setAttribute("name", file.name);

        QDomElement size = doc.createElement("size");
        QDomText text = doc.createTextNode(QString::number(file.size));
        size.appendChild(text);
        elem.appendChild(size);

        saveCommonData(file.data, elem);
        saveResources(file.resources, elem);
        saveVerification(file.verification, elem);

        filesElem.appendChild(elem);
    }

    e.appendChild(filesElem);
}

void KGetMetalink::Metalink_v3::saveResources(const Resources &resources, QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement res = doc.createElement("resources");

    foreach (const Url &url, resources.urls) {
        QDomElement elem = doc.createElement("url");
        const uint priority = url.priority;
        if (priority) {
            int preference = MAX_PREFERENCE - priority + 1;
            if (preference <= 0) {
                preference = 1;//HACK if priority is larger MAX_PREFERENCE makes it 1
            }
            elem.setAttribute("preference", preference);
        }
        if (!url.location.isEmpty()) {
            elem.setAttribute("location", url.location);
        }

        QDomText text = doc.createTextNode(url.url.url());
        elem.appendChild(text);

        res.appendChild(elem);
    }

    foreach (const Metaurl &metaurl, resources.metaurls) {
        if (metaurl.type == "torrent") {
            QDomElement elem = doc.createElement("url");
            elem.setAttribute("type", "bittorrent");
            const uint priority = metaurl.priority;
            if (priority) {
                int preference = MAX_PREFERENCE - priority + 1;
                if (preference <= 0) {
                    preference = 1;//HACK if priority is larger MAX_PREFERENCE makes it 1
                }
                elem.setAttribute("preference", preference);
            }

            QDomText text = doc.createTextNode(metaurl.url.url());
            elem.appendChild(text);

            res.appendChild(elem);
        }
    }

    e.appendChild(res);
}

void KGetMetalink::Metalink_v3::saveVerification(const KGetMetalink::Verification &verification, QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();
    QDomElement veri = doc.createElement("verification");

    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = verification.hashes.constEnd();
    for (it = verification.hashes.constBegin(); it != itEnd; ++it) {
        QDomElement elem = doc.createElement("hash");
        elem.setAttribute("type", it.key());
        QDomText text = doc.createTextNode(it.value());
        elem.appendChild(text);

        veri.appendChild(elem);
    }

    foreach (const Pieces &pieces, verification.pieces) {
        QDomElement elem = doc.createElement("pieces");
        elem.setAttribute("type", pieces.type);
        elem.setAttribute("length", QString::number(pieces.length));

        for (int i = 0; i < pieces.hashes.count(); ++i) {
            QDomElement hash = doc.createElement("hash");
            hash.setAttribute("piece", i);
            QDomText text = doc.createTextNode(pieces.hashes.at(i));
            hash.appendChild(text);

            elem.appendChild(hash);
        }
        veri.appendChild(elem);
    }

    itEnd = verification.signatures.constEnd();
    for (it = verification.signatures.constBegin(); it != itEnd; ++it) {
        QDomElement elem = doc.createElement("signature");
        elem.setAttribute("type", it.key());
        QDomText text = doc.createTextNode(it.value());
        elem.appendChild(text);

        veri.appendChild(elem);
    }

    e.appendChild(veri);
}

void KGetMetalink::Metalink_v3::saveCommonData(const KGetMetalink::CommonData &data, QDomElement &e) const
{
    QDomDocument doc = e.ownerDocument();

    CommonData commonData = data;

    if (!commonData.publisher.isEmpty()) {
        QDomElement elem = doc.createElement("publisher");
        QDomElement elemName = doc.createElement("name");
        QDomElement elemUrl = doc.createElement("url");

        QDomText text = doc.createTextNode(commonData.publisher.name);
        elemName.appendChild(text);
        elem.appendChild(elemName);

        text = doc.createTextNode(commonData.publisher.url.url());
        elemUrl.appendChild(text);
        elem.appendChild(elemUrl);

        e.appendChild(elem);

        commonData.publisher.clear();
    }

    if (commonData.oses.count() > 1) {//only one OS can be set in 3.0
        commonData.oses.clear();
    }

    commonData.save(e);
}

QString KGetMetalink::Metalink_v3::dateConstructToString(const KGetMetalink::DateConstruct &date) const
{
    QString dateString;
    if (!date.isValid()) {
        return dateString;
    }

    QLocale locale = QLocale::c();

    //"Fri, 01 Apr 2009 00:00:01 +1030"
    dateString += locale.toString(date.dateTime, "ddd, dd MMM yyyy hh:mm:ss ");

    if (date.timeZoneOffset.isValid()) {
        dateString += (date.negativeOffset ? '-' : '+');
        dateString += date.timeZoneOffset.toString("hhmm");
    } else {
        dateString += "+0000";
    }

    return dateString;
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
        Metalink_v3 metalink_v3;
        metalink_v3.load(root);
        *metalink = metalink_v3.metalink();
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

    metalink->clear();
    QDomElement root = doc.documentElement();
    if (root.attribute("xmlns") == "urn:ietf:params:xml:ns:metalink")
    {
        metalink->load(root);
        return true;
    }
    else if ((root.attribute("xmlns") == "http://www.metalinker.org/") || (root.attribute("version") == "3.0"))
    {
        Metalink_v3 metalink_v3;
        metalink_v3.load(root);
        *metalink = metalink_v3.metalink();
        return true;
    }

    return false;
}

bool KGetMetalink::HandleMetalink::save(const KUrl &destination, KGetMetalink::Metalink *metalink)
{
    QFile file(destination.pathOrUrl());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDomDocument doc;
    QString fileName = destination.fileName();
    if (fileName.endsWith(QLatin1String("meta4"))) {
        doc = metalink->save();
    } else if (fileName.endsWith(QLatin1String("metalink"))) {
        Metalink_v3 metalink_v3;
        metalink_v3.setMetalink(*metalink);
        doc = metalink_v3.save();
    } else {
        file.close();
        return false;
    }

    QTextStream stream(&file);
    doc.save(stream, 2);
    file.close();

    return true;
}

#ifdef HAVE_NEPOMUK
void KGetMetalink::HandleMetalink::addProperty(QList<QPair<QUrl, Nepomuk::Variant> > *data, const QByteArray &uriBa, const QString &value)
{
    if (!uriBa.isEmpty()) {
        addProperty(data, QUrl::fromEncoded(uriBa, QUrl::StrictMode), value);
    }
}

void KGetMetalink::HandleMetalink::addProperty(QList<QPair<QUrl, Nepomuk::Variant> > *data, const QUrl &uri, const QString &value)
{
    if (data && !uri.isEmpty() && !value.isEmpty()) {
        (*data) << qMakePair(uri, Nepomuk::Variant(value));
    }
}
#endif //HAVE_NEPOMUK


KGetMetalink::MetalinkHttpParser::~MetalinkHttpParser()
{

}

QString* KGetMetalink::MetalinkHttpParser::getEtag()
{
    return &m_EtagValue;
}

void KGetMetalink::MetalinkHttpParser::checkMetalinkHttp()
{
    if (!m_Url.isValid()) {
        kDebug() << "Url not valid";
        return;
    }

    KIO::TransferJob *job;
    job = KIO::get(m_Url);
    job->addMetaData("PropagateHttpHeader", "true");
    job->setRedirectionHandlingEnabled(false);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotHeaderResult(KJob*)));  // Finished
    connect(job, SIGNAL(redirection(KIO::Job*,KUrl)), this, SLOT(slotRedirection(KIO::Job*,KUrl))); // Redirection
    connect(job,SIGNAL(mimetype(KIO::Job*,QString)),this,SLOT(detectMime(KIO::Job*,QString))); // Mime detection.
    kDebug() << " Verifying Metalink/HTTP Status" ;
    m_loop.exec();
}

void KGetMetalink::MetalinkHttpParser::detectMime(KIO::Job *job, const QString &type)
{
    kDebug() << "Mime Type: " << type ;
    job->kill();
    m_loop.exit();
}

void KGetMetalink::MetalinkHttpParser::slotHeaderResult(KJob* kjob)
{
    KIO::Job* job = qobject_cast<KIO::Job*>(kjob);
    const QString httpHeaders = job ? job->queryMetaData("HTTP-Headers") : QString();
    parseHeaders(httpHeaders);
    setMetalinkHSatus();

    // Handle the redirection... (Comment out if not desired)
    if (m_redirectionUrl.isValid()) {
       m_Url = m_redirectionUrl;
       m_redirectionUrl = KUrl();
       checkMetalinkHttp();
    }

}

void KGetMetalink::MetalinkHttpParser::slotRedirection(KIO::Job *job, const KUrl & url)
{
    Q_UNUSED(job)
    m_redirectionUrl = url;
}

bool KGetMetalink::MetalinkHttpParser::isMetalinkHttp()
{
    if (m_MetalinkHSatus) {
        kDebug() << "Metalink Http detected" ;
    }
    else {
        kDebug() << "No Metalink HTTP response found" ;
    }
    return m_MetalinkHSatus;
}

void KGetMetalink::MetalinkHttpParser::parseHeaders(const QString &httpHeader)
{
    QString trimedHeader = httpHeader.mid(httpHeader.indexOf('\n') + 1).trimmed();

    foreach(QString line, trimedHeader.split('\n')) {
        int colon = line.indexOf(':');
        QString headerName = line.left(colon).trimmed();
        QString headerValue = line.mid(colon + 1).trimmed();
        m_headerInfo.insertMulti(headerName, headerValue);
    }

    m_EtagValue = m_headerInfo.value("ETag");
}

void KGetMetalink::MetalinkHttpParser::setMetalinkHSatus()
{
    bool linkStatus, digestStatus;
    linkStatus = digestStatus = false;
    if (m_headerInfo.contains("link")) {
        QList<QString> linkValues = m_headerInfo.values("link");

        foreach(QString linkVal, linkValues) {
            if (linkVal.contains("rel=duplicate")) {
                linkStatus = true;
                break;
            }
        }
    }

    if (m_headerInfo.contains("digest")) {
        QList<QString> digestValues = m_headerInfo.values("digest");

        foreach(QString digestVal, digestValues) {
            if (digestVal.contains("sha-256", Qt::CaseInsensitive)) {
                digestStatus = true;
                break;
            }
        }
    }

    if ((linkStatus) && (digestStatus)) {
        m_MetalinkHSatus = true;
    }

}

KUrl KGetMetalink::MetalinkHttpParser::getUrl()
{
    return m_Url;
}

QMultiMap<QString, QString>* KGetMetalink::MetalinkHttpParser::getHeaderInfo()
{
    return & m_headerInfo;
}

bool KGetMetalink::httpLinkHeader::operator<(const httpLinkHeader &other) const
{
    return m_depth < other.m_depth;
}

void KGetMetalink::httpLinkHeader::headerBuilder(const QString &line)
{
    url = line.mid(line.indexOf("<") + 1,line.indexOf(">") -1).trimmed();
    QList<QString> attribList = line.split(";");
    foreach ( QString str, attribList) {
        QString attribId = str.mid(0,str.indexOf("=")).trimmed();
        QString attribValue = str.mid(str.indexOf("=")+1).trimmed();
        if (attribId == "rel") {
            m_reltype = attribValue;
        }
        if (attribId == "depth") {
            m_depth = attribValue.toInt();
        }
        if (attribId == "geo") {
            m_geo = attribValue;
        }
        if (attribId == "pref") {
            m_pref = true;
        }
        if (attribId == "pri") {
            priority = attribValue.toUInt();
        }
        if (attribId == "type") {
            type = attribValue;
        }

        if (attribId == "name") {
            name = attribValue;
        }
    }
}

#include "metalinker.moc"
