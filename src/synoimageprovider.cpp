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


#include "synoimageprovider.h"
#include "synoimageprovider_p.h"
#include "synoconn.h"
#include "synops.h"

#include <QBuffer>
#include <QImageReader>
#include <QQuickTextureFactory>


SynoImageProvider::SynoImageProvider()
{
}

QQuickImageResponse* SynoImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    SynoImageResponse* response = new SynoImageResponse(requestedSize);

    QString thumbPrefix(QStringLiteral("thumb/"));
    if (id.startsWith(thumbPrefix)) {
        response->loadThumb(id.mid(thumbPrefix.size()).toLatin1());
    }

    return response;
}

SynoImageResponse::SynoImageResponse(const QSize& size)
    : m_size(size)
{
}

void SynoImageResponse::loadThumb(const QByteArray& id)
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=get");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("size=small");
    formData << QByteArrayLiteral("id=") + id;

    Q_ASSERT(!m_req);
    m_req = SynoPS::instance().conn()->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Thumb"), formData);
    m_req->send(this, [this] {
        if (m_req->errorString().isEmpty()) {
            processImage();
        } else {
            m_errorString = tr("Error during retrieving thumbnail. %1").arg(m_req->errorString());
            qWarning() << __FUNCTION__ << m_errorString;
        }
    });
}

QQuickTextureFactory* SynoImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QString SynoImageResponse::errorString() const
{
    return m_errorString;
}

void SynoImageResponse::cancel()
{
    Q_ASSERT(m_req);
    m_req->cancel();
    emit finished();
}

void SynoImageResponse::processImage()
{
    Q_ASSERT(m_req);

    // assume JPEG as the most widely used format
    QByteArray imageFormat(QByteArrayLiteral("JPG"));
    if (m_req->contentType() != SynoRequest::IMAGE_JPEG) {
        QList<QByteArray> imageFormatsForMimeType = QImageReader::imageFormatsForMimeType(m_req->contentMimeTypeRaw());
        if (imageFormatsForMimeType.size() > 0) {
            imageFormat = imageFormatsForMimeType[0];
        } else {
            // unknown mime type
            imageFormat = QByteArray();
        }
    }

    QByteArray data(m_req->replyBody());
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, imageFormat);
    if (!reader.read(&m_image) || m_image.isNull()) {
        QString readerError = reader.errorString();
        if (!readerError.isEmpty()) {
            m_errorString = tr("Error on image decoding: %1").arg(readerError);
        } else {
            m_errorString = tr("Unknown error on image decoding");
        }
    }

    emit finished();
}
