#ifndef GPX_TRACK_H
#define GPX_TRACK_H

#include <QGeoCoordinate>
#include <QList>

namespace GPX {

class Track
{
public:
    using Segment = QList<QGeoCoordinate>;

    void clear();
    void setName(const QString& name);
    void addSegment(const Segment& segment);

    const QList<Segment>& segments() const { return mSegments; }

private:
    QList<Segment> mSegments;
    QString mName;
};

} // namespace GPX

#endif // GPX_TRACK_H
