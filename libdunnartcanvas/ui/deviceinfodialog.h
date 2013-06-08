#ifndef DEVICEINFODIALOG_H
#define DEVICEINFODIALOG_H
#include <QTreeWidgetItem>
#include <QTablewidgetItem>
#include <QDialog>
#include <QComboBox>
#include <QKeyEvent>
namespace Ui {
    class DeviceInfoDialog;
}

class DeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceInfoDialog(QWidget *parent = 0);
    ~DeviceInfoDialog();
protected:
    void keyPressEvent(QKeyEvent *);

private:
    Ui::DeviceInfoDialog *ui;
    QComboBox *devTypeComboBox;
    QMap<int,int> map;
    QMap<int,QStringList> primaryMap;//get changed primaryID of row
    int row;
    int column;
    bool isAddRow;
    void excuteSQL(QString sql);
    bool deleteData();



private slots:
    void on_showFactory(QTreeWidgetItem*,int);
    void on_searchBtn_clicked();
    void on_addBtn_clicked();
    void on_deleteBtn_clicked();
    void on_saveBtn_clicked();
    void on_cancelBtn_clicked();
    void on_itemChanged(QTableWidgetItem* item);
    void on_comboBoxChanged(QString);
    void on_tableWidget_cellDoubleClicked(int,int);
};

#endif // DEVICEINFODIALOG_H
