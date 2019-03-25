#include "widgetsWindow.h"
#include "ui_widgetsWindow.h"
#include "sliderw.h"
#include "dialw.h"

widgetsWindow::widgetsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::widgetsWindow) {
    ui->setupUi(this);
    QWidget* topWidget = new QWidget();
    widgetsLayout = new WidgetsAreaLayout();
    topWidget->setLayout(widgetsLayout);
    ui->scrollArea->setWidget(topWidget);
}

widgetsWindow::~widgetsWindow() {
    delete ui;
}

void widgetsWindow::on_actionAdd_SliderWidget_triggered() {
    SliderW* f = new SliderW("Slider_" + QString::number(widgetsLayout->count()));
    widgetsLayout->addWidget(f);
}

void widgetsWindow::on_actionAdd_Dial_Widget_triggered() {
    dialW* f = new dialW("Dial_" + QString::number(widgetsLayout->count()));
    widgetsLayout->addWidget(f);
}
