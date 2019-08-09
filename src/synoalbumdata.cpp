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

#include "synoalbumdata.h"

#include <QJsonObject>
#include <QMetaProperty>

static inline
std::tuple<QSize, int> readThumbInfo(const QJsonObject& albumSizeDataObject) {
    QSize size(albumSizeDataObject[QStringLiteral("resolutionx")].toInt(),
               albumSizeDataObject[QStringLiteral("resolutiony")].toInt());
    int mtime = albumSizeDataObject[QStringLiteral("mtime")].toInt();
    return std::make_tuple(size, mtime);
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
