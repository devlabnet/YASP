#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QStyle>
#include "sliderw.h"

//! [0]
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
    void formClicked(QWidget *li, Qt::MouseButton b);
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
