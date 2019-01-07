#include "customwidget.h"
#include <QDebug>
#include <QMenu>
#include <QMessageBox>

customWidget::customWidget(QTableWidget *tbl, QWidget *parent) : QWidget(parent)
{
    tableWidget = tbl;
    layout = new QVBoxLayout();
    onglets = new QTabWidget();
    layout->addWidget(onglets);
    this->setLayout(layout);
    // Create pages
    cmdPage = new QWidget();
    settingsPage = new QWidget();
    helpPage = new QWidget();

    hBoxCommands = new QHBoxLayout();
    vBoxCommands = new QVBoxLayout();

    cmdLabelId = new QLabel("ID");
    cmdLabelId->setStyleSheet("font-weight: bold; color: blue");
    cmdLabelValue = new QLabel("CMD");
    cmdLabelLine = new QLineEdit();
    cmdLabelLine->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    cmdLabelLine->setText("");
    cmdLabelLine->setMaxLength(5);
    hBoxCommands->addWidget(cmdLabelId);
    hBoxCommands->addWidget(cmdLabelValue);
    controlsLayout = new QGridLayout;
    commandLabel = new QLabel("Command Label");
    controlsLayout->addWidget(commandLabel, 0, 0);
    controlsLayout->addWidget(cmdLabelLine, 0, 1);
    connect(cmdLabelLine, SIGNAL(editingFinished()), this, SLOT(cmdIdEditingFinished()));

    cmdPage->setLayout(vBoxCommands);
    cmdPage->setEnabled(false);
    vBoxSettings = new QVBoxLayout;
    vBoxSettings->addLayout(controlsLayout);
    settingsPage->setLayout(vBoxSettings);

    // helpPage
    QVBoxLayout *vboxHelp = new QVBoxLayout;
    QIcon appIcon(":/Icons/Icons/serial_port_icon.icns"); // Get the icon
    //lab3->setPixmap(QPixmap(appIcon.pixmap(appIcon.actualSize(QSize(64, 64)))));
    help = new QTextEdit;
    help->setAlignment(Qt::AlignCenter);
    help->setReadOnly(true);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    help->setSizePolicy(sizePolicy);
    help->setMaximumHeight(80);
    //vboxHelp->addWidget(lab3);
    vboxHelp->addWidget(help);
    helpPage->setLayout(vboxHelp);

    // Add Tabs
    onglets->addTab(cmdPage, "Command");
    onglets->addTab(settingsPage, "Settings");
    onglets->addTab(helpPage, "Help");
    onglets->setTabEnabled(0, false);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));

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
//    qDebug() << "DialogWidgets::rows: " << tableWidget->rowCount();
    for (int i=0; i < tableWidget->rowCount(); i++)
      for (int j=0; j < tableWidget->columnCount(); j++) {
          if (tableWidget->cellWidget(i,j) == this) {
//              qDebug() << "DialogWidgets::deleteWidget row : " << i;
              tableWidget->removeRow(i);
          }
      }
}

void customWidget::sendToPort(int v) {
    QString msg = cmdLabelLine->text() + QString::number(v);
    qDebug() << "---> " << msg;
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
    for (int i=0; i < tableWidget->rowCount(); i++) {
      for (int j=0; j < tableWidget->columnCount(); j++) {
          customWidget* cw = dynamic_cast<customWidget*>(tableWidget->cellWidget(i,j));
          if (tableWidget->cellWidget(i,j) != this) {
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
              //qDebug() << " row : " << i << " CMD ID: " << cw->getCommandId();
          } else {
              //qDebug() << " row : " << i << " ---> CMD ID: " << cw->getCommandId();
          }
          }
      }

    cmdLabelValue->setText(cmdLabelLine->text());
    cmdPage->setEnabled(true);
    onglets->setTabEnabled(0, true);
}
