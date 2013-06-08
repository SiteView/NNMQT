#include "interfaceinfo.h"
#include "ui_interfaceinfo.h"
#include <QIntValidator>

InterfaceInfo::InterfaceInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InterfaceInfo)
{
    ui->setupUi(this);
    ui->unitComboBox->addItem(tr("Mbps"));
    ui->unitComboBox->addItem(tr("Kbps"));

    ui->flowThreshold->setText("1");

    QIntValidator* aIntValidator = new QIntValidator;
    aIntValidator->setRange(1, 1023);
    ui->flowThreshold->setValidator(aIntValidator);

    setModal(true);
}

InterfaceInfo::~InterfaceInfo()
{
    delete ui;
}

void InterfaceInfo::on_confirmBtn_clicked()
{
    int iSpeed = 0;
    QString strSpeed = ui->flowThreshold->text();
    iSpeed = strSpeed.toInt();
//    if (0 == ui->unitComboBox->currentIndex())
//    {
//        iSpeed = 1024 * iSpeed;
//    }
    emit flowThreChanged(iSpeed, ui->unitComboBox->currentIndex());
    accept();

}

void InterfaceInfo::on_cancelBtn_clicked()
{
    accept();
}

void InterfaceInfo::onModifyFlow(int iSpeed, int iIndex)
{
    QString strSpeed = QString::number(iSpeed);
    ui->flowThreshold->setText(strSpeed);
    ui->unitComboBox->setCurrentIndex(iIndex);
}
