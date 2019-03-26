#include <QtWidgets>
#include "widgetsarealayout.h"

WidgetsAreaLayout::WidgetsAreaLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

WidgetsAreaLayout::WidgetsAreaLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing) {
    setContentsMargins(margin, margin, margin, margin);
    setSizeConstraint(QLayout::SetMinimumSize);
}

WidgetsAreaLayout::~WidgetsAreaLayout() {
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void WidgetsAreaLayout::widgetMoveClicked(QWidget *li, Qt::MouseButton b) {
    if (widgetsList.size() <2) return;
    int ind = widgetsList.indexOf(li);
    if (b == Qt::LeftButton) {
        if (ind == 0) return;
        widgetsList.swap(ind, ind-1);
        update();
    }
    if (b == Qt::RightButton) {
        if (ind == widgetsList.size()-1) return;
        widgetsList.swap(ind, ind+1);
        update();
    }
     qDebug() << "FlowLayout::labelClicked " << ind;
}

void WidgetsAreaLayout::widgetDelClicked(QWidget *li) {
     qDebug() << "WidgetsAreaLayout::widgetDelClicked " << li;
     if (widgetsList.removeOne(li)) {
        delete li;
        update();
     }
}

void WidgetsAreaLayout::addItem(QLayoutItem*) {
}

void WidgetsAreaLayout::addWidget(QWidget *w) {
    QLayout::addWidget(w);
    widgetsList.append(w);
}
//! [3]

//! [4]
int WidgetsAreaLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    }
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int WidgetsAreaLayout::verticalSpacing() const {
    if (m_vSpace >= 0) {
        return m_vSpace;
    }
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int WidgetsAreaLayout::count() const {
    return widgetsList.size();
}

QLayoutItem *WidgetsAreaLayout::itemAt(int index) const {
    QWidget* wid = widgetsList.value(index);
    if (wid) {
        return wid->layout();
    }
    return nullptr;
}

QLayoutItem *WidgetsAreaLayout::takeAt(int index) {
    if (index >= 0 && index < widgetsList.size()) return widgetsList.takeAt(index)->layout();
    return nullptr;
}

Qt::Orientations WidgetsAreaLayout::expandingDirections() const {
    return nullptr;
}

bool WidgetsAreaLayout::hasHeightForWidth() const
{
    return true;
}

int WidgetsAreaLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    // qDebug() << "heightForWidth W: " << width << " H: " << height;
    return height;
}

void WidgetsAreaLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize WidgetsAreaLayout::sizeHint() const {
    return minimumSize();
}

QSize WidgetsAreaLayout::minimumSize() const {
    QSize size;
    QWidget * wid;
    foreach (wid, widgetsList) {
        size = size.expandedTo(wid->layout()->minimumSize());
    }
    size += QSize(2*margin(), 2*margin());
    return size;
}

int WidgetsAreaLayout::doLayout(const QRect &rect, bool testOnly) const {
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QWidget *wid;
    foreach (wid, widgetsList) {
        int spaceX = horizontalSpacing();
        if (spaceX == -1) {
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }
        int spaceY = verticalSpacing();
        if (spaceY == -1) {
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }
        int nextX = x + wid->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + wid->sizeHint().width() + spaceX;
            lineHeight = 0;
        }
        if (!testOnly) {
            wid->setGeometry(QRect(QPoint(x, y), wid->sizeHint()));
        }
        x = nextX;
        lineHeight = qMax(lineHeight, wid->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int WidgetsAreaLayout::smartSpacing(QStyle::PixelMetric pm) const {
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    }
    if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(parent)->spacing();
}
