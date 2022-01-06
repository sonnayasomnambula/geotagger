#ifndef TMPJPEGFILE_H
#define TMPJPEGFILE_H

class QString;

class TmpJpegFile
{
    static QString mLastError;
    static QString emptyFile(const QString& message, const QString& fileName);
    static QString instance(const QString& name);

public:
    static QString withGps();
    static QString withoutGps();
    static QString withoutExif();
};

#endif // TMPJPEGFILE_H
