#include "deviceinfo.h"
#include "ui_deviceinfo.h"

DeviceInfo::DeviceInfo(IDBody devInfo, QString ip, QString strUserDefinedName, QString strRemark, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceInfo)
{
    ui->setupUi(this);
    ui->devIp->setText(ip);
    ui->devMac->setText(devInfo.baseMac.c_str());
    ui->devName->setText(devInfo.sysName.c_str());
    ui->devUserDefinedName->setText(strUserDefinedName);
    ui->devType->setText(devInfo.devTypeName.c_str());
    ui->devFac->setText(devInfo.devFactory.c_str());
    QString strDevMaskList;
    std::vector<std::string>::iterator it = devInfo.msks.begin();
    for(it; it != devInfo.msks.end(); it++)
    {
        strDevMaskList += (*it).c_str();
        strDevMaskList += ",";
    }
    ui->devSubnet->setText(strDevMaskList);
    ui->devOid->setText(devInfo.sysOid.c_str());
    ui->devRemark->setText(strRemark);
}

DeviceInfo::~DeviceInfo()
{
    delete ui;
}

void DeviceInfo::on_confirmButton_clicked()
{
    QString strUserDefinedName = ui->devUserDefinedName->text();
    QString strRemark = ui->devRemark->toPlainText();
    emit onConfirmButtonClicked(strUserDefinedName, strRemark);
    close();
}

void DeviceInfo::on_cancelButton_clicked()
{
    close();
}
