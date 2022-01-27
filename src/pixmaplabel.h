#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QLabel>
#include <QVariant>

class PixmapLabel : public QLabel
{
    QPixmap mPixmap;
    QString mPath;

    QPixmap scaledPixmap() const;

public:
    explicit PixmapLabel(QWidget* parent = nullptr);

    void setPixmap(const QPixmap& pixmap);
    void setPath(const QString& path);
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* e) override;
    int heightForWidth(int width) const override;

protected:
    void mousePressEvent(QMouseEvent* e) override;
};

#endif // PIXMAPLABEL_H
