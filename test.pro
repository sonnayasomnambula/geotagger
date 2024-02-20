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
    src/exif/file.cpp \
    src/exif/utils.cpp \
    src/gpx/loader.cpp \
    src/gpx/statistic.cpp \
    src/test/tmpjpegfile.cpp \
    src/test/tst_libexif.cpp \
    src/test/tst_libexif_trivial.cpp

HEADERS += \
    src/exif/file.h \
    src/exif/utils.h \
    src/gpx/loader.h \
    src/gpx/statistic.h \
    src/gpx/track.h \
    src/test/tmpjpegfile.h

RESOURCES += \
    rsc/test.qrc
