#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QLayoutItem>

namespace Ui {
class sliderW;
}

class SliderW : public QWidget {
    Q_OBJECT

public:
    explicit SliderW(QString name, QWidget *parent = nullptr);
    ~SliderW();
signals:

private slots:
    void on_checkBox_stateChanged(int arg1);
    void labelClicked(Qt::MouseButton b);
protected:
//    void enterEvent(QEvent * event);
//    void mouseMoveEvent(QMouseEvent * event);

private:
    Ui::sliderW *ui;
};

#endif // FORM_H
