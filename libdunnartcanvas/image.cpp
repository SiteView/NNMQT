#include "image.h"
#include "libdunnartcanvas/ui/deviceportinformation.h"



namespace dunnart {


Image::Image(string ip, IDBody devInfo, list<IFREC> iflist, EDGE_LIST nodeEdge)
    :ShapeObj(x_shImage),
    m_strUserDefinedName(""),
    m_strDevRemark("")
{
    m_ip = ip;
    m_devInfo = devInfo;
    m_devIfList = iflist;
    m_nodeEdgeList = nodeEdge;

    setImage(m_ip, devInfo.devType);
    update();
}

Image::Image(string ip, IDBody devInfo)
    :ShapeObj(x_shImage),
    m_strUserDefinedName(""),
    m_strDevRemark("")
{
    m_ip = ip;
    m_devInfo = devInfo;
    setImage(m_ip, devInfo.devType);
    update();
}


Image::Image(const double x, const double y, const char *txt, int ptSize, string ip, IDBody devInfo)
    :ShapeObj(x_shImage),
    m_strUserDefinedName(""),
    m_strDevRemark("")
{
    CanvasItem::setPos(x, y);
    setPainterPath(buildPainterPath());
    m_ip = ip;
    m_devInfo = devInfo;

    setImage(m_ip, devInfo.devType);
    update();
}

Image::Image(QString id, const double x, const double y,
        int w, int h, const char *txt, int ptSize, string ip, IDBody devInfo)
    : ShapeObj(x_shImage),
    m_strUserDefinedName(""),
    m_strDevRemark("")
{
    setPainterPath(buildPainterPath());
    m_ip = ip;
    m_devInfo = devInfo;

    setImage(m_ip, devInfo.devType);
    update();
}

QString Image::getDevIp(void) const
{
    return m_ip.c_str();
}

QString Image::getDevMac(void) const
{
    return m_devInfo.baseMac.c_str();
}

QString Image::getUserDefinedName(void) const
{
    return m_strUserDefinedName;
}

void Image::setUserDefinedName(QString strUserDefinedName)
{
    m_strUserDefinedName = strUserDefinedName;
    update();
}

QString Image::getDevType(void) const
{
    return m_devInfo.devType.c_str();
}

QString Image::getDevName(void) const
{
    return m_devInfo.sysName.c_str();
}

QString Image::getDevFac(void) const
{
    return m_devInfo.devFactory.c_str();
}

//QString Image::getDevSubnet(void) const
//{
//    QString strSubnet;
//    return m_devInfo.
//}

QString Image::getDevNetmask(void) const
{
    QString strNetmask;

    for (int i = 0; i < m_devInfo.msks.size(); i++)
    {
        strNetmask += m_devInfo.msks.at(i).c_str();
        strNetmask += ",";
    }
    return strNetmask;
}

QString Image::getDevSubnet(void) const
{
    QString strSubnet;

    for(int i = 0; i < m_devInfo.ips.size(); i++)
    {
        strSubnet += m_devInfo.ips.at(0).c_str();
        strSubnet += ",";
    }
    return strSubnet;
}

QString Image::getDevOid(void) const
{
    return m_devInfo.sysOid.c_str();
}

QString Image::getDevDesc(void) const
{
    //return m_devInfo.sysSvcs.c_str();
}

QString Image::getDevRemark(void) const
{
    return m_strDevRemark;
}

void Image::setDevRemark(QString strDevRemark)
{
    m_strDevRemark = strDevRemark;
    update();
}

//test Image::getIDBody(void) const
//{
//    return te;
//}

//void Image::setIDBody(test test1)
//{
//    te = test1;
//    update();
//}

//QString Image::getDevType(void) const
//{
//    return m_devType;
//}

//void Image::setDevType(const QString &devType)
//{
//    m_devType = devType;
//    update();
//}
IDBody Image::getIDBody(void) const
{
    return m_devInfo;
}
EDGE_LIST Image::getEdgeList(void) const
{
    return m_nodeEdgeList;
}
void Image::setImage(string ip, string devType)
{

    if (!ip.find("DUMB"))
    {
        pix.load(":/resources/node_images/Other_Blue.png","png");
    }
    else
    {
        int iDevType = atoi(devType.c_str());
        switch(iDevType)
        {
        case 0:
            {
                pix.load(":/resources/node_images/SwitchRouter_Blue.png","png");
            }
            break;
        case 1:
            {
                pix.load(":/resources/node_images/Switch_Blue.png","png");
            }
            break;
        case 2:
            {
                pix.load(":/resources/node_images/Router_Blue.png","png");
            }
            break;
        case 3:
            {
                pix.load(":/resources/node_images/Firewall_Blue.png","png");
            }
            break;
        case 4:
            {
                pix.load(":/resources/node_images/Server_Blue.png","png");
            }
            break;
        case 5:
            {
                pix.load(":/resources/node_images/PC_Blue.png","png");
            }
            break;
        case 6:
            {
                pix.load(":/resources/node_images/Other_Blue.png","png");
            }
            break;
        case 7:
            {
                pix.load(":/resources/node_images/Hub_Blue.ico","ico");
            }
        default:
            {
                 pix.load(":/resources/node_images/Other_Blue.png","png");
            }
            break;
        }
    }
    update();
}

void Image::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->drawPixmap(-(pix.width() / 2), -(pix.height() / 2),pix);
    if (!m_ip.find("DUMB"))
    {
        //painter->drawText(painterPath().pointAtPercent(0.15),  m_ip.c_str());
        painter->drawText(-(pix.width() / 2), pix.height(), m_ip.c_str());
    }
    else
    {
        //painter->drawText(painterPath().pointAtPercent(0.15),  m_ip.c_str());
        painter->drawText(-(pix.width()), pix.height(), m_ip.c_str());
    }

}
QAction* Image::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu &menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    // Menu items to break from guidelines (alignment relationships).
    QList<QString> relationshipStrings;
    relationshipStrings << tr("Top Alignment")
                        << tr("Middle Alignment")
                        << tr("Bottom Alignment")
                        << tr("Left Alignment")
                        << tr("Centre Alignment")
                        << tr("Right Alignment");
    bool attachedToGuidelines = false;
    QList<QAction *> dettachActions;
    for (int i = 0; i < 6; ++i)
    {
        dettachActions.append(
                    menu.addAction(tr("Break ") + relationshipStrings[i]));
        if ( ! rels[i] )
        {
            // Hide the menu item, if not attached to a guideline in this
            // type of relationship.
            dettachActions[i]->setVisible(false);
        }
        else
        {
            attachedToGuidelines = true;
        }
    }
    if (attachedToGuidelines)
    {
        menu.addSeparator();
    }

    QAction* frontAction = menu.addAction(tr("Bring to Front"));
    QAction* backAction = menu.addAction(tr("Send to Back"));
     QAction* devicePropertyAction;
    if(m_devInfo.devType=="0"||m_devInfo.devType=="1"||m_devInfo.devType=="2"||m_devInfo.devType=="3")
    {
        devicePropertyAction = menu.addAction(tr("Device interface information"));
    }
    QAction *devInfoAction = menu.addAction(tr("Device properties"));

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == frontAction)
    {
        bringToFront();
    }
    else if (action == backAction)
    {
        sendToBack();
    }

    if(action == devicePropertyAction && devicePropertyAction!=NULL)
    {
        DevicePortInformation *devicePort = new DevicePortInformation(m_devIfList,QString(m_ip.c_str()));
        devicePort->show();
    }
    else if (action == devInfoAction)
    {
        devInfo= new DeviceInfo(m_devInfo, m_ip.c_str(), m_strUserDefinedName, m_strDevRemark);
        connect(devInfo, SIGNAL(onConfirmButtonClicked(QString,QString)), this, SLOT(changeValue(QString,QString)));
        devInfo->show();
    }

    for (int i = 0; i < 6; ++i)
    {
        if (action == dettachActions[i])
        {
            canvas()->beginUndoMacro(tr("Dettach From Guideline"));
            rels[i]->Deactivate(BOTH_SIDE);
            canvas()->interrupt_graph_layout();
        }
    }

    return action;
}

void Image::changeValue(QString strUserDefinedName, QString strRemark)
{
    m_strUserDefinedName = strUserDefinedName;
    m_strDevRemark = strRemark;
}


}



