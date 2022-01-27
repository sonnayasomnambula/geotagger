#include "timeadjustwidget.h"
#include "ui_timeadjustwidget.h"

TimeAdjustWidget::TimeAdjustWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeAdjustWidget)
{
    ui->setupUi(this);

    for (QSpinBox* box : { ui->d, ui->h, ui->m, ui->s })
        connect(box, qOverload<int>(&QSpinBox::valueChanged), this, &TimeAdjustWidget::changed);
}

TimeAdjustWidget::~TimeAdjustWidget()
{
    delete ui;
}

qint64 TimeAdjustWidget::value() const
{
    return days() * 3600 * 24 + hours() * 3600 + minutes() * 60 + seconds();
}

int TimeAdjustWidget::days() const
{
    return ui->d->value();
}

int TimeAdjustWidget::hours() const
{
    return ui->h->value();
}

int TimeAdjustWidget::minutes() const
{
    return ui->m->value();
}

int TimeAdjustWidget::seconds() const
{
    return ui->s->value();
}

void TimeAdjustWidget::setDays(int value)
{
    ui->d->setValue(value);
}

void TimeAdjustWidget::setHours(int value)
{
    ui->h->setValue(value);
}

void TimeAdjustWidget::setMinutes(int value)
{
    ui->m->setValue(value);
}

void TimeAdjustWidget::setSeconds(int value)
{
    ui->s->setValue(value);
}

void TimeAdjustWidget::on_clear_clicked()
{
    setHours(0);
    setMinutes(0);
    setSeconds(0);
}
