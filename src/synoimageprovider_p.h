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


#ifndef SYNOIMAGEPROVIDER_P_H
#define SYNOIMAGEPROVIDER_P_H

#include <QQuickImageResponse>

#include "synorequest.h"

class SynoImageResponse : public QQuickImageResponse
{
public:
    SynoImageResponse(const QSize& size);

    void loadThumb(const QByteArray& id);

    QQuickTextureFactory* textureFactory() const override;
    QString errorString() const override;
    void cancel() override;

protected:
    void processImage();

protected:
    QSize m_size;
    QString m_errorString;
    QImage m_image;
    std::shared_ptr<SynoRequest> m_req;
};

#endif // SYNOIMAGEPROVIDER_P_H
