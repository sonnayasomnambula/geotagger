#include "saver.h"

#include <QGeoCoordinate>

#include <cmath>

QVector<QPair<quint32, quint32>> GPX::Saver::toExifLatitude(double lat)
{
    quint32 d = lat;
    quint32 m = static_cast<quint32>(lat * 60) % 60;
    quint32 s = static_cast<quint32>(std::round(lat * 60 * 60 * 10000)) % (60 * 10000);
    return { { d, 1 }, { m, 1 }, { s, 10000 } };
}

QVector<QPair<quint32, quint32>> GPX::Saver::toExifLongitude(double lon)
{
    return toExifLatitude(lon);
}

QVector<QPair<quint32, quint32>> GPX::Saver::toExifAltitude(double alt)
{
    return { { std::round(alt * 1000), 1000 } };
}

QString GPX::Saver::toExifLatitudeRef(double lat)
{
    return lat >= 0 ? "N" : "S";
}

QString GPX::Saver::toExifLongitudeRef(double lon)
{
    return lon >= 0 ? "E" : "W";
}

QString GPX::Saver::toExifAltitudeRef(double /*alt*/)
{
    return ""; // FIXME find out what is returned for the altitude above sea level
}

QVector<QPair<quint32, quint32> > GPX::Saver::toExifLatitude(const QGeoCoordinate& coordinate)
{
    return toExifLatitude(coordinate.latitude());
}

QVector<QPair<quint32, quint32> > GPX::Saver::toExifLongitude(const QGeoCoordinate& coordinate)
{
    return toExifLongitude(coordinate.longitude());
}

QVector<QPair<quint32, quint32> > GPX::Saver::toExifAltitude(const QGeoCoordinate& coordinate)
{
    return toExifAltitude(coordinate.altitude());
}

QString GPX::Saver::toExifLatitudeRef(const QGeoCoordinate& coordinate)
{
    return toExifLatitudeRef(coordinate.latitude());
}

QString GPX::Saver::toExifLongitudeRef(const QGeoCoordinate& coordinate)
{
    return toExifLongitudeRef(coordinate.longitude());
}

QString GPX::Saver::toExifAltitudeRef(const QGeoCoordinate& coordinate)
{
    return toExifAltitudeRef(coordinate.altitude());
}
