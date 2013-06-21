#ifndef SVLOG_H
#define SVLOG_H

//#include <boost/thread/mutex.hpp>
#include "commondef.h"

using namespace std;

class SvLog
{
public:
        //static boost::mutex m_log_mutex;
        //static boost::mutex m_err_log_mutex;

public:
        SvLog(void);
public:
        ~SvLog(void);

public:
        // 记录日志
        static void writeLogInit(const string & msg);
        static void writeLog(const string & msg);
        // 记录错误日志
        static void writeErrorLog(const string& msg, const string& LogFileName);
};


#endif // SVLOG_H
