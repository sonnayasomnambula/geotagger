#include "statistic.h"

#include <QDebug>
#include <QGeoCoordinate>
#include <QSize>

#include <QtMath>

#include <cmath>

Statistic::Statistic()
{
    clear();
}

void Statistic::add(double lat, double lon)
{
    if (!(qFuzzyIsNull(lat) && qFuzzyIsNull(lon))) {
        mSum += QPointF(lat, lon);
        mTotal++;

        mLatMin = std::min(lat, mLatMin);
        mLatMax = std::max(lat, mLatMax);
        mLonMin = std::min(lon, mLonMin);
        mLonMax = std::max(lon, mLonMax);
    }
}

void Statistic::clear()
{
    mTotal = 0;
    mSum = {};

    mLatMax = -360;
    mLatMin =  360;
    mLonMax = -360;
    mLonMin =  360;
}

int Statistic::total() const
{
    return mTotal;
}

QGeoCoordinate Statistic::center() const
{
    return QGeoCoordinate(mSum.x() / mTotal, mSum.y() / mTotal);
}

double Statistic::zoom(const QSize& mapSize) const
{
    // https://wiki.openstreetmap.org/wiki/Zoom_levels
    static constexpr double C = 40075016.686 / 2.0;
    double width = QGeoCoordinate(mLatMin, mLonMin).distanceTo(QGeoCoordinate(mLatMin, mLonMax));
    double height = QGeoCoordinate(mLatMin, mLonMin).distanceTo(QGeoCoordinate(mLatMax, mLonMin));
    double h_zoom = log2(mapSize.width() * C * std::cos(qDegreesToRadians(mLatMin)) / width) - 8;
    double v_zoom = log2(mapSize.height() * C * std::cos(qDegreesToRadians(mLatMin)) / height) - 8;
    return qBound(5.0, std::min(h_zoom, v_zoom), 18.0);
}
