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

signals:
    void geoPathChanged();
    void centerChanged();

public:
    using QObject::QObject;

    QGeoPath geoPath() const { return mGeoPath; }
    void setGeoPath(const QGeoPath& geoPath);

    QGeoCoordinate center() const { return mCenter; }
    void setCenter(const QGeoCoordinate& center);

    QGeoPath mGeoPath;
    QGeoCoordinate mCenter;

};

#endif // PATHCONTROLLER_H
