#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QGeoPath>
#include <QPixmap>
#include <QPointF>
#include <QQmlEngine>
#include <QString>

#include "gpx/track.h"

class GeoPoint : public QPointF
{
public:
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
    Q_PROPERTY(QGeoCoordinate center MEMBER mCenter WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoom MEMBER mZoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QStringList files MEMBER mFiles WRITE setFiles NOTIFY filesChanged)

signals:
    void trackChanged();
    void centerChanged();
    void zoomChanged();
    void filesChanged();
    void progress(int i, int total);

public:
    explicit Model(QObject* parent = nullptr);

    bool setFiles(const QStringList& files);
    void setTrack(const GPX::Track& track);
    void setCenter(const QGeoCoordinate& center);
    void setZoom(int zoom);

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
    struct Item
    {
        QString baseName;
        QDateTime time; // shot time from EXIF or last modified
        GeoPoint position;
        QString pixmap; // base64 thumbnail

        class Flags
        {
            int mFlags = 0;
        public:
            enum Value
            {
                NoFlags = 0x00,
                HaveShotTime  = 0x01,
                HaveGps   = 0x02,
            };

            void set(Value v);
            void unset(Value v);
            bool operator &(Value v) const;
        } flags;
    };

    Item item(int row) const;

    static QString tooltip(const QString & path, const Item& item);

    QStringList mFiles;
    QMap<QString, Item> mData;

    QList<QGeoPositionInfo> mTrack;
    QGeoPath mPath;
    QGeoCoordinate mCenter;
    qreal mZoom = 3;

    QStringList mErrors;
};

namespace Pics
{
QPixmap thumbnail(const QPixmap& pixmap, int size);
QString toBase64(const QPixmap& pixmap);
}

#endif // MODEL_H
