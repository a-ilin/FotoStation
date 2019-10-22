/*
 * The MIT License (MIT)
 * Copyright (c) 2019 by Aleksei Ilin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
        m_errorString = tr("Error on image decoding: %1").arg(reader.errorString());
        if (m_errorString.isEmpty()) {
            m_errorString = tr("Unknown error on image decoding");
        }
    }

    emit finished();
}
