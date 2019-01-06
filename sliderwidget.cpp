#include "sliderwidget.h"
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSlider>
#include <QTextEdit>
#include <QMenu>
#include <QDebug>
#include "dialogwidgets.h"
#include <QSpinBox>

sliderWidget::sliderWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    QTabWidget *onglets = new QTabWidget();
    layout->addWidget(onglets);
    this->setLayout(layout);
    // Create pages
    QWidget *page1 = new QWidget;
    QWidget *page2 = new QWidget;
    //QWidget *page3 = new QWidget;
    // Page 1
    QHBoxLayout* hbox1 = new QHBoxLayout();
    minLabelBox1 = new QLabel("MIN");
    minLabelBox1->setStyleSheet("font-weight: bold; color: red");
    maxLabelBox1 = new QLabel("MAX");
    maxLabelBox1->setStyleSheet("font-weight: bold; color: red");
    minValLabelBox1 = new QLabel(QString::number(-maxValRange));
    maxValLabelBox1 = new QLabel(QString::number((maxValRange)));

    slide = new QSlider(Qt::Horizontal);

    slide->setTickPosition(QSlider::TicksBelow);
    connect(slide, SIGNAL(valueChanged(int)), this, SLOT(slideValue(int)));

    slide->setValue(0);
    slide->setRange(-maxValRange, maxValRange);
    slide->setTickInterval(tickInterval);
    slide->setSingleStep(tickInterval);
    valLabelBox1 = new QLabel("VALUE : " + QString::number(slide->value()));
    valLabelBox1->setStyleSheet("font-weight: bold; border: 1px solid blue; margin: 2px;");
    valLabelBox1->setAlignment(Qt::AlignHCenter);

//        QLineEdit *lineEdit = new QLineEdit("Entrez votre nom");
//        QPushButton *bouton1 = new QPushButton("Cliquez ici");
//        QPushButton *bouton2 = new QPushButton("Ou là…");
    QVBoxLayout *vbox1 = new QVBoxLayout;
//        vbox1->addWidget(lineEdit);
//        vbox1->addWidget(bouton1);
    hbox1->addWidget(minLabelBox1);
    hbox1->addWidget(minValLabelBox1);
    hbox1->addWidget(maxLabelBox1);
    hbox1->addWidget(maxValLabelBox1);
    vbox1->addLayout(hbox1);
    vbox1->addWidget(slide);
    vbox1->addWidget(valLabelBox1);
    page1->setLayout(vbox1);
    // Page 2
    QLabel* minimumLabel = new QLabel("Minimum");
    minimumSpinBox = new QSpinBox();
    minimumSpinBox->setRange(-maxValRange, maxValRange);
    minimumSpinBox->setValue(-maxValRange);
    minimumSpinBox->setSingleStep(100);

    QLabel* maximumLabel = new QLabel("Maximum");
    maximumSpinBox = new QSpinBox();
    maximumSpinBox->setRange(-maxValRange, maxValRange);
    maximumSpinBox->setValue(maxValRange);
    maximumSpinBox->setSingleStep(100);

    QLabel* tickIntervalLabel = new QLabel("Ticks Interval");
    QSpinBox* ticksIntervalSpinBox = new QSpinBox();
    ticksIntervalSpinBox->setRange(0, maxValRange);
    ticksIntervalSpinBox->setValue(tickInterval);
    ticksIntervalSpinBox->setSingleStep(tickInterval);

    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->addWidget(minimumLabel, 0, 0);
    controlsLayout->addWidget(maximumLabel, 1, 0);
    controlsLayout->addWidget(tickIntervalLabel, 2, 0);
    controlsLayout->addWidget(minimumSpinBox, 0, 1);
    controlsLayout->addWidget(maximumSpinBox, 1, 1);
    controlsLayout->addWidget(ticksIntervalSpinBox, 2, 1);

    connect(minimumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMinimumSlide(int)));
    connect(maximumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximumSlide(int)));
    connect(ticksIntervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSingleStepSlide(int)));

//    QProgressBar *progress = new QProgressBar;
//    progress->setValue(50);
//    QSlider *slider = new QSlider(Qt::Horizontal);
//    QPushButton *bouton3 = new QPushButton("Valider");
    QVBoxLayout *vbox2 = new QVBoxLayout;
    vbox2->addLayout(controlsLayout);
//    vbox2->addWidget(slider);
//    vbox2->addWidget(bouton3);
    page2->setLayout(vbox2);
    /*
    // Page 3
    QVBoxLayout *vbox3 = new QVBoxLayout;
    QLabel* lab3 = new QLabel("xxx");
    QIcon appIcon(":/Icons/Icons/serial_port_icon.icns"); // Get the icon
    //lab3->setPixmap(QPixmap(appIcon.pixmap(appIcon.actualSize(QSize(64, 64)))));
    QTextEdit *help = new QTextEdit;
    help->setAlignment(Qt::AlignCenter);
    help->setReadOnly(true);
    help->setHtml("<h1>Help</h1>");
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    help->setSizePolicy(sizePolicy);
    help->setMaximumHeight(80);
    //vbox3->addWidget(lab3);
    vbox3->addWidget(help);
    page3->setLayout(vbox3);
    */
    // Add Tabs
    onglets->addTab(page1, "Command");
    onglets->addTab(page2, "Settings");
    //onglets->addTab(page3, "Help");

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));

}

void sliderWidget::slideValue(int value) {
    valLabelBox1->setText("VALUE : " + QString::number(value));
}

void sliderWidget::setMinimumSlide(int v) {
    if (v > slide->maximum()) {
        minimumSpinBox->setValue(slide->maximum());
        v = slide->maximum();
    }
    slide->setMinimum(v);
    minValLabelBox1->setText(QString::number(v));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setMaximumSlide(int v) {
    if (v < slide->minimum()) {
        maximumSpinBox->setValue(slide->maximum());
        v = slide->minimum();
    }
    slide->setMaximum(v);
    maxValLabelBox1->setText(QString::number(v));
    slide->setValue((slide->maximum() + slide->minimum()) / 2);
}

void sliderWidget::setSingleStepSlide(int v) {
    qDebug() << "setSingleStepSlide: " << v;
    slide->setTickInterval(v);
    slide->setSingleStep(v);
    if (v == 0) {
        slide->setTickPosition(QSlider::NoTicks);
    } else {
        slide->setTickPosition(QSlider::TicksBelow);
    }
}

void sliderWidget::ShowContextMenu(const QPoint &pos)
{
   QMenu contextMenu(tr("Context menu"), this);
   QAction action1("Delete Widget", this);
   connect(&action1, SIGNAL(triggered()), this, SLOT(deleteWidget()));
   contextMenu.addAction(&action1);
   contextMenu.exec(mapToGlobal(pos));
}

void sliderWidget::deleteWidget() {
   qDebug() << "sliderWidget::deleteWidget";
   emit deleteRow(this);
//   DialogWidgets* p =  dynamic_cast<DialogWidgets*>(parent());
//   p->deleteWidget(this);
}
