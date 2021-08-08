#include "model.h"

#include <QBuffer>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
#include <QPointF>

#include <sigvdr.de/qexifimageheader.h>

#include "gpx/loader.h"
#include "gpx/track.h"

namespace Pics
{

QPixmap thumbnail(const QPixmap& pixmap, int size)
{
    QPixmap pic = (pixmap.width() > pixmap.height()) ? pixmap.scaledToHeight(size) : pixmap.scaledToWidth(size);
    return pic.copy((pic.width() - size) / 2, (pic.height() - size) / 2, size, size);
}

QString toBase64(const QPixmap& pixmap)
{
    QByteArray raw;
    QBuffer buff(&raw);
    buff.open(QIODevice::WriteOnly);
    pixmap.save(&buff, "JPEG");

    QString base64("data:image/jpg;base64,");
    base64.append(QString::fromLatin1(raw.toBase64().data()));
    return base64;
}

} // namespace Pics

GeoPoint::GeoPoint(const QGeoCoordinate & coord) :
    QPointF(coord.latitude(), coord.longitude())
{

}

Model::Model(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &Model::photosChanged, this, &Model::guessPhotoCoordinates);
    connect(this, &Model::trackChanged, this, &Model::guessPhotoCoordinates);
}

bool Model::setPhotos(const QList<FilePath> &files)
{
    beginResetModel();

    mPhotos.clear();
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

        Photo item;
        item.baseName = file.baseName();
        item.time = file.lastModified();

        QExifImageHeader exif;
        if (!exif.loadFromJpeg(name))
        {
            mErrors.append(tr("Unable to read EXIF from '%1'").arg(name));
            continue;
        }

        if (exif.contains(QExifImageHeader::GpsLatitude) &&
            exif.contains(QExifImageHeader::GpsLatitudeRef) &&
            exif.contains(QExifImageHeader::GpsLongitude) &&
            exif.contains(QExifImageHeader::GpsLongitudeRef))
        {
            QVector<QPair<quint32, quint32>> lat = exif.value(QExifImageHeader::GpsLatitude).toRationalVector();
            QVector<QPair<quint32, quint32>> lon = exif.value(QExifImageHeader::GpsLongitude).toRationalVector();
            QString latRef = exif.value(QExifImageHeader::GpsLatitudeRef).toString();
            QString lonRef = exif.value(QExifImageHeader::GpsLongitudeRef).toString();

            item.position = GPX::Loader::fromExifInfernalFormat(lat, latRef, lon, lonRef);
            item.flags |= Photo::Exif::HaveGps;
        }

        if (exif.contains(QExifImageHeader::DateTime))
        {
            QString timeString = exif.value(QExifImageHeader::DateTime).toString();
            qDebug().noquote() << item.baseName << timeString;
            QString pattern = "yyyy:MM:dd hh:mm:ss";
            if (timeString.size() == pattern.size())
            {
                QDateTime time = QDateTime::fromString(timeString, pattern);
                if (time.isValid())
                {
                    item.time = time;
                    item.flags |= Photo::Exif::HaveShotTime;
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

        mPhotos.append(name);
        mData.insert(name, item);

        emit progress(mPhotos.size(), files.size());
    }

    endResetModel();
    emit photosChanged();

    return true;
}

void Model::setTrack(const GPX::Track& track)
{
    mTrack.clear();
    mPath.clearPath();

    for (const auto& segment: track)
        mTrack.append(segment);

    for (const auto& point: mTrack)
        mPath.addCoordinate(point.coordinate());

    emit trackChanged();
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

    const Photo& item = *i;
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

    if (index.row() >= mPhotos.size())
        return {};

    const QString& path = mPhotos.at(index.row());

    if (role == Role::Path)
        return path;

    Q_ASSERT(mData.contains(path));
    const Photo& item = mData[path];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case TableHeader::Name:      return item.baseName;
        case TableHeader::Time:      return item.flags & Photo::Exif::HaveGps ? item.time : item.time.addSecs(mTimeAdjust);
        case TableHeader::Position:  return item.position;
        default:                     return {};
        }
    }

    if (role == Qt::ToolTipRole)
        return tooltip(path, item);


    if (role == Qt::ForegroundRole)
    {
        // TODO use palette?
        switch (index.column())
        {
        case TableHeader::Time:
            return QColor(item.flags & Photo::Exif::HaveShotTime ? Qt::black : Qt::gray);
        case TableHeader::Position:
            return QColor(item.flags & Photo::Exif::HaveGps ? Qt::black : Qt::gray);
        default:
            return QColor(Qt::black);
        }
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

void Model::guessPhotoCoordinates()
{
    if (mPath.isEmpty() || mData.isEmpty()) return;

    beginResetModel();
    for (const auto& key: mData.keys())
    {
        Photo& item = mData[key];
        if (item.time.isNull())
            continue;
        if (item.flags & Photo::Exif::HaveGps)
            continue;

        auto i = std::find_if(mTrack.begin(), mTrack.end(), [time = item.time.addSecs(mTimeAdjust)](const QGeoPositionInfo& info) {
            return info.timestamp() > time; });
        if (i == mTrack.begin() || i == mTrack.end())
            continue;
        const QGeoPositionInfo& after = *i;
        const QGeoPositionInfo& before = *(--i);
        qDebug().noquote() << item.baseName << "found" <<
                              before.timestamp().time().toString() <<
                              item.time.time().toString() <<
                              after.timestamp().time().toString();
        item.position = GeoPoint(GPX::interpolated(before, after, item.time.addSecs(mTimeAdjust)));
    }
    endResetModel();
}

Model::Photo Model::item(int row) const
{
    if (row < 0 || row >= mPhotos.size()) return Photo();

    const QString& name = mPhotos.at(row);
    return mData.value(name);
}

QString Model::tooltip(const QString& path, const Model::Photo& item)
{
    QStringList strings;

    strings.append(path);

    if (item.flags & Photo::Exif::HaveShotTime)
        strings.append(tr("EXIF have shot time"));
    else
        strings.append(tr("EXIF have no shot time"));

    if (item.flags & Photo::Exif::HaveGps)
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
