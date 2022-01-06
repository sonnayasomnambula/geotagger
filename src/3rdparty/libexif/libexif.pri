unix:!macx: LIBS += -L$$PWD/lib/ -lexif

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libexif.a

HEADERS += \
    $$PWD/include/libexif/_stdint.h \
    $$PWD/include/libexif/exif-byte-order.h \
    $$PWD/include/libexif/exif-content.h \
    $$PWD/include/libexif/exif-data-type.h \
    $$PWD/include/libexif/exif-data.h \
    $$PWD/include/libexif/exif-entry.h \
    $$PWD/include/libexif/exif-format.h \
    $$PWD/include/libexif/exif-ifd.h \
    $$PWD/include/libexif/exif-loader.h \
    $$PWD/include/libexif/exif-log.h \
    $$PWD/include/libexif/exif-mem.h \
    $$PWD/include/libexif/exif-mnote-data.h \
    $$PWD/include/libexif/exif-tag.h \
    $$PWD/include/libexif/exif-utils.h
