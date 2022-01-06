# libjpeg taken from exifyay project
# https://github.com/NarrativeTeam/exifyay/tree/master/libjpeg
# this is modified libjpeg from libexif/exif utility
# https://github.com/libexif/exif/tree/master/libjpeg


HEADERS += \
    $$PWD/jpeg-data.h \
    $$PWD/jpeg-marker.h

SOURCES += \
    $$PWD/jpeg-data.c \
    $$PWD/jpeg-marker.c

INCLUDEPATH += $$PWD/..
