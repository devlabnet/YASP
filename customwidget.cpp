#include "customwidget.h"
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include "dialogwidgets.h"

customWidget::customWidget(QWidget *parent, QDomElement *domElt) : QWidget(parent)
{
    dw = qobject_cast<DialogWidgets*>(parent);
    Q_ASSERT( dw != nullptr ); // Check that a cast was successfull


    if (domElt != nullptr) {
        qDebug() << "customWidget ----> " << domElt;
        QDomElement Child = *domElt;
        bool cmdIdFound = false;
        while (!Child.isNull() && !cmdIdFound) {
            // Read Name and value
            if (Child.tagName() == "CMD_ID") {
                commandIdStr = Child.firstChild().toText().data();
                break;
            }
            // Next child
            if (!cmdIdFound) {
                Child = Child.nextSibling().toElement();
            }
        }
    }

    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout = new QVBoxLayout();
    onglets = new QTabWidget();
    layout->addWidget(onglets);
    this->setLayout(layout);
    // Create pages
    cmdPage = new QWidget();
    settingsPage = new QWidget();
//    helpPage = new QWidget();

    hBoxInfos = new QHBoxLayout();
    hBoxCommands = new QHBoxLayout();
    hBoxValue = new QHBoxLayout();
    vBoxCommands = new QVBoxLayout();
    vBoxCommands->addLayout(hBoxInfos);
    vBoxCommands->addLayout(hBoxCommands);
    vBoxCommands->addLayout(hBoxValue);

    cmdLabelId = new QLabel("ID");
    cmdLabelId->setStyleSheet("font-weight: bold; color: blue");
    cmdLabelLine = new QLineEdit();
    cmdLabelValue = new QLabel(commandIdStr);
    cmdLabelValue->setStyleSheet("font-weight: bold;");
    cmdLabelLine->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    cmdLabelLine->setText(commandIdStr);
    cmdLabelLine->setMaxLength(5);
    hBoxInfos->addWidget(cmdLabelId);
    hBoxInfos->addWidget(cmdLabelValue);
    controlsLayout = new QGridLayout;
    commandLabel = new QLabel("Command Label");
    commandLabel->setStyleSheet("font-weight: bold;");
    controlsLayout->addWidget(commandLabel, 0, 0);
    controlsLayout->addWidget(cmdLabelLine, 0, 1);
    connect(cmdLabelLine, SIGNAL(editingFinished()), this, SLOT(cmdIdEditingFinished()));

    cmdPage->setLayout(vBoxCommands);
    cmdPage->setEnabled(false);
    vBoxSettings = new QVBoxLayout;
    vBoxSettings->addLayout(controlsLayout);
    settingsPage->setLayout(vBoxSettings);
    settingsPage->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

//    // helpPage
//    QVBoxLayout *vboxHelp = new QVBoxLayout;
//    QIcon appIcon(":/Icons/Icons/serial_port_icon.icns"); // Get the icon
//    //lab3->setPixmap(QPixmap(appIcon.pixmap(appIcon.actualSize(QSize(64, 64)))));
//    help = new QTextEdit;
//    help->setAlignment(Qt::AlignCenter);
//    help->setReadOnly(true);
//    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
//    sizePolicy.setHorizontalStretch(0);
//    sizePolicy.setVerticalStretch(0);
//    help->setSizePolicy(sizePolicy);
//    help->setMaximumHeight(80);
//    //vboxHelp->addWidget(lab3);
//    vboxHelp->addWidget(help);
//    helpPage->setLayout(vboxHelp);

    // Add Tabs
    onglets->addTab(cmdPage, "Command");
    onglets->addTab(settingsPage, "Settings");
//    onglets->addTab(helpPage, "Help");
    onglets->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    onglets->setTabEnabled(0, (!cmdLabelLine->text().isEmpty()));
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));
    connect(onglets, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
//    // Add Border
//    this->setObjectName("hello");
//    this->setWindowFlags(Qt::FramelessWindowHint);
// qDebug()  << "border " << this->objectName();
//    this->setStyleSheet(
//                "#hello{"
//                "border: 4px solid red;"
//                "padding: 6px; }"
//                );

}

void customWidget::currentTabChanged(int i) {
    Q_UNUSED(i);
//    qDebug()  << "currentTabChanged " << i << " -> sizeHint " << sizeHint();
    adjustSize();
}

void customWidget::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);
    QAction action1("Delete Widget", this);
    connect(&action1, SIGNAL(triggered()), this, SLOT(deleteWidget()));
    contextMenu.addAction(&action1);
    contextMenu.exec(mapToGlobal(pos));
}

QString customWidget::getCommandId() {
    return cmdLabelLine->text();
}

void customWidget::deleteWidget() {
    for (int i=0; i < dw->getTableWidget()->rowCount(); i++) {
        for (int j=0; j < dw->getTableWidget()->columnCount(); j++) {
            if (dw->getTableWidget()->cellWidget(i,j) == this) {
                dw->getTableWidget()->removeRow(i);
            }
        }
    }
}

void customWidget::adjustSize() {
//    qDebug() << "layout->totalSizeHint " << layout->totalSizeHint();
//    qDebug() << "sizeHint " << sizeHint();
    hide();
    show();
//    qDebug() << "sizeHint " << sizeHint();
//    qDebug() << vBoxSettings->totalSizeHint();
    dw->adjustSize();
}

void customWidget::sendToPort(double v) {
//    QString msg = "#" + cmdLabelLine->text() + " " + QString::number(v);
    QString msg =  cmdLabelLine->text() + " " + QString::number(v);
    qDebug() << "---> " << msg;
    dw->sendToPort(msg);
}

void customWidget::cmdIdEditingFinished() {
    // Check for duplicate
    QString newId = cmdLabelLine->text();
    if (newId.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("Command ID cannot be empty !!!.");
        msgBox.exec();
        return;
    }
    for (int i=0; i < dw->getTableWidget()->rowCount(); i++) {
        for (int j=0; j < dw->getTableWidget()->columnCount(); j++) {
            customWidget* cw = dynamic_cast<customWidget*>(dw->getTableWidget()->cellWidget(i,j));
            if (dw->getTableWidget()->cellWidget(i,j) != this) {
                if (cmdLabelLine->text() == cw->getCommandId()) {
                    QMessageBox msgBox;
                    msgBox.setText("Command ID already used by another command !!!.");
                    msgBox.exec();
                    cmdLabelLine->setText("");
                    cmdLabelValue->setText("");
                    cmdPage->setEnabled(false);
                    onglets->setTabEnabled(0, false);
                    return;
                }
            }
        }
    }
    cmdLabelValue->setText(cmdLabelLine->text());
    cmdPage->setEnabled(true);
    onglets->setTabEnabled(0, true);
}
