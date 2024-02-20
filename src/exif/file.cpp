#include <QDebug>
#include <QVector>

#include <cstdio>

#include <libexif/exif-content.h>
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
#include <libjpeg/jpeg-data.h>

#include "exif/file.h"

void Exif::File::log(ExifLog* /*log*/, ExifLogCode code, const char* domain, const char* format, va_list args, void* self)
{
    constexpr int size = 512;
    char buffer[size];
    vsnprintf(buffer, size, format, args);

    QString& message = reinterpret_cast<Exif::File*>(self)->mErrorString;
    message = QString("[%1] %2").arg(domain).arg(buffer);

    (code == EXIF_LOG_CODE_DEBUG ? qDebug() : qWarning()).noquote() << message;
}

Exif::File::File()
{
    mAllocator = exif_mem_new_default();
    mLog = exif_log_new_mem(mAllocator);

    if (mLog)
        exif_log_set_func(mLog, &File::log, this);
}

Exif::File::~File()
{
    exif_log_unref(mLog);
    exif_mem_unref(mAllocator);
}


/// \brief load all EXIF tags from \a fileName;
/// creates an empty storage if there are no tags in the file
bool Exif::File::load(const QString& fileName, bool createIfEmpty)
{
    mFileName = fileName;

    std::wstring ws = mFileName.toStdWString();
    const wchar_t* path = ws.c_str();

    // here some copy-paste from exif-data.c to support wchar_t

    {
        // exif_data_new_from_file

        ExifData *edata;
        ExifLoader *loader;

        loader = exif_loader_new ();

        if (mLog)
            exif_log_set_func(mLog, &File::log, this);

        {
            // exif_loader_write_file

            FILE *f;
            int size;
            unsigned char data[1024];

            if (!loader || !path)
                return false;

            f = _wfopen (path, L"rb");
            if (!f) {
                mErrorString = QString("[%1] The file '%2' could not be opened.")
                                   .arg("ExifLoader")
                                   .arg(path);
                qWarning().noquote() << mErrorString;
                return false;
            }
            while (1) {
                size = fread (data, 1, sizeof (data), f);
                if (size <= 0)
                    break;
                if (!exif_loader_write (loader, data, size))
                    break;
            }
            fclose (f);
        }

        edata = exif_loader_get_data (loader);
        exif_loader_unref (loader);

        mExifData = edata;
    }

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
    JPEGData *data = jpeg_data_new ();

    if (mLog)
        jpeg_data_log(data, mLog);

    std::wstring ws = fileName.toStdWString();
    const wchar_t* path = ws.data();

    // here some copy-paste from jpeg-data.c to support wchar_t

    {
        // jpeg_data_load_file

        FILE *f;
        unsigned char *d;
        unsigned int size;

        if (!data) return false;
        if (!path) return false;

        f = _wfopen (path, L"rb");
        if (!f) {
            mErrorString = QString("[%1] Path '%2' invalid.").arg("jpeg-data").arg(path);
            qWarning().noquote() << mErrorString;
            return false;
        }

        /* For now, we read the data into memory. Patches welcome... */
        fseek (f, 0, SEEK_END);
        size = ftell (f);
        fseek (f, 0, SEEK_SET);
        d = (unsigned char*) malloc (size);
        if (!d) {
            EXIF_LOG_NO_MEMORY (mLog, "jpeg-data", size);
            fclose (f);
            return false;
        }
        if (fread (d, 1, size, f) != size) {
            free (d);
            fclose (f);
            mErrorString = QString("[%1] Could not read '%2'.")
                               .arg("jpeg-data")
                               .arg(path);
            qWarning().noquote() << mErrorString;
            return false;
        }
        fclose (f);

        jpeg_data_load_data (data, d, size);
        free (d);
    }

    jpeg_data_set_exif_data(data, mExifData);

    {
        // jpeg_data_save_file

        FILE *f;
        unsigned char *d = NULL;
        unsigned int size = 0, written;

        jpeg_data_save_data (data, &d, &size);
        if (!d)
            return false;

        _wremove (path);
        f = _wfopen (path, L"wb");
        if (!f) {
            free (d);
            return false;
        }
        written = fwrite (d, 1, size, f);
        fclose (f);
        free (d);
        if (written == size)  {
            return true;
        }
        _wremove(path);
        return false;
    }
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
