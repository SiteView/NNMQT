#include "scanperformanceconfigwizardform.h"
#include "ui_scanperformanceconfigwizardform.h"
#include "currentconfigwizardform.h"
#include "scanrangeconfigwizardform.h"

ScanPerformanceConfigWizardForm::ScanPerformanceConfigWizardForm(Wizard * w, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScanPerformanceConfigWizardForm)
{
    ui->setupUi(this);
    this->parent = parent;
    this->wizard = w;
    Initialize(w);
    QFont font = this->ui->label_Parameter->font();
    font.setBold(true);
    this->ui->label_Parameter->setFont(font);
    setAttribute(Qt::WA_DeleteOnClose);
}

void ScanPerformanceConfigWizardForm::Initialize(Wizard * w)
{
    this->ui->spinBox_Deep->setValue(w->scanPerformanceConfigWizard->ScanDeep);
    this->ui->spinBox_ThreadCount->setValue(w->scanPerformanceConfigWizard->SynThreadCount);
    this->ui->spinBox_ReTry->setValue(w->scanPerformanceConfigWizard->Retry);
    this->ui->spinBox_Timeout->setValue(w->scanPerformanceConfigWizard->TimeOut);
}

void ScanPerformanceConfigWizardForm::SaveTempCfg()
{
    this->wizard->scanPerformanceConfigWizard->ScanDeep = this->ui->spinBox_Deep->value();
    this->wizard->scanPerformanceConfigWizard->SynThreadCount = this->ui->spinBox_ThreadCount->value();
    this->wizard->scanPerformanceConfigWizard->Retry = this->ui->spinBox_ReTry->value();
    this->wizard->scanPerformanceConfigWizard->TimeOut = this->ui->spinBox_Timeout->value();
}

ScanPerformanceConfigWizardForm::~ScanPerformanceConfigWizardForm()
{
    delete ui;
}

void ScanPerformanceConfigWizardForm::on_pushButton_4_clicked()
{
    SaveTempCfg();
    CurrentConfigWizardForm * d = new CurrentConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void ScanPerformanceConfigWizardForm::on_pushButton_3_clicked()
{
    SaveTempCfg();
    ScanRangeConfigWizardForm * d = new ScanRangeConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void ScanPerformanceConfigWizardForm::on_pushButton_2_clicked()
{
    SaveTempCfg();
    this->wizard->Save();
    this->close();
}

void ScanPerformanceConfigWizardForm::on_pushButton_clicked()
{
    this->close();
}
