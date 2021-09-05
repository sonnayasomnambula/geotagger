#include <QCoreApplication>
#include <QDirIterator>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

#include <gtest/gtest.h>

#include <qexifimageheader.h>

#include "gpx/loader.h"
#include "gpx/saver.h"
#include "tmpjpegfile.h"

using GPX::Loader;
using GPX::Saver;

static const auto GpsLatitude       = QExifImageHeader::GpsLatitude;
static const auto GpsLongitude      = QExifImageHeader::GpsLongitude;
static const auto GpsAltitude       = QExifImageHeader::GpsAltitude;
static const auto GpsLatitudeRef    = QExifImageHeader::GpsLatitudeRef;
static const auto GpsLongitudeRef   = QExifImageHeader::GpsLongitudeRef;
static const auto GpsAltitudeRef    = QExifImageHeader::GpsAltitudeRef;

TEST(QExifImageHeader, save_load)
{
    QString jpeg = TmpJpegFile::instance();
    ASSERT_FALSE(jpeg.isEmpty()) << qPrintable(TmpJpegFile::lastError());

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
        QExifImageHeader exif;
        exif.setValue(GpsLatitude, generated);
        ASSERT_TRUE(exif.saveToJpeg(jpeg));
    }

    {
        // load
        QExifImageHeader exif;
        ASSERT_TRUE(exif.loadFromJpeg(jpeg));
        ASSERT_TRUE(exif.contains(GpsLatitude));

        auto loaded = exif.value(GpsLatitude).toRationalVector();
        ASSERT_EQ(3, loaded.size());

        EXPECT_EQ(generated[0].first, loaded[0].first);
        EXPECT_EQ(generated[0].second, loaded[0].second);
        EXPECT_EQ(generated[1].first, loaded[1].first);
        EXPECT_EQ(generated[1].second, loaded[1].second);
        EXPECT_EQ(generated[2].first, loaded[2].first);
        EXPECT_EQ(generated[2].second, loaded[2].second);
    }
}
