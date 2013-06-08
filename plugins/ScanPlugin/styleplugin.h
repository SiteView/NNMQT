#ifndef STYLEPLUGIN_H
#define STYLEPLUGIN_H

#include <QtGui/QStylePlugin>
#include <QStyle>
#include <QStringList>


class StylePlugin : public QStylePlugin {
    Q_OBJECT
public:
    StylePlugin(QObject *parent = 0);

    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif // STYLEPLUGIN_H
