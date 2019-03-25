#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent* event);

signals:
    void clicked(Qt::MouseButton);

public slots:
};

#endif // CLICKABLELABEL_H
