#include "photoslistmodel.h"

#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QPointF>

#include <sigvdr.de/qexifimageheader.h>

#include "gpx/loader.h"

using Exif = QExifImageHeader;

PhotosListModel::PhotosListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

bool PhotosListModel::setFiles(const QStringList & files)
{
    beginResetModel();

    for (const QString& name: files)
    {
        QFileInfo file(name);
        if (!file.exists())
        {
            mLastError = tr("Unable to add '%1': no such file").arg(name);
            return false;
        }

        Item item;
        item[Header::Name] = file.baseName();
        item[Header::Time] = file.lastModified();

        Exif exif;
        if (!exif.loadFromJpeg(name))
        {
            mLastError = tr("Unable to read EXIF from '%1'").arg(name);
            return false;
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

            QGeoCoordinate coord = GPX::Loader::fromExifInfernalFormat(lat, latRef, lon, lonRef);
            item[Header::Position] = QPointF(coord.latitude(), coord.longitude());
        }

        mData.append(item);
    }

    endResetModel();

    return true;
}

QVariant PhotosListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation != Qt::Horizontal) return {};

    return Header::name(section);
}

int PhotosListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mData.size();
}

int PhotosListModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;

    return Header::Count;
}

QVariant PhotosListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    if (role != Qt::DisplayRole)
        return {};

    if (index.row() >= mData.size())
        return {};

    const Item& d = mData.at(index.row());
    return index.column() < d.size() ? d[index.column()] : QVariant();
}

QVariant PhotosListModel::Header::name(int section)
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
