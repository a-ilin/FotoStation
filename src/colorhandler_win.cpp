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

#include "colorhandler_p.h"

#include <memory>

#include <QFile>

#include <Windows.h>
#include <wingdi.h>

void ColorHandlerPrivate::readPlatformIccProfile()
{
    if (!appWindow) {
        return;
    }

    // get top level
    QWindow* topWnd = appWindow;
    while(QWindow* wndParent = appWindow->parent()) {
        topWnd = wndParent;
    }

    HWND hWnd = (HWND)topWnd->winId();
    if (!hWnd) {
        qWarning() << __FUNCTION__ << QStringLiteral("hWnd is NULL") << GetLastError();
        return;
    }

    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    if (!hMonitor) {
        qWarning() << __FUNCTION__ << QStringLiteral("hMonitor is NULL") << GetLastError();
        return;
    }

    BOOL res = FALSE;

    MONITORINFOEX monInfo;
    monInfo.cbSize = sizeof(monInfo);
    res = ::GetMonitorInfo(hMonitor, &monInfo);
    if (FALSE == res) {
        qWarning() << __FUNCTION__ << QStringLiteral("GetMonitorInfo failed") << GetLastError();
        return;
    }

    qDebug() << __FUNCTION__ << QStringLiteral("Active Monitor:") << QString::fromWCharArray(monInfo.szDevice);

    std::unique_ptr<std::remove_pointer<HDC>::type, void(*)(HDC)> hMonitorDC(
                ::CreateDC(monInfo.szDevice, NULL, NULL, NULL),
                [](HDC hDC) { ::DeleteDC(hDC); });

    if (!hMonitorDC) {
        qWarning() << __FUNCTION__ << QStringLiteral("hMonitorDC is NULL") << GetLastError();
        return;
    }

    DWORD iccPathBufSize = 0;
    ::GetICMProfile(hMonitorDC.get(), &iccPathBufSize, NULL);

    std::unique_ptr<TCHAR[]> iccPathPtr(new TCHAR[iccPathBufSize]);
    res = ::GetICMProfile(hMonitorDC.get(), &iccPathBufSize, iccPathPtr.get());
    if (FALSE == res) {
        qWarning() << __FUNCTION__ << QStringLiteral("GetICMProfile failed") << GetLastError();
        return;
    }

#ifdef _UNICODE
    QString iccPath = QString::fromWCharArray(iccPathPtr.get());
#else
    QString iccPath = QString::fromLocal8Bit(iccPathPtr.get());
#endif

    qDebug() << "Path to monitor ICC profile:" << iccPath;

    QFile iccFile(iccPath);
    if (!iccFile.open(QIODevice::ReadOnly)) {
        qWarning() << __FUNCTION__ << QStringLiteral("Cannot open ICC file for reading");
        return;
    }

    iccProfile = iccFile.readAll();
}
