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

#ifndef SYNOALBUMDATA_H
#define SYNOALBUMDATA_H

#include <QObject>
#include <QSize>

struct SynoAlbumData
{
    Q_GADGET

    Q_PROPERTY(QByteArray id MEMBER id)
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString path MEMBER path)

    Q_PROPERTY(QString sharepath MEMBER sharepath)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(int hits MEMBER hits)
    Q_PROPERTY(QString info_type MEMBER info_type)
    Q_PROPERTY(bool conversion MEMBER conversion)
    Q_PROPERTY(bool allow_comment MEMBER allow_comment)
    Q_PROPERTY(bool allow_embed MEMBER allow_embed)

    Q_PROPERTY(bool perm_browse MEMBER perm_browse)
    Q_PROPERTY(bool perm_upload MEMBER perm_upload)
    Q_PROPERTY(bool perm_manage MEMBER perm_manage)
    Q_PROPERTY(QString file_location MEMBER file_location)
    Q_PROPERTY(QSize thumb_preview_size MEMBER thumb_preview_size)
    Q_PROPERTY(int thumb_preview_mtime MEMBER thumb_preview_mtime)
    Q_PROPERTY(QSize thumb_small_size MEMBER thumb_small_size)
    Q_PROPERTY(int thumb_small_mtime MEMBER thumb_small_mtime)
    Q_PROPERTY(QSize thumb_large_size MEMBER thumb_large_size)
    Q_PROPERTY(int thumb_large_mtime MEMBER thumb_large_mtime)
    Q_PROPERTY(QString thumb_sig MEMBER thumb_sig)
    Q_PROPERTY(QStringList thumbnail_status MEMBER thumbnail_status)

public:
    void readFrom(const QJsonObject& albumDataObject);

    bool operator==(const SynoAlbumData& o) const;
    bool operator!=(const SynoAlbumData& o) const {
        return !(*this == o);
    }

public:
    QByteArray id;
    QString name;
    QString path;
    QString sharepath;
    QString title;
    QString description;
    int hits;
    QString info_type;
    bool conversion;
    bool allow_comment;
    bool allow_embed;
    QString type;
    bool perm_browse;
    bool perm_upload;
    bool perm_manage;
    QString file_location;
    QSize thumb_preview_size;
    int thumb_preview_mtime;
    QSize thumb_small_size;
    int thumb_small_mtime;
    QSize thumb_large_size;
    int thumb_large_mtime;
    QString thumb_sig;
    QStringList thumbnail_status;
};

Q_DECLARE_METATYPE(SynoAlbumData)

#endif // SYNOALBUMDATA_H
