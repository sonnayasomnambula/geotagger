#ifndef TIMEADJUSTWIDGET_H
#define TIMEADJUSTWIDGET_H

#include <QWidget>

namespace Ui {
class TimeAdjustWidget;
}

class TimeAdjustWidget : public QWidget
{
    Q_OBJECT

signals:
    void changed();

public:
    explicit TimeAdjustWidget(QWidget *parent = nullptr);
    ~TimeAdjustWidget();

    qint64 value() const;

private:
    Ui::TimeAdjustWidget *ui;
};

#endif // TIMEADJUSTWIDGET_H
