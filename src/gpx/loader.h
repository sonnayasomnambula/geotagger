#ifndef GPX_LOADER_H
#define GPX_LOADER_H

#include <QCoreApplication>
#include <QPointF>

#include "statistic.h"
#include "track.h"

class QGeoPath;

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
    const Statistic& statistic() const { return mStatistic; }

private:
    bool warn(const QString& text);

    static QDateTime stringToDateTime(const QString& s);

    static const char* mscModuleName;

    Track mTrack;
    QString mName;
    Statistic mStatistic;

    QString mLastError;
};

} // namespace GPX

#endif // GPX_LOADER_H
