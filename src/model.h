#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPositionInfo>
#include <QPixmap>
#include <QPointF>
#include <QQmlEngine>
#include <QString>

#include "gpx/track.h"

struct GeoPoint : QPointF
{
    using QPointF::QPointF;
    GeoPoint(const QPointF& point) : QPointF(point) {}
    GeoPoint(const QGeoCoordinate& coord);

    constexpr inline qreal lat() const { return x(); }
    constexpr inline qreal lon() const { return y(); }
};

class Model : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QGeoPath path MEMBER mPath NOTIFY trackChanged)
    Q_PROPERTY(QStringList photos MEMBER mPhotos WRITE setPhotos NOTIFY photosChanged)
    Q_PROPERTY(QGeoCoordinate center MEMBER mCenter WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoom MEMBER mZoom WRITE setZoom NOTIFY zoomChanged)

signals:
    void trackChanged();
    void photosChanged();
    void centerChanged();
    void zoomChanged();
    void progress(int i, int total);

public:
    explicit Model(QObject* parent = nullptr);

    using FilePath = QString;

    void setTrack(const GPX::Track& track);
    bool setPhotos(const QList<FilePath>& files);
    void setCenter(const QGeoCoordinate& center);
    void setZoom(int zoom);

    void setTimeAdjust(qint64 timeAdjust) { mTimeAdjust = timeAdjust; }

    const QList<QGeoPositionInfo>& track() const { return mTrack; }

    Q_INVOKABLE QGeoCoordinate coordinate(const QString& name) const;

    QString lastError() const;

    struct TableHeader
    {
        enum { Name, Time, Position, Count };
        static QVariant name(int section);
    };

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;

    struct Role
    {
        enum
        {
            Path = Qt::UserRole,
            BaseName,
            Latitude,
            Longitude,
            Pixmap
        };
    };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void guessPhotoCoordinates();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Photo
    {
        QString baseName;
        QDateTime time; // shot time from EXIF or last modified
        GeoPoint position;
        QString pixmap; // base64 thumbnail
        int flags = 0;

        struct Exif
        {
            enum
            {
                HaveShotTime    = 0x01,
                HaveGps         = 0x02,
            };
        };
    };

    Photo item(int row) const;

    static QString tooltip(const QString & path, const Photo& item);

    QList<FilePath> mPhotos;
    QMap<FilePath, Photo> mData;
    qint64 mTimeAdjust = 0; // photo timestamp adjustment, seconds

    QList<QGeoPositionInfo> mTrack;
    QGeoPath mPath;
    QGeoCoordinate mCenter;
    qreal mZoom = 3;

    QStringList mErrors;
};


#endif // MODEL_H
