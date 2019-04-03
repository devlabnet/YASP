#include "boxwidget.h"
#include "widgetsarealayout.h"
#include <QDebug>

boxWidget::boxWidget(QWidget *parent) : QWidget(parent), id("") {

}

void boxWidget::sendToPort(QString str) {
//    QString msg = "#" + cmdLabelLine->text() + " " + QString::number(v);
//    qDebug() << "boxWidget::sendToPort ---> " << str;
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->sendToPort(str);
}

void boxWidget::enterEvent(QEvent * event) {
    Q_UNUSED(event);
    mouseInWidget = true;
//    qDebug() << "enterEvent";
//    setStyleSheet("background-color:rgb(228, 228, 228)");
    setStyleSheet("#frame {background-color:rgb(245,255,255)}");
//    setStyleSheet("background-color: qconicalgradient(cx:0, cy:0, angle:135, stop:0 rgba(255, 255, 0, 69), stop:0.375 rgba(255, 255, 0, 69), stop:0.423533 rgba(251, 255, 0, 145), stop:0.45 rgba(247, 255, 0, 208), stop:0.477581 rgba(255, 244, 71, 130), stop:0.518717 rgba(255, 218, 71, 130), stop:0.55 rgba(255, 255, 0, 255), stop:0.57754 rgba(255, 203, 0, 130), stop:0.625 rgba(255, 255, 0, 69), stop:1 rgba(255, 255, 0, 69));");
}

void boxWidget::leaveEvent(QEvent * event) {
    Q_UNUSED(event);
    mouseInWidget = false;
//    qDebug() << "leaveEvent";
    setStyleSheet("#frame {background-color:rgb(240,240,240)}");
}

//void boxWidget::mouseMoveEvent(QMouseEvent * event) {
//    qDebug() << "mouseMoveEvent";
//}

void boxWidget::labelMoveClicked(Qt::MouseButton b) {
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetMoveClicked(this, b);
}

void boxWidget::labelDelClicked(Qt::MouseButton b) {
    Q_UNUSED(b);
    dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->widgetDelClicked(this);
}

bool boxWidget::checkId(QString newId) {
//    if (mouseInWidget == false) return true;
    // Check for duplicate
    return dynamic_cast<WidgetsAreaLayout*>(this->parentWidget()->layout())->checkWidgetId(this, newId, mouseInWidget);
}
