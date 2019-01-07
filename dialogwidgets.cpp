#include "dialogwidgets.h"
#include "ui_dialogwidgets.h"
#include <QDebug>
#include <QDial>
#include <QSlider>
#include "sliderwidget.h"

DialogWidgets::DialogWidgets(QSerialPort* p, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWidgets),
    port(p)
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
        widget = new sliderWidget(ui->tableWidgets, this);
        widget->show();
    } else if (wText == "Slider") {
        qDebug() << "SLIDER WIDGET";
        widget = new sliderWidget(ui->tableWidgets, this);
        widget->show();
    }
    int rCount = ui->tableWidgets->rowCount();
    ui->tableWidgets->setRowCount(rCount + 1);
//    qDebug() << "w: " << ui->tableWidgets->columnWidth(0);
    ui->tableWidgets->setCellWidget(rCount, 0, widget);
//    connect(widget, SIGNAL(deleteRow(QWidget*)), this, SLOT(deleteRow(QWidget*)));
    ui->tableWidgets->resizeRowsToContents();
}

void DialogWidgets::on_comboBoxWidgets_activated(const QString &arg1)
{
    qDebug() << "on_comboBoxWidgets_activated : " << arg1;
}
