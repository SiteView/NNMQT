#include "excluderangeconfigwizardform.h"
#include "ui_excluderangeconfigwizardform.h"
#include "scanrangeconfigwizardform.h";
#include "communityofrangeconfigwizardform.h"
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include "Winsock2.h"

ExcludeRangeConfigWizardForm::ExcludeRangeConfigWizardForm(Wizard * w, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExcludeRangeConfigWizardForm)
{
    ui->setupUi(this);
    this->parent = parent;
    this->wizard = w;
    commonUtil = new CommonUtil;
    setAttribute(Qt::WA_DeleteOnClose);
    this->ui->exludeLoad_Btn->setVisible(false);
    isFirst = true;
    QStringList header;
    header<<tr("Starting ip address")<<tr("Ending ip address");
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    this->ui->tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
    this->ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    int i = 0;
    foreach(QString item,w->excludeRangeConfigWizard->ExcludeRangeList)
    {
        QTableWidgetItem * itemfrom = new QTableWidgetItem(item.split("-")[0]);
        QTableWidgetItem * itemto = new QTableWidgetItem(item.split("-")[1]);
        if(i<w->excludeRangeConfigWizard->ExcludeRangeList.count())
        {
            this->ui->tableWidget->setItem(i,0,itemfrom);
            this->ui->tableWidget->setItem(i,1,itemto);
            i++;
        }
    }
    this->ui->tableWidget->setRowCount(i+1);
    this->ui->tableWidget->setColumnCount(2);
    connect(this->ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(on_addRow_DoubleClicked(int,int)));
    connect(ui->tableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_tableWidget_itemChanged(QTableWidgetItem*)));
}

ExcludeRangeConfigWizardForm::~ExcludeRangeConfigWizardForm()
{
    delete ui;
    delete commonUtil;
}

void ExcludeRangeConfigWizardForm::SaveTempCfg()
{
    QString item = "";
    this->wizard->excludeRangeConfigWizard->ExcludeRangeList.clear();
    for(int a = 0; a < this->ui->tableWidget->rowCount();a++)
    {
        if(this->ui->tableWidget->item(a,0)!=0&&this->ui->tableWidget->item(a,1)!=0)
        {
            if(this->ui->tableWidget->item(a,0)->text()!=""&&this->ui->tableWidget->item(a,1)->text()!="")
            {
                item = this->ui->tableWidget->item(a,0)->text() + "-" +this->ui->tableWidget->item(a,1)->text();
                this->wizard->excludeRangeConfigWizard->ExcludeRangeList.append(item);
            }
        }
     }

}

void ExcludeRangeConfigWizardForm::on_pushButton_4_clicked()
{
    SaveTempCfg();
    ScanRangeConfigWizardForm * d = new ScanRangeConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void ExcludeRangeConfigWizardForm::on_pushButton_3_clicked()
{
    SaveTempCfg();
    CommunityOfRangeConfigWizardForm * d = new CommunityOfRangeConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void ExcludeRangeConfigWizardForm::on_pushButton_2_clicked()
{
    SaveTempCfg();
    this->wizard->Save();
    this->close();
}

void ExcludeRangeConfigWizardForm::on_pushButton_clicked()
{
    this->close();
}
void ExcludeRangeConfigWizardForm::on_addRow_DoubleClicked(int r, int c)
{
    isFirst = false;
    int lastRow = this->ui->tableWidget->rowCount();
    if(r == lastRow-1)
    {
        this->ui->tableWidget->insertRow(lastRow);
    }
}
void ExcludeRangeConfigWizardForm::keyPressEvent(QKeyEvent *event)
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
void ExcludeRangeConfigWizardForm::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    int r = item->row();
    int c = item->column();
    if(item != 0)
    {
        if(item->text()!=""||!item->text().isEmpty())
        {
            if(!commonUtil->isIpAddress(item->text()))
            {
                QMessageBox::information(ui->tableWidget,"Information","IP address error !",QMessageBox::Ok);
                item->setText("");
                item->setSelected(true);
            }else{
                if(!isFirst)
                {
                    if(c==0)
                    {
                        commonUtil->isStart_stop(ui->tableWidget,r);
                        if(ui->tableWidget->item(r,1)!=0&&ui->tableWidget->item(r,1)->text()!="")
                        {
                            if(ntohl(inet_addr(ui->tableWidget->item(r,1)->text().toStdString().c_str()))< ntohl(inet_addr(item->text().toStdString().c_str())))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("The starting ip address:")+item->text()+tr("is not greater than the ending ip address:"),QMessageBox::Ok);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }
                            if(!commonUtil->isIpAddressRange(item->text(),ui->tableWidget->item(r,1)->text()))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("Inputting the IP address ranges has repeat"),QMessageBox::Ok);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }

                        }else
                        {
                            if(!commonUtil->isIpAddressRange(item->text(),""))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("Inputting the IP address ranges has repeat"),QMessageBox::Ok);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }
                        }
                    }
                    if(c==1)
                    {
                        commonUtil->isStart_stop(ui->tableWidget,r);
                        if(ui->tableWidget->item(r,0)!=0&&ui->tableWidget->item(r,0)->text()!="")
                        {
                            if(ntohl(inet_addr(ui->tableWidget->item(r,0)->text().toStdString().c_str()))> ntohl(inet_addr(item->text().toStdString().c_str())))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("The ending ip address:")+item->text()+tr("can't be less than starting IP address:"),QMessageBox::Ok);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }
                            if(!commonUtil->isIpAddressRange(ui->tableWidget->item(r,0)->text(),item->text()))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("Inputting the IP address ranges has repeat"),QMessageBox::NoButton);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }

                        }else
                        {
                            if(!commonUtil->isIpAddressRange("",item->text()))
                            {
                                QMessageBox::information(ui->tableWidget,"",tr("Inputting the IP address ranges has repeat"),QMessageBox::NoButton);
                                item->setText("");
                                ui->tableWidget->selectRow(r);
                            }
                        }
                    }
                }
            }
        }
    }
}
