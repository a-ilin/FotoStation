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

#include "colorhandlercms_p.h"

bool ColorHandlerCms::convertColorSpace(QImage& image, const QColorSpace& source, const QColorSpace& target)
{
    Q_D(ColorHandlerCms);

    const cmsUInt32Number cmsFormat = qtFormatToCmsFormat(image.format());
    if (0 == cmsFormat) {
#ifdef QT_DEBUG
        qDebug() << QStringLiteral("Unknown image format, color is not converted: ") << image.format();
#endif
        return false;
    }

    // NOTE: icc profiles should be checked, but not QColorSpace
    //       because of QColorSpace does not support some valid icc profiles
    const QByteArray iccSource = source.iccProfile();
    const QByteArray iccTarget = target.iccProfile();
    if (iccSource.size() < sizeof(cmsICCHeader) ||
        iccTarget.size() < sizeof(cmsICCHeader) ||
        iccSource == iccTarget) {
        // formats are empty, or the same, do nothing
        return false;
    }

    TransformKey key{iccSource, iccTarget, cmsFormat};

    // do not release mutex until conversion is finished
    QMutexLocker locker(&d->cacheMutex);

    std::shared_ptr<TransformValue> pTransform = d->cache.object(key);

    if (!pTransform) {
        pTransform = d->createTransform(key);

        if (!pTransform) {
            return false;
        }

        d->cache.insert(key, pTransform, 1);
    }

    d->doTransform(image, pTransform);

    image.setColorSpace(target);

    return true;
}

ColorHandlerCms& ColorHandlerCms::instance()
{
    static ColorHandlerCms colorHandlerCms;
    return colorHandlerCms;
}

ColorHandlerCms::ColorHandlerCms()
    : d_ptr(new ColorHandlerCmsPrivate())
{
    d_ptr->q_ptr = this;

    SynoSettings settings(QStringLiteral("performance"));
    int maximumCountItems = qBound(0, settings.value(QStringLiteral("cmsTransformCacheItems"), 50).toInt(), std::numeric_limits<int>::max());

    d_ptr->cache = ColorHandlerCmsPrivate::CacheType(maximumCountItems);
}

ColorHandlerCmsPrivate::TransformPtr ColorHandlerCmsPrivate::createTransform(const TransformKey& key)
{
    cmsHPROFILE hSourceProfile = cmsOpenProfileFromMem(key.iccSource.data(), key.iccSource.size());
    cmsHPROFILE hTargetProfile = cmsOpenProfileFromMem(key.iccTarget.data(), key.iccTarget.size());

    // NOTE: transform object is not thread-safe, need to duplicate
    int partitions = QThread::idealThreadCount();
    std::vector<TransformValue::TransformUPtr> vecPtr;
    vecPtr.reserve(partitions);

    cmsHTRANSFORM hTransform = nullptr;

    for (int i = 0; i < partitions; ++i) {
        hTransform = cmsCreateTransform(hSourceProfile,
                                        key.cmsFormat,
                                        hTargetProfile,
                                        key.cmsFormat,
                                        INTENT_PERCEPTUAL, 0);

        if (!hTransform) {
            break;
        }

        vecPtr.push_back(std::move(TransformValue::TransformUPtr(hTransform,
                                                [](cmsHTRANSFORM hTransform) {
                  ::cmsDeleteTransform(hTransform);
        })));
    }

    cmsCloseProfile(hSourceProfile);
    cmsCloseProfile(hTargetProfile);

    return hTransform ? std::make_shared<TransformValue>(std::move(vecPtr)) : nullptr;
}

void ColorHandlerCmsPrivate::doTransform(QImage& image, const ColorHandlerCmsPrivate::TransformPtr& pTransform)
{
    struct WorkLoad {
        int begin;
        int end;
        cmsHTRANSFORM hTransform;
    };

    Q_ASSERT(pTransform);
    Q_ASSERT(!pTransform->vecTransform.empty());

    const int partitions = pTransform->vecTransform.size();
    uchar* const bits = image.bits();
    const int bytesPerLine = image.bytesPerLine();
    const int height = image.height();
    const int width = image.width();

    auto worker = [bits, bytesPerLine, width](WorkLoad work) {
        for (int i = work.begin; i < work.end; ++i) {
            cmsDoTransform(work.hTransform, bits + i * bytesPerLine, bits + i * bytesPerLine, width);
        }
    };

    if (height < partitions) {
        WorkLoad work{0, height, pTransform->vecTransform[0].get()};
        worker(work);
    } else {
        // partitions equals to number of CPUs
        WorkLoad* pWorkArray = (WorkLoad*)alloca(sizeof(WorkLoad) * partitions);

        for (int i = 0; i < partitions - 1; ++i) {
            pWorkArray[i] = WorkLoad{height / partitions * i,
                                     height / partitions * (i + 1),
                                     pTransform->vecTransform[i].get()};
        }
        pWorkArray[partitions - 1] =
                WorkLoad{height / partitions * (partitions - 1),
                         height,
                         pTransform->vecTransform[partitions - 1].get()};

        QtConcurrent::blockingMap(&pWorkArray[0], &pWorkArray[partitions], worker);
    }
}
