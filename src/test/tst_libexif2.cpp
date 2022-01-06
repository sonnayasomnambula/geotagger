#include <QCoreApplication>
#include <QDirIterator>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

#include <gtest/gtest.h>



#include "gpx/loader.h"
#include "gpx/saver.h"
#include "gpx/libexif.h"
#include "tmpjpegfile.h"

using GPX::Loader;
using GPX::Saver;

namespace EXIF {
namespace TAG {
namespace GPS {
static const ExifTag LATITUDE      = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE);
static const ExifTag LONGITUDE     = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE);
static const ExifTag ALTITUDE      = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE);
static const ExifTag LATITUDE_REF  = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF);
static const ExifTag LONGITUDE_REF = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF);
static const ExifTag ALTITUDE_REF  = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF);
}
}
}


TEST(libexif, save_load)
{
    QString jpeg = TmpJpegFile::withoutGps();
//    QString jpeg = TmpJpegFile::withoutExif();
    ASSERT_FALSE(jpeg.isEmpty());

    double lat = 58.7203335774538746;

    auto generated = Saver::toExifLatitude(lat);

    ASSERT_EQ(58u, generated[0].first);
    ASSERT_EQ(1u,  generated[0].second);
    ASSERT_EQ(43u, generated[1].first);
    ASSERT_EQ(1u, generated[1].second);
    ASSERT_EQ(132009u, generated[2].first);
    ASSERT_EQ(10000u, generated[2].second);

    {
        // save
        LibExif exif;
        ASSERT_TRUE(exif.load(jpeg));
        exif.setValue(EXIF_IFD_GPS, EXIF::TAG::GPS::LATITUDE, generated);
        exif.setValue(EXIF_IFD_GPS, EXIF::TAG::GPS::LATITUDE_REF, "N");
        ASSERT_TRUE(exif.save(jpeg));
    }

    {
        // load
        LibExif exif;
        ASSERT_TRUE(exif.load(jpeg));

        auto loaded = exif.uRationalVector(EXIF_IFD_GPS, EXIF::TAG::GPS::LATITUDE);
        ASSERT_EQ(3, loaded.size());

        EXPECT_EQ(generated[0].first, loaded[0].first);
        EXPECT_EQ(generated[0].second, loaded[0].second);
        EXPECT_EQ(generated[1].first, loaded[1].first);
        EXPECT_EQ(generated[1].second, loaded[1].second);
        EXPECT_EQ(generated[2].first, loaded[2].first);
        EXPECT_EQ(generated[2].second, loaded[2].second);
    }
}
