#include "deviceportinformation.h"
#include "ui_deviceportinformation.h"
#include "plugins/ScanPlugin/CommonDef.h"
#include "plugins/ScanPlugin/ScanUtils.h"
#include <QtDebug>

DevicePortInformation::DevicePortInformation(IFRecList deviceInfoList,QString ip,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DevicePortInformation)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    this->setWindowState(Qt::WindowMaximized);
    this->setWindowFlags(Qt::Dialog|Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint);
    this->deviceInfoList = deviceInfoList;
    ui->ipAddress_lineEdit->setText(ip);
    QStringList itemList;
    itemList<<tr("Don't refresh")<<tr("10 seconds")<<tr("20 seconds")<<tr("30 seconds");
    ui->interval_comboBox->addItems(itemList);;
    deviceInfo_Model = new QStandardItemModel;
    showDeviceInfo();
    connect(ui->interval_comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(on_interval_comboBox_changed(int)));
    connect(timer,SIGNAL(timeout()),this,SLOT(on_timer_run()));

    deviceMonitor = new monitor(ip);
    typedef std::list<IFREC> IFRecList;
    qRegisterMetaType<IFRecList>("IFRecList");
    qRegisterMetaType<IFFlowList>("IFFlowList");
    connect(deviceMonitor, SIGNAL(SendIfrecInfo(IFRecList,IFFlowList)), this, SLOT(ReceiveIfInfo(IFRecList,IFFlowList)));

    bStartUpdate = false;

    deviceMonitor->Start();
}



DevicePortInformation::~DevicePortInformation()
{
    delete ui;
    delete timer;
    delete deviceMonitor;
}
void DevicePortInformation::showDeviceInfo()
{

    QStringList headerList;
        headerList<<tr("Interface index")<<tr("Interface Description")<<tr("MAC address")<<tr("Interface Type")<<tr("Interface rate")<<tr("Work status")<<tr("Management status")<<tr("Recovery time")
            <<tr("total flow(Kbps)")<<tr("Received flow(Kbps)")<<tr("Sended flow(Kbps)")<<tr("Received frame flow(Pkts/s)")<<tr("Sended frame flow(Pkts/s)")<<tr("Received broadcast amount(Pkts/s)")<<tr("Sended broadcast amount(Pkts/s)");
    deviceInfo_Model->setHorizontalHeaderLabels(headerList);
    ui->tableView->setModel(deviceInfo_Model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
//    ui->tableView->verticalHeader()->hide();
    if(deviceInfoList.size()!=0)
    {
        list<IFREC>::iterator i = deviceInfoList.begin();
        int r = 0;
        for(;i!=deviceInfoList.end();i++)
        {
            deviceInfo_Model->setItem(r,0,new QStandardItem(QString((*i).ifIndex.c_str())));
            deviceInfo_Model->setItem(r,1,new QStandardItem(QString((*i).ifDesc.c_str())));
            deviceInfo_Model->setItem(r,2,new QStandardItem(QString((*i).ifMac.c_str())));
            deviceInfo_Model->setItem(r,3,new QStandardItem(QString((*i).ifType.c_str())));
            deviceInfo_Model->setItem(r,4,new QStandardItem(QString((*i).ifSpeed.c_str())));
            r++;
        }
    }

    if(currentDeviceFlowList.size() != 0)
    {
        //qDebug() << "device flow : " << currentDeviceFlowList.size();
        list<IFFLOW>::iterator i = currentDeviceFlowList.begin();
        int r = 0;
        for (; i!=currentDeviceFlowList.end(); i++)
        {
            //qDebug() << "device info : " << i->ifInOctets.c_str();
            if ("1" == (*i).ifAdminStatus)
            {
                deviceInfo_Model->setItem(r,5, new QStandardItem(QString("up")));
            }
            else
            {
                deviceInfo_Model->setItem(r,5, new QStandardItem(QString("down")));
            }

            if ("1" == (*i).ifStatus)
            {
                deviceInfo_Model->setItem(r,6, new QStandardItem(QString("up")));
            }
            else
            {
                deviceInfo_Model->setItem(r,6, new QStandardItem(QString("down")));
            }



//            deviceInfo_Model->setItem(r,9, new QStandardItem(QString((*i).ifInOctets.c_str())));
//            deviceInfo_Model->setItem(r,13, new QStandardItem(QString((*i).ifInUcastPkgs.c_str())));
//            deviceInfo_Model->setItem(r,10, new QStandardItem(QString((*i).ifOutOctets.c_str())));
//            deviceInfo_Model->setItem(r,14, new QStandardItem(QString((*i).ifOutUcastPkgs.c_str())));
//            qDebug() << "current index " << (*i).ifIndex.c_str();
//            if (bStartUpdate)
//            {
//                list<IFFLOW>::iterator lastFlowList = deviceFlowList.begin();
//                for(; lastFlowList != deviceFlowList.end(); lastFlowList++)
//                {
//                    qDebug() << "last index : " << (*lastFlowList).ifIndex.c_str();
//                    if ((*i).ifIndex == (*lastFlowList).ifIndex)
//                    {
//                        qDebug() << "find index";
//                        QString strInOctets = s2q((*i).ifInOctets);
//                        QString strLastInOctets = s2q((*lastFlowList).ifInOctets);
//                        int iTime = (*i).iTickCount -(*lastFlowList).iTickCount;
//                        qDebug() << "strIn : " << strInOctets << " strLast : " << strLastInOctets << " iTick : " << (*i).iTickCount << " :iLastTick : " << (*lastFlowList).iTickCount;
//                        if ("" != strInOctets && iTime>0)
//                        {

//                            int iInSpeed = (s2q((*i).ifInOctets).toInt()-s2q((*lastFlowList).ifInOctets).toInt())/iTime;
//                            qDebug() << "i in speed : " << iInSpeed;
//                            QString strInSpeed = QString::number(iInSpeed);
//                            deviceInfo_Model->setItem(r,11, new QStandardItem(strInSpeed));
//                            break;
//                        }

//                    }
//                }
//            }

            r++;
        }
    }
}

void DevicePortInformation::ReceiveIfInfo(IFRecList ifList, IFFlowList ifFlowList)
{
    this->deviceInfoList.clear();
    this->deviceInfoList = ifList;
    if (bStartUpdate)
    {
        this->deviceFlowList.clear();
        this->deviceFlowList = this->currentDeviceFlowList;
        this->currentDeviceFlowList.clear();

        this->currentDeviceFlowList = ifFlowList;
        showDeviceInfo();

    }
    else
    {

        qDebug() << "receive if info";

        this->currentDeviceFlowList.clear();

        this->currentDeviceFlowList = ifFlowList;
        showDeviceInfo();
        bStartUpdate = true;

    }



}

void DevicePortInformation::on_reflesh_Btn_clicked()
{
    showDeviceInfo();
}
void DevicePortInformation::on_interval_comboBox_changed(int index)
{
    int id = 0;
//    switch(index)
//    {
//    case 0:
//        timer->stop();
//        break;
//    case 1:
//        {
//            timer->start(10*1000);

//        }
//        break;
//    case 2:
//        timer->start(20*1000);
//        break;
//    case 3:
//        timer->start(30*1000);
//        break;
 //   }
    //deviceMonitor->Start();
}
void DevicePortInformation::on_timer_run()
{
//    this->deviceInfoList.clear();
//    this->deviceInfoList = deviceMonitor->getCiscoInfProp();
//    deviceInfo_Model->clear();
//    showDeviceInfo();
    deviceMonitor->Start();

}
