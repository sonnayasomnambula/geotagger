#ifndef GPX_TRACK_H
#define GPX_TRACK_H

#include <QGeoPositionInfo>
#include <QList>

namespace GPX {

class Track
{
public:
    using Segment = QList<QGeoPositionInfo>;

    void clear();
    void setName(const QString& name);
    void addSegment(const Segment& segment);

    const QList<Segment>& segments() const { return mSegments; }

    QDateTime startTime() const;
    QDateTime finishTime() const;

private:
    QList<Segment> mSegments;
    QString mName;
};

} // namespace GPX

#endif // GPX_TRACK_H
