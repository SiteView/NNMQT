#include "commonutil.h"
#include "winsock2.h"
#include <QStringList>
CommonUtil::CommonUtil()
{

}
bool CommonUtil::isIpAddress(QString ip)
{
    QRegExp ipRegExp("((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d))))");
    bool isIp = ipRegExp.exactMatch(ip);
    return isIp;
}
void CommonUtil::isStart_stop(QTableWidget *tableWidget, int r)
{
    std::string startIp="";
    std::string endIp="";
    qulonglong start = 0;
    qulonglong end = 0;
    QString item;
    ipAddressList.clear();
    for(int a = 0; a <tableWidget->rowCount();a++)
    {
        if(tableWidget->item(a,0)!=0&&tableWidget->item(a,1)!=0)
        {
            startIp = tableWidget->item(a,0)->text().toStdString();
            endIp = tableWidget->item(a,1)->text().toStdString();
            start = inet_addr(startIp.c_str());
            end = inet_addr(endIp.c_str());
            if(startIp!=""&&endIp!="")
            {
                if((ntohl(start)< ntohl(end))&&a!=r)
                {
                    item = QString::number(start)+"-"+QString::number(end);
                    ipAddressList.append(item);
                }
            }
        }
    }
}
bool CommonUtil::isIpAddressRange(QString startip,QString endip)
{
    QString item1;
    qulonglong startItem1;
    qulonglong endItem1;
    qulonglong ip = 0;
    qulonglong goalStartIp =ntohl(inet_addr(startip.toStdString().c_str()));
    qulonglong goalEndIp = ntohl(inet_addr(endip.toStdString().c_str()));
    if(goalStartIp==0xFFFFFFFF)
    {
        goalStartIp = 0;
    }
    if(goalEndIp==0xFFFFFFFF)
    {
        goalEndIp = 0;
    }
    if(ipAddressList.count()>0)
    {
        for(int i =0;i<ipAddressList.count();i++)
        {
            item1 = ipAddressList.at(i);
            startItem1 = ntohl(QString(item1.split("-")[0]).toULongLong());
            endItem1 = ntohl(QString(item1.split("-")[1]).toULongLong());
            if(goalStartIp!=0&&goalEndIp!=0)
            {
                if(!(goalStartIp>endItem1||goalEndIp<startItem1))
                {
                    return false;
                }
            }
            if(goalStartIp!=0&&goalEndIp==0)
            {
                ip = goalStartIp;
            }
            if(goalStartIp==0&&goalEndIp!=0)
            {
                ip = goalEndIp;
            }
            if(ip>=startItem1&&ip<=endItem1)
            {
                return false;
            }
        }
    }
    return true;
}
