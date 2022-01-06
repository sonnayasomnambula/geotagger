QT -= gui
QT += location positioning

CONFIG += c++11 console
CONFIG -= app_bundle

GOOGLETEST_DIR = src/test/google
include(google_dependency.pri)

include(src/3rdparty/libexif/libexif.pri)
include(src/3rdparty/libjpeg/libjpeg.pri)

INCLUDEPATH += \
    src \
    src/3rdparty/sigvdr.de

SOURCES += \
    src/3rdparty/sigvdr.de/qexifimageheader.cpp \
    src/gpx/libexif.cpp \
    src/gpx/loader.cpp \
    src/gpx/saver.cpp \
    src/test/tmpjpegfile.cpp \
    src/test/tst_libexif.cpp \
    src/test/tst_libexif2.cpp \
    src/test/tst_qexifimageheade.cpp

HEADERS += \
    src/3rdparty/sigvdr.de/qexifimageheader.h \
    src/gpx/libexif.h \
    src/gpx/loader.h \
    src/gpx/saver.h \
    src/gpx/track.h \
    src/test/tmpjpegfile.h

RESOURCES += \
    rsc/test.qrc
