#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "widgetsarealayout.h"

namespace Ui {
class widgetsWindow;
}

class widgetsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit widgetsWindow(QWidget *parent = nullptr);
    ~widgetsWindow();

private slots:
    void on_actionAdd_SliderWidget_triggered();

    void on_actionAdd_Dial_Widget_triggered();

private:
    Ui::widgetsWindow *ui;
    WidgetsAreaLayout *widgetsLayout;
};

#endif // MAINWINDOW_H
