#ifndef EXIF_FILE_H
#define EXIF_FILE_H

#include <QString>

#include <libexif/exif-tag.h>
#include <libexif/exif-log.h>
#include <libexif/exif-utils.h>

typedef struct _ExifData ExifData;
struct _ExifData;
typedef struct _ExifMem ExifMem;
struct _ExifMem;
typedef struct _ExifLog ExifLog;
struct _ExifLog;

namespace Exif {

/// EXIF tags are stored in several groups called IFDs.
/// You can load all tags from the file with load function.
/// Set functions replaces an existing tag in a ifd or creates a new one.
/// You must know the format of the tag in order to get its value.
class File
{
    QString mFileName;
    ExifData * mExifData = nullptr;
    ExifMem * mAllocator = nullptr;
    ExifLog * mLog = nullptr;

    QString mErrorString;

    static void log(ExifLog *log, ExifLogCode code, const char* domain, const char* format, va_list args, void* self);

public:
    File();
   ~File();

    bool load(const QString& fileName, bool createIfEmpty = true);
    bool save(const QString& fileName);

    void setValue(ExifIfd ifd, ExifTag tag, const QVector<ExifRational> urational);
    QVector<ExifRational> uRationalVector(ExifIfd ifd, ExifTag tag) const;

    void setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii);
    QByteArray ascii(ExifIfd ifd, ExifTag tag) const;

    const QString& errorString() const { return mErrorString; }
};

} // namespace Exif

#endif // EXIF_FILE_H
