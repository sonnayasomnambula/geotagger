#include "loader.h"

#include <QDebug>
#include <QCoreApplication>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QTextCodec>

#include "xml/xmlnodereader.h"

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
    double aLat = 0., aLon = 0.;
    int total = 0;

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
                                        aLat += lat;
                                        aLon += lon;
                                        ++total;
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

    mCenter = QGeoCoordinate(aLat / total, aLon / total);

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

bool GPX::Loader::warn(const QString& text)
{
    mLastError = text;
    qWarning().noquote() << mLastError;
    return false;
}
