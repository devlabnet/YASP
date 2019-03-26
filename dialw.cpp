#include "dialw.h"
#include "ui_dialW.h"
#include <QDebug>
#include "widgetsarealayout.h"

dialW::dialW(QString name, boxWidget *parent) : boxWidget(parent), ui(new Ui::dialw) {
    ui->setupUi(this);
    ui->label->setText(name);
    ui->tabBox->setTabEnabled(0, !ui->cmdLabel->text().isEmpty());
    connect(ui->labelPos, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelMoveClicked(Qt::MouseButton)));
    connect(ui->labelDel, SIGNAL(clicked(Qt::MouseButton)), this, SLOT(labelDelClicked(Qt::MouseButton)));
    connect(ui->tabBox, SIGNAL(currentChanged(int)),this, SLOT(updateTabSizes(int)));
}

dialW::~dialW() {
    delete ui;
}

QString dialW::getId() {
    return ui->cmdLabel->text();
}

void dialW::updateTabSizes(int index) {
    Q_UNUSED(index);
    packTabs(ui->tabBox);
}

void dialW::on_cmdLabel_editingFinished() {
    // Check for duplicate
    QString newId = ui->cmdLabel->text();
    if (checkId(newId)) {
        id = newId;
        ui->tabBox->setTabEnabled(0, true);
    } else {
        // restore last id
        ui->cmdLabel->setText(id);
        if (id.isEmpty()) {
            ui->tabBox->setTabEnabled(0, false);
        }
    }
    ui->cmdLabelId->setText(id);
//    ui->cmdLabelId.
//    ui->cmd->setText(cmdLabelLine->text());

}
