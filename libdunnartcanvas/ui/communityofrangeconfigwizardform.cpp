#include "communityofrangeconfigwizardform.h"
#include "ui_communityofrangeconfigwizardform.h"
#include "excluderangeconfigwizardform.h"
#include "seedsconfigwizardform.h"

#include <QMessageBox>

CommunityOfRangeConfigWizardForm::CommunityOfRangeConfigWizardForm(Wizard * w, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommunityOfRangeConfigWizardForm)
{
    ui->setupUi(this);
    this->parent = parent;
    this->wizard = w;
    commonUtil = new CommonUtil;
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->ui->communityLoad_Btn->setVisible(false);

    this->ui->EditCommunityGet->setText(w->communityOfRangeConfigWizard->DefaultGetCommunity);
    this->ui->EditComunitySet->setText(w->communityOfRangeConfigWizard->DefaultSetCommunity);

    QStringList header;
    header<<tr("Starting ip address")<<tr("Ending ip address")<<"GET";
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
    this->ui->tableWidget->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
    this->ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    int i = 0;
    foreach(QString item,w->communityOfRangeConfigWizard->CommunityOfRangeList)
    {
        QStringList items = item.split("-",QString::SkipEmptyParts);
        QTableWidgetItem * itemfrom = new QTableWidgetItem(items[0]);
        QTableWidgetItem * itemto = new QTableWidgetItem(items[1]);
        QTableWidgetItem * community = new QTableWidgetItem(items[2]);
        if(i<this->wizard->communityOfRangeConfigWizard->CommunityOfRangeList.count())
        {
            this->ui->tableWidget->setItem(i,0,itemfrom);
            this->ui->tableWidget->setItem(i,1,itemto);
            this->ui->tableWidget->setItem(i,2,community);
            i++;
        }
    }
    this->ui->tableWidget->setRowCount(i+1);
    this->ui->tableWidget->setColumnCount(3);
    connect(this->ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(on_addRow_DoubleClicked(int,int)));
    connect(ui->tableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_tableWidget_itemChanged(QTableWidgetItem*)));
}

CommunityOfRangeConfigWizardForm::~CommunityOfRangeConfigWizardForm()
{
    delete ui;
    delete commonUtil;
}

void CommunityOfRangeConfigWizardForm::SaveTempCfg()
{
    this->wizard->communityOfRangeConfigWizard->DefaultGetCommunity = this->ui->EditCommunityGet->text();
    this->wizard->communityOfRangeConfigWizard->DefaultSetCommunity = this->ui->EditComunitySet->text();
    QString CommunityOfRange = NULL;
    this->wizard->communityOfRangeConfigWizard->CommunityOfRangeList.clear();
    for(int i = 0; i < this->ui->tableWidget->rowCount(); i++)
    {
        if(this->ui->tableWidget->item(i,0)!=0&&this->ui->tableWidget->item(i,1)!=0&&this->ui->tableWidget->item(i,2)!=0)
        {
            if(this->ui->tableWidget->item(i,0)->text()!=""&&this->ui->tableWidget->item(i,1)->text()!=""&&this->ui->tableWidget->item(i,2)->text()!="")
            {
                CommunityOfRange = this->ui->tableWidget->item(i,0)->text() + "-" +
                                    this->ui->tableWidget->item(i,1)->text() + "-" +
                                    this->ui->tableWidget->item(i,2)->text();
                this->wizard->communityOfRangeConfigWizard->CommunityOfRangeList.append(CommunityOfRange);
            }
        }

    }
}

void CommunityOfRangeConfigWizardForm::on_pushButton_4_clicked()
{
    SaveTempCfg();
    ExcludeRangeConfigWizardForm * d = new ExcludeRangeConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void CommunityOfRangeConfigWizardForm::on_pushButton_3_clicked()
{
    SaveTempCfg();
    SeedsConfigWizardForm * d = new SeedsConfigWizardForm(this->wizard,this->parent);
    d->show();
    this->close();
}

void CommunityOfRangeConfigWizardForm::on_pushButton_2_clicked()
{
    SaveTempCfg();
    this->wizard->Save();
    this->close();
}

void CommunityOfRangeConfigWizardForm::on_pushButton_clicked()
{
    this->close();
}

void CommunityOfRangeConfigWizardForm::on_BtnLoad_clicked()
{

}
void CommunityOfRangeConfigWizardForm::on_addRow_DoubleClicked(int r, int c)
{
    int lastRow = this->ui->tableWidget->rowCount();
    if(r == lastRow-1)
    {
        this->ui->tableWidget->insertRow(lastRow);
    }
}
void CommunityOfRangeConfigWizardForm::keyPressEvent(QKeyEvent *event)
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
void CommunityOfRangeConfigWizardForm::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if(item != 0)
    {
        if(item->text()!=""||!item->text().isEmpty())
        {
            if(item->column()!=2)
            {
                if(!commonUtil->isIpAddress(item->text()))
                {
                    QMessageBox::information(ui->tableWidget,"Information","IP address error !",QMessageBox::Ok);
                    item->setText("");
                }
            }
        }
    }
}
