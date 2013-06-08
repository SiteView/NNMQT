#ifndef QCONFIG_H
#define QCONFIG_H

#include <QString>
#include <list>
#include <utility>
#include <string>
#include <vector>
using namespace std;

struct Community
{
    string StartIp;
    string EndIp;
    string Get;
    string Set;
};

typedef vector<Community> CommunityLst;
typedef vector<string> SeedsList;

class QConfig
{
public:
    QConfig();
    ~QConfig();

    QString getDefaultCommunityGet() {return m_strDefaultCommunityGet; }
    QString getDefaultCommuitySet() {return m_strDefaultCommunitySet; }
    int GetDepth() { return m_iDepth; }
    int GetTryTimes() { return m_iTryTimes; }
    int GetMaxThreadCount() { return m_iMaxThreadCount; }
    int GetTimeOut() { return m_iTimeOut; }

    std::list<std::pair<std::string, std::string> > m_listScanScales;
    CommunityLst communityList;
    list<string> scan_seeds_list;
    SeedsList seeds;

private:
    // 缺省读共同体名
    QString m_strDefaultCommunityGet;
    // 缺省写共同体名
    QString m_strDefaultCommunitySet;
    // 扫描深度
    unsigned int m_iDepth;
    // 重试次数
    unsigned int m_iTryTimes;
    // 线程数量
    unsigned int m_iMaxThreadCount;
    // 超时时间
    unsigned int m_iTimeOut;
    //list<QString> m_lstScanSeeds;
    //std::list<std::pair<std::string, std::string> > m_listScanScales;
    //std::list<std::pair<unsigned long, unsigned long> > scan_scales_num;
    //<<范围>,<读写共同体名>>
    //std::list<std::pair<std::pair<std::string, std::string>, std::pair<std::string,std::string>> > communitys;
    //std::list<std::pair<std::pair<unsigned long, unsigned long>, std::pair<std::string,std::string>> > communitys_num;
    //排除的范围
    //std::list<std::pair<std::string,std::string> > filter_scales;
    //std::list<std::pair<unsigned long, unsigned long> > filter_scales_num;
};

#endif // QCONFIG_H
