#ifndef BOWWIDGET_H
#define BOWWIDGET_H

#include <QWidget>
#include <QDomDocument>

template <typename T>
void packTabs(T *stacked) {
   for(int i=0; i<stacked->count(); i++) {
       stacked->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
   }
   QWidget *widget=stacked->currentWidget();
   widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
   widget->resize(widget->minimumSizeHint());
   widget->adjustSize();
   stacked->resize(stacked->minimumSizeHint());
   stacked->adjustSize();
}

class boxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit boxWidget(QWidget *parent = nullptr);
    QString getId() { return id; }
    virtual void buildXml(QDomDocument& doc) = 0;

private:
    bool mouseInWidget = false;
protected:
    QString id;
    bool checkId(QString newId);
    void sendToPort(QString str);

private slots:
    void labelMoveClicked(Qt::MouseButton b);
    void labelDelClicked(Qt::MouseButton b);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
    //    void mouseMoveEvent(QMouseEvent * event);

signals:

public slots:
};

#endif // BOWWIDGET_H
