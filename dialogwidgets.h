#ifndef DIALOGWIDGETS_H
#define DIALOGWIDGETS_H

#include <QDialog>
#include <QMenu>
#include <QTableWidget>
#include <QSerialPort>
#include <QDomDocument>

namespace Ui {
class DialogWidgets;
}

class DialogWidgets : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWidgets(QSerialPort* p, QWidget *parent = nullptr);
    ~DialogWidgets();
    QTableWidget* getTableWidget();
    void sendToPort(QString msg);
    void adjustSize();
public slots:
//    void deleteRow(QWidget* w);
    void setSerialPort(QSerialPort* p) {
        port = p;
    }
private slots:
    void on_addWidgetBtn_clicked();
    void on_comboBoxWidgets_activated(const QString &arg1);
    void ShowContextMenu(const QPoint &pos);
    void saveWidgets();
    void loadWidgets();

private:
    Ui::DialogWidgets *ui;
    QMenu *editMenu;
    QSerialPort* port;
    void createWidget(QString type, QDomElement *domElt = nullptr);
    void cleanWidgets();
    bool openXml();
    void doXml();
    QDomDocument buildXml();
    void saveXml();
    QDomDocument xmlWidgets;

signals:
    void messageSent(QString msg);

};

#endif // DIALOGWIDGETS_H
