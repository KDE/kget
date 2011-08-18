/* This file is part of the KDE project

   Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>
   Copyright (C) 2011 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "httpserver.h"

#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include "core/kget.h"
#include "settings.h"

#include <KDebug>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <kwallet.h>

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QHttpRequestHeader>
#include <QDateTime>

HttpServer::HttpServer(QWidget *parent)
    : QObject(parent),
      m_wallet(0)
{
    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                                           parent->winId(),///Use MainWindow?
                                           KWallet::Wallet::Asynchronous);
    if (m_wallet) {
        connect(m_wallet, SIGNAL(walletOpened(bool)), SLOT(init(bool)));
    } else {
        KGet::showNotification(parent, "error", i18n("Unable to start WebInterface: Could not open KWallet"));
    }
}

HttpServer::~HttpServer()
{
    delete m_wallet;
}

void HttpServer::init(bool opened)
{
    if (opened &&
        m_wallet->hasFolder("KGet") &&
        m_wallet->setFolder("KGet")) {
        m_wallet->readPassword("Webinterface", m_pwd);
    } else {
        KGet::showNotification(static_cast<QWidget*>(parent()), "error", i18n("Unable to start WebInterface: Could not open KWallet"));
        return;
    }
    m_tcpServer = new QTcpServer(this);
    if (!m_tcpServer->listen(QHostAddress::Any, Settings::webinterfacePort())) {
        KGet::showNotification(static_cast<QWidget*>(parent()), "error", i18nc("@info", "Unable to start WebInterface: %1", m_tcpServer->errorString()));
        return;
    }

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(handleRequest()));
}

void HttpServer::settingsChanged()
{
    if (m_wallet) {
        m_wallet->readPassword("Webinterface", m_pwd);
    }
}

void HttpServer::handleRequest()
{
    int responseCode = 200;
    QString responseText = "OK";
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    if (!clientConnection->waitForReadyRead()) {
        clientConnection->disconnectFromHost();
    }

    QByteArray request(clientConnection->readAll());
    QHttpRequestHeader header(request);

    QByteArray data;

    // for HTTP authorization information see: http://www.governmentsecurity.org/articles/OverviewofHTTPAuthentication.php
    QString auth = header.value("Authorization");
    if (auth.length() < 6 || QByteArray::fromBase64(auth.right(auth.length() - 6).toUtf8()) !=
            QString(Settings::webinterfaceUser() + ':' + m_pwd)) {
        responseCode = 401;
        responseText = "Authorization Required";
        // DO NOT TRANSLATE THE FOLLOWING MESSAGE! webserver messages are never translated.
        QString authRequiredText = QString("<html><head><title>401 Authorization Required</title></head><body>"
                                           "<h1>Authorization Required</h1>This server could not verify that you "
                                           "are authorized to access the document requested. Either you supplied "
                                           "the wrong credentials (e.g., bad password), or your browser does "
                                           "not understand how to supply the credentials required.</body></html>");
        data.append(authRequiredText.toUtf8());
    } else {

    if (header.path().endsWith(QLatin1String("data.json"))) {
        data.append("{\"downloads\":[");
        bool needsToBeClosed = false;
        foreach(TransferHandler *transfer, KGet::allTransfers()) {
            if (needsToBeClosed)
                data.append(","); // close the last line
            data.append(QString("{\"name\":\"" + transfer->source().fileName() +
                             "\", \"src\":\"" + transfer->source().prettyUrl() +
                             "\", \"dest\":\"" + transfer->dest().pathOrUrl()  +
                             "\", \"status\":\"" + transfer->statusText() +
                             "\", \"size\":\"" + KIO::convertSize(transfer->totalSize()) +
                             "\", \"progress\":\"" + QString::number(transfer->percent()) + "%"
                             "\", \"speed\":\"" + i18nc("@item speed of transfer per seconds", "%1/s",
                                                        KIO::convertSize(transfer->downloadSpeed())) + "\"}").toUtf8());
            needsToBeClosed = true;
        }
        data.append("]}");
    } else if (header.path().startsWith(QLatin1String("/do"))) {
        kDebug(5001) << request;

        QString args = header.path().right(header.path().length() - 4);

        if (!args.isEmpty()) {
            QString action;
            QString data;
            QString group;
            QStringList argList = args.split('&');
            foreach (const QString &s, argList) {
                QStringList map = s.split('=');
                if (map.at(0) == "action")
                    action = map.at(1);
                else if (map.at(0) == "data")
                    data = KUrl::fromPercentEncoding(QByteArray(map.at(1).toUtf8()));
                // action specific parameters
                else if (map.at(0) == "group")
                    group = KUrl::fromPercentEncoding(QByteArray(map.at(1).toUtf8()));
            }
            kDebug(5001) << action << data << group;
            if (action == "add") {
                //find a folder to store the download in 
                QString defaultFolder;

                //prefer the defaultFolder of the selected group
                TransferGroupHandler *groupHandler = KGet::findGroup(group);
                if (groupHandler) {
                    defaultFolder = groupHandler->defaultFolder();
                }
                if (defaultFolder.isEmpty()) {
                    QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(KUrl(data));
                    if (groups.isEmpty() || groups.first()->defaultFolder().isEmpty()) {
                        defaultFolder = KGet::generalDestDir();
                    } else {
                        // take first item of default folder list (which should be the best one)
                        groupHandler = groups.first();
                        group = groupHandler->name();
                        defaultFolder = groupHandler->defaultFolder();
                    }
                }
                KGet::addTransfer(data, defaultFolder, KUrl(data).fileName(), group);
                data.append(QString("Ok, %1 added!").arg(data).toUtf8());
            } else if (action == "start") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    transfer->start();
            } else if (action == "stop") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    transfer->stop();
            } else if (action == "remove") {
                TransferHandler *transfer = KGet::findTransfer(data);
                if (transfer)
                    KGet::delTransfer(transfer);
            } else {
                kWarning(5001) << "not implemented action" << action << data;
            }
        }
    } else { // read it from filesystem
        QString fileName = header.path().remove(".."); // disallow changing directory
        if (fileName.endsWith('/'))
            fileName = "index.htm";

        QString path = KStandardDirs::locate("data", "kget/www/" + fileName);
        QFile file(path);

        if (path.isEmpty() || !file.open(QIODevice::ReadOnly)) {
            responseCode = 404;
            responseText = "Not Found";
            // DO NOT TRANSLATE THE FOLLOWING MESSAGE! webserver messages are never translated.
            QString notfoundText = QString("<html><head><title>404 Not Found</title></head><body>"
                                           "<h1>Not Found</h1>The requested URL <code>%1</code> "
                                           "was not found on this server.</body></html>")
                                           .arg(header.path());
            data.append(notfoundText.toUtf8());
        } else {
            while (!file.atEnd()) {
                data.append(file.readLine());
            }
        }
        if (fileName == "index.htm") { // translations
            data.replace("#{KGet Webinterface}", i18nc("@label", "KGet Web Interface").toUtf8());
            data.replace("#{Nr}", i18nc("@label number", "Nr").toUtf8());
            data.replace("#{File name}", i18nc("@label", "File name").toUtf8());
            data.replace("#{Finished}", i18nc("@label Progress of transfer", "Finished").toUtf8());
            data.replace("#{Speed}", i18nc("@label Speed of transfer", "Speed").toUtf8());
            data.replace("#{Status}", i18nc("@label Status of transfer", "Status").toUtf8());
            data.replace("#{Start}", i18nc("@action:button start a transfer", "Start").toUtf8());
            data.replace("#{Stop}", i18nc("@action:button", "Stop").toUtf8());
            data.replace("#{Remove}", i18nc("@action:button", "Remove").toUtf8());
            data.replace("#{Source:}", i18nc("@label Download from", "Source:").toUtf8());
            data.replace("#{Saving to:}", i18nc("@label Save download to", "Saving to:").toUtf8());
            data.replace("#{Webinterface}", i18nc("@label Title in header", "Web Interface").toUtf8());
            data.replace("#{Settings}", i18nc("@action", "Settings").toUtf8());
            data.replace("#{Refresh}", i18nc("@action", "Refresh").toUtf8());
            data.replace("#{Enter URL: }", i18nc("@action", "Enter URL: ").toUtf8());
            data.replace("#{OK}", i18nc("@action:button", "OK").toUtf8());
            data.replace("#{Refresh download list every}",
                         i18nc("@action Refresh download list every x (seconds)", "Refresh download list every").toUtf8());
            data.replace("#{seconds}", i18nc("@action (Refresh very x )seconds", "seconds").toUtf8());
            data.replace("#{Save Settings}", i18nc("@action:button", "Save Settings").toUtf8());
            data.replace("#{Downloads}", i18nc("@title", "Downloads").toUtf8());
            data.replace("#{KGet Webinterface | Valid XHTML 1.0 Strict &amp; CSS}",
                         i18nc("@label text in footer", "KGet Web Interface | Valid XHTML 1.0 Strict &amp; CSS").toUtf8().replace('&', "&amp;"));

            // delegate group combobox
            QString groupOptions = "";
            Q_FOREACH(const QString &group, KGet::transferGroupNames())
                groupOptions += QString("<option>%1</option>").arg(group);
            data.replace("#{groups}", groupOptions.toUtf8());
        }
    }
    }

    // for HTTP information see: http://www.jmarshall.com/easy/http/
    QByteArray block;
    block.append(QString("HTTP/1.1 %1 %2\r\n").arg(responseCode).arg(responseText).toUtf8());
    block.append(QString("Date: %1 GMT\r\n").arg(QDateTime(QDateTime::currentDateTime())
                                                 .toString("ddd, dd MMM yyyy hh:mm:ss")).toUtf8());
    block.append(QString("Server: KGet\r\n").toUtf8()); //TODO: add KGet version
    if (responseCode == 401)
        block.append(QString("WWW-Authenticate: Basic realm=\"KGet Webinterface Authorization\"\r\n").toUtf8());
    if (header.path().endsWith(QLatin1String(".png")) && responseCode == 200)
        block.append(QString("Content-Type: image/png\r\n").toUtf8());
    else if (header.path().endsWith(QLatin1String(".json")) && responseCode == 200)
        block.append(QString("Content-Type: application/x-json\r\n").toUtf8());
    else if (header.path().endsWith(QLatin1String(".gif")) && responseCode == 200)
        block.append(QString("Content-Type: image/gif\r\n").toUtf8());
    else if (header.path().endsWith(QLatin1String(".js")) && responseCode == 200)
        block.append(QString("Content-Type: text/javascript\r\n").toUtf8());
    else if (header.path().endsWith(QLatin1String(".htc")) && responseCode == 200)
        block.append(QString("Content-Type: text/x-component\r\n").toUtf8());
    else
        block.append(QString("Content-Type: text/html; charset=UTF-8\r\n").toUtf8());
    block.append(QString("Content-Length: " + QString::number(data.length())+"\r\n").toUtf8());
    block.append(QString("\r\n").toUtf8());
    block.append(data);

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}
