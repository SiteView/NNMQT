
#include <QtCore/QCoreApplication>
#include <QtDebug>
#include "NetScan.h"
#include <QtSql\qsql.h>
#include <QtSql\qsqlquery.h>
#include <QtSql\qsqldatabase.h>
#include <QtSql\qsqlerror.h>
#include <QTextCodec>

DEVICE_TYPE_MAP dev_type_map;
SCAN_PARAM scanParam;
void initDevType();
void initScanParam();

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	SPECIAL_OID_LIST specialOidList;
	//SCAN_PARAM scanParam;

	initDevType();
	initScanParam();
	NetScan *NS = new NetScan(dev_type_map, specialOidList, scanParam);

	NS->Start();

	return a.exec();
}

void initDevType()
{
	//QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	//QTextCodec::setCod
	//QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB18030"));
    QSqlDatabase dbconn=QSqlDatabase::addDatabase("QSQLITE");    //添加数据库驱动

    dbconn.setDatabaseName("DeviceType.db");  //在工程目录新建一个DeviceType.db的文件

    if(!dbconn.open())    {
        qDebug()<<"open failed";
    }
    else
    {
        //QSqlQuery query;//以下执行相关QSL语句

		QSqlQuery query(dbconn);

		query = QSqlQuery::QSqlQuery(dbconn);

        if (query.exec("select id,unknown,devtype,devname,romversion,factory from sysobjectid where id>=1"))
		{

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
		else
		{
			QSqlError str = query.lastError();
		}

    }
}

void initScanParam()
{
	scanParam.depth = 1;
	scanParam.retrytimes = 3;
	scanParam.timeout = 200;
	scanParam.thrdamount = 1;
	scanParam.community_get_dft = "public";
	scanParam.scan_scales.push_back(make_pair("192.168.0.248", "192.168.0.248"));
	scanParam.communitys.push_back(make_pair(make_pair("192.168.0.248", "192.168.0.248"), make_pair("public","public")));
	unsigned long startnum = ntohl(inet_addr("192.168.0.248"));

	scanParam.scan_scales_num.push_back(make_pair(startnum, startnum));
	scanParam.communitys_num.push_back(make_pair(make_pair(startnum,startnum), make_pair("public","public")));
}




 