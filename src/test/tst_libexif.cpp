#include <QCoreApplication>
#include <QDirIterator>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

#include <gtest/gtest.h>

#include "exif.h"
#include "tmpjpegfile.h"


TEST(libexif, save_load)
{
    QString jpeg = TmpJpegFile::withoutExif();
    ASSERT_FALSE(jpeg.isEmpty()) << TmpJpegFile::lastError();

    double lat = 58.7203335774538746;

    auto generated = Exif::Utils::toDMS(lat);

    ASSERT_EQ(58u, generated[0].numerator);
    ASSERT_EQ(1u,  generated[0].denominator);
    ASSERT_EQ(43u, generated[1].numerator);
    ASSERT_EQ(1u, generated[1].denominator);
    ASSERT_EQ(132009u, generated[2].numerator);
    ASSERT_EQ(10000u, generated[2].denominator);

    {
        // save
        Exif::File exif;
        ASSERT_TRUE(exif.load(jpeg));
        exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE, generated);
        exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE_REF, "N");
        ASSERT_TRUE(exif.save(jpeg));
    }

    {
        // load
        Exif::File exif;
        ASSERT_TRUE(exif.load(jpeg));

        auto loaded = exif.uRationalVector(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE);
        ASSERT_EQ(3, loaded.size());

        auto s = exif.ascii(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE_REF);

        EXPECT_EQ(generated[0].numerator, loaded[0].numerator);
        EXPECT_EQ(generated[0].denominator, loaded[0].denominator);
        EXPECT_EQ(generated[1].numerator, loaded[1].numerator);
        EXPECT_EQ(generated[1].denominator, loaded[1].denominator);
        EXPECT_EQ(generated[2].numerator, loaded[2].numerator);
        EXPECT_EQ(generated[2].denominator, loaded[2].denominator);
    }
}

TEST(libexif, replace)
{
    QString jpeg = TmpJpegFile::withGps();
    ASSERT_FALSE(jpeg.isEmpty()) << TmpJpegFile::lastError();

    const ExifIfd ifd = EXIF_IFD_0;
    const ExifTag tag = EXIF_TAG_DATE_TIME;

    {
        Exif::File exif;
        ASSERT_TRUE(exif.load(jpeg, false));
        const QByteArray previous = exif.ascii(ifd, tag);
        ASSERT_FALSE(previous.isEmpty());

        const QByteArray replaced = "1983:04:09 23:10:00";
        ASSERT_NE(previous, replaced);
        exif.setValue(ifd, tag, replaced);

        const QByteArray current = exif.ascii(ifd, tag);

        EXPECT_EQ(replaced, current);
    }
}
