/*
 * GNU General Public License (GPL)
 * Copyright (c) 2019 by Aleksei Ilin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYNOCONN_P_H
#define SYNOCONN_P_H

#include <memory>

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QSet>
#include <QUrl>

#include "synoauth.h"
#include "synoconn.h"
#include "synorequest.h"
#include "synosslconfig.h"

#include <QtCore/private/qobject_p.h>


class SynoConnPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(SynoConn)

public:
    SynoConnPrivate() {}

    void setErrorString(const QString& err);
    QString pathForAPI(const QByteArray& api) const;
    void sendApiMapRequest();
    bool processApiMapReply(const SynoRequest* req);
    void setStatus(SynoConn::SynoConnStatus status);
    void onReplyFinished(SynoRequest* request);

public:
    QString errorString;
    QNetworkAccessManager networkManager;
    /*! Set of active requests */
    QSet< SynoRequest* > pendingRequests;
    /*! Url containing protocol, host name, port, and path to PS */
    QUrl synoUrl;
    /*! Map of API query to API path */
    QMap<QByteArray, QString> apiMap;
    /*! Path to API directory */
    QString apiDir;
    /*! Connection status */
    SynoConn::SynoConnStatus status;
    /*! Authorization handler */
    SynoAuth* auth;
    /*! SSL config */
    SynoSslConfig* sslConfig;
};

#endif // SYNOCONN_P_H
