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

#include "synosslconfig.h"
#include "synoconn.h"

#ifndef QT_NO_SSL

#include "synosettings.h"
#include "synotraits.h"

#include <QtCore/private/qobject_p.h>
#include <QAbstractListModel>
#include <QByteArray>
#include <QDataStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslError>
#include <QSslSocket>
#include <QUrl>


static QDataStream& operator<<(QDataStream& ds, const QSslError& e) {
    ds << e.error();
    ds << e.certificate().toPem();
    return ds;
}

static QDataStream& operator>>(QDataStream& ds, QSslError& e) {
    int error;
    QByteArray pem;

    ds >> error;
    ds >> pem;

    QSslCertificate cert(pem, QSsl::Pem);

    e = QSslError(static_cast<QSslError::SslError>(error), cert);
    return ds;
}


class SynoSslErrorModel : public QAbstractListModel
{
public:
    enum SynoSslErrorRoles
    {
        RoleCertificateText = Qt::UserRole + 1,
        RoleCertificateDigestMD5,
        RoleCertificateDigestSha1,
        RoleCertificateIssuer,
        RoleCertificateSubject,
        RoleCertificateEffectiveDate,
        RoleCertificateExpiryDate,
        RoleErrorText
    };

public:
    SynoSslErrorModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {
    }

    void setErrors(const QList<QSslError>& errors = QList<QSslError>()) {
        if (m_errors != errors) {
            beginResetModel();
            m_errors = errors;
            endResetModel();
        }
    }

    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
        roles.insert(RoleCertificateText, QByteArrayLiteral("certificateText"));
        roles.insert(RoleCertificateDigestMD5, QByteArrayLiteral("certificateDigestMD5"));
        roles.insert(RoleCertificateDigestSha1, QByteArrayLiteral("certificateDigestSha1"));
        roles.insert(RoleCertificateIssuer, QByteArrayLiteral("certificateIssuer"));
        roles.insert(RoleCertificateSubject, QByteArrayLiteral("certificateSubject"));
        roles.insert(RoleCertificateEffectiveDate, QByteArrayLiteral("certificateEffectiveDate"));
        roles.insert(RoleCertificateExpiryDate, QByteArrayLiteral("certificateExpiryDate"));
        roles.insert(RoleErrorText, QByteArrayLiteral("errorText"));
        return roles;
    }

    int rowCount(const QModelIndex& parent) const override {
        if (parent.isValid()) {
            return 0;
        }

        return m_errors.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_errors.size()) {
            return QVariant();
        }

        const QSslError& e = m_errors[index.row()];

        switch (role) {
        case Qt::DisplayRole:
        case RoleErrorText:
            return e.errorString();
        case RoleCertificateDigestMD5:
            return QString::fromLatin1(e.certificate().digest(QCryptographicHash::Md5).toHex());
        case RoleCertificateDigestSha1:
            return QString::fromLatin1(e.certificate().digest(QCryptographicHash::Sha1).toHex());
        case RoleCertificateIssuer:
            return e.certificate().issuerDisplayName();
        case RoleCertificateSubject:
            return e.certificate().subjectDisplayName();
        case RoleCertificateEffectiveDate:
            return e.certificate().effectiveDate().toString(Qt::ISODate);
        case RoleCertificateExpiryDate:
            return e.certificate().expiryDate().toString(Qt::ISODate);
        case RoleCertificateText:
            return e.certificate().toText();
        default:
            break;
        }

        return QVariant();
    }

private:
    QList<QSslError> m_errors;
};

class SynoSslConfigPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(SynoSslConfig)

public:
    using LoadAllExceptionsFromStorageCallback = std::function<void(const QMap< QUrl, QList<QSslError> >& allExceptions)>;

public:
    SynoSslConfigPrivate()
        : QObjectPrivate()
    {
    }

    void updateSslErrors(const QList<QSslError>& errors = QList<QSslError>());
    void updateExpectedSslErrors(const QList<QSslError>& errors = QList<QSslError>());

    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    void loadAllExceptionsFromStorage(LoadAllExceptionsFromStorageCallback callback,
                                      SynoSettings::SecureFailureCallback callbackFailure = SynoSettings::SecureFailureCallback());

public:
    /*! Connection object */
    SynoConn* conn;
    /*! NetworkAccessManager object */
    QPointer<QNetworkAccessManager> nma;
    /*! Error model */
    SynoSslErrorModel* errorsModel;
    /*! Expected errors model */
    SynoSslErrorModel* expectedErrorsModel;
    /*! List of last SSL errors */
    QList<QSslError> sslErrors;
    /*! List of SSL errors confirmed by user */
    QList<QSslError> expectedSslErrors;
    /*! Settings object */
    SynoSettings settings;
};

SynoSslConfig::SynoSslConfig(QNetworkAccessManager* nma, SynoConn* parent)
    : QObject(*new SynoSslConfigPrivate(), parent)
{
    Q_D(SynoSslConfig);

    Q_ASSERT(nma);
    Q_ASSERT(parent);

    d->conn = parent;
    d->nma = nma;
    d->errorsModel = new SynoSslErrorModel();
    d->expectedErrorsModel = new SynoSslErrorModel();

    d->settings.setGroup(QStringLiteral("ssl"));

    connect(d->nma, &QNetworkAccessManager::sslErrors, this, [d](QNetworkReply *reply, const QList<QSslError> &errors) {
        d->onSslErrors(reply, errors);
    });

    loadExceptionsFromStorage();
}

bool SynoSslConfig::isSslAvailable() const
{
    return QSslSocket::supportsSsl();
}

bool SynoSslConfig::isSslError() const
{
    Q_D(const SynoSslConfig);

    return !d->sslErrors.isEmpty();
}

void SynoSslConfig::connectToHostEncrypted(const QString& hostName, quint16 port)
{
    Q_D(SynoSslConfig);

    d->nma->connectToHostEncrypted(hostName, port);
}

void SynoSslConfig::clearErrors()
{
    Q_D(SynoSslConfig);

    d->updateSslErrors();
}

QObject* SynoSslConfig::errorsModel() const
{
    Q_D(const SynoSslConfig);

    return d->errorsModel;
}

QObject* SynoSslConfig::expectedErrorsModel() const
{
    Q_D(const SynoSslConfig);

    return d->expectedErrorsModel;
}

void SynoSslConfig::confirmSslExceptions()
{
    Q_D(SynoSslConfig);

    QList<QSslError> neuExpectedErrors(d->expectedSslErrors);
    neuExpectedErrors.reserve(neuExpectedErrors.size() + d->sslErrors.size());

    for (const QSslError& error : std::as_const(d->sslErrors)) {
        if (!neuExpectedErrors.contains(error)) {
            neuExpectedErrors.append(error);
        }
    }

    d->updateExpectedSslErrors(neuExpectedErrors);

    emit confirmedSslExceptions();
}

void SynoSslConfigPrivate::loadAllExceptionsFromStorage(LoadAllExceptionsFromStorageCallback callback,
                                                        SynoSettings::SecureFailureCallback callbackFailure)
{
    Q_Q(SynoSslConfig);

    settings.readSecureC(QStringLiteral("exceptions"), q,
                         [callback](const QString&, const QString& secureValue) {
        QByteArray ba = QByteArray::fromHex(secureValue.toUtf8());
        QDataStream ds(&ba, QIODevice::ReadOnly);

        QMap< QUrl, QList<QSslError> > allExceptions;
        ds >> allExceptions;

        callback(allExceptions);
    }, callbackFailure);
}

void SynoSslConfig::loadExceptionsFromStorage()
{
    Q_D(SynoSslConfig);

    d->loadAllExceptionsFromStorage([d](const QMap<QUrl, QList<QSslError> >& allExceptions) {
        QList<QSslError> neuExpectedErrors = allExceptions.value(d->conn->synoUrl());
        d->updateExpectedSslErrors(neuExpectedErrors);
    }, [](const QString&, const QString&) {
        qCritical() << QObject::tr("SSL exceptions are not loaded due to keychain error.");
    });
}

void SynoSslConfig::saveExceptionsToStorage()
{
    Q_D(SynoSslConfig);

    auto storeExceptions = [this, d] (const QMap<QUrl, QList<QSslError> >& exceptions) {
        QByteArray ba;
        ba.reserve(8192);

        {
            QDataStream ds(&ba, QIODevice::WriteOnly);
            ds << exceptions;
        }

        QString secureValue = QString::fromUtf8(ba.toHex());

        d->settings.saveSecureC(QStringLiteral("exceptions"), secureValue, this, [](const QString&) {
            qCritical() << tr("SSL exceptions are not stored due to keychain error.");
        });
    };

    auto processReadExceptions = [this, d, storeExceptions](const QMap<QUrl, QList<QSslError> >& allExceptions) {
        QMap<QUrl, QList<QSslError> > updatedExceptions(allExceptions);
        updatedExceptions.insert(d->conn->synoUrl(), d->expectedSslErrors);
        storeExceptions(updatedExceptions);
    };

    d->loadAllExceptionsFromStorage([processReadExceptions](const QMap<QUrl, QList<QSslError> >& allExceptions) {
        processReadExceptions(allExceptions);
    }, [processReadExceptions](const QString&, const QString&) {
        QMap<QUrl, QList<QSslError> > empty;
        processReadExceptions(empty);
    });
}

void SynoSslConfigPrivate::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
    QList<QSslError> unexpectedSslErrors;
    unexpectedSslErrors.reserve(errors.size());

    for (const QSslError& error : errors) {
        if (!expectedSslErrors.contains(error)) {
            unexpectedSslErrors.append(error);
        }
    }

    if (unexpectedSslErrors.isEmpty()) {
        // all exceptions are confirmed by user
        reply->ignoreSslErrors(expectedSslErrors);
    } else {
        // save unexpected errors and notify user
        updateSslErrors(unexpectedSslErrors);
    }
}

void SynoSslConfigPrivate::updateSslErrors(const QList<QSslError>& errors)
{
    Q_Q(SynoSslConfig);

    if (sslErrors != errors) {
        sslErrors = errors;
        errorsModel->setErrors(errors);
        emit q->isSslErrorChanged();
    }
}

void SynoSslConfigPrivate::updateExpectedSslErrors(const QList<QSslError>& errors)
{
    Q_Q(SynoSslConfig);

    if (expectedSslErrors != errors) {
        expectedSslErrors = errors;
        expectedErrorsModel->setErrors(errors);
        emit q->isSslErrorChanged();
    }
}

#else // QT_NO_SSL

SynoSslConfig::SynoSslConfig(QNetworkAccessManager*, SynoConn* parent)
    : QObject(parent)
{
}

bool SynoSslConfig::isSslAvailable() const
{
    return false;
}

bool SynoSslConfig::isSslError() const
{
    return false;
}

void SynoSslConfig::connectToHostEncrypted(const QString&, quint16)
{
}

QObject* SynoSslConfig::errorsModel() const
{
    return nullptr;
}

QObject* SynoSslConfig::expectedErrorsModel() const
{
    return nullptr;
}

void SynoSslConfig::confirmSslExceptions()
{
}

void SynoSslConfig::loadExceptionsFromStorage()
{
}

void SynoSslConfig::saveExceptionsToStorage()
{
}

void SynoSslConfig::clearErrors()
{
}

#endif // QT_NO_SSL
