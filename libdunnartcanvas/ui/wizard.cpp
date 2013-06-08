#include "wizard.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QStringList>
#include <QMessageBox>

Wizard::Wizard() : fileName("./NetScanConfig.xml")
{
    seedsConfigWizard = new SeedsConfigWizard();
    communityOfRangeConfigWizard = new CommunityOfRangeConfigWizard();
    excludeRangeConfigWizard = new ExcludeRangeConfigWizard();
    scanRangeConfigWizard = new ScanRangeConfigWizard();
    scanPerformanceConfigWizard = new ScanPerformanceConfigWizard();
    currentConfigWizard = new CurrentConfigWizard();
    this->LoadConfigFile();
    currentInfo =QObject::tr("Scanning parameters:")+"\n"
                      +QObject::tr("  Search Depth: ")+QString::number(this->scanPerformanceConfigWizard->ScanDeep)+"\r\n"
                      +QObject::tr("  The number of parallel threads: ")+QString::number(this->scanPerformanceConfigWizard->SynThreadCount)+"\r\n"
                      +QObject::tr("  Retry count: ")+QString::number(this->scanPerformanceConfigWizard->Retry)+"\r\n"
                      +QObject::tr("  Timeout period: ")+QString::number(this->scanPerformanceConfigWizard->TimeOut)+"\r\n\n"
                      +QObject::tr("Scanning ranges:")+"\n";
       foreach(QString item,this->scanRangeConfigWizard->ScanRangeList)
       {
           currentInfo += "  " +item + ";";
       }
       currentInfo +="\n\n" + QObject::tr("Excluding ranges:")+"\n";
       foreach(QString item,this->excludeRangeConfigWizard->ExcludeRangeList)
       {
           currentInfo +=" " + item + ";";
       }
       currentInfo +="\n\n" + QObject::tr("Communities:")+"\n"
                     + QObject::tr("  Default Reading community: ")
                     +this->communityOfRangeConfigWizard->DefaultGetCommunity + "\n"
                     +QObject::tr("  Defaults writing community: ")
                     +this->communityOfRangeConfigWizard->DefaultSetCommunity + "\n";
       foreach(QString item,this->communityOfRangeConfigWizard->CommunityOfRangeList)
       {
           currentInfo +="  " + item + ";\n";
       }
       currentInfo +="\n" + QObject::tr("Scanning seeds:")+"\n";
       foreach(QString item,this->seedsConfigWizard->SeedsList)
       {
           currentInfo +="  " + item + "\n";
       }
       currentInfo +="\n" + QObject::tr("Scanning algorithm:")+"\n";

       this->currentConfigWizard->CurrentConfigString = currentInfo;
}

Wizard::~Wizard()
{
    delete communityOfRangeConfigWizard;
    delete excludeRangeConfigWizard;
    delete scanRangeConfigWizard;
    delete scanPerformanceConfigWizard;
    delete currentConfigWizard;
}

void Wizard::changeValue(QDomElement &startElem, QString key, QString value)
{
    QDomNode oldElem, newElem;
    QDomElement firstElem;
    firstElem = startElem.firstChildElement(key);
    oldElem = firstElem.firstChild();
    firstElem.firstChild().setNodeValue(value);
    newElem = firstElem.firstChild();
    startElem.replaceChild(newElem,oldElem);
}


void Wizard::Save()
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(&file,false,&errorStr,&errorLine,&errorColumn))
    {
        file.close();
        return;
    }
    file.close();

    QDomElement startElem = doc.documentElement();

    //
    //--------ScanPerformanceConfigWizard
    //
    changeValue(startElem,"ScanDeep",QString::number(this->scanPerformanceConfigWizard->ScanDeep));
    changeValue(startElem,"SynThreadCount",QString::number(this->scanPerformanceConfigWizard->SynThreadCount));
    changeValue(startElem,"Retry",QString::number(this->scanPerformanceConfigWizard->Retry));
    changeValue(startElem,"TimeOut",QString::number(this->scanPerformanceConfigWizard->TimeOut));

    //
    //----------ScanRangeConfigWizard
    //
    QString ScanRange;
    if(this->scanRangeConfigWizard->ScanRangeList.count() == 0)
    {
        ScanRange = ";";
    }else
    {
        foreach(QString item,this->scanRangeConfigWizard->ScanRangeList)
        {
            ScanRange += item + ";";
        }
    }
    changeValue(startElem,"ScanRange",ScanRange);

    //
    //----------ExcludeRangeConfigWizard
    //
    QString ExcludeRange;
    if(this->excludeRangeConfigWizard->ExcludeRangeList.count() == 0)
    {
        ExcludeRange = ";";
    }else
    {
        foreach(QString item,this->excludeRangeConfigWizard->ExcludeRangeList)
        {
            ExcludeRange += item + ";";
        }
    }
    changeValue(startElem,"ExcludeRange",ExcludeRange);

    //
    //---------CommunityOfRangeConfigWizard
    //
    QString CommunityOfRange;
    if(this->communityOfRangeConfigWizard->CommunityOfRangeList.count() == 0)
    {
        CommunityOfRange = ";";
    }else
    {
        foreach(QString item,this->communityOfRangeConfigWizard->CommunityOfRangeList)
        {
            CommunityOfRange += item + ";";
        }
    }
    changeValue(startElem, "CommunityOfRange", CommunityOfRange);
    changeValue(startElem, "DefaultCommunityGet", this->communityOfRangeConfigWizard->DefaultGetCommunity);
    changeValue(startElem, "DefaultCommunitySet", this->communityOfRangeConfigWizard->DefaultSetCommunity);

    //
    //---------SeedsConfigWizard
    //
    QString Seeds;
    if(this->seedsConfigWizard->SeedsList.count() == 0)
    {
        Seeds = ";";
    }else
    {
        foreach(QString item,this->seedsConfigWizard->SeedsList)
        {
            Seeds += item + ";";
        }
    }
    changeValue(startElem, "Seeds", Seeds);



    QFile f(fileName);
    if (!f.open(QFile::WriteOnly | QFile::Text))
        return;


    QTextStream out(&f);
    out.reset();
    out.setCodec("utf-8");
    startElem.save(out, 4, QDomNode::EncodingFromTextStream);
    //doc.save(out, 4, QDomNode::EncodingFromTextStream);
    f.close();
}

bool Wizard::LoadConfigFile()
{
    QFile file(this->fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox msgbox;
        msgbox.setText("NetScanConfig.xml does not exist,please check it");
        msgbox.exec();
        return false;
    }
    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        file.close();
        return false;
    }
    QDomElement startElem = doc.documentElement();

    //
    //--------ScanPerformanceConfigWizard
    //
    this->scanPerformanceConfigWizard->ScanDeep = startElem.firstChildElement("ScanDeep").text().toInt();
    this->scanPerformanceConfigWizard->SynThreadCount = startElem.firstChildElement("SynThreadCount").text().toInt();
    this->scanPerformanceConfigWizard->Retry = startElem.firstChildElement("Retry").text().toInt();
    this->scanPerformanceConfigWizard->TimeOut = startElem.firstChildElement("TimeOut").text().toInt();
    //
    //--------ScanRangeConfigWizard
    //
    this->scanRangeConfigWizard->ScanRangeList.clear();
    QStringList ScanRangeList = startElem.firstChildElement("ScanRange").text().split(";",QString::SkipEmptyParts);
    foreach(QString item,ScanRangeList)
    {
        this->scanRangeConfigWizard->ScanRangeList.append(item);
    }

    //
    //--------ExcludeRangeConfigWizard
    //
    this->excludeRangeConfigWizard->ExcludeRangeList.clear();
    QStringList ExcludeRangeList = startElem.firstChildElement("ExcludeRange").text().split(";",QString::SkipEmptyParts);
    foreach(QString item,ExcludeRangeList)
    {
        this->excludeRangeConfigWizard->ExcludeRangeList.append(item);
    }

    //
    //--------CommunityOfRangeConfigWizard
    //
    this->communityOfRangeConfigWizard->DefaultGetCommunity = startElem.firstChildElement("DefaultCommunityGet").text();
    this->communityOfRangeConfigWizard->DefaultSetCommunity = startElem.firstChildElement("DefaultCommunitySet").text();
    this->communityOfRangeConfigWizard->CommunityOfRangeList.clear();
    QStringList CommunityOfRangeList = startElem.firstChildElement("CommunityOfRange").text().split(";",QString::SkipEmptyParts);
    foreach(QString item,CommunityOfRangeList)
    {
        this->communityOfRangeConfigWizard->CommunityOfRangeList.append(item);
    }

    //
    //--------SeedsConfigWizard
    //
    this->seedsConfigWizard->SeedsList.clear();
    QStringList SeedsList = startElem.firstChildElement("Seeds").text().split(";",QString::SkipEmptyParts);
    foreach(QString item,SeedsList)
    {
        this->seedsConfigWizard->SeedsList.append(item);
    }

    file.close();

    return true;
}

