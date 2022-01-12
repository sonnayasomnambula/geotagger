#ifndef EXIF_H
#define EXIF_H

#include <QString>
#include <QVector>
#include <QPair>

#include <libexif/exif-tag.h>
#include <libexif/exif-utils.h>


class QGeoCoordinate;


typedef struct _ExifData ExifData;
struct _ExifData;
typedef struct _ExifMem ExifMem;
struct _ExifMem;

namespace Exif {


namespace Tag {
namespace GPS {
static const ExifTag LATITUDE      = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE);
static const ExifTag LONGITUDE     = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE);
static const ExifTag ALTITUDE      = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE);
static const ExifTag LATITUDE_REF  = static_cast<ExifTag>(EXIF_TAG_GPS_LATITUDE_REF);
static const ExifTag LONGITUDE_REF = static_cast<ExifTag>(EXIF_TAG_GPS_LONGITUDE_REF);
static const ExifTag ALTITUDE_REF  = static_cast<ExifTag>(EXIF_TAG_GPS_ALTITUDE_REF);
} // namespace GPS
} // namespace Tag


namespace Utils {

QVector<ExifRational> toDMS(double degrees, unsigned precision = 10000);
QVector<ExifRational> toSingleRational(double value, unsigned precision = 1000);
QByteArray toLatitudeRef(double lat);
QByteArray toLongitudeRef(double lon);
QByteArray toAltitudeRef(double alt);

QGeoCoordinate fromLatLon(const QVector<ExifRational>& lat, const QByteArray& latRef,
                          const QVector<ExifRational>& lon, const QByteArray& lonRef);
double fromSingleRational(const QVector<ExifRational>& rational, const QByteArray& ref);

} // namespace Utils


/// EXIF tags are stored in several groups called IFDs.
/// You can load all tags from the file with load function.
/// Set functions replaces an existing tag in a ifd or creates a new one.
/// You must know the format of the tag in order to get its value.
class File
{
    QString mFileName;
    ExifData * mExifData = nullptr;
    ExifMem * mAllocator = nullptr;

public:
    File();
   ~File();

    bool load(const QString& fileName);
    bool save(const QString& fileName);

    void setValue(ExifIfd ifd, ExifTag tag, const QVector<ExifRational> urational);
    QVector<ExifRational> uRationalVector(ExifIfd ifd, ExifTag tag) const;

    void setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii);
    QByteArray ascii(ExifIfd ifd, ExifTag tag) const;
};


} // namespace Exif

#endif // EXIF_H
