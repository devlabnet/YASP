#ifndef DIALWIDGET_H
#define DIALWIDGET_H

#include <QWidget>

namespace Ui {
class dialW;
}

class dialW : public QWidget
{
    Q_OBJECT

public:
    explicit dialW(QString name, QWidget *parent = nullptr);
    ~dialW();

private:
    Ui::dialW *ui;
private slots:
    void on_checkBox_stateChanged(int arg1);
    void labelClicked(Qt::MouseButton b);
};

#endif // DIALWIDGET_H
