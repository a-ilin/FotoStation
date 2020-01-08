/*
 * GNU General Public License (GPL)
 * Copyright (c) 2020 by Aleksei Ilin
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

#ifndef QMLOBJECTWRAPER_H
#define QMLOBJECTWRAPER_H

#include <QObject>

#include <memory>

class QmlObjectWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* object READ object NOTIFY objectChanged)

public:
    QmlObjectWrapper(std::shared_ptr<QObject> objectPtr = std::shared_ptr<QObject>());

    QObject* object() const;
    std::shared_ptr<QObject> share() const;

    void reset(std::shared_ptr<QObject> objectPtr);

public slots:
    void clear();

signals:
    void objectChanged();

protected:
    std::shared_ptr<QObject> m_objectPtr;
};

#endif // QMLOBJECTWRAPER_H
