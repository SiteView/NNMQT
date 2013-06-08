#ifndef IMAGE_H
#define IMAGE_H

#include <QPainter>
#include "libdunnartcanvas/shape.h"
#include "plugins/ScanPlugin/CommonDef.h"
#include "libdunnartcanvas/ui/deviceinfo.h"
#include <string>
using namespace std;

namespace dunnart {


class Image : public ShapeObj
{
    Q_OBJECT
//    Q_PROPERTY (QString ip READ getDevIp)
//    Q_PROPERTY (QString mac READ getDevMac)
//    Q_PROPERTY (QString name READ getDevName)
//    Q_PROPERTY (QString userDefinedName READ getUserDefinedName WRITE setUserDefinedName)
//    Q_PROPERTY (QString devType READ getDevType)
//    Q_PROPERTY (QString fac READ getDevFac)
//    Q_PROPERTY (QString subnet READ getDevSubnet)
//    Q_PROPERTY (QString netmask READ getDevNetmask)
//    Q_PROPERTY (QString oid READ getDevOid)
//    //Q_PROPERTY (QString desc READ getDevDesc)
//    Q_PROPERTY (QString remark READ getDevRemark WRITE setDevRemark)
//    //Q_PROPERTY (IDBody devInfo READ )
//    //Q_PROPERTY (QString devType READ getDevType)
//    //Q_PROPERTY (test te READ getIDBody WRITE setIDBody)
//    Q_ENUMS(IDBody)

public:
    Image(string ip, IDBody devInfo, list<IFREC>, EDGE_LIST nodeEdge);
    Image(string ip, IDBody devInfo);
    Image(const double x, const double y, const char *txt, int ptSize, string ip, IDBody devInfo);
    Image(QString id, const double x, const double y, int w, int h,
            const char *txt, int ptSize, string ip, IDBody devInfo);

    //QString getDevType(void) const;
    QString getDevIp(void) const;
    QString getDevMac(void) const;
    QString getUserDefinedName(void) const;
    void setUserDefinedName(QString strUserDefinedName);
    QString getDevType(void) const;
    QString getDevName(void) const;
    QString getDevFac(void) const;
    //QString getDevSubnet(void) const;
    QString getDevNetmask(void) const;
    QString getDevSubnet(void) const;
    QString getDevOid(void) const;
    QString getDevDesc(void) const;
    QString getDevRemark(void) const;
    IDBody getIDBody(void) const;
    EDGE_LIST getEdgeList(void) const;
    void setDevRemark(QString strDevRemark);

    //test getIDBody(void) const;
    //void setIDBody(test devInfo);
    //QString
    //virtual void setDevType(const QString& devType);

    void setImage(string ip, string devType);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //void setDevType(const QString &devType);
    QAction *buildAndExecContextMenu(
                    QGraphicsSceneMouseEvent *event, QMenu& menu);

public slots:
    void changeValue(QString strUserDefinedName, QString strRemark);
private:
    QPixmap pix;
    //string m_ip;
    QString m_strUserDefinedName;
    QString m_strDevRemark;

    DeviceInfo *devInfo;
    EDGE_LIST m_nodeEdgeList;

    //IDBody m_devInfo;
    //list<IFREC> m_devIfList;
   // test te;



};

}

#endif // IMAGE_H
