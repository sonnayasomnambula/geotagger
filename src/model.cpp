#include "model.h"

#include <QBuffer>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
#include <QPointF>

#include "gpx/loader.h"
#include "gpx/track.h"
#include "exif.h"

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


void jpeg::Photo::setPosition(const QGeoCoordinate& coord)
{
    position = { coord.latitude(), coord.longitude() };
}


bool jpeg::Loader::load(const QStringList& fileNames)
{
    if (fileNames.isEmpty()) return true;

    errors.clear();
    Statistic stat;

    for (QFileInfo file: fileNames)
    {
        emit progress(stat.total(), fileNames.size());

        if (!file.exists())
        {
            errors.append(tr("Unable to add '%1': no such file").arg(file.absoluteFilePath()));
            continue;
        }

        // TODO check if already contains

        Photo item;
        item.path = file.absoluteFilePath();
        item.name = file.baseName();
        item.time = file.lastModified();

        Exif::File exif;
        if (!exif.load(file.absoluteFilePath()))
        {
            errors.append(tr("Unable to read EXIF from '%1'").arg(file.absoluteFilePath()));
            continue;
        }

        {
            auto lat = exif.uRationalVector(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE);
            auto lon = exif.uRationalVector(EXIF_IFD_GPS, Exif::Tag::GPS::LONGITUDE);
            auto alt = exif.uRationalVector(EXIF_IFD_GPS, Exif::Tag::GPS::ALTITUDE);
            auto latRef = exif.ascii(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE_REF);
            auto lonRef = exif.ascii(EXIF_IFD_GPS, Exif::Tag::GPS::LONGITUDE_REF);
            auto altRef = exif.ascii(EXIF_IFD_GPS, Exif::Tag::GPS::ALTITUDE_REF);

            if (!lat.isEmpty() && !lon.isEmpty() && !latRef.isEmpty() && !lonRef.isEmpty())
            {
                item.setPosition(Exif::Utils::fromLatLon(lat, latRef, lon, lonRef));
                if (!alt.isEmpty() && !altRef.isEmpty())
                {
                    item.altitude = Exif::Utils::fromSingleRational(alt, altRef);
                }

                item.flags |= Photo::Exif::HaveGpsCoord;

                stat.add(item.lat(), item.lon());
            }
        }

        {
            QByteArray timeString = exif.ascii(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED);
            if (timeString.isEmpty())
                timeString = exif.ascii(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL);
            // we can also check EXIF_TAG_DATE_TIME in EXIF_IFD_0,
            // however this one may be the file editing / raw export time
            if (timeString.isEmpty())
                timeString = exif.ascii(EXIF_IFD_0, EXIF_TAG_DATE_TIME);
            QString pattern = "yyyy:MM:dd hh:mm:ss";
            if (timeString.size() == pattern.size())
            {
                QDateTime time = QDateTime::fromString(QString::fromLatin1(timeString), pattern);
                if (time.isValid())
                {
                    item.time = time;
                    item.flags |= Photo::Exif::HaveShotTime;
                }
            }
        }

        {
            // TODO move to thread
            // TODO use exif thumbnail
            QPixmap pix;
            if (!pix.load(item.path))
                item.pixmap = ":/img/not_available.png";
            else
                item.pixmap = Pics::toBase64(Pics::thumbnail(pix, 32));
        }

        loaded.append(item);
    }

    center = stat.center();

    return true;
}

// TODO add QDir where to save
bool jpeg::Saver::save(const QList<Photo>& items, qint64 addsecs)
{
    errors.clear();

    int i = 0;
    for (const Photo& item : items)
    {
        emit progress(i++, items.size());

        // we should only save the file if it does not have its own GPS tag
        if (!item.haveGPSCoord())
        {
            Exif::File exif;
            if (!exif.load(item.path)) {
                errors.append(tr("Unable to read EXIF from '%1'").arg(item.path));
                continue;
            }

            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE, Exif::Utils::toDMS(item.lat()));
            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LATITUDE_REF, Exif::Utils::toLatitudeRef(item.lat()));
            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LONGITUDE, Exif::Utils::toDMS(item.lon()));
            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::LONGITUDE_REF, Exif::Utils::toLongitudeRef(item.lon()));
            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::ALTITUDE, Exif::Utils::toSingleRational(item.altitude));
            exif.setValue(EXIF_IFD_GPS, Exif::Tag::GPS::ALTITUDE_REF, Exif::Utils::toAltitudeRef(item.altitude));

            if (addsecs && item.time.isValid()) {
                const QString pattern = "yyyy:MM:dd hh:mm:ss";
                const QByteArray timeString = item.time.addSecs(addsecs).toString(pattern).toLatin1();
                exif.setValue(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, timeString);
                exif.setValue(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED, timeString);
            }

            // TODO save as
            if (!exif.save(item.path)) {
                errors.append(tr("Unable to save EXIF to '%1'").arg(item.path));
                continue;
            }
        }
    }

    return errors.isEmpty();
}


Model::Model(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &Model::dataChanged, this, &Model::guessPhotoCoordinates);
    connect(this, &Model::trackChanged, this, &Model::guessPhotoCoordinates);
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

void Model::add(const QList<jpeg::Photo> photos)
{
    QSet<QString> existing;
    for (const jpeg::Photo& item: qAsConst(mPhotos))
        existing.insert(item.path);

    for (const jpeg::Photo& item: qAsConst(photos))
    {
        if (!existing.contains(item.path))
        {
            existing.insert(item.path);

            beginInsertRows({}, rowCount(), rowCount());
            mPhotos += item;
            endInsertRows();
        }
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

void Model::remove(const QModelIndexList& indexes)
{
    QList<QPersistentModelIndex> persistent;
    for (const auto& i: indexes)
        persistent.append(QPersistentModelIndex(i));
    for (const auto& i: persistent)
    {
        if (i.isValid() && i.row() < rowCount())
        {
            beginRemoveRows({}, i.row(), i.row());
            mPhotos.removeAt(i.row());
            endRemoveRows();
        }
    }
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case Section::Name:
            return tr("Name");
        case Section::Time:
            return tr("Time");
        case Section::Position:
            return tr("Position");
        }
    }

    return {};
}

int Model::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return mPhotos.size();
}

int Model::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;

    return Section::Count;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mPhotos.size())
        return {};

    const jpeg::Photo& item = mPhotos[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case Section::Name:      return item.name;
        case Section::Time:      return item.haveGPSCoord() ? item.time : item.time.addSecs(mTimeAdjust);
        case Section::Position:  return item.position;
        default:                     return {};
        }
    }

    if (role == Qt::ToolTipRole)
        return tooltip(item);


    if (role == Qt::ForegroundRole)
    {
        // TODO use palette?
        switch (index.column())
        {
        case Section::Time:
            return QColor(item.haveShotTime() ? Qt::black : Qt::gray);
        case Section::Position:
            return QColor(item.haveGPSCoord() ? Qt::black : Qt::gray);
        default:
            return QColor(Qt::black);
        }
    }

    if (role == Role::Index)
        return index.row();

    if (role == Role::Path)
        return item.path;

    if (role == Role::Name)
        return item.name;

    if (role == Role::Latitude)
        return item.lat();

    if (role == Role::Longitude)
        return item.lon();

    if (role == Role::Altitude)
        return item.altitude;

    if (role == Role::Pixmap)
        return item.pixmap;

    return {};
}

void Model::guessPhotoCoordinates()
{
    if (mTrack.isEmpty() || mPhotos.isEmpty()) return;

    beginResetModel();
    for (auto& item: mPhotos)
    {
        if (item.time.isNull())
            continue;
        if (item.haveGPSCoord())
            continue;

        auto i = std::find_if(mTrack.begin(), mTrack.end(), [time = item.time.addSecs(mTimeAdjust)](const QGeoPositionInfo& info) {
            return info.timestamp() > time; });
        if (i == mTrack.begin() || i == mTrack.end()) {
            qWarning() << item.name << item.time.addSecs(mTimeAdjust) << "is beyond track time";
            item.position = {}; // clear position if guessed earlier
            continue;
        }
        const QGeoPositionInfo& after = *i;
        const QGeoPositionInfo& before = *(--i);
//        qDebug().noquote() << item.baseName << "found" <<
//                              before.timestamp().time().toString() <<
//                              item.time.time().toString() <<
//                              after.timestamp().time().toString();
        item.setPosition(GPX::interpolated(before, after, item.time.addSecs(mTimeAdjust)));
    }
    endResetModel();
}

const jpeg::Photo& Model::item(int row) const
{
    static jpeg::Photo empty;
    return (row < 0 || row >= mPhotos.size()) ? empty : mPhotos[row];
}

QString Model::tooltip(const jpeg::Photo& item)
{
    QStringList strings;

    strings.append(item.path);

    if (item.haveShotTime())
        strings.append(tr("EXIF have shot time"));
    else
        strings.append(tr("EXIF have no shot time"));

    if (item.haveGPSCoord())
        strings.append(tr("EXIF have GPS tag"));
    else
        strings.append(tr("EXIF have no GPS tag"));

    return strings.join("\n");
}

QHash<int, QByteArray> Model::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Role::Index] = "_index_";
    roles[Role::Path] = "_path_";
    roles[Role::Name] = "_name_";
    roles[Role::Latitude] = "_latitude_";
    roles[Role::Longitude] = "_longitude_";
    roles[Role::Pixmap] = "_pixmap_";
    return roles;
}
