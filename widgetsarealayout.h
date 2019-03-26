#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QStyle>
#include <QWidget>

template <typename T>
void packTabs(T *stacked)
{
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


class WidgetsAreaLayout : public QLayout
{
    Q_OBJECT

public:
    explicit WidgetsAreaLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit WidgetsAreaLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~WidgetsAreaLayout() override;


    void addItem(QLayoutItem *) override;
    void addWidget(QWidget *w);

    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    void widgetMoveClicked(QWidget *li, Qt::MouseButton b);
    void widgetDelClicked(QWidget *li);

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QWidget *> widgetsList;
    int m_hSpace;
    int m_vSpace;

public slots:
};
//! [0]

#endif // FLOWLAYOUT_H
