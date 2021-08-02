#ifndef GPX_LOADER_H
#define GPX_LOADER_H

#include <QObject>
#include <QQmlEngine>

#include "track.h"

class QGeoPath;

namespace GPX
{

class Loader : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    using QObject::QObject;

    Q_INVOKABLE bool load(const QString& url);
    Q_INVOKABLE QString lastError() const { return mLastError; }

    QGeoPath geoPath() const;
    QGeoCoordinate center() const { return mCenter; }

    static QGeoCoordinate fromExifInfernalFormat(const QVector<QPair<quint32, quint32>>& lat,
                                         const QString& latRef,
                                         const QVector<QPair<quint32, quint32>>& lon,
                                         const QString& lonRef);

    QDateTime startTime() const { return mTrack.startTime(); }
    QDateTime finishTime() const { return mTrack.finishTime(); }

private:
    bool warn(const QString& text);

    static QDateTime stringToDateTime(const QString& s);

    Track mTrack;
    QString mLastError;
    QGeoCoordinate mCenter;
};

} // namespace GPX

#endif // GPX_LOADER_H
