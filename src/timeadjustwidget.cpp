#include "timeadjustwidget.h"
#include "ui_timeadjustwidget.h"

TimeAdjustWidget::TimeAdjustWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimeAdjustWidget)
{
    ui->setupUi(this);

    for (QSpinBox* box : { ui->h, ui->m, ui->s })
        connect(box, qOverload<int>(&QSpinBox::valueChanged), this, &TimeAdjustWidget::changed);
}

TimeAdjustWidget::~TimeAdjustWidget()
{
    delete ui;
}

qint64 TimeAdjustWidget::value() const
{
    return ui->h->value() * 3600 + ui->m->value() * 60 + ui->s->value();
}
