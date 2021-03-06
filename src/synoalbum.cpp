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
#include "synosettings.h"

#include <QDebug>
#include <QJsonArray>
#include <QMetaEnum>
#include <QTimer>

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

SynoAlbum::SynoAlbum(SynoConn* conn, const SynoAlbumData& synoData, QObject* parent)
    : QAbstractListModel(parent)
    , m_conn(conn)
    , m_selfData(synoData.isNull() ? nullptr : new SynoAlbumData(synoData))
    , m_path(synoData.path)
    , m_id(albumIdByPath(m_path))
{
    SynoSettings settings("performance");
    m_batchSize = qBound(1, settings.value("albumBatchSize", 50).toInt(), std::numeric_limits<int>::max());

    // TBD: implement path, id, hasParent change on album move
}

SynoAlbum::SynoAlbum(SynoConn *conn, const QString& path, QObject *parent)
    : SynoAlbum(conn, SynoAlbumData::null, parent)
{
    m_path = path;
    m_id = albumIdByPath(path);
}

SynoAlbum::~SynoAlbum()
{
    clear();
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
    if (!index.isValid()) {
        return QVariant();
    }

    SynoAlbumData* pSynoData = const_cast<SynoAlbum*>(this)->getPtr(index.row());
    if (!pSynoData) {
        return QVariant();
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

SynoAlbumData SynoAlbum::get(int index) const
{
    SynoAlbumData* pSynoData = const_cast<SynoAlbum*>(this)->getPtr(index);
    return pSynoData ? *pSynoData : SynoAlbumData::null;
}

SynoAlbumData* SynoAlbum::getPtr(int index)
{
    if (index < 0 || index >= m_descendantData.size()) {
        return nullptr;
    }

    if (!m_descendantData[index]) {
        // allocate data according to batch size
        int begin = index - (index % m_batchSize);
        int end = std::min(begin + m_batchSize, m_descendantData.size());
        for (int i = begin; i < end; ++i) {
            if (!m_descendantData[i]) {
                m_descendantData[i] = new SynoAlbumData();
            }
        }

        // send request
        load(begin);
    }

    return m_descendantData[index];
}

void SynoAlbum::clear()
{
    if (m_descendantData.size()) {
        resetSize(0);
    }
}

void SynoAlbum::refresh(bool force)
{
    if (force || !m_selfData || !m_descendantData.size()) {
        loadInfo();
        clear();
        load(0);
    }
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
    formData << QByteArrayLiteral("id=") + m_id;

    std::shared_ptr<SynoRequest> req = m_conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Album"), formData);
    req->send(this, [this, offset, req] {
        if (req->errorString().isEmpty()) {
            SynoReplyJSON replyJSON(req.get());

            int total = replyJSON.dataObject()[QStringLiteral("total")].toInt();
            if (total < 0) {
                qWarning() << __FUNCTION__ << tr("Negative total value received: ") << total;
                total = 0;
            }

            if (m_descendantData.size() != total) {
                resetSize(total);
            }

            QJsonArray items = replyJSON.dataObject()[QStringLiteral("items")].toArray();
            if (offset + items.size() <= total) {
                int i = offset;
                for (auto iter = items.cbegin(); iter != items.cend(); ++iter, ++i) {
                    if (!m_descendantData[i]) {
                        m_descendantData[i] = new SynoAlbumData();
                    }

                    m_descendantData[i]->readFrom(iter->toObject());
                }

                emit dataChanged(index(offset), index(offset + items.size() - 1));
            } else {
                qWarning() << __FUNCTION__ << tr("Too much items received: ") << items.size();
            }
        } else {
            qWarning() << __FUNCTION__ << tr("Error during retrieving album data. %1").arg(req->errorString());
        }
    });
}

void SynoAlbum::loadInfo()
{
    QByteArrayList formData;
    formData << QByteArrayLiteral("method=getinfo");
    formData << QByteArrayLiteral("version=1");
    formData << QByteArrayLiteral("additional=album_permission,photo_exif,video_codec,video_quality,thumb_size,file_location");
    formData << QByteArrayLiteral("id=") + m_id;

    std::shared_ptr<SynoRequest> req = m_conn->createRequest(QByteArrayLiteral("SYNO.PhotoStation.Album"), formData);
    req->send(this, [this, req] {
        if (req->errorString().isEmpty()) {
            SynoReplyJSON replyJSON(req.get());
            QJsonArray items = replyJSON.dataObject()[QStringLiteral("items")].toArray();
            if (items.size()) {
                m_selfData.reset(new SynoAlbumData());
                m_selfData->readFrom(items[0].toObject());
                emit synoDataChanged();
            } else {
                qWarning() << __FUNCTION__ << tr("Error during retrieving album data. %1").arg(replyJSON.errorString());
            }
        } else {
            qWarning() << __FUNCTION__ << tr("Error during retrieving album data. %1").arg(req->errorString());
        }
    });
}

void SynoAlbum::resetSize(int size)
{
    beginResetModel();
    qDeleteAll(m_descendantData);
    m_descendantData.resize(size);
    endResetModel();
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
    return m_selfData ? *m_selfData : SynoAlbumData::null;
}

const QString& SynoAlbum::path() const
{
    return m_path;
}

const QByteArray& SynoAlbum::id() const
{
    return m_id;
}

bool SynoAlbum::hasParent() const
{
    return m_path.size() > 0;
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
