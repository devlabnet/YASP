#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

class customWidget : public QWidget
{
    Q_OBJECT
public:
    explicit customWidget(QTableWidget *tbl, QWidget *parent = nullptr);

private:
    QTableWidget* tableWidget;
    QVBoxLayout* layout;
    QTabWidget *onglets = new QTabWidget();
    QLineEdit* cmdLabelLine;
    QLabel* cmdLabelValue;
    QLabel* cmdLabelId;
    QLabel* commandLabel;

    QString getCommandId();
protected:
    // pages
    QWidget *cmdPage;
    QWidget *settingsPage;
    QWidget *helpPage;
    QVBoxLayout *vBoxCommands;
    QHBoxLayout* hBoxCommands;
    QVBoxLayout *vBoxSettings;
    QGridLayout *controlsLayout;
    QTextEdit *help;
    void sendToPort(int v);

signals:

public slots:
private slots:
    void deleteWidget();
    void ShowContextMenu(const QPoint &pos);
    void cmdIdEditingFinished();

};

#endif // CUSTOMWIDGET_H
