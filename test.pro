QT -= gui
QT += location positioning

CONFIG += c++11 console
CONFIG -= app_bundle

GOOGLETEST_DIR = src/test/google
include(google_dependency.pri)

INCLUDEPATH += \
    src \
    src/3rdparty/sigvdr.de

SOURCES += \
    src/3rdparty/sigvdr.de/qexifimageheader.cpp \
    src/gpx/loader.cpp \
    src/gpx/saver.cpp \
    src/test/tst_qexifimageheade.cpp

HEADERS += \
    src/3rdparty/sigvdr.de/qexifimageheader.h \
    src/gpx/loader.h \
    src/gpx/saver.h \
    src/gpx/track.h
