#include "plotsinfowidget.h"
#include "ui_plotsinfowidget.h"

plotsInfoWidget::plotsInfoWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::plotsInfoWidget)
{
    ui->setupUi(this);
}

plotsInfoWidget::~plotsInfoWidget() {
    delete ui;
}

void plotsInfoWidget::updateDataInfo(QString info) {
    ui->plotsInfoText->setText(info);
}
