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
#include "gpx/statistic.h"

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
    struct Flags
    {
        Flags() { memset(this, 0, sizeof(Flags)); }

        uint8_t haveShotTime : 1; // have EXIF digitized / original timestamp in the file
        uint8_t haveGPSCoord : 1; // have EXIF GPS position tags in the file
        uint8_t coordGuessed : 1; // position guessed from time and track
    } flags;

    double lat() const { return position.x(); }
    double lon() const { return position.y(); }
    void setPosition(const QGeoCoordinate& coord);
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

    QList<Photo> loaded;
    Statistic statistic;
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
    void trackChanged(int reason);
    void centerChanged();
    void zoomChanged();

public:
    struct Reason { enum { Set, Clear }; };
    struct Role { enum { Index = Qt::UserRole, Path, Name, Latitude, Longitude, Altitude, Pixmap }; };
    struct Column { enum { Name, Time, Position, Count }; };

    explicit Model(); // QML-used objects must be destoyed after QML engine so don't pass parent here

    void setTrack(const GPX::Track& track);
    void setCenter(const QGeoCoordinate& center);
    void setZoom(qreal zoom);

    void add(const QList<jpeg::Photo> photos);
    void remove(const QModelIndexList& indexes);

    void clear();

    void setTimeAdjust(qint64 timeAdjust) { mTimeAdjust = timeAdjust; }
    qint64 timeAdjust() const { return mTimeAdjust; }

    const QList<jpeg::Photo>& photos() const { return mPhotos; }
    const QList<QGeoPositionInfo>& track() const { return mTrack; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void guessPhotoCoordinates();


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
