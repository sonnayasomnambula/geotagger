#ifndef LIBEXIF_H
#define LIBEXIF_H

#include <QString>
#include <QVector>
#include <QPair>

#include <libexif/exif-tag.h>

typedef struct _ExifData ExifData;
struct _ExifData;
typedef struct _ExifMem ExifMem;
struct _ExifMem;

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

class LibExif
{
    QString mFileName;
    ExifData * mExifData = nullptr;
    ExifMem * mAllocator = nullptr;

public:
    LibExif();
   ~LibExif();

    bool load(const QString& fileName);
    bool save(const QString& fileName);

    void setValue(ExifIfd ifd, ExifTag tag, const QVector<QPair<quint32, quint32>>& urational);
    QVector<QPair<quint32, quint32>> uRationalVector(ExifIfd ifd, ExifTag tag) const;

    void setValue(ExifIfd ifd, ExifTag tag, const char* ascii);
    void setValue(ExifIfd ifd, ExifTag tag, const QString& ascii);
    void setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii);
    QByteArray ascii(ExifIfd ifd, ExifTag tag) const;
};

#endif // LIBEXIF_H
