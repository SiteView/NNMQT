#include "deviceinfodialog.h"
#include "ui_deviceinfodialog.h"
#include <QtSql>
#include <QTableWidgetItem>
#include <QMessageBox>
DeviceInfoDialog::DeviceInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceInfoDialog)
{
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);
    setWindowFlags(Qt::Dialog|Qt::WindowMinMaxButtonsHint|Qt::WindowMinimizeButtonHint);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QSqlDatabase dbconn=QSqlDatabase::addDatabase("QSQLITE");    //添加数据库驱动
    dbconn.setDatabaseName("DeviceType.db");  //在工程目录新建一个DeviceType.db的文件

    if(!dbconn.open())    {
        qDebug()<<"open failed";
    }
    else
    {
        //以下执行相关SQL语句
        QSqlQuery query;
        ui->treeWidget->header()->hide();
        ui->treeWidget->setColumnCount(1);
        QStringList header;
        header<<tr("id")<<tr("devtype")<<tr("devname")<<tr("romversion")<<tr("factory")<<tr("primaryID");
        ui->tableWidget->setHorizontalHeaderLabels(header);
        ui->tableWidget->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget->horizontalHeader()->setResizeMode(3,QHeaderView::Stretch);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        QTreeWidgetItem* rootItem = new QTreeWidgetItem(ui->treeWidget,QStringList(tr("Device manufacturers")));
        rootItem->setExpanded(true);
        query.exec("select distinct factory from sysobjectid ");
        while(query.next())
        {
            QTreeWidgetItem* factoryItem = new QTreeWidgetItem(rootItem,QStringList(query.value(0).toString()));
            rootItem->addChild(factoryItem);
        }
        excuteSQL("select * from sysobjectid");
    }
    connect(ui->treeWidget,SIGNAL(itemActivated(QTreeWidgetItem*,int)),this,SLOT(on_showFactory(QTreeWidgetItem*,int)));
    connect(ui->tableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(on_itemChanged(QTableWidgetItem*)));
    connect(ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(on_tableWidget_cellDoubleClicked(int,int)));
}

DeviceInfoDialog::~DeviceInfoDialog()
{
    delete ui;
}

void DeviceInfoDialog::on_showFactory(QTreeWidgetItem * item, int column)
{
    QString itemText = item->text(column);
    if(itemText.isEmpty()||itemText == "")
    {
        excuteSQL("select * from sysobjectid where factory is null");
    }else
    {
        excuteSQL("select * from sysobjectid where factory = '"+itemText+"'");
    }
}
void DeviceInfoDialog::excuteSQL(QString sql)
{
    QSqlQuery query;
    int r = 0;
    int numRows;
    map.clear();
    query.exec(sql);
    QSqlDatabase defaultDB = QSqlDatabase::database();
    if (defaultDB.driver()->hasFeature(QSqlDriver::QuerySize)) {
     numRows = query.size();
    } else {
    // this can be very slow
    query.last();
    numRows = query.at() + 1;
    }
    ui->tableWidget->setRowCount(numRows);
    ui->tableWidget->setColumnCount(query.record().count()-1);
    query.exec(sql);
    while(query.next())
    {
        isAddRow = true;
        QTableWidgetItem* idItem = new QTableWidgetItem(query.value(0).toString());
        QTableWidgetItem* devtypeItem = new QTableWidgetItem(query.value(2).toString());
        QTableWidgetItem* devnameItem = new QTableWidgetItem(query.value(3).toString());
        QTableWidgetItem* romversionItem = new QTableWidgetItem(query.value(4).toString());
        QTableWidgetItem* factoryItem = new QTableWidgetItem(query.value(5).toString());
        QTableWidgetItem* primaryIdItem = new QTableWidgetItem(query.value(6).toString());

        switch(query.value(2).toInt())
        {
        case 0:
            devtypeItem->setText(tr("three-layer switch"));
            break;
        case 1:
            devtypeItem->setText(tr("two-layer switch"));
            break;
        case 2:
            devtypeItem->setText(tr("Router"));
            break;
        case 3:
            devtypeItem->setText(tr("Firewall"));
            break;
        case 4:
            devtypeItem->setText(tr("Host"));
            break;
        case 5:
            devtypeItem->setText(tr("Server"));
            break;
        default:
            devtypeItem->setText(tr("Other"));
            break;
        }

        ui->tableWidget->setItem(r,0,idItem);
        ui->tableWidget->setItem(r,1,devtypeItem);
        ui->tableWidget->setItem(r,2,devnameItem);
        ui->tableWidget->setItem(r,3,romversionItem);
        ui->tableWidget->setItem(r,4,factoryItem);
        ui->tableWidget->setItem(r,5,primaryIdItem);
        map.insert(query.value(6).toInt(),r);
        r++;
    }
    ui->tableWidget->hideColumn(2);
    ui->tableWidget->hideColumn(5);
}
void DeviceInfoDialog::on_addBtn_clicked()
{
    isAddRow = true;
    int lastRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(lastRow);
    ui->tableWidget->selectRow(lastRow);
    QSqlQuery query;
    int max;
    query.exec("select max(CAST(primaryID as int)) from sysobjectid");
    while(query.next())
    {
        max = query.value(0).toInt()+1;
        ui->tableWidget->setItem(lastRow,5,new QTableWidgetItem(QString::number(max)));
    }
    query.prepare("insert into sysobjectid (primaryID) values('"+QString::number(max)+"')");
    query.exec();
    ui->tableWidget->setItem(lastRow,0,new QTableWidgetItem);
    ui->tableWidget->setItem(lastRow,1,new QTableWidgetItem);
    ui->tableWidget->setItem(lastRow,2,new QTableWidgetItem);
    ui->tableWidget->setItem(lastRow,3,new QTableWidgetItem);
    ui->tableWidget->setItem(lastRow,4,new QTableWidgetItem);

}
void DeviceInfoDialog::on_searchBtn_clicked()
{
    excuteSQL("select * from sysobjectid");
    QSqlQuery query;
    query.exec("select * from sysobjectid where id  = '"+ui->lineEditID->text()+"'");
    while(query.next())
    {
        ui->tableWidget->selectRow( map.find(query.value(6).toInt()).value());
        ui->treeWidget->setCurrentItem( ui->treeWidget->findItems(query.value(5).toString(),Qt::MatchRecursive).at(0));
    }

}
void DeviceInfoDialog::on_saveBtn_clicked()
{
     QMapIterator<int, QStringList> rowMapIterator(primaryMap);
     rowMapIterator.toFront();
     QStringList rowList;
     QSqlQuery query;
     bool isSucceed;
     while(rowMapIterator.hasNext())
     {
         rowMapIterator.next();
         rowList = rowMapIterator.value();
         query.prepare("update sysobjectid set id=?,devtype=?,devname=?,romversion=?,factory=? where primaryID = '"+QString::number(rowMapIterator.key())+"'");
         for(int i = 0;i<rowList.count()-1;i++)
         {
             if(i == 1)
             {
                 QString deviceTpye =rowList.at(i);
                 if(deviceTpye.compare(tr("three-layer switch"))==0)
                 {
                     query.bindValue(i,0);
                 }else if(deviceTpye.compare(tr("two-layer switch"))==0)
                 {
                     query.bindValue(i,1);
                 }else if(deviceTpye.compare(tr("Router"))==0)
                 {
                     query.bindValue(i,2);
                 }else if(deviceTpye.compare(tr("Firewall"))==0)
                 {
                     query.bindValue(i,3);
                 }else if(deviceTpye.compare(tr("Host"))==0)
                 {
                     query.bindValue(i,4);
                 }else if(deviceTpye.compare(tr("Server"))==0)
                 {
                     query.bindValue(i,5);
                 }else if(deviceTpye.compare(tr("Other"))==0)
                 {
                     query.bindValue(i,6);
                 }

             }else
             {
                query.bindValue(i,rowList.at(i));
             }

         }
         isSucceed = query.exec();
         if(!isSucceed)
         {
             QMessageBox::information(this,tr("Information"),tr("Data save failed"),QMessageBox::Ok|QMessageBox::Cancel);
             return;
         }
     }
     if(isSucceed)
     {
         QMessageBox::information(this,tr("Information"),tr("Data save succeed "),QMessageBox::Ok|QMessageBox::Cancel);
     }
}
void DeviceInfoDialog::on_cancelBtn_clicked()
{
    this->close();
}
void DeviceInfoDialog::on_itemChanged(QTableWidgetItem *item)
{
     ui->tableWidget->removeCellWidget(row,column);
    if(!isAddRow)
    {
        int primaryID =ui->tableWidget->item(item->row(),5)->text().toInt();
        QStringList rowStringList;
        for(int i = 0;i<ui->tableWidget->columnCount();i++)
        {

            rowStringList.append(ui->tableWidget->item(item->row(),i)->text());
        }
        primaryMap.insert(primaryID,rowStringList);
    }

}
void DeviceInfoDialog::on_comboBoxChanged(QString text)
{
    ui->tableWidget->removeCellWidget(row,column);
    QTableWidgetItem* devtypeItem = new QTableWidgetItem(text);
    ui->tableWidget->setItem(row,column,devtypeItem);
}
void DeviceInfoDialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    isAddRow = false;
    if(column==1)
    {
        QStringList devTypeList;
        devTypeList<<tr("three-layer switch")<<tr("Router")<<tr("two-layer switch")<<tr("Firewall")<<tr("Host")<<tr("Server")<<tr("Other");
        QComboBox* deviceType = new QComboBox(ui->tableWidget);
        deviceType->addItems(devTypeList);
        int index = deviceType->findText(ui->tableWidget->item(row,column)->text());
        if(index!=-1)
        {
            deviceType->setCurrentIndex(index);
        }else
        {
            deviceType->setCurrentIndex(1);
        }

        ui->tableWidget->setCellWidget(row,column,deviceType);
        this->row = row;
        this->column = column;
        connect(deviceType,SIGNAL(currentIndexChanged(QString)),this,SLOT(on_comboBoxChanged(QString)));
    }
}
bool DeviceInfoDialog::deleteData()
{
    if(QMessageBox::Yes==QMessageBox::information(ui->tableWidget,tr("Information"),tr("Are you sure delete the data ?"),QMessageBox::Yes|QMessageBox::No))
    {
        QItemSelectionModel *selectionModel = this->ui->tableWidget->selectionModel();
        QModelIndexList selected = selectionModel->selectedIndexes();
        QMap<int, int> rowMap;
        QSqlQuery query;
        bool isOk;
        foreach(QModelIndex index,selected)
        {

            rowMap.insert(index.row(), ui->tableWidget->item(index.row(),5)->text().toInt());
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
            isOk = query.exec("delete from sysobjectid where primaryID = '"+QString::number(rowMapIterator.value())+"'");
            if(!isOk)
            {
                QMessageBox::information(ui->tableWidget,tr("Information"),tr("Data delete failed"),QMessageBox::Ok);
            }
        }
        return isOk;
    }
}

void DeviceInfoDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        deleteData();
    }
}
void DeviceInfoDialog::on_deleteBtn_clicked()
{
    deleteData();
}
