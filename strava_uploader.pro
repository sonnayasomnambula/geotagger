QT       += core gui widgets quick location positioning quickwidgets

CONFIG += c++17

SOURCES += \
    src/3rdparty/sigvdr.de/qexifimageheader.cpp \
    src/gpx/loader.cpp \
    src/gpx/track.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/model.cpp \
    src/pixmaplabel.cpp \
    src/selectionwatcher.cpp \
    src/timeadjustwidget.cpp \

HEADERS += \
    src/3rdparty/sigvdr.de/qexifimageheader.h \
    src/abstractsettings.h \
    src/gpx/loader.h \
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
