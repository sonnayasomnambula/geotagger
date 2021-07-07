QT       += core gui widgets quick location positioning quickwidgets

CONFIG += c++11

SOURCES += \
    src/gpx/loader.cpp \
    src/gpx/track.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/pathcontroller.cpp \
    src/xml/xmlnode.cpp \
    src/xml/xmlnodereader.cpp

HEADERS += \
    src/gpx/loader.h \
    src/gpx/track.h \
    src/mainwindow.h \
    src/pathcontroller.h \
    src/xml/xmlnode.h \
    src/xml/xmlnodereader.h

FORMS += \
    src/mainwindow.ui

INCLUDEPATH += \
    src

TRANSLATIONS += \
    rsc/en_US.ts

DISTFILES += \
    qml/map.qml

RESOURCES += \
    rsc/resources.qrc
