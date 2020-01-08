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

#include "qmlobjectwrapper.h"

QmlObjectWrapper::QmlObjectWrapper(std::shared_ptr<QObject> objectPtr)
    : m_objectPtr(objectPtr)
{
}

QObject* QmlObjectWrapper::object() const
{
    return m_objectPtr.get();
}

std::shared_ptr<QObject> QmlObjectWrapper::share() const
{
    return m_objectPtr;
}

void QmlObjectWrapper::reset(std::shared_ptr<QObject> objectPtr)
{
    if (m_objectPtr != objectPtr) {
        m_objectPtr = objectPtr;
        emit objectChanged();
    }
}

void QmlObjectWrapper::clear()
{
    if (m_objectPtr) {
        m_objectPtr.reset();
        emit objectChanged();
    }
}
