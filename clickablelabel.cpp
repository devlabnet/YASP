#include "clickablelabel.h"
#include <QDebug>
#include <QLayoutItem>
#include "clickablelabel.h"
#include <QMouseEvent>

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent) {
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
    qDebug() << "ClickableLabel clicked()";
    emit clicked(event->button());
}
