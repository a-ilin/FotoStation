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

#include "synoalbum.h"
#include "synoconn.h"
#include "synoreplyjson.h"
#include "synorequest.h"

#include <QDebug>
#include <QJsonArray>
#include <QMetaEnum>

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
    : QAbstractListModel(parent)
    , m_conn(conn)
    , m_batchSize(50)
{

}

QHash<int, QByteArray> SynoAlbum::roleNames() const {
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(RoleSynoData, QByteArrayLiteral("synoData"));
    return roles;
}

int SynoAlbum::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_descendantData.size();
}

QVariant SynoAlbum::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 ||
        index.row() >= m_descendantData.size()) {
        return QVariant();
    }

    SynoAlbumData* pSynoData = m_descendantData[index.row()];
    if (!pSynoData) {
        return QString();
    }

    switch (role) {
    case Qt::DisplayRole:
        return pSynoData->name;
    case RoleSynoData:
        return QVariant::fromValue(*pSynoData);
    default:
        break;
    }

    return QVariant();
}

void SynoAlbum::clear()
{
    beginResetModel();
    qDeleteAll(m_descendantData);
    m_descendantData.clear();
    endResetModel();
}

void SynoAlbum::refresh()
{
    load(0);
}

void SynoAlbum::load(int offset)
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=list");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("type=album,photo,video");
    formData << QByteArrayLiteral("offset=") + QByteArray::number(offset);
    formData << QByteArrayLiteral("limit=") + QByteArray::number(m_batchSize);
    formData << QByteArrayLiteral("recursive=false");
    formData << QByteArrayLiteral("additional=album_permission,photo_exif,video_codec,video_quality,thumb_size,file_location");
    formData << QByteArrayLiteral("id=") + m_selfData.id;

    std::shared_ptr<SynoRequest> req = m_conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Album"), formData);
    req->send(this, [this, offset, req] {
        if (req->errorString().isEmpty()) {
            SynoReplyJSON replyJSON(req.get());
            int total = replyJSON.dataObject()[QStringLiteral("total")].toInt();
            if (!total) {
                return;
            }

            if (m_descendantData.size() && m_descendantData.size() != total) {
                clear();
                refresh();
                return;
            } else if (!m_descendantData.size()) {
                this->beginInsertRows(QModelIndex(), 0, total - 1);
                m_descendantData.resize(total);
                this->endInsertRows();
            }

            QJsonArray items = replyJSON.dataObject()[QStringLiteral("items")].toArray();
            if (items.size()) {
                if (offset + items.size() > total) {
                    qWarning() << QStringLiteral("Too much items received: ") << items.size();
                    return;
                }

                int i = offset;
                for (auto iter = items.cbegin(); iter != items.cend(); ++iter, ++i) {
                    SynoAlbumData* albumData = new SynoAlbumData();
                    albumData->readFrom(iter->toObject());
                    delete m_descendantData[i];
                    m_descendantData[i] = albumData;
                }
                emit this->dataChanged(index(offset), index(offset + items.size() - 1));

                if (offset + items.size() < total) {
                    load(offset + items.size());
                }
            } else {
                qWarning() << __FUNCTION__ << tr("Error during retrieving album data. %1").arg(replyJSON.errorString());
            }
        } else {
            qWarning() << __FUNCTION__ << tr("Error during retrieving album data. %1").arg(req->errorString());
        }
    });
}

SynoAlbum* SynoAlbum::getDescendantAlbum(int index)
{
    if (index < 0 || index >= m_descendantData.size()) {
        return nullptr;
    }

    SynoAlbumData* pSynoData = m_descendantData[index];
    if (!pSynoData) {
        return nullptr;
    }

    SynoAlbum* album = new SynoAlbum(m_conn);
    album->setSynoData(*pSynoData);
    return album;
}

QString SynoAlbum::path() const
{
    return m_selfData.path;
}

void SynoAlbum::setPath(const QString& path)
{
    QString normPath = normalizedPath(path);
    if (normPath != path) {
        qWarning() << __FUNCTION__
                   << QStringLiteral("Provided path is not normalized. ")
                   << QStringLiteral("Converting to normalized: ")
                   << normPath;
    }

    if (normPath != m_selfData.path) {
        m_selfData.path = normPath;
        emit pathChanged();
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

const SynoAlbumData& SynoAlbum::synoData() const
{
    return m_selfData;
}

void SynoAlbum::setSynoData(const SynoAlbumData& synoData)
{
    if (synoData != m_selfData) {
        m_selfData = synoData;
        emit synoDataChanged();
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
