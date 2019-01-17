#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QStyleOptionTab>
#include <QStylePainter>
#include <QTabWidget>
#include <QDebug>

class TabBar: public QTabBar
{
public:
    TabBar(const QHash<QString, QColor> &colors, QWidget *parent=nullptr) : QTabBar(parent) {
        mColors = colors;
    }

protected:
    void paintEvent(QPaintEvent */*event*/) {
        qDebug() << "***";
        QStylePainter painter(this);
        QStyleOptionTab opt;
        for(int i = 0; i < count(); i++) {
            initStyleOption(&opt, i);
            qDebug() << " ---> " << opt;
            if(mColors.contains(opt.text)){
                opt.palette.setColor(QPalette::Button, mColors[opt.text]);
            }
            painter.drawControl(QStyle::CE_TabBarTabShape, opt);
            painter.drawControl(QStyle::CE_TabBarTabLabel, opt);
        }
    }

private:
    QHash<QString, QColor> mColors;
};

class TabWidget : public QTabWidget {
public:
    TabWidget(QWidget *parent = nullptr) : QTabWidget(parent){
        // text - color
        QHash <QString, QColor> dict;

        dict["Plot 0"] = QColor("yellow");
        dict["1"] = QColor("#87ceeb");
        dict["2"] = QColor("#90EE90");
        dict["3"] = QColor("pink");
        dict["4"] = QColor("#800080");
        dict["5"] = QColor("yellow");
        dict["6"] = QColor("#87ceeb");
        dict["7"] = QColor("#90EE90");
        dict["8"] = QColor("pink");
        dict["9"] = QColor("#800080");
        qDebug() << dict;

        setTabBar(new TabBar(dict));
    }
};

#endif // TABWIDGET_H
