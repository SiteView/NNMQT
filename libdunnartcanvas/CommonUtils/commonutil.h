#ifndef COMMONUTIL_H
#define COMMONUTIL_H
#include <QString>
#include <QTableWidget>
#include <QStringList>
class CommonUtil
{
public:
    CommonUtil();
    bool isIpAddress(QString ip);
    void isStart_stop(QTableWidget* tableWidget,int r);
    bool isIpAddressRange(QString startip,QString endip);
private:
    QStringList ipAddressList;

};

#endif // COMMONUTIL_H
