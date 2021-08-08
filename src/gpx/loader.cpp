#include "loader.h"

#include <cmath>

#include <QDebug>
#include <QCoreApplication>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QTextCodec>
#include <QPointF>

#include "xml/xmlnodereader.h"

inline double linear_interpolation(double v1, double v2, double passed) {
    Q_ASSERT(passed >= 0. && passed < 1.);
    return v1 + (v2 - v1) * passed;
}

QGeoCoordinate GPX::interpolated(const QGeoPositionInfo& before, const QGeoPositionInfo& after, const QDateTime time)
{
    double total = before.timestamp().secsTo(after.timestamp());
    double passed = before.timestamp().secsTo(time);
    passed /= total; // [0; 1)

    double lat = linear_interpolation(before.coordinate().latitude(), after.coordinate().latitude(), passed);
    double lon = linear_interpolation(before.coordinate().longitude(), after.coordinate().longitude(), passed);
    double alt = linear_interpolation(before.coordinate().altitude(), after.coordinate().altitude(), passed);

    return QGeoCoordinate(lat, lon, alt);
}

class Statistic
{
public:
    void add(double lat, double lon) {
        if (mTotal == 0) {

        }

        mSum += QPointF(lat, lon);
        mTotal++;
    }

    QGeoCoordinate center() const {
        return QGeoCoordinate(mSum.x() / mTotal, mSum.y() / mTotal);
    }

private:
    QPointF mSum;
    QGeoCoordinate mLatMax, mLatMin, mLonMax, mLonMin;
    int mTotal = 0;
};

bool GPX::Loader::load(const QString & url)
{
    qDebug() << "Loading" << url << "...";

    const QString fileName = QUrl(url).path();

    const QString suffix = QFileInfo(fileName).suffix();
    if (suffix != "gpx")
        return warn(tr("No '*.%1' files support").arg(suffix));

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return warn(tr("Unable to open '%1': %2").arg(fileName, file.errorString()));

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    QString data = stream.readAll();

    mTrack.clear();
    Statistic stat;

    QXmlStreamReader xml(data);
    XmlNodeReader rootNode(&xml);

    while (rootNode.readNextStartElement())
    {
        auto gpxNode = rootNode.child();
        if (gpxNode.isCalled("gpx"))
        {
            while (gpxNode.readNextStartElement())
            {
                auto trkNode = gpxNode.child();
                if (trkNode.isCalled("trk"))
                {
                    while (trkNode.readNextStartElement())
                    {
                        auto node = trkNode.child();
                        if (node.isCalled("name") && node.read())
                        {
//                            mTrack.setName(node.value().toString());
                        }
                        if (node.isCalled("trkseg"))
                        {
                            GPX::Segment segment;
                            auto& segmentNode = node;
                            while (segmentNode.readNextStartElement())
                            {
                                auto pointNode = segmentNode.child();
                                QGeoCoordinate coord;
                                QDateTime timestamp;
                                if (pointNode.isCalled("trkpt"))
                                {
                                    if (pointNode.readAttribute("lat", QVariant::Double))
                                        coord.setLatitude(pointNode.value().toDouble());
                                    if (pointNode.readAttribute("lon", QVariant::Double))
                                        coord.setLongitude(pointNode.value().toDouble());
                                }

                                while (pointNode.readNextStartElement())
                                {
                                    auto pointPropsNode = pointNode.child();
                                    if (pointPropsNode.isCalled("ele", XmlNodeReader::Requirement::NotRequired) && pointPropsNode.read(QVariant::Double))
                                    {
                                        coord.setAltitude(pointPropsNode.value().toDouble());
                                    }
                                    if (pointPropsNode.isCalled("time", XmlNodeReader::Requirement::NotRequired) && pointPropsNode.read())
                                    {
                                        timestamp = stringToDateTime(pointPropsNode.value().toString());
                                    }
                                }

                                if (coord.isValid())
                                {
                                    QGeoPositionInfo point(coord, timestamp);
//                                    qDebug() << point;
                                    segment.append(point);
                                    stat.add(coord.latitude(), coord.longitude());
                                }
                            }
                            mTrack.append(segment);
                        }
                    }
                }
            }
        }
    }

    mCenter = stat.center();

    return true;
}

QGeoCoordinate GPX::Loader::fromExifInfernalFormat(const QVector<QPair<quint32, quint32>> & lat, const QString & latRef, const QVector<QPair<quint32, quint32> > & lon, const QString & lonRef)
{
    struct HMS {
        double h, m, s;
        explicit HMS(const QVector<QPair<quint32, quint32>>& value) :
            h(1.0 * value[0].first / value[0].second),
            m(1.0 * value[1].first / value[1].second),
            s(1.0 * value[2].first / value[2].second)
        {}
        double join() { return h + m / 60 + s / 60 / 60; }
    };

    if (lat.size() != 3 || lon.size() != 3) return {};

    double llat = HMS(lat).join();
    double llon = HMS(lon).join();

    if (latRef == "S")
        llat = -llat;
    if (lonRef == "W")
        llon = -llon;

    return QGeoCoordinate(llat, llon);
}

bool GPX::Loader::warn(const QString& text)
{
    mLastError = text;
    qWarning().noquote() << mLastError;
    return false;
}

#include <QTimeZone>

QDateTime GPX::Loader::stringToDateTime(const QString& s)
{
    static const int isoStrLen = QString("2021-07-01T06:58:09").length();

    if (s.length() == isoStrLen + 1 && (s[isoStrLen] == 'Z' || s[isoStrLen] == 'z'))
    {
        // ISO 8601 with Z suffix
        QDateTime dt = QDateTime::fromString(s.left(isoStrLen), Qt::ISODate);
        dt.setTimeSpec(Qt::UTC);
        return dt.toLocalTime();
    }

    if (s.length() == isoStrLen)
    {
        return QDateTime::fromString(s, Qt::ISODate);
    }

    return {};
}
