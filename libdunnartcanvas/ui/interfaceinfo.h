#ifndef INTERFACEINFO_H
#define INTERFACEINFO_H

#include <QDialog>

namespace Ui {
    class InterfaceInfo;
}

class InterfaceInfo : public QDialog
{
    Q_OBJECT

public:
    explicit InterfaceInfo(QWidget *parent = 0);
    ~InterfaceInfo();

private:
    Ui::InterfaceInfo *ui;

signals:
    void flowThreChanged(int iSpeed, int iIndex);

public slots:
    void onModifyFlow(int iSpeed, int iIndex);
private slots:
    void on_cancelBtn_clicked();
    void on_confirmBtn_clicked();

};

#endif // INTERFACEINFO_H
