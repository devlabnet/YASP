#ifndef DIALOGWIDGETS_H
#define DIALOGWIDGETS_H

#include <QDialog>
#include <QMenu>
namespace Ui {
class DialogWidgets;
}

class DialogWidgets : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWidgets(QWidget *parent = nullptr);
    ~DialogWidgets();
public slots:
    void deleteRow(QWidget* w);

private slots:
    void on_addWidgetBtn_clicked();
    void on_comboBoxWidgets_activated(const QString &arg1);

private:
    Ui::DialogWidgets *ui;
    QMenu *editMenu;

signals:

};

#endif // DIALOGWIDGETS_H
