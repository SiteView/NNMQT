#include "styleplugin.h"


StylePlugin::StylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
}

QStringList StylePlugin::keys() const
{
    return QStringList() << "netscan";
}

QStyle *StylePlugin::create(const QString &key)
{
    if (key.toLower() == "netscan")
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

Q_EXPORT_PLUGIN2(ScanPlugin, StylePlugin)
