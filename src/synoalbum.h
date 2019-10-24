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
    ~SynoAlbum();

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
