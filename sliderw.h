#ifndef SLIDERW_H
#define SLIDERW_H

#include <QWidget>
#include <QLayoutItem>

namespace Ui {
class sliderw;
}

class SliderW : public QWidget {
    Q_OBJECT

public:
    explicit SliderW(QString name, QWidget *parent = nullptr);
    ~SliderW();
signals:

private slots:
    void labelMoveClicked(Qt::MouseButton b);
    void labelDelClicked(Qt::MouseButton b);
    void on_cmdLabel_editingFinished();

protected:
//    void enterEvent(QEvent * event);
//    void mouseMoveEvent(QMouseEvent * event);

private:
    Ui::sliderw *ui;
};

#endif // SLIDERW_H
