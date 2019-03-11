#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include "dialogwidgets.h"

class customWidget : public QWidget
{
    Q_OBJECT
public:
    explicit customWidget(QWidget *parent, QDomElement* domElt = nullptr);
    virtual void buildXml(QDomDocument& doc) = 0;

private:
    DialogWidgets* dw;
    QVBoxLayout* layout;
    QTabWidget *onglets = new QTabWidget();
    QLabel* cmdLabelValue;
    QLabel* cmdLabelId;
    QLabel* commandLabel;
    QString commandIdStr = "";
    QString getCommandId();
protected:
    // pages
    QWidget *cmdPage;
    QWidget *settingsPage;
//    QWidget *helpPage;
    QVBoxLayout *vBoxCommands;
    QHBoxLayout* hBoxInfos;
    QHBoxLayout* hBoxValue;
    QHBoxLayout* hBoxCommands;
    QVBoxLayout *vBoxSettings;
    QGridLayout *controlsLayout;
//    QTextEdit *help;
    QLineEdit* cmdLabelLine;
    void sendToPort(double v);

signals:

public slots:
private slots:
    void deleteWidget();
    void ShowContextMenu(const QPoint &pos);
    void cmdIdEditingFinished();

};

#endif // CUSTOMWIDGET_H
