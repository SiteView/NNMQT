#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QDialog>
#include "plugins/ScanPlugin/CommonDef.h"
#include <QString>

namespace Ui {
    class DeviceInfo;
}

class DeviceInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceInfo(IDBody devInfo, QString ip, QString strUserDefinedName, QString strRemark, QWidget *parent = 0);
    ~DeviceInfo();

private:
    Ui::DeviceInfo *ui;

signals:
    void onConfirmButtonClicked(QString strUserDefinedName, QString strRemark);

private slots:
    void on_cancelButton_clicked();
    void on_confirmButton_clicked();
};

#endif // DEVICEINFO_H
