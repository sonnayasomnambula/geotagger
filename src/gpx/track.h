#ifndef GPX_TRACK_H
#define GPX_TRACK_H

#include <QGeoPositionInfo>
#include <QList>

namespace GPX
{

using Segment = QList<QGeoPositionInfo>;
using Track = QList<Segment>;

} // namespace GPX

#endif // GPX_TRACK_H
