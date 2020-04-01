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

#include <memory>

#include <QAbstractListModel>
#include <QQmlEngine>

#include "synoalbumdata.h"

class SynoConn;

class SynoAlbum : public QAbstractListModel
{
    Q_OBJECT

    QML_ELEMENT
    QML_UNCREATABLE("Use SynoAlbumFactory to obtain an instance")

    Q_PROPERTY(SynoAlbumData synoData READ synoData NOTIFY synoDataChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(QByteArray id READ id NOTIFY idChanged)
    Q_PROPERTY(bool hasParent READ hasParent NOTIFY hasParentChanged)
    Q_PROPERTY(int batchSize READ batchSize WRITE setBatchSize NOTIFY batchSizeChanged)

public:
    enum SynoAlbumRoles
    {
        RoleSynoData = Qt::UserRole + 1
    };
    Q_ENUM(SynoAlbumRoles)

public:
    SynoAlbum(SynoConn* conn, const SynoAlbumData& synoData, QObject* parent = nullptr);
    SynoAlbum(SynoConn* conn, const QString& path, QObject* parent = nullptr);
    ~SynoAlbum();

    /*!
     * \brief This method returns album data for the specified index.
     *
     * \param index Index of the data to be returned
     *
     * \returns Album data for the index, or empty data
     */
    Q_INVOKABLE SynoAlbumData get(int index) const;
    SynoAlbumData* getPtr(int index);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    int batchSize() const;
    void setBatchSize(int size);

    const SynoAlbumData& synoData() const;
    const QString& path() const;
    const QByteArray& id() const;
    bool hasParent() const;

    static QString normalizedPath(const QString& path);
    static QByteArray albumIdByPath(const QString& path);
    static QString pathByAlbumId(const QByteArray& albumId);

signals:
    void synoDataChanged();
    void pathChanged();
    void idChanged();
    void hasParentChanged();
    void batchSizeChanged();

public slots:
    void clear();
    void refresh(bool force);

private:
    void load(int offset);
    void loadInfo();
    void resetSize(int size);

private:
    SynoConn *m_conn;
    std::unique_ptr<SynoAlbumData> m_selfData;
    QVector<SynoAlbumData*> m_descendantData;
    QString m_path;
    QByteArray m_id;
    int m_batchSize;
};

#endif // SYNOALBUM_H
