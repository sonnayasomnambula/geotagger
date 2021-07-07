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
                            mTrack.setName(node.value().toString());
                        }
                        if (node.isCalled("trkseg"))
                        {
                            GPX::Track::Segment segment;
                            auto& segmentNode = node;
                            while (segmentNode.readNextStartElement())
                            {
                                auto pointNode = segmentNode.child();
                                double lat, lon;
                                if (pointNode.isCalled("trkpt"))
                                {
                                    if (pointNode.readAttribute("lat", QVariant::Double))
                                        lat = pointNode.value().toDouble();
                                    if (pointNode.readAttribute("lon", QVariant::Double))
                                        lon = pointNode.value().toDouble();
                                    if (pointNode.isValid()) {
                                        segment.append(QGeoCoordinate(lat, lon));
                                        stat.add(lat, lon);
                                    }
                                }
                            }
                            mTrack.addSegment(segment);
                        }
                    }
                }
            }
        }
    }

    mCenter = stat.center();

    return true;
}

QGeoPath GPX::Loader::geoPath() const
{
    QGeoPath path;
    for (const GPX::Track::Segment& segment : mTrack.segments())
    {
        for (const QGeoCoordinate& coord : segment)
        {
            path.addCoordinate(coord);
        }
    }

    return path;
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
