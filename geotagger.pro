QT       += core gui widgets quick location positioning quickwidgets

CONFIG += c++17

include(src/3rdparty/libexif/libexif.pri)
include(src/3rdparty/libjpeg/libjpeg.pri)

SOURCES += \
    src/exif.cpp \
    src/gpx/loader.cpp \
    src/gpx/statistic.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/model.cpp \
    src/pixmaplabel.cpp \
    src/selectionwatcher.cpp \
    src/timeadjustwidget.cpp \

HEADERS += \
    src/abstractsettings.h \
    src/exif.h \
    src/gpx/loader.h \
    src/gpx/statistic.h \
    src/gpx/track.h \
    src/mainwindow.h \
    src/model.h \
    src/pixmaplabel.h \
    src/selectionwatcher.h \
    src/timeadjustwidget.h \

FORMS += \
    src/mainwindow.ui \
    src/timeadjustwidget.ui

INCLUDEPATH += \
    src \
    src/3rdparty

TRANSLATIONS += \
    rsc/en_US.ts

DISTFILES += \
    qml/map.qml

RESOURCES += \
    rsc/resources.qrc
