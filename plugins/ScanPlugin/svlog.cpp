#include "StdAfx.h"
#include "SvLog.h"
#include "scanutils.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <iostream>

boost::mutex SvLog::m_log_mutex;
boost::mutex SvLog::m_err_log_mutex;

SvLog::SvLog(void)
{
}

SvLog::~SvLog(void)
{
}

// 记录错误日志
void SvLog::writeErrorLog(const string& msg, const string& LogFileName)
{
        boost::mutex::scoped_lock lock(m_err_log_mutex);
        QFile file(LogFileName.c_str());
        if (!file.open(QIODevice::WriteOnly))
        {
            return ;
        }

        QDateTime time = QDateTime::currentDateTime();
        QString str = time.toString("yyyy-MM-dd hh:mm:ss ddd");

        QTextStream out(&file);
        out << str << ":" << msg.c_str() << endl;
}

// 记录日志(初始)
void SvLog::writeLogInit(const string & msg)
{
    QFile file("scan_log.txt");
    if (!file.open(QIODevice::WriteOnly))
    {
        return ;
    }

    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss ddd");

    QTextStream out(&file);
    out << str << ":" << msg.c_str() << endl;
}
// 记录日志(追加)
void SvLog::writeLog(const string& msg)
{
        boost::mutex::scoped_lock lock(m_log_mutex);
        QFile file("scan_log.txt");
        if (!file.open(QIODevice::Append))
        {
            return ;
        }

        QDateTime time = QDateTime::currentDateTime();
        QString str = time.toString("yyyy-MM-dd hh:mm:ss ddd");

        QTextStream out(&file);
        out << str << ":" << msg.c_str() << endl;
}
