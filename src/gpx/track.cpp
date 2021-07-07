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
