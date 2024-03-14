#include "model.h"

#include <QBuffer>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include <QImageReader>
#include <QPixmap>
#include <QPointF>

#include "gpx/loader.h"
#include "gpx/track.h"
#include "exif/file.h"
#include "exif/utils.h"

namespace Pics
{

QPixmap thumbnail(QImageReader* reader, int width, int height)
{
    if (width == 0 || height == 0)
        return QPixmap::fromImageReader(reader);

    QSize size = reader->size();

    double dw = 1.0 * width / size.width();
    double dh = 1.0 * height / size.height();
    QSize cropped_size = size * std::max(dw, dh);
    reader->setScaledSize(cropped_size);
    reader->setScaledClipRect(QRect((cropped_size.width() - width) / 2,
                                    (cropped_size.height() - height) / 2,
                                    width,
                                    height));
    return QPixmap::fromImageReader(reader);
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
    statistic.clear();

    for (QFileInfo file: fileNames)
    {
        emit progress(statistic.total(), fileNames.size());

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

            if (!lat.isEmpty() && !lon.isEmpty())
            {
                item.setPosition(Exif::Utils::fromLatLon(lat, latRef, lon, lonRef));
                if (!alt.isEmpty())
                {
                    item.altitude = Exif::Utils::fromSingleRational(alt, altRef);
                }

                item.flags.haveGPSCoord = true;

                statistic.add(item.lat(), item.lon());
            }
        }

        {
            QByteArray timeString = exif.ascii(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED);
            if (timeString.isEmpty())
                timeString = exif.ascii(EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL);
            // we can also check EXIF_TAG_DATE_TIME in EXIF_IFD_0,
            // however this one may be the file editing / raw export time
            QString pattern = "yyyy:MM:dd hh:mm:ss";
            if (timeString.size() == pattern.size())
            {
                QDateTime time = QDateTime::fromString(QString::fromLatin1(timeString), pattern);
                if (time.isValid())
                {
                    item.time = time;
                    item.flags.haveShotTime = true;
                }
            }
        }

        {
            // TODO move to thread
            QPixmap pix;
            QByteArray thumbnail = exif.thumbnail();
            if (thumbnail.size())
            {
                QBuffer buffer(&thumbnail);
                QImageReader reader(&buffer);
                pix = Pics::thumbnail(&reader, 32, 32);
            }

            if (pix.isNull())
            {
                QImageReader reader(item.path);
                pix = Pics::thumbnail(&reader, 32, 32);
            }

            if (pix.isNull())
                item.pixmap = ":/img/not_available.png";
            else
                item.pixmap = Pics::toBase64(pix);
        }

        loaded.append(item);
    }

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

        if (item.flags.coordGuessed)
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


Model::Model()
{
    connect(this, &Model::rowsInserted, this, &Model::guessPhotoCoordinates);
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

    emit trackChanged(Reason::Set);
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

void Model::setZoom(qreal zoom)
{
    if (!qFuzzyCompare(zoom, mZoom)) {
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

void Model::clear()
{
    mTrack.clear();
    mPath.clearPath();
    emit trackChanged(Reason::Clear);

    beginResetModel();
    mPhotos.clear();
    endResetModel();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case Column::Name:
            return tr("Name");
        case Column::Time:
            return tr("Time");
        case Column::Position:
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

    return Column::Count;
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
        case Column::Name:      return item.name;
        case Column::Time:      return item.flags.haveGPSCoord ? item.time : item.time.addSecs(mTimeAdjust);
        case Column::Position:  return item.position;
        default:                return {};
        }
    }

    if (role == Qt::ToolTipRole)
        return tooltip(item);


    if (role == Qt::ForegroundRole)
    {
        // TODO use palette?
        switch (index.column())
        {
        case Column::Time:
            return QColor(item.flags.haveShotTime ? Qt::black : Qt::gray);
        case Column::Position:
            return QColor(item.flags.haveGPSCoord ? Qt::black : Qt::gray);
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
        if (item.time.isNull() || item.flags.haveGPSCoord)
            continue;

        auto i = std::find_if(mTrack.begin(), mTrack.end(), [time = item.time.addSecs(mTimeAdjust)](const QGeoPositionInfo& info) {
            return info.timestamp() > time; });
        if (i == mTrack.begin() || i == mTrack.end()) {
            qWarning() << item.name << item.time.addSecs(mTimeAdjust) << "is beyond track time";
            // clear position if guessed earlier
            item.position = {};
            item.flags.coordGuessed = false;
            continue;
        }
        const QGeoPositionInfo& after = *i;
        const QGeoPositionInfo& before = *(--i);
//        qDebug().noquote() << item.baseName << "found" <<
//                              before.timestamp().time().toString() <<
//                              item.time.time().toString() <<
//                              after.timestamp().time().toString();
        item.setPosition(GPX::interpolated(before, after, item.time.addSecs(mTimeAdjust)));
        item.flags.coordGuessed = true;
    }
    endResetModel();
}

QString Model::tooltip(const jpeg::Photo& item)
{
    return QStringList({
        item.path,
        item.flags.haveShotTime ?
            tr("EXIF have shot time") :
            tr("EXIF have no shot time"),
        item.flags.haveGPSCoord ?
            tr("EXIF have GPS tag") :
            tr("EXIF have no GPS tag")
    }).join("\n");
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
