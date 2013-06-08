#include "seedsconfigwizardform.h"
#include "ui_seedsconfigwizardform.h"
#include "communityofrangeconfigwizardform.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include "CommonUtils/commonutil.h"
SeedsConfigWizardForm::SeedsConfigWizardForm(Wizard * w, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SeedsConfigWizardForm)
{
    ui->setupUi(this);
    this->parent = parent;
    this->wizard = w;
    this->ui->pushButton_3->setEnabled(false);
    setAttribute(Qt::WA_DeleteOnClose);
    this->ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->ui->tableWidget->horizontalHeader()->setVisible(false);
    int i = 0;
    foreach(QString itemString,this->wizard->seedsConfigWizard->SeedsList)
    {
        QTableWidgetItem* item = new QTableWidgetItem(itemString);
        if(i<this->wizard->seedsConfigWizard->SeedsList.count())
        {
            this->ui->tableWidget->setItem(i,0,item);
            i++;
        }
    }
    this->ui->tableWidget->setRowCount(i+1);
    this->ui->tableWidget->setColumnCount(1);
    connect(this->ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(on_addRow_DoubleClicked(int,int)));
    connect(ui->tableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_tableWidget_itemChanged(QTableWidgetItem*)));
}

SeedsConfigWizardForm::~SeedsConfigWizardForm()
{
    delete ui;
}

void SeedsConfigWizardForm::SaveTempCfg()
{
    this->wizard->seedsConfigWizard->SeedsList.clear();
    for(int i = 0; i < this->ui->tableWidget->rowCount(); i++)
    {
        if(this->ui->tableWidget->item(i,0)!=0)
        {
            if(this->ui->tableWidget->item(i,0)->text()!="")
            {
                this->wizard->seedsConfigWizard->SeedsList.append(this->ui->tableWidget->item(i,0)->text());
            }
        }

    }
}

void SeedsConfigWizardForm::on_pushButton_4_clicked()
{
    SaveTempCfg();
    CommunityOfRangeConfigWizardForm * d = new CommunityOfRangeConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void SeedsConfigWizardForm::on_pushButton_3_clicked()
{
    this->close();
}

void SeedsConfigWizardForm::on_pushButton_2_clicked()
{
    SaveTempCfg();
    this->wizard->Save();
    this->close();
}

void SeedsConfigWizardForm::on_pushButton_clicked()
{
    this->close();
}
void SeedsConfigWizardForm::on_addRow_DoubleClicked(int r, int c)
{
    int lastRow = this->ui->tableWidget->rowCount();
    if(r == lastRow-1)
    {
        this->ui->tableWidget->insertRow(lastRow);
    }
}
void SeedsConfigWizardForm::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Delete)
    {
        QItemSelectionModel *selectionModel = this->ui->tableWidget->selectionModel();
        QModelIndexList selected = selectionModel->selectedIndexes();
        QMap<int, int> rowMap;
        foreach(QModelIndex index,selected)
        {
            rowMap.insert(index.row(), 0);
        }
        int rowToDel; //记录当前删除行
        //定义一个迭代器，并定位rowmap的反面
        QMapIterator<int, int> rowMapIterator(rowMap);
        rowMapIterator.toBack();
        //从后往前依次删除行
        while (rowMapIterator.hasPrevious())
        {
            rowMapIterator.previous();
            rowToDel = rowMapIterator.key();
            if(ui->tableWidget->rowCount()!= 1)
            {
                this->ui->tableWidget->removeRow(rowToDel);
            }
        }
    }
}
void SeedsConfigWizardForm::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if(item != 0)
    {
        if(item->text()!=""||!item->text().isEmpty())
        {
            CommonUtil* commonUtil ;
            if(!commonUtil->isIpAddress(item->text()))
            {
                QMessageBox::information(ui->tableWidget,"Information","IP address error !",QMessageBox::Ok);
                item->setText("");
            }
        }
    }
}
