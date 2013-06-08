#ifndef MONITOR_H
#define MONITOR_H

#include "plugins/ScanPlugin/CommonDef.h"
#include <string>
#include <map>
#include "plugins/ScanPlugin/SnmpDG.h"
#include "plugins/ScanPlugin/SnmpPara.h"
#include "ui/wizard.h"
#include <QString>
#include <string>
#include <QThread>
#include <QTimer>
using namespace std;
//#include "plugins/ScanPlugin/CommonDef.h"

typedef std::list<IFREC> IFRecList;

typedef struct {
    string ifIndex;
    string ifAdminStatus;
    string ifStatus;
    string ifInOctets;
    string ifInUcastPkgs;
    string ifOutOctets;
    string ifOutUcastPkgs;
    int iTickCount;
}IFFLOW;

typedef list<IFFLOW> IFFlowList;

class monitor : public QThread
{
    Q_OBJECT
public:
    monitor(QString ip, bool bGetPortFlow = false, string port = "", QObject *parent = 0);
    ~monitor();

    IFPROP_LIST getInterfaceInfo();
    IFRecList getUnixInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList, bool bRouter);
    void  getCiscoInfProp(IFRecList &infprops, IFFlowList &infFlow, bool bRouter = false);
    void getPortFlow(bool bFirstGetPortFlow);
    //getInfProp(snmp, spr, ROUTER==devType);

    void loadConfig(Wizard *wizard);

    void Start();
public slots:
    void stopMonitor();

    void run();
signals:
    void SendIfrecInfo(IFRecList ifList, IFFlowList ifFlowList);
    void SendPortFlow(int iSpeed);

private:
    SnmpPara spr;
    SnmpDG snmp;

    bool m_bGetPortFlow;
    string m_ip;
    string m_port;

    uint m_iCurrentPortFlow;
    uint m_iLastPortFlow;
    int m_iLastTickCount;
    int m_iCurrentTickCount;

    bool m_bStop;

};

#endif // MONITOR_H
