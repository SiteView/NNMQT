#ifndef COMMUNITYOFRANGECONFIGWIZARD_H
#define COMMUNITYOFRANGECONFIGWIZARD_H

#include <QList>
#include <QString>

class CommunityOfRangeConfigWizard
{
public:
    CommunityOfRangeConfigWizard();

    QList<QString> CommunityOfRangeList;
    QString DefaultSetCommunity;
    QString DefaultGetCommunity;
};

#endif // COMMUNITYOFRANGECONFIGWIZARD_H
