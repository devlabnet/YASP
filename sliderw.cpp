#include "sliderw.h"
#include "ui_sliderW.h"
#include <QDebug>
#include "widgetsarealayout.h"

SliderW::SliderW(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sliderW) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->pushButton_2->setVisible(false);
    ui->horizontalSlider->setVisible(false);
    setFocusPolicy(Qt::ClickFocus);
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelClicked(Qt::MouseButton)));
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

void SliderW::labelClicked(Qt::MouseButton b) {
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->formClicked(this, b);
}

void SliderW::on_checkBox_stateChanged(int arg1) {
    if (arg1 == Qt::Checked) {
        ui->pushButton_2->setVisible(true);
        ui->horizontalSlider->setVisible(true);
    } else {
        ui->pushButton_2->setVisible(false);
        ui->horizontalSlider->setVisible(false);
    }
}
