#ifndef EXIF_UTILS_H
#define EXIF_UTILS_H


#include <QVector>
#include <QPair>

#include <libexif/exif-tag.h>
#include <libexif/exif-utils.h>


class QGeoCoordinate;


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


} // namespace Exif

#endif // EXIF_UTILS_H
