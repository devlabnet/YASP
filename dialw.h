#ifndef DIALW_H
#define DIALW_H

#include "boxwidget.h"

namespace Ui {
class dialw;
}

class dialW : public boxWidget
{
    Q_OBJECT

public:
    explicit dialW(QString name, boxWidget *parent = nullptr);
    ~dialW();
    QString getId();
protected:
    Ui::dialw *ui;
private slots:
    void updateTabSizes(int index);
//    void labelMoveClicked(Qt::MouseButton b);
//    void labelDelClicked(Qt::MouseButton b);
    void on_cmdLabel_editingFinished();
};

#endif // DIALW_H
