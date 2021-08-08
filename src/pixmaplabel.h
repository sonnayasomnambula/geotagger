#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QLabel>

class PixmapLabel : public QLabel
{
    QPixmap mPixmap;

    QPixmap scaledPixmap() const;

public:
    explicit PixmapLabel(QWidget* parent = nullptr);

    void setPixmap(const QPixmap& pixmap);
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* e) override;
    int heightForWidth(int width) const override;
};

#endif // PIXMAPLABEL_H
