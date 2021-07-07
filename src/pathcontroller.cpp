#include "pathcontroller.h"

void PathController::setGeoPath(const QGeoPath& geoPath)
{
    if (geoPath != mGeoPath) {
        mGeoPath = geoPath;
        emit geoPathChanged();
    }
}

void PathController::setCenter(const QGeoCoordinate& center)
{
    if (center != mCenter) {
        mCenter = center;
        emit centerChanged();
    }
}

void PathController::setZoom(int zoom)
{
    if (zoom != mZoom) {
        mZoom = zoom;
        emit zoomChanged();
    }
}
