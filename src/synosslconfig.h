/*
 * GNU General Public License (GPL)
 * Copyright (c) 2020 by Aleksei Ilin
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

#ifndef SYNOSSLCONFIG_H
#define SYNOSSLCONFIG_H

#include <QObject>

class QNetworkAccessManager;

class SynoConn;
class SynoSslConfigPrivate;

class SynoSslConfig : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SynoSslConfig)

    Q_PROPERTY(bool isSslAvailable READ isSslAvailable NOTIFY isSslAvailableChanged)
    Q_PROPERTY(bool isSslError READ isSslError NOTIFY isSslErrorChanged)
    Q_PROPERTY(QObject* errorsModel READ errorsModel NOTIFY errorsModelChanged)
    Q_PROPERTY(QObject* expectedErrorsModel READ expectedErrorsModel NOTIFY expectedErrorsModelChanged)

public:
    explicit SynoSslConfig(QNetworkAccessManager* nma, SynoConn* parent);

    bool isSslAvailable() const;
    bool isSslError() const;

    void connectToHostEncrypted(const QString &hostName, quint16 port);
    void clearErrors();

    QObject* errorsModel() const;
    QObject* expectedErrorsModel() const;

public slots:
    void confirmSslExceptions();
    void loadExceptionsFromStorage();
    void saveExceptionsToStorage();

signals:
    void isSslErrorChanged();
    void confirmedSslExceptions();

    // those signals are never emitted, it is added to suppress
    // Qt warning about non-NOTIFYable properties
    void isSslAvailableChanged();
    void errorsModelChanged();
    void expectedErrorsModelChanged();
};

#endif // SYNOSSLCONFIG_H
