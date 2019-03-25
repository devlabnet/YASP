#include "dialw.h"
#include "ui_dialw.h"
#include <QDebug>
#include "widgetsarealayout.h"

dialW::dialW(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dialW) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->tabCommand->setVisible(false);
//    ui->dial->setVisible(false);
    setFocusPolicy(Qt::ClickFocus);
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelClicked(Qt::MouseButton)));
}

dialW::~dialW() {
    delete ui;
}

void dialW::labelClicked(Qt::MouseButton b) {
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->formClicked(this, b);
}

void dialW::on_checkBox_stateChanged(int arg1) {
//    if (arg1 == Qt::Checked) {
//        ui->dial->setVisible(true);
//    } else {
//        ui->dial->setVisible(false);
//    }
}
