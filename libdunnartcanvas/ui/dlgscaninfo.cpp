#include "dlgscaninfo.h"
#include "ui_dlgscaninfo.h"

#include <QtDebug>
#include "NetScan.h"
#include "svlog.h"
#include "CommonDef.h"
#include "wizard.h"
#include "currentconfigwizardform.h"
#include <QtSql>
#include <string>
//#include "ScanUtils.h"
using namespace std;

//bool isStop = false;


DlgScanInfo::DlgScanInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgScanInfo)
{

//    monitor test;
//    test.getInterfaceInfo();

    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QSqlDatabase dbconn=QSqlDatabase::addDatabase("QSQLITE");    //添加数据库驱动

    dbconn.setDatabaseName("DeviceType.db");  //在工程目录新建一个DeviceType.db的文件

    if(!dbconn.open())    {
        qDebug()<<"open failed";
    }
    else
    {
        QSqlQuery query;//以下执行相关QSL语句

        query.exec("select id,unknown,devtype,devname,romversion,factory from sysobjectid where id>=1");
        while(query.next())
        {
            string sysOid = q2s(query.value(0).toString());
            DEVICE_PRO dpr;
            dpr.devType = q2s(query.value(2).toString());
            dpr.devTypeName = q2s(query.value(3).toString());
            dpr.devModel = q2s(query.value(4).toString());
            dpr.devFac = q2s(query.value(5).toString());
            dev_type_map.insert(make_pair(sysOid, dpr));

            qDebug() << "id " << sysOid.c_str() << " devType : " << dpr.devType.c_str() << " devFac : " << dpr.devFac.c_str() << " devTypeNmae : " << dpr.devTypeName.c_str();
        }
    }
    wizard = new Wizard();
    loadConfig(wizard);
    ui->setupUi(this);    
    connect(this,SIGNAL(sendMessage(QString)),this,SLOT(ReceiveScanMessage(QString)));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
}

DlgScanInfo::~DlgScanInfo()
{
    delete ui;
}

void DlgScanInfo::loadConfig(Wizard *wizard)
{
    scanParam.community_get_dft = q2s(wizard->communityOfRangeConfigWizard->DefaultGetCommunity);

    int iDepth = wizard->scanPerformanceConfigWizard->ScanDeep;
    int iTryTimes = wizard->scanPerformanceConfigWizard->Retry;
    int iTimeOut = wizard->scanPerformanceConfigWizard->TimeOut;
    //int iThread = wizard->scanPerformanceConfigWizard->SynThreadCount;

    if(iDepth <= 0)
    {
            scanParam.depth = 5;
    }
    else
    {
            scanParam.depth = iDepth;
    }

    if(iTryTimes <= 0)
    {
            scanParam.retrytimes = 1;
    }
    else
    {
            scanParam.retrytimes = iTryTimes;
    }


    if(iTimeOut < 100)
    {
            scanParam.timeout = 200;//==2000ms 低于100ms的超时设置启用默认值200 *10ms
    }
    else
    {
            scanParam.timeout = iTimeOut/10;
    }

    //int cpu_total = getCPUCount();
    int cpu_total = 4;
    qDebug() << "cpu count : " << cpu_total;
    if (cpu_total > 0)
    {
            scanParam.thrdamount = min(MINUS_THREAD_AMOUNT, cpu_total * 20);
            qDebug() << "thrdamount : " << scanParam.thrdamount;
    }
    else
    {
            scanParam.thrdamount = 40;
    }

    scanParam.scan_scales.clear();
    scanParam.scan_scales_num.clear();
    foreach(QString scanscales, wizard->scanRangeConfigWizard->ScanRangeList)
    {
        QStringList startendip = scanscales.split("-", QString::SkipEmptyParts);
        string startip = q2s(startendip.at(0));
        string endip = q2s(startendip.at(1));
        unsigned long startnum = ntohl(inet_addr(startip.c_str()));
        unsigned long endnum   = ntohl(inet_addr(endip.c_str()));

        // 将大范围分成小范围
        unsigned long endnum_temp;
        struct in_addr addr;
        while ((endnum-startnum) > 256)
        {
                addr.S_un.S_addr = htonl(startnum);
                startip = inet_ntoa(addr);
                endnum_temp = startnum + 256;
                addr.S_un.S_addr = htonl(endnum_temp);
                endip = inet_ntoa(addr);
                scanParam.scan_scales.push_back(make_pair(startip, endip));
                scanParam.scan_scales_num.push_back(make_pair(startnum, endnum_temp));
                startnum = endnum_temp + 1;
        }
        if ((endnum-startnum) >= 0)
        {
                addr.S_un.S_addr = htonl(startnum);
                startip = inet_ntoa(addr);
                addr.S_un.S_addr = htonl(endnum);
                endip = inet_ntoa(addr);
                scanParam.scan_scales.push_back(make_pair(startip, endip));
                scanParam.scan_scales_num.push_back(make_pair(startnum, endnum));
        }
    }



    scanParam.communitys.clear();
    scanParam.communitys_num.clear();
    foreach(QString communityRangeList, wizard->communityOfRangeConfigWizard->CommunityOfRangeList)
    {
        QStringList community = communityRangeList.split("-", QString::SkipEmptyParts);
        string startip = q2s(community.at(0));
        string endip = q2s(community.at(1));
        unsigned long startnum = ntohl(inet_addr(startip.c_str()));
        unsigned long endnum   = ntohl(inet_addr(endip.c_str()));

        std::pair<std::string, std::string> cmnty = make_pair( q2s(community.at(2)),"");

        qDebug() << "start : " <<startip.c_str() << " end : " << endip.c_str() << " get: " << q2s(community.at(2)).c_str();

        scanParam.communitys.push_back(make_pair(make_pair(startip,endip), cmnty));
        scanParam.communitys_num.push_back(make_pair(make_pair(startnum,endnum), cmnty));
    }

    //QStringList ExcludeRangeList = startElem.firstChildElement("ExcludeRange").text().split(";",QString::SkipEmptyParts);

    scanParam.scan_seeds.clear();
    foreach(QString seeds, wizard->seedsConfigWizard->SeedsList)
    {
        scanParam.scan_seeds.push_back(q2s(seeds));
    }

    scanParam.filter_scales.clear();
    scanParam.filter_scales_num.clear();
    foreach(QString str, wizard->excludeRangeConfigWizard->ExcludeRangeList)
    {
            QStringList startendIp = str.split("-", QString::SkipEmptyParts);
            string excludestartip = q2s(startendIp.at(0));
            string excludeendip   = q2s(startendIp.at(1));
            unsigned long startnum = ntohl(inet_addr(excludestartip.c_str()));
            unsigned long endnum   = ntohl(inet_addr(excludeendip.c_str()));
            scanParam.filter_scales.push_back(make_pair(excludestartip, excludeendip));
            scanParam.filter_scales_num.push_back(make_pair(startnum, endnum));
    }

}

void DlgScanInfo::show()
{
    ui->listWidget->clear();
    ui->pushButton_2->setEnabled(true);
    ui->pushButton->setEnabled(true);
    setVisible(true);
}

//void DlgScanInfo::closeEvent(QCloseEvent *event)
//{
//    NS->SetStop(true);

//    setVisible(false);

//    disconnect(NS,SIGNAL(FinishAnalyse()),this,SLOT(ReceiveAnalyseResult()));
//    disconnect(NS,SIGNAL(SendScanMessage(QString)),this,SLOT(ReceiveScanMessage(QString)));
//    disconnect(NS,SIGNAL(StopScan()),this,SLOT(ReceiveStopScan()));
//}

void DlgScanInfo::ReceiveScanMessage(QString msg)
{
    QListWidgetItem *item =  new QListWidgetItem(msg);
    ui->listWidget->addItem(item);
    ui->listWidget->setCurrentItem(item);
}

void DlgScanInfo::ReceiveStopScan()
{
    disconnect(NS,SIGNAL(SendScanMessage(QString)),this,SLOT(ReceiveScanMessage(QString)));
    accept();
}

void DlgScanInfo::ShowText(QString msg)
{
    ui->listWidget->addItem(new QListWidgetItem(msg));
}

void DlgScanInfo::ReceiveAnalyseResult()
{
    accept();
    ui->listWidget->clear();
    emit ShowDiagram(NS->getTopoEntityList(), NS->getTopoEdgeList(), NS->getInfPropList());
}

void DlgScanInfo::on_pushButton_2_clicked()
{   
    qDebug() << "dlg start";
    wizard =new Wizard();
    loadConfig(wizard);
    //if (wizard->LoadConfigFile())
   // {
        NS->SetStop(false);

        ui->pushButton_2->setEnabled(false);
//        SvLog::writeLogInit("Start scan");

        //SCAN_PARAM param;
        NS = new NetScan(dev_type_map, specialOidList, scanParam);
        connect(NS,SIGNAL(FinishAnalyse()),this,SLOT(ReceiveAnalyseResult()));
        connect(NS,SIGNAL(SendScanMessage(QString)),this,SLOT(ReceiveScanMessage(QString)));
        connect(NS,SIGNAL(StopScan()),this,SLOT(ReceiveStopScan()));
//        //connect(NS)

//        //NS->ReadConfig(wizard);
        NS->Start();
   // }
}

void DlgScanInfo::on_pushButton_clicked()
{
    if (!ui->pushButton_2->isEnabled())
    {
        qDebug() << "test";
        NS->SetStop(true);
        ui->pushButton->setEnabled(false);

        disconnect(NS,SIGNAL(FinishAnalyse()),this,SLOT(ReceiveAnalyseResult()));
        disconnect(NS,SIGNAL(SendScanMessage(QString)),this,SLOT(ReceiveScanMessage(QString)));
    }
    else
    {
        accept();
    }

}
