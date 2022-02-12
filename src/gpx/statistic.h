#ifndef STATISTIC_H
#define STATISTIC_H

#include <QPointF>

class QGeoCoordinate;
class QSize;

class Statistic
{
public:
    Statistic();
    void add(double lat, double lon);
    void clear();

    int total() const;
    QGeoCoordinate center() const;
    double zoom(const QSize& mapSize) const;

private:
    int mTotal;
    QPointF mSum;
    double mLatMax, mLatMin, mLonMax, mLonMin;
};

#endif // STATISTIC_H
