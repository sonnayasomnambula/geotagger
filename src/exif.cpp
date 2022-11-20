#include "exif.h"

#include <QDebug>
#include <QGeoCoordinate>
#include <QString>

#include <cmath>

#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libjpeg/jpeg-data.h>


QDebug operator<<(QDebug debug, const ExifRational& urational)
{
    debug << "ExifRational(" << urational.numerator << '/' << urational.denominator << ')';
    return debug;
}


/// latitude and longitude are stored in degrees, minutes and seconds
/// each one as rational value (numerator and denominator)
/// \param degrees      number of degrees as floating point to convert
/// \param precision    integer value used as ExifRational denominator
QVector<ExifRational> Exif::Utils::toDMS(double degrees, unsigned precision)
{
    quint32 d = degrees;
    quint32 m = static_cast<quint32>(degrees * 60) % 60;
    quint32 s = static_cast<quint32>(std::round(degrees * 60 * 60 * precision)) % (60 * precision);
    return { { d, 1 }, { m, 1 }, { s, precision } };
}


/// altitude is stored as single rational value (numerator and denominator)
/// \param value      floating point to convert
/// \param precision    integer value used as ExifRational denominator
QVector<ExifRational> Exif::Utils::toSingleRational(double value, unsigned precision)
{
    // FIXME what about altitude below sea level?
    return { { std::round(value * precision), precision } };
}

QByteArray Exif::Utils::toLatitudeRef(double lat)
{
    return lat >= 0 ? "N" : "S";
}

QByteArray Exif::Utils::toLongitudeRef(double lon)
{
    return lon >= 0 ? "E" : "W";
}

QByteArray Exif::Utils::toAltitudeRef(double /*alt*/)
{
    return ""; // FIXME find out what is returned for the altitude below sea level
}

QGeoCoordinate Exif::Utils::fromLatLon(const QVector<ExifRational>& lat, const QByteArray& latRef, const QVector<ExifRational>& lon, const QByteArray& lonRef)
{
    if (lat.size() != 3 || lon.size() != 3) {
        qWarning() << "Exif: unsupported latlon format" << lat << latRef << lon << lonRef;
        return {};
    }

    class DMS {
        double d, m, s;
    public:
        explicit DMS(const QVector<ExifRational>& value) :
            d(1.0 * value[0].numerator / value[0].denominator),
            m(1.0 * value[1].numerator / value[1].denominator),
            s(1.0 * value[2].numerator / value[2].denominator)
        {}
        double join() { return d + m / 60 + s / 60 / 60; }
    };

    double llat = DMS(lat).join();
    double llon = DMS(lon).join();

    if (latRef == "S")
        llat = -llat;
    if (lonRef == "W")
        llon = -llon;

    return QGeoCoordinate(llat, llon);
}

double Exif::Utils::fromSingleRational(const QVector<ExifRational>& rational, const QByteArray& ref)
{
    if (rational.size() != 1) {
        qWarning() << "Exif: unsupported altitude format" << rational << ref;
        return 0.;
    }

    double alt = 1.0 * rational.first().numerator / rational.first().denominator;
    if (!ref.isEmpty()) // TODO check this
        alt = -alt;

    return alt;
}


Exif::File::File()
{
    mAllocator = exif_mem_new_default();
}

Exif::File::~File()
{
    exif_mem_unref(mAllocator);
}


/// \brief load all EXIF tags from \a fileName;
/// creates an empty storage if there are no tags int the file
bool Exif::File::load(const QString& fileName, bool createIfEmpty)
{
    mFileName = fileName;

    mExifData = exif_data_new_from_file(mFileName.toLocal8Bit().data());
    if (!mExifData)
    {
        if (!createIfEmpty)
            return false;

        mExifData = exif_data_new();
        exif_data_fix(mExifData);
    }

    exif_data_set_option(mExifData, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(mExifData, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(mExifData, EXIF_BYTE_ORDER_INTEL);

    return mExifData;
}

bool Exif::File::save(const QString& fileName)
{
    JPEGData * jpegData = jpeg_data_new_from_file(mFileName.toLocal8Bit().data());
    jpeg_data_set_exif_data(jpegData, mExifData);
    return jpeg_data_save_file(jpegData, fileName.toLocal8Bit().data());
}

void Exif::File::setValue(ExifIfd ifd, ExifTag tag, const QVector<ExifRational> urational)
{
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    void* memory;

    const size_t components = urational.size();
    const size_t size = components * exif_format_get_size(EXIF_FORMAT_RATIONAL);

    if (entry)
    {
        if (entry->components == components)
        {
            memory = entry->data;
        }
        else
        {
            memory = exif_mem_realloc(mAllocator, entry->data, size);
        }
    }
    else
    {
        entry = exif_entry_new_mem(mAllocator);
        exif_content_add_entry(mExifData->ifd[ifd], entry);
        exif_entry_initialize(entry, tag);
        memory = exif_mem_alloc(mAllocator, size);
    }

    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = components;
    entry->size = size;
    entry->data = static_cast<unsigned char*>(memory);

    for (int i = 0; i < urational.size(); ++i)
    {
        exif_set_rational(entry->data + 8 * i, exif_data_get_byte_order(mExifData), urational[i]);
    }
}

QVector<ExifRational> Exif::File::uRationalVector(ExifIfd ifd, ExifTag tag) const
{
    QVector<ExifRational> value;
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    if (!entry) return value;

    value.reserve(entry->components);
    for (size_t i = 0; i < entry->components; ++i)
        value.append(exif_get_rational(entry->data + i * 8, exif_data_get_byte_order(mExifData)));

    return value;
}


void Exif::File::setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii)
{
    void* memory;
    size_t size = static_cast<size_t>(ascii.size());
    if (size && *ascii.rbegin())
        ++size; // add 1 for the '\0' terminator (supported by QByteArray, see the docs)
    ExifEntry *entry = exif_content_get_entry(mExifData->ifd[ifd], tag);

    if (entry)
    {
        if (entry->size == size)
        {
            memcpy(entry->data, ascii.data(), size);
            return;
        }
        else
        {
            memory = exif_mem_realloc(mAllocator, entry->data, size);
        }
    }
    else
    {
        entry = exif_entry_new_mem(mAllocator);
        memory = exif_mem_alloc(mAllocator, size);
    }

    memcpy(memory, ascii.data(), size);

    entry->data = static_cast<unsigned char*>(memory);
    entry->size = size;
    entry->tag = tag;
    entry->components = entry->size;
    entry->format = EXIF_FORMAT_ASCII;

    exif_content_add_entry(mExifData->ifd[ifd], entry); // Attach the ExifEntry to an IFD
    exif_entry_unref(entry);
}

QByteArray Exif::File::ascii(ExifIfd ifd, ExifTag tag) const
{
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    if (!entry) return {};

    QByteArray d(reinterpret_cast<char*>(entry->data), entry->size);
    if (d.endsWith('\0'))
        d.resize(d.size() - 1);
    return d;
}
