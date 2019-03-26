#include "dialw.h"
#include "ui_dialW.h"
#include <QDebug>
#include "widgetsarealayout.h"

dialW::dialW(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dialw) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
//    ui->dial->setVisible(false);
    setFocusPolicy(Qt::ClickFocus);
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
}

dialW::~dialW() {
    delete ui;
}

void dialW::labelMoveClicked(Qt::MouseButton b) {
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetMoveClicked(this, b);
}

void dialW::labelDelClicked(Qt::MouseButton b) {
    qDebug() << "dialW::labelDelClicked";
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetDelClicked(this);
}

void dialW::on_cmdLabel_editingFinished() {
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
}
