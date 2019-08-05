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

#include <QObject>

class SynoConn;

// TODO: Inherit from QAbstractItemModel
class SynoAlbum : public QObject
{
    Q_OBJECT

    Q_PROPERTY (QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY (int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY (int batchSize READ batchSize WRITE setBatchSize NOTIFY batchSizeChanged)

public:
    explicit SynoAlbum(SynoConn *conn, QObject *parent = nullptr);

    Q_INVOKABLE void list();

    Q_INVOKABLE SynoAlbum* getDescendantAlbum(const QString& name);
    Q_INVOKABLE SynoAlbum* getAncestorAlbum();

    QString path() const;
    void setPath(const QString& path);

    int offset() const;
    void setOffset(int offt);

    int batchSize() const;
    void setBatchSize(int size);

signals:
    void pathChanged();
    void offsetChanged();
    void batchSizeChanged();

public slots:

private:
    static QString normalizedPath(const QString& path);
    static QByteArray albumIdByPath(const QString& path);
    static QString pathByAlbumId(const QByteArray& albumId);

private:
    SynoConn *m_conn;
    QString m_path;
    int m_offset;
    int m_batchSize;
};

#endif // SYNOALBUM_H
