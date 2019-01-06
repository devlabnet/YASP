#include "dialogwidgets.h"
#include "ui_dialogwidgets.h"
#include <QDebug>
#include <QDial>
#include <QSlider>
#include "sliderwidget.h"

DialogWidgets::DialogWidgets(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWidgets)
{
    ui->setupUi(this);
    ui->tableWidgets->setColumnCount(1);
    ui->tableWidgets->setRowCount(0);
    ui->tableWidgets->setShowGrid(true);
    ui->tableWidgets->horizontalHeader()->hide();
    ui->tableWidgets->verticalHeader()->hide();
    ui->tableWidgets->resizeRowsToContents();
    ui->tableWidgets->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgets->setStyleSheet("QTableWidget::item { border: 2px solid black; background-color: lavender;}");
    setMinimumSize(700, 500);
}

DialogWidgets::~DialogWidgets()
{
    delete ui;
}

void DialogWidgets::on_addWidgetBtn_clicked()
{
    QString wText = ui->comboBoxWidgets->currentText();
    QWidget* widget = nullptr;
    qDebug() << "Add Widget : " << wText;
    if (wText == "Dial") {
        qDebug() << "DIAL WIDGET";
        widget = new sliderWidget(this);
        widget->show();
    } else if (wText == "Slider") {
        qDebug() << "SLIDER WIDGET";
        widget = new sliderWidget(this);
        widget->show();
    }
    int rCount = ui->tableWidgets->rowCount();
    ui->tableWidgets->setRowCount(rCount + 1);
//    qDebug() << "w: " << ui->tableWidgets->columnWidth(0);
    ui->tableWidgets->setCellWidget(rCount, 0, widget);
    connect(widget, SIGNAL(deleteRow(QWidget*)), this, SLOT(deleteRow(QWidget*)));
    ui->tableWidgets->resizeRowsToContents();
}

void DialogWidgets::on_comboBoxWidgets_activated(const QString &arg1)
{
    qDebug() << "on_comboBoxWidgets_activated : " << arg1;
}

void DialogWidgets::deleteRow(QWidget* w) {
    qDebug() << "DialogWidgets::rows: " << ui->tableWidgets->rowCount();
    for (int i=0; i < ui->tableWidgets->rowCount(); i++)
      for (int j=0; j < ui->tableWidgets->columnCount(); j++) {
          if (ui->tableWidgets->cellWidget(i,j) == w) {
              qDebug() << "DialogWidgets::deleteWidget row : " << i;
              ui->tableWidgets->removeRow(i);
          }
      }
}
