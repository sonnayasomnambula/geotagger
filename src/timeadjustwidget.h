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

    int days() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    void setDays(int value);
    void setHours(int value);
    void setMinutes(int value);
    void setSeconds(int value);

private slots:
    void on_clear_clicked();

private:
    Ui::TimeAdjustWidget *ui;
};

#endif // TIMEADJUSTWIDGET_H
