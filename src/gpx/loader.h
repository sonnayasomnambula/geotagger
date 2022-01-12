#ifndef GPX_LOADER_H
#define GPX_LOADER_H

#include <QCoreApplication>
#include <QPointF>

#include "track.h"

class QGeoPath;

class Statistic
{
public:
    void add(double lat, double lon) {
        mSum += QPointF(lat, lon);
        mTotal++;
    }

    QGeoCoordinate center() const {
        return QGeoCoordinate(mSum.x() / mTotal, mSum.y() / mTotal);
    }

    int total() const { return mTotal; }

private:
    QPointF mSum;
    QGeoCoordinate mLatMax, mLatMin, mLonMax, mLonMin;
    int mTotal = 0;
};

namespace GPX
{

QGeoCoordinate interpolated(const QGeoPositionInfo& before, const QGeoPositionInfo& after, const QDateTime time);

class Loader
{
    Q_DECLARE_TR_FUNCTIONS(Loader)

public:
    bool load(const QString& url);
    QString lastError() const { return mLastError; }

    Track track() const { return mTrack; }
    QString name() const { return mName; }
    QGeoCoordinate center() const { return mCenter; }

private:
    bool warn(const QString& text);

    static QDateTime stringToDateTime(const QString& s);

    static const char* mscModuleName;

    Track mTrack;
    QString mName;
    QGeoCoordinate mCenter;
    QString mLastError;
};

} // namespace GPX

#endif // GPX_LOADER_H
