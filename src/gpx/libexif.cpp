#include "libexif.h"

#include <QString>

#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libjpeg/jpeg-data.h>

LibExif::LibExif()
{
    mAllocator = exif_mem_new_default();
}

LibExif::~LibExif()
{
    exif_mem_unref(mAllocator);
}

bool LibExif::load(const QString& fileName)
{
    mFileName = fileName;

    mExifData = exif_data_new_from_file(mFileName.toStdString().c_str());
    if (!mExifData)
    {
        mExifData = exif_data_new(); // maybe return false?
        exif_data_fix(mExifData);
    }

    exif_data_set_option(mExifData, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(mExifData, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(mExifData, EXIF_BYTE_ORDER_INTEL);

    return mExifData;
}

bool LibExif::save(const QString& fileName)
{
    JPEGData * jpegData = jpeg_data_new_from_file(mFileName.toStdString().c_str());
    jpeg_data_set_exif_data(jpegData, mExifData);
    return jpeg_data_save_file(jpegData, fileName.toStdString().c_str());
}

void LibExif::setValue(ExifIfd ifd, ExifTag tag, const QVector<QPair<quint32, quint32> >& urational)
{
    ExifEntry *entry = exif_content_get_entry(mExifData->ifd[ifd], tag);

    if (entry)
    {
        printf("Entry: %s\n", entry->data);
        // TODO
    }

    entry = exif_entry_new_mem(mAllocator);

    exif_content_add_entry (mExifData->ifd[ifd], entry);
    exif_entry_initialize (entry, tag);

    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = urational.size();
    entry->size = entry->components * exif_format_get_size(EXIF_FORMAT_RATIONAL);

    /* Allocate memory to use for holding the tag data */
    entry->data = static_cast<unsigned char*>(exif_mem_alloc(mAllocator  , entry->size));

    for (int i = 0; i < urational.size(); ++i)
    {
        ExifRational value = { urational[i].first, urational[i].second };
        exif_set_rational(entry->data + 8 * i, exif_data_get_byte_order(mExifData), value);
    }
}

QVector<QPair<quint32, quint32>> LibExif::uRationalVector(ExifIfd ifd, ExifTag tag) const
{
    QVector<QPair<quint32, quint32>> value;
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    if (!entry) return value;

    value.reserve(entry->components);
    for (size_t i = 0; i < entry->components; ++i)
    {
        ExifRational rational = exif_get_rational(entry->data + i * 8, exif_data_get_byte_order(mExifData));
        value.append({ rational.numerator, rational.denominator });
    }
    return value;
}

void LibExif::setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii)
{
    void *buf;
    size_t size = static_cast<size_t>(ascii.size());
    if (size && *ascii.rbegin())
        ++size; // add 1 for the '\0' terminator (supported by QByteArray, see the docs)
    ExifEntry *entry = exif_content_get_entry(mExifData->ifd[ifd], tag);

    if (entry)
    {
        printf("Entry: %s\n", entry->data);
        // TODO
    }

    /* Create a new ExifEntry using our allocator */
    entry = exif_entry_new_mem(mAllocator);

    /* Allocate memory to use for holding the tag data */
    buf = exif_mem_alloc(mAllocator, size);

    /* Fill in the entry */
    entry->data = (unsigned char*)buf;
    entry->size = size;
    entry->tag = tag;
    entry->components = entry->size;
    entry->format = EXIF_FORMAT_ASCII;

    /* Attach the ExifEntry to an IFD */
    exif_content_add_entry(mExifData->ifd[ifd], entry);

    /* The ExifMem and ExifEntry are now owned elsewhere */
    exif_entry_unref(entry);

    memcpy(entry->data, ascii.data(), size);
}

QByteArray LibExif::ascii(ExifIfd ifd, ExifTag tag) const
{
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    if (!entry) return {};

    QByteArray d(reinterpret_cast<char*>(entry->data), entry->size);
    if (d.endsWith('\0'))
        d.resize(d.size() - 1);
    return d;
}
