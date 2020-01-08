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

#include "synoalbumdata.h"

#include <QJsonObject>
#include <QMetaProperty>

const SynoAlbumData SynoAlbumData::null = SynoAlbumData();

static inline
std::tuple<QSize, int> readThumbInfo(const QJsonObject& albumSizeDataObject) {
    QSize size(albumSizeDataObject[QStringLiteral("resolutionx")].toInt(),
               albumSizeDataObject[QStringLiteral("resolutiony")].toInt());
    int mtime = albumSizeDataObject[QStringLiteral("mtime")].toInt();
    return std::make_tuple(size, mtime);
}

bool SynoAlbumData::isNull() const
{
    return (this == &null) || (*this == null);
}

void SynoAlbumData::readFrom(const QJsonObject& albumDataObject)
{
    id = albumDataObject[QStringLiteral("id")].toString().toLatin1();
    type = albumDataObject[QStringLiteral("type")].toString();
    thumbnail_status = albumDataObject[QStringLiteral("thumbnail_status")].toString().split(',', QString::SkipEmptyParts);

    QJsonObject infoObject = albumDataObject[QStringLiteral("info")].toObject();
    sharepath = infoObject[QStringLiteral("sharepath")].toString();
    name = infoObject[QStringLiteral("name")].toString();
    title = infoObject[QStringLiteral("title")].toString();
    description = infoObject[QStringLiteral("description")].toString();
    hits = infoObject[QStringLiteral("hits")].toInt();
    info_type = infoObject[QStringLiteral("type")].toString();
    conversion = infoObject[QStringLiteral("conversion")].toBool();
    allow_comment = infoObject[QStringLiteral("allow_comment")].toBool();
    allow_embed = infoObject[QStringLiteral("allow_embed")].toBool();

    QJsonObject additionalObject = albumDataObject[QStringLiteral("additional")].toObject();
    file_location = additionalObject[QStringLiteral("file_location")].toString();
    path = file_location;

    QJsonObject additionalAlbumPermissionObject = additionalObject[QStringLiteral("album_permission")].toObject();
    perm_browse = additionalAlbumPermissionObject[QStringLiteral("browse")].toBool();
    perm_upload = additionalAlbumPermissionObject[QStringLiteral("upload")].toBool();
    perm_manage = additionalAlbumPermissionObject[QStringLiteral("manage")].toBool();

    QJsonObject additionalThumbSizeObject = additionalObject[QStringLiteral("thumb_size")].toObject();
    thumb_sig = additionalThumbSizeObject[QStringLiteral("sig")].toString();

    QJsonObject additionalThumbSizePreviewObject = additionalThumbSizeObject[QStringLiteral("preview")].toObject();
    std::tie(thumb_preview_size, thumb_preview_mtime) = readThumbInfo(additionalThumbSizePreviewObject);

    QJsonObject additionalThumbSizeSmallObject = additionalThumbSizeObject[QStringLiteral("small")].toObject();
    std::tie(thumb_small_size, thumb_small_mtime) = readThumbInfo(additionalThumbSizeSmallObject);

    QJsonObject additionalThumbSizeLargeObject = additionalThumbSizeObject[QStringLiteral("large")].toObject();
    std::tie(thumb_large_size, thumb_large_mtime) = readThumbInfo(additionalThumbSizeLargeObject);
}

bool SynoAlbumData::operator==(const SynoAlbumData& o) const
{
    for (int i = 0; i < staticMetaObject.propertyCount(); ++i) {
        QMetaProperty prop = staticMetaObject.property(i);
        QVariant propValSelf = prop.readOnGadget(this);
        QVariant propValOther = prop.readOnGadget(&o);
        if (propValSelf != propValOther) {
            return false;
        }
    }

    return true;
}
