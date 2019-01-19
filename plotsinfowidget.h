#ifndef PLOTSINFOWIDGET_H
#define PLOTSINFOWIDGET_H

#include <QDialog>

namespace Ui {
class plotsInfoWidget;
}

class plotsInfoWidget : public QDialog
{
    Q_OBJECT

public:
    explicit plotsInfoWidget(QWidget *parent = nullptr);
    ~plotsInfoWidget();
    void updateDataInfo(QString info);
private:
    Ui::plotsInfoWidget *ui;
};

#endif // PLOTSINFOWIDGET_H
