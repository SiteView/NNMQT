#ifndef DLGSCANINFO_H
#define DLGSCANINFO_H

#include <QDialog>
#include <QString>
#include "NetScan.h"
#include "wizard.h"

namespace Ui {
    class DlgScanInfo;
}

class DlgScanInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DlgScanInfo(QWidget *parent = 0);
    ~DlgScanInfo();
    void ShowText(QString);
    void loadConfig(Wizard *wizard);

    void show();

    //void closeEvent(QCloseEvent *event);
    signals:
           void SendMessage1(QString);
           void ShowDiagram(DEVID_LIST device_list, EDGE_LIST edge_list, IFPROP_LIST ifprop_list);
public slots:
       void ReceiveScanMessage(QString);
       void ReceiveAnalyseResult();
       void ReceiveStopScan();

private:
    Ui::DlgScanInfo *ui;
    QConfig *config;
    NetScan *NS;

    DEVID_LIST device_list;
    EDGE_LIST edge_list;

    DEVICE_TYPE_MAP dev_type_map;
    SPECIAL_OID_LIST specialOidList;

    Wizard *wizard;
    SCAN_PARAM scanParam;
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};



#endif // DLGSCANINFO_H
