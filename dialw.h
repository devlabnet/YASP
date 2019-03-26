#ifndef DIALW_H
#define DIALW_H

#include <QWidget>

namespace Ui {
class dialw;
}

class dialW : public QWidget
{
    Q_OBJECT

public:
    explicit dialW(QString name, QWidget *parent = nullptr);
    ~dialW();

private:
    Ui::dialw *ui;
private slots:
    void labelMoveClicked(Qt::MouseButton b);
    void labelDelClicked(Qt::MouseButton b);
    void on_cmdLabel_editingFinished();
};

#endif // DIALW_H
