#ifndef GPX_SAVER_H
#define GPX_SAVER_H

#include <QVector>

class QGeoCoordinate;

namespace GPX
{

class Saver
{
public:
    static QVector<QPair<quint32, quint32>> toExifLatitude(double lat);
    static QVector<QPair<quint32, quint32>> toExifLongitude(double lon);
    static QVector<QPair<quint32, quint32>> toExifAltitude(double alt);
    static QString toExifLatitudeRef(double lat);
    static QString toExifLongitudeRef(double lon);
    static QString toExifAltitudeRef(double alt);

    static QVector<QPair<quint32, quint32>> toExifLatitude(const QGeoCoordinate& coordinate);
    static QVector<QPair<quint32, quint32>> toExifLongitude(const QGeoCoordinate& coordinate);
    static QVector<QPair<quint32, quint32>> toExifAltitude(const QGeoCoordinate& coordinate);
    static QString toExifLatitudeRef(const QGeoCoordinate& coordinate);
    static QString toExifLongitudeRef(const QGeoCoordinate& coordinate);
    static QString toExifAltitudeRef(const QGeoCoordinate& coordinate);
};

} // namespace GPX

#endif // GPX_SAVER_H
