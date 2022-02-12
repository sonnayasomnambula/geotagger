#include "pixmaplabel.h"

#include <QMouseEvent>
#include <QDesktopServices>

QPixmap PixmapLabel::scaledPixmap() const
{
    return mPixmap.isNull() ? mPixmap : mPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

PixmapLabel::PixmapLabel(QWidget* parent) :
    QLabel(parent)
{
    setMinimumSize(1,1);
    setScaledContents(false);
    setAlignment(Qt::AlignCenter);
}

void PixmapLabel::setPixmap(const QPixmap &pixmap)
{
    mPixmap = pixmap;
    QLabel::setPixmap(scaledPixmap());
}

void PixmapLabel::setPath(const QString& path)
{
    mPath = path;
    setCursor(path.isEmpty() ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

QSize PixmapLabel::sizeHint() const
{
    int w = width();
    return QSize(w, heightForWidth(w));
}

void PixmapLabel::resizeEvent(QResizeEvent* /*e*/)
{
    if(!mPixmap.isNull())
        QLabel::setPixmap(scaledPixmap());
}

int PixmapLabel::heightForWidth(int width) const
{
    return mPixmap.isNull() ? height() : 1.0 * mPixmap.height() * width / mPixmap.width();
}

void PixmapLabel::mousePressEvent(QMouseEvent* e)
{
    if (mPath.isEmpty())
    {
        e->ignore();
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(mPath));
    e->accept();
}
