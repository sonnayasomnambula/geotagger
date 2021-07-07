#ifndef PATHCONTROLLER_H
#define PATHCONTROLLER_H

#include <QGeoCoordinate>
#include <QGeoPath>
#include <QObject>
#include <QQmlEngine>

class PathController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QGeoPath geopath READ geoPath WRITE setGeoPath NOTIFY geoPathChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

signals:
    void geoPathChanged();
    void centerChanged();
    void zoomChanged();

public:
    using QObject::QObject;

    QGeoPath geoPath() const { return mGeoPath; }
    void setGeoPath(const QGeoPath& geoPath);

    QGeoCoordinate center() const { return mCenter; }
    void setCenter(const QGeoCoordinate& center);

    int zoom() const { return mZoom; }
    void setZoom(int zoom);

    QGeoPath mGeoPath;
    QGeoCoordinate mCenter;
    qreal mZoom = 3;
};

#endif // PATHCONTROLLER_H
