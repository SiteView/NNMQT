#ifndef WIZARD_H
#define WIZARD_H

#include "seedsconfigwizard.h"
#include "communityofrangeconfigwizard.h"
#include "excluderangeconfigwizard.h"
#include "scanrangeconfigwizard.h"
#include "scanperformanceconfigwizard.h"
#include "currentconfigwizard.h"
#include <QString>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

class Wizard
{
public:
    Wizard();
    ~Wizard();

    SeedsConfigWizard * seedsConfigWizard;
    CommunityOfRangeConfigWizard * communityOfRangeConfigWizard;
    ExcludeRangeConfigWizard * excludeRangeConfigWizard;
    ScanRangeConfigWizard * scanRangeConfigWizard;
    ScanPerformanceConfigWizard * scanPerformanceConfigWizard;
    CurrentConfigWizard * currentConfigWizard;

    void Save();
    bool LoadConfigFile();
private:
    QString fileName;
    QString currentInfo;
    void changeValue(QDomElement &startElem, QString key, QString value);


};

#endif // WIZARD_H
