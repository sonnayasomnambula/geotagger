#include "model.h"

#include <QBuffer>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
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
        item.time = file.lastModified();

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
            item.flags.set(Item::Flags::HaveGps);
        }

        if (exif.contains(Exif::DateTime))
        {
            QString timeString = exif.value(Exif::DateTime).toString();
            qDebug().noquote() << item.baseName << timeString;
            QString pattern = "yyyy:MM:dd hh:mm:ss";
            if (timeString.size() == pattern.size())
            {
                QDateTime time = QDateTime::fromString(timeString, pattern);
                if (time.isValid())
                {
                    item.time = time;
                    item.flags.set(Item::Flags::HaveShotTime);
                }
            }
        }

        {
            // TODO move to thread
            QPixmap pix;
            if (!pix.load(name))
                item.pixmap = ":/img/not_available.png";
            else
                item.pixmap = Pics::toBase64(Pics::thumbnail(pix, 32));
        }

        mFiles.append(name);
        mData.insert(name, item);

        emit progress(mFiles.size(), files.size());
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

    const QString& path = mFiles.at(index.row());

    if (role == Role::Path)
        return path;

    Q_ASSERT(mData.contains(path));
    const Item& item = mData[path];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case TableHeader::Name:      return item.baseName;
        case TableHeader::Time:      return item.time;
        case TableHeader::Position:  return item.position;
        default:                     return {};
        }
    }

    if (role == Qt::ToolTipRole)
    {
        return tooltip(path, item);
    }

    if (role == Role::BaseName)
        return item.baseName;

    if (role == Role::Latitude)
        return item.position.lat();

    if (role == Role::Longitude)
        return item.position.lon();

    if (role == Role::Pixmap)
        return item.pixmap;

    return {};
}

Model::Item Model::item(int row) const
{
    if (row < 0 || row >= mFiles.size()) return Item();

    const QString& name = mFiles.at(row);
    return mData.value(name);
}

QString Model::tooltip(const QString& path, const Model::Item& item)
{
    QStringList strings;

    strings.append(path);

    if (item.flags & Item::Flags::HaveShotTime)
        strings.append(tr("EXIF have shot time"));
    else
        strings.append(tr("EXIF have no shot time"));

    if (item.flags & Item::Flags::HaveGps)
        strings.append(tr("EXIF have GPS tag"));
    else
        strings.append(tr("EXIF have no GPS tag"));

    return strings.join("\n");
}

QHash<int, QByteArray> Model::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Role::BaseName] = "_base_name_";
    roles[Role::Latitude] = "_latitude_";
    roles[Role::Longitude] = "_longitude_";
    roles[Role::Pixmap] = "_pixmap_";

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

QPixmap Pics::thumbnail(const QPixmap& pixmap, int size)
{
    QPixmap pic = (pixmap.width() > pixmap.height()) ? pixmap.scaledToHeight(size) : pixmap.scaledToWidth(size);
    return pic.copy((pic.width() - size) / 2, (pic.height() - size) / 2, size, size);
}

QString Pics::toBase64(const QPixmap& pixmap)
{
    QByteArray raw;
    QBuffer buff(&raw);
    buff.open(QIODevice::WriteOnly);
    pixmap.save(&buff, "JPEG");

    QString base64("data:image/jpg;base64,");
    base64.append(QString::fromLatin1(raw.toBase64().data()));
    return base64;
}

void Model::Item::Flags::set(Model::Item::Flags::Value v)
{
    mFlags |= static_cast<int>(v);
}

void Model::Item::Flags::unset(Model::Item::Flags::Value v)
{
    mFlags &= ~static_cast<int>(v);
}

bool Model::Item::Flags::operator &(Model::Item::Flags::Value v) const
{
    return mFlags & static_cast<int>(v);
}
