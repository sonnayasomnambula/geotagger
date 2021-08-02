#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QGeoPath>
#include <QPixmap>
#include <QPointF>
#include <QQmlEngine>
#include <QString>

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

    Q_PROPERTY(QGeoPath track MEMBER mTrack WRITE setTrack NOTIFY trackChanged)
    Q_PROPERTY(QGeoCoordinate center MEMBER mCenter WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal zoom MEMBER mZoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QStringList files MEMBER mFiles WRITE setFiles NOTIFY filesChanged)

signals:
    void trackChanged();
    void centerChanged();
    void zoomChanged();
    void filesChanged();

public:
    explicit Model(QObject* parent = nullptr);

    bool setFiles(const QStringList& files);
    void setTrack(const QGeoPath& geoPath);
    void setCenter(const QGeoCoordinate& center);
    void setZoom(int zoom);

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

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Item
    {
        QString baseName;
        QDateTime lastModified;
        GeoPoint position;
        QString pixmap;
    };

    Item item(int row) const;

    QStringList mFiles;
    QMap<QString, Item> mData;

    QGeoPath mTrack;
    QGeoCoordinate mCenter;
    qreal mZoom = 3;

    QStringList mErrors;
};

#endif // MODEL_H
