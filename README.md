# FotoStation
###### Desktop client for Synology(R) Photo Station

This project is an attempt to implement desktop client for Synology(R) Photo Station service.
The application is in very early development stage. It requires much love.

The reason this project has been started is improving browsing experience for huge photo sets (especially actual for HDD-based NAS setups): 
- improving launch time of browsing UI
- improving loading speed of images
- eliminating lags of user interface

Another reason is implementation of desktop-based encoding for images & video during upload to NAS.
The official upload tool exists for Windows only, and does not support some new video formats.

## How to build

Currently the supported environment is Windows + MSVC. Other platforms are expected to be added later.

1. Install Qt SDK for Desktop apps: [https://www.qt.io/]

2. Follow usual steps to build Qt-based applications:
```bat
cd FotoStation
qmake
nmake
```

## License

**The author is not related to Synology Inc.**
**This is a personal project. No official support is provided.**
**Please read license agreement carefully.**

[**GPL**](/LICENSE?raw=true)

## 3rd party components

  The application is based on Qt framework, which licensing includes GPL: [https://www.qt.io/licensing/]

  The application uses some visual assets provided under different licenses.
  - Flaticon license by [shareicon.net]:
    [https://file000.flaticon.com/downloads/license/license.pdf]

  - Free for non-commercial use by [preloaders.net]:
    [https://icons8.com/preloaders/en/terms_of_use]
