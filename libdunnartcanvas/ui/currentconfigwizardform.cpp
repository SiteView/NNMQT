#include "currentconfigwizardform.h"
#include "ui_currentconfigwizardform.h"
#include "scanperformanceconfigwizardform.h"

CurrentConfigWizardForm::CurrentConfigWizardForm(Wizard * w, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CurrentConfigWizardForm)
{
    ui->setupUi(this);
    this->parent = parent;
    ui->pushButton_4->setEnabled(false);
    setAttribute(Qt::WA_DeleteOnClose);
    this->wizard = w;
    this->ui->textBrowser->setText(wizard->currentConfigWizard->CurrentConfigString);
}

CurrentConfigWizardForm::~CurrentConfigWizardForm()
{
    delete ui;
}

void CurrentConfigWizardForm::on_pushButton_4_clicked()
{

}

void CurrentConfigWizardForm::on_pushButton_3_clicked()
{
    ScanPerformanceConfigWizardForm * d = new ScanPerformanceConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void CurrentConfigWizardForm::on_pushButton_2_clicked()
{
    this->wizard->Save();
    this->close();
}

void CurrentConfigWizardForm::on_pushButton_clicked()
{
    this->close();
}
