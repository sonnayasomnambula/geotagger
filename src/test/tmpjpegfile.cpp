#include "tmpjpegfile.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

QString TmpJpegFile::mLastError;

QString TmpJpegFile::emptyFile(const QString &message, const QString &fileName)
{
    mLastError = message + "'" + fileName + "'";
    return "";
}

QString TmpJpegFile::lastError()
{
    return mLastError;
}

QString TmpJpegFile::instance()
{
    QFileInfo originalFile(":/img/IMG_3904.jpg");
    QDir temp(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    QString name = temp.absoluteFilePath(originalFile.fileName());

    QFile copy(name);
    if (copy.exists() && !copy.remove())
        return emptyFile("Unable to remove", name);

    QFile original(originalFile.absoluteFilePath());
    if (!original.copy(name))
        return emptyFile("Unable to copy", name);

#ifdef Q_OS_LINUX
    if (!copy.setPermissions(QFile::ReadUser | QFile::WriteUser))
        return emptyFile("Unable to set permissions", name);
#endif
    return name;
}
