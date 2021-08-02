QT       += core gui widgets quick location positioning quickwidgets

CONFIG += c++11

SOURCES += \
    src/3rdparty/sigvdr.de/qexifimageheader.cpp \
    src/gpx/loader.cpp \
    src/gpx/track.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/model.cpp \
    src/xml/xmlnode.cpp \
    src/xml/xmlnodereader.cpp

HEADERS += \
    src/3rdparty/sigvdr.de/qexifimageheader.h \
    src/abstractsettings.h \
    src/gpx/loader.h \
    src/gpx/track.h \
    src/mainwindow.h \
    src/model.h \
    src/xml/xmlnode.h \
    src/xml/xmlnodereader.h

FORMS += \
    src/mainwindow.ui

INCLUDEPATH += \
    src \
    src/3rdparty

TRANSLATIONS += \
    rsc/en_US.ts

DISTFILES += \
    qml/map.qml

RESOURCES += \
    rsc/resources.qrc
