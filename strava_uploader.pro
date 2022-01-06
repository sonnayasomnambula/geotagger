QT       += core gui widgets quick location positioning quickwidgets

CONFIG += c++17

include(src/3rdparty/libexif/libexif.pri)
include(src/3rdparty/libjpeg/libjpeg.pri)

SOURCES += \
    src/gpx/libexif.cpp \
    src/gpx/loader.cpp \
    src/gpx/saver.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/model.cpp \
    src/pixmaplabel.cpp \
    src/selectionwatcher.cpp \
    src/timeadjustwidget.cpp \

HEADERS += \
    src/abstractsettings.h \
    src/gpx/libexif.h \
    src/gpx/loader.h \
    src/gpx/saver.h \
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
