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
    // ȱʡ����ͬ����
    QString m_strDefaultCommunityGet;
    // ȱʡд��ͬ����
    QString m_strDefaultCommunitySet;
    // ɨ�����
    unsigned int m_iDepth;
    // ���Դ���
    unsigned int m_iTryTimes;
    // �߳�����
    unsigned int m_iMaxThreadCount;
    // ��ʱʱ��
    unsigned int m_iTimeOut;
    //list<QString> m_lstScanSeeds;
    //std::list<std::pair<std::string, std::string> > m_listScanScales;
    //std::list<std::pair<unsigned long, unsigned long> > scan_scales_num;
    //<<��Χ>,<��д��ͬ����>>
    //std::list<std::pair<std::pair<std::string, std::string>, std::pair<std::string,std::string>> > communitys;
    //std::list<std::pair<std::pair<unsigned long, unsigned long>, std::pair<std::string,std::string>> > communitys_num;
    //�ų��ķ�Χ
    //std::list<std::pair<std::string,std::string> > filter_scales;
    //std::list<std::pair<unsigned long, unsigned long> > filter_scales_num;
};

#endif // QCONFIG_H
