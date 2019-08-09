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

#ifndef SYNOALBUM_H
#define SYNOALBUM_H

#include <QAbstractListModel>

#include "synoalbumdata.h"

class SynoConn;

class SynoAlbum : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(SynoAlbumData synoData READ synoData WRITE setSynoData NOTIFY synoDataChanged)

    Q_PROPERTY(int batchSize READ batchSize WRITE setBatchSize NOTIFY batchSizeChanged)

public:
    enum SynoAlbumRoles
    {
        RoleSynoData = Qt::UserRole + 1
    };
    Q_ENUM(SynoAlbumRoles)

public:
    explicit SynoAlbum(SynoConn *conn, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    Q_INVOKABLE SynoAlbum* getDescendantAlbum(int index);

    QString path() const;
    void setPath(const QString& path);

    int batchSize() const;
    void setBatchSize(int size);

    const SynoAlbumData& synoData() const;
    void setSynoData(const SynoAlbumData& synoData);

signals:
    void synoDataChanged();
    void pathChanged();
    void offsetChanged();
    void batchSizeChanged();

public slots:
    void clear();
    void refresh();

private:
    void load(int offset);

    static QString normalizedPath(const QString& path);
    static QByteArray albumIdByPath(const QString& path);
    static QString pathByAlbumId(const QByteArray& albumId);

private:
    SynoConn *m_conn;
    SynoAlbumData m_selfData;
    QVector<SynoAlbumData*> m_descendantData;
    QString m_path;
    int m_batchSize;
};

#endif // SYNOALBUM_H
