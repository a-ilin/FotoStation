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

#ifndef COLORHANDLERCMS_P_H
#define COLORHANDLERCMS_P_H

#include "colorhandlercms.h"

#include "cache.h"
#include "synosettings.h"
#include "synotraits.h"

#include <QtConcurrent>
#include <QByteArray>
#include <QColorSpace>
#include <QDebug>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <cstdint>

#pragma warning(push)
#pragma warning(disable: 5033) // 'register' is no longer a supported storage class
#include <lcms2.h>
#pragma warning(pop)

struct TransformKey {
    bool operator== (const TransformKey& o) const {
        return iccSource == o.iccSource &&
               iccTarget == o.iccTarget &&
               cmsFormat == o.cmsFormat;
    }

    QByteArray iccSource;
    QByteArray iccTarget;
    uint32_t cmsFormat;
};

struct TransformValue {
    using TransformUPtr = std::unique_ptr<std::remove_pointer<cmsHTRANSFORM>::type, void(*)(cmsHTRANSFORM)>;

    TransformValue(std::vector<TransformUPtr>&& vecTransform)
        : vecTransform(std::move(vecTransform))
    {}

    std::vector<TransformUPtr> vecTransform;
};

inline uint32_t qHash(const TransformKey& key) {
    if (!key.cmsFormat ||
        key.iccSource.size() < sizeof(cmsICCHeader) ||
        key.iccTarget.size() < sizeof(cmsICCHeader)) {
        return 0;
    }

    auto csToKey = [](const QByteArray& icc) -> uint16_t {
        const cmsICCHeader* hdr = reinterpret_cast<const cmsICCHeader*>(icc.constData());
        return hdr->profileID.ID32[0] ^
               hdr->profileID.ID32[1] ^
               hdr->profileID.ID32[2] ^
               hdr->profileID.ID32[3];
    };

    uint32_t sourceKey = csToKey(key.iccSource);
    uint32_t targetKey = csToKey(key.iccTarget);

    uint16_t val = (sourceKey << 16) | (targetKey ^ key.cmsFormat);
    return val;
}

inline cmsUInt32Number qtFormatToCmsFormat(QImage::Format format)
{
    switch (format) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return TYPE_BGRA_8;
    case QImage::Format_BGR888:
        return TYPE_ARGB_8;
    }

    return 0;
}

class ColorHandlerCmsPrivate
{
    Q_DISABLE_COPY(ColorHandlerCmsPrivate)
    Q_DECLARE_PUBLIC(ColorHandlerCms)

public:
    using TransformPtr = std::shared_ptr<TransformValue>;
    using CacheType = Cache< TransformKey, TransformPtr >;

public:
    ColorHandlerCmsPrivate() {}

    static TransformPtr createTransform(const TransformKey& key);

    void doTransform(QImage& image, const TransformPtr& pTransform);

public:
    ColorHandlerCms* q_ptr = nullptr;
    QMutex cacheMutex;
    CacheType cache;
};

#endif // COLORHANDLERCMS_P_H
