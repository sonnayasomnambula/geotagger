#include "model.h"

#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QPointF>

#include <sigvdr.de/qexifimageheader.h>

#include "gpx/loader.h"

using Exif = QExifImageHeader;

GeoPoint::GeoPoint(const QGeoCoordinate & coord) :
    QPointF(coord.latitude(), coord.longitude())
{

}

Model::Model(QObject *parent)
    : QAbstractListModel(parent)
{
}

bool Model::setFiles(const QStringList & files)
{
    beginResetModel();

    mFiles.clear();
    mData.clear();
    mErrors.clear();

    for (const QString& name: files)
    {
        QFileInfo file(name);
        if (!file.exists())
        {
            mErrors.append(tr("Unable to add '%1': no such file").arg(name));
            continue;
        }

        Item item;
        item.baseName = file.baseName();
        item.lastModified = file.lastModified();

        Exif exif;
        if (!exif.loadFromJpeg(name))
        {
            mErrors.append(tr("Unable to read EXIF from '%1'").arg(name));
            continue;
        }

        if (exif.contains(Exif::GpsLatitude) &&
            exif.contains(Exif::GpsLatitudeRef) &&
            exif.contains(Exif::GpsLongitude) &&
            exif.contains(Exif::GpsLongitudeRef))
        {
            QVector<QPair<quint32, quint32>> lat = exif.value(Exif::GpsLatitude).toRationalVector();
            QVector<QPair<quint32, quint32>> lon = exif.value(Exif::GpsLongitude).toRationalVector();
            QString latRef = exif.value(Exif::GpsLatitudeRef).toString();
            QString lonRef = exif.value(Exif::GpsLongitudeRef).toString();

            item.position = GPX::Loader::fromExifInfernalFormat(lat, latRef, lon, lonRef);
        }

        mFiles.append(name);
        mData.insert(name, item);
    }

    endResetModel();
    emit filesChanged();

    return true;
}

void Model::setTrack(const QGeoPath& geoPath)
{
    if (geoPath != mTrack) {
        mTrack = geoPath;
        emit trackChanged();
    }
}

void Model::setCenter(const QGeoCoordinate& center)
{
    if (center != mCenter) {
        mCenter = center;
        emit centerChanged();
    }
}

void Model::setZoom(int zoom)
{
    if (zoom != mZoom) {
        mZoom = zoom;
        emit zoomChanged();
    }
}

QGeoCoordinate Model::coordinate(const QString& name) const
{
    const auto i = mData.find(name);
    if (i == mData.end())
        return QGeoCoordinate();

    const Item& item = *i;
    return QGeoCoordinate(item.position.lat(), item.position.lon());
}

QString Model::lastError() const
{
    return mErrors.join("\n");
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation != Qt::Horizontal) return {};

    return TableHeader::name(section);
}

int Model::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mData.size();
}

int Model::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;

    return TableHeader::Count;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    if (index.row() >= mFiles.size())
        return {};

    const QString& name = mFiles.at(index.row());
    Q_ASSERT(mData.contains(name));
    const Item& item = mData[name];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case TableHeader::Name:      return item.baseName;
        case TableHeader::Time:      return item.lastModified;
        case TableHeader::Position:  return item.position;
        default:                return QVariant();
        }
    }

    if (role == QmlRole::Name)
        return item.baseName;

    if (role == QmlRole::Latitude)
        return item.position.lat();

    if (role == QmlRole::Longitude)
        return item.position.lon();

    return {};
}

Model::Item Model::item(int row) const
{
    if (row < 0 || row >= mFiles.size()) return Item();

    const QString& name = mFiles.at(row);
    return mData.value(name);
}

QHash<int, QByteArray> Model::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[QmlRole::Name] = "_name_";
    roles[QmlRole::Latitude] = "_latitude_";
    roles[QmlRole::Longitude] = "_longitude_";

    return roles;
}

QVariant Model::TableHeader::name(int section)
{
    switch (section)
    {
    case Name:
        return tr("Name");
    case Time:
        return tr("Time");
    case Position:
        return tr("Position");
    case Count:
        return {};
    }

    return {};
}
