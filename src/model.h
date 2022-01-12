#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QCoreApplication>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPositionInfo>
#include <QPixmap>
#include <QPointF>
#include <QQmlEngine>
#include <QString>

#include "gpx/track.h"

namespace jpeg
{

struct Photo
{
    QString path;
    QString name;
    QDateTime time; // shot time from EXIF or last modified
    QPointF position;
    double altitude = 0.;
    QString pixmap; // base64 thumbnail
    int flags = 0;

    bool haveShotTime() const { return flags & Exif::HaveShotTime; }
    bool haveGPSCoord() const { return flags & Exif::HaveGpsCoord; }

    double lat() const { return position.x(); }
    double lon() const { return position.y(); }
    void setPosition(const QGeoCoordinate& coord);

    struct Exif
    {
        enum
        {
            HaveShotTime    = 0x01,
            HaveGpsCoord    = 0x02,
        };
    };
};


class FileProcessor : public QObject
{
    Q_OBJECT

signals:
    void progress(int i, int total);

public:
    QStringList errors;
};

struct Loader : FileProcessor
{
    bool load(const QStringList& fileNames);
    QGeoCoordinate center;
    QList<Photo> loaded;
};

struct Saver : FileProcessor
{
    bool save(const QList<Photo>& items, qint64 addsecs);
};

} // namespace jpeg


class Model : public QAbstractListModel
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    QML_ELEMENT
#endif

    // TODO extract?
    Q_PROPERTY(QGeoPath path MEMBER mPath NOTIFY trackChanged)
    Q_PROPERTY(QGeoCoordinate center MEMBER mCenter WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoom MEMBER mZoom WRITE setZoom NOTIFY zoomChanged)

signals:
    void trackChanged();
    void centerChanged();
    void zoomChanged();

public:
    struct Role { enum { Index = Qt::UserRole, Path, Name, Latitude, Longitude, Altitude, Pixmap }; };
    struct Section { enum { Name, Time, Position, Count }; };

    explicit Model(QObject* parent = nullptr);

    void setTrack(const GPX::Track& track);
    void setCenter(const QGeoCoordinate& center);
    void setZoom(int zoom);

    void add(const QList<jpeg::Photo> photos);
    void remove(const QModelIndexList& indexes);

    void setTimeAdjust(qint64 timeAdjust) { mTimeAdjust = timeAdjust; }
    qint64 timeAdjust() const { return mTimeAdjust; }

    const QList<jpeg::Photo>& photos() const { return mPhotos; }
    const QList<QGeoPositionInfo>& track() const { return mTrack; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    const jpeg::Photo& item(int row) const;

    void guessPhotoCoordinates();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:

    static QString tooltip(const jpeg::Photo& item);

    QList<jpeg::Photo> mPhotos;
    qint64 mTimeAdjust = 0; // photo timestamp adjustment, seconds

    QList<QGeoPositionInfo> mTrack;
    QGeoPath mPath;
    QGeoCoordinate mCenter;
    qreal mZoom = 3;

};


#endif // MODEL_H
