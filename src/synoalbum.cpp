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

#include "synoalbum.h"
#include "synoconn.h"

int constexpr const_str_length(const char* str)
{
    return *str ? 1 + const_str_length(str + 1) : 0;
}

// copy-pasted from QStringAlgorithms and allowed to use required comparison symbol
static inline QString simplified_helper(const QString &str, QChar symbol)
{
    if (str.isEmpty())
        return str;
    const QChar *src = str.cbegin();
    const QChar *end = str.cend();
    QString result = QString(str.size(), Qt::Uninitialized);

    QChar *dst = const_cast<QChar *>(result.cbegin());
    QChar *ptr = dst;
    bool unmodified = true;
    forever {
        while (src != end && (*src == symbol))
            ++src;
        while (src != end && !(*src == symbol))
            *ptr++ = *src++;
        if (src == end)
            break;
        if (*src != symbol)
            unmodified = false;
        *ptr++ = symbol;
    }
    if (ptr != dst && ptr[-1] == symbol)
        --ptr;

    int newlen = ptr - dst;
    if (newlen == str.size() && unmodified) {
        // nothing happened, return the original
        return str;
    }
    result.resize(newlen);
    return result;
}

SynoAlbum::SynoAlbum(SynoConn *conn, QObject *parent)
    : QObject(parent)
    , m_conn(conn)
    , m_offset(0)
    , m_batchSize(10)
{

}

void SynoAlbum::list()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=list");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("type=album,photo,video");
    formData << QByteArrayLiteral("offset=") + QByteArray::number(m_offset);
    formData << QByteArrayLiteral("limit=") + QByteArray::number(m_batchSize);
    formData << QByteArrayLiteral("recursive=false");
    formData << QByteArrayLiteral("additional=album_permission,photo_exif,video_codec,video_quality,thumb_size,file_location");
    formData << QByteArrayLiteral("id=") + albumIdByPath(m_path);

    m_conn->sendRequest(QByteArrayLiteral("SYNO.PhotoStation.Album"), formData, [=](QNetworkReply* /*reply*/, const QJsonDocument& json) {
        QJsonValue jsonSuccess = json.object()["success"];

    }, [=](QNetworkReply* /*reply*/, const QJsonDocument& /*json*/) {


    });
}

SynoAlbum* SynoAlbum::getDescendantAlbum(const QString& name)
{
    SynoAlbum* album = new SynoAlbum(m_conn, this);
    album->setPath(m_path + '/' + name);
    return album;
}

SynoAlbum* SynoAlbum::getAncestorAlbum()
{
    QString parentPath;
    int pos = m_path.lastIndexOf('/');
    if (pos != -1) {
        parentPath = m_path.mid(0, pos - 1);
    }

    SynoAlbum* album = new SynoAlbum(m_conn, this);
    album->setPath(parentPath);
    return album;
}

QString SynoAlbum::path() const
{
    return m_path;
}

void SynoAlbum::setPath(const QString& path)
{
    QString normPath = normalizedPath(path);
    if (normPath != path) {
        qWarning() << QStringLiteral(__FUNCTION__) << QStringLiteral("Provided path is not normalized. ")
                   << QStringLiteral("Converting to normalized: ") << normPath;
    }

    if (normPath != m_path) {
        m_path = normPath;
        emit pathChanged();
    }
}

int SynoAlbum::offset() const
{
    return m_offset;
}

void SynoAlbum::setOffset(int offt)
{
    if (offt != m_offset) {
        m_offset = offt;
        emit offsetChanged();
    }
}

int SynoAlbum::batchSize() const
{
    return m_batchSize;
}

void SynoAlbum::setBatchSize(int size)
{
    if (size != m_batchSize) {
        m_batchSize = size;
        emit batchSizeChanged();
    }
}

QString SynoAlbum::normalizedPath(const QString& path)
{
    QString pathMutable = path;
    pathMutable.replace('\\', '/');
    return simplified_helper(pathMutable, '/');
}

QByteArray SynoAlbum::albumIdByPath(const QString& path)
{
    if (path.isEmpty()) {
        return QByteArray();
    }

    QByteArray id = QByteArrayLiteral("album_") + path.toUtf8().toHex();
    return id;
}

QString SynoAlbum::pathByAlbumId(const QByteArray& albumId)
{
    if (!albumId.startsWith(QByteArrayLiteral("album_"))) {
        return QString();
    }

    constexpr int prefixLen = const_str_length("album_");
    QByteArray rawHex = QByteArray::fromRawData(albumId.data() + prefixLen, albumId.size() - prefixLen);
    QByteArray utf8 = QByteArray::fromHex(rawHex);
    QString path = QString::fromUtf8(utf8);
    return path;
}
