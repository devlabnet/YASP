#include "sliderw.h"
#include "ui_sliderW.h"
#include <QDebug>
#include "widgetsarealayout.h"

SliderW::SliderW(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sliderw) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
}

SliderW::~SliderW() {
    delete ui;
}

//void Form::enterEvent(QEvent * event) {
//    qDebug() << "enterEvent";
//}

//void Form::mouseMoveEvent(QMouseEvent * event) {
//    qDebug() << "mouseMoveEvent";
//}
void SliderW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void SliderW::labelMoveClicked(Qt::MouseButton b) {
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetMoveClicked(this, b);
}

void SliderW::labelDelClicked(Qt::MouseButton b) {
    Q_UNUSED(b);
    qDebug() << "SliderW::labelDelClicked";
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetDelClicked(this);
}

void SliderW::on_cmdLabel_editingFinished() {
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
}
