#ifndef LIBEXIF_H
#define LIBEXIF_H

#include <QString>
#include <QVector>
#include <QPair>

#include <libexif/exif-tag.h>

typedef struct _ExifData ExifData;
struct _ExifData;
typedef struct _ExifMem ExifMem;
struct _ExifMem;

class LibExif
{
    QString mFileName;
    ExifData * mExifData = nullptr;
    ExifMem * mAllocator = nullptr;

public:
    LibExif();
   ~LibExif();

    bool load(const QString& fileName);
    bool save(const QString& fileName);

    void setValue(ExifIfd ifd, ExifTag tag, const QVector<QPair<quint32, quint32>>& urational);
    QVector<QPair<quint32, quint32>> uRationalVector(ExifIfd ifd, ExifTag tag) const;

    void setValue(ExifIfd ifd, ExifTag tag, const QByteArray& ascii);
    QByteArray ascii(ExifIfd ifd, ExifTag tag) const;
};

#endif // LIBEXIF_H
