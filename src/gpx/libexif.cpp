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
        // maybe return false?
        mExifData = exif_data_new();
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
    ExifEntry* entry = exif_content_get_entry(mExifData->ifd[ifd], tag);
    void* memory;

    const size_t components = urational.size();
    const size_t size = components * exif_format_get_size(EXIF_FORMAT_RATIONAL);

    if (entry)
    {
        Q_ASSERT(entry->format == EXIF_FORMAT_RATIONAL); // TODO can format for the tag be changed?
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

void LibExif::setValue(ExifIfd ifd, ExifTag tag, const char* ascii)
{
    setValue(ifd, tag, QByteArray(ascii));
}

void LibExif::setValue(ExifIfd ifd, ExifTag tag, const QString& ascii)
{
    setValue(ifd, tag, ascii.toLatin1());
}

void LibExif::setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii)
{
    void* memory;
    size_t size = static_cast<size_t>(ascii.size());
    if (size && *ascii.rbegin())
        ++size; // add 1 for the '\0' terminator (supported by QByteArray, see the docs)
    ExifEntry *entry = exif_content_get_entry(mExifData->ifd[ifd], tag);

    if (entry)
    {
        Q_ASSERT(entry->format == EXIF_FORMAT_ASCII); // TODO can format for the tag be changed?
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
    exif_entry_unref(entry); // ExifEntry are now owned elsewhere
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
