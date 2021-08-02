#include "track.h"


void GPX::Track::clear()
{
    mSegments.clear();
    mName.clear();
}

void GPX::Track::setName(const QString & name)
{
    mName = name;
}

void GPX::Track::addSegment(const GPX::Track::Segment& segment)
{
    mSegments.append(segment);
}

QDateTime GPX::Track::startTime() const
{
    for (const auto& segment: mSegments)
    {
        if (!segment.isEmpty())
        {
            return segment.first().timestamp();
        }
    }

    return {};
}

QDateTime GPX::Track::finishTime() const
{
    for (auto i = mSegments.rbegin(); i != mSegments.rend(); ++i)
    {
        const Segment& segment = *i;
        if (!segment.isEmpty())
        {
            return segment.last().timestamp();
        }
    }

    return {};
}
