#ifndef TMPJPEGFILE_H
#define TMPJPEGFILE_H

class QString;

class TmpJpegFile
{
    static QString mLastError;
    static QString emptyFile(const QString& message, const QString& fileName);

public:
    static QString lastError();
    static QString instance();
};

#endif // TMPJPEGFILE_H
