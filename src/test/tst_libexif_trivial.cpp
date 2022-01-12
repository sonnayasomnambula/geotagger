#include <QDebug>
#include <QString>

#include <gtest/gtest.h>

#include <libexif/exif-data.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-format.h>

#include "tmpjpegfile.h"

TEST(libexif, DISABLED_trivial)
{
    QString jpeg = TmpJpegFile::withoutGps();
    ASSERT_FALSE(jpeg.isEmpty());

    ExifData* d = exif_data_new_from_file(jpeg.toStdString().c_str());
    ASSERT_TRUE(d);

    exif_data_foreach_content(d, [](ExifContent* content, void*){
        ExifIfd ifd = exif_content_get_ifd(content);
        qDebug("IFD [%d] (%s)\n", ifd, exif_ifd_get_name(ifd));
        exif_content_foreach_entry(content, [](ExifEntry* entry, void*){
            char buf[2000];
            exif_entry_get_value(entry, buf, sizeof(buf));
            qDebug("TAG %s (0x%04X, %s)\n"
               "  Size: %d, %d components\n"
               "  Value: %s\n",
               exif_tag_get_name(entry->tag),
               entry->tag,
               exif_format_get_name(entry->format),
               entry->size,
               (int)(entry->components),
               exif_entry_get_value(entry, buf, sizeof(buf)));
        }, nullptr);
    }, nullptr);
}
