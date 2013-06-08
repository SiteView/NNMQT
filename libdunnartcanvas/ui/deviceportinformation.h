#ifndef DEVICEPORTINFORMATION_H
#define DEVICEPORTINFORMATION_H

#include <QDialog>
#include "plugins/ScanPlugin/CommonDef.h"
#include <QTimer>
#include <QStandardItemModel>
#include "monitor.h"
using namespace std;
namespace Ui {
    class DevicePortInformation;
}

class DevicePortInformation : public QDialog
{
    Q_OBJECT

public:
    explicit DevicePortInformation(list<IFREC> deviceInfoList ,QString ip = "",QWidget *parent = 0);
    ~DevicePortInformation();

public slots:
    void ReceiveIfInfo(IFRecList ifList, IFFlowList ifFlowList);

private:
    Ui::DevicePortInformation *ui;
    QTimer* timer;
    IFRecList deviceInfoList;
    IFFlowList deviceFlowList;
    IFFlowList currentDeviceFlowList;
    void showDeviceInfo();

    bool bStartUpdate;

    monitor *deviceMonitor;
    QStandardItemModel* deviceInfo_Model;
private slots:
   void on_reflesh_Btn_clicked();
   void on_interval_comboBox_changed(int index);
   void on_timer_run();
};

#endif // DEVICEPORTINFORMATION_H
