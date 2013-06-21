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
        // ��¼��־
        static void writeLogInit(const string & msg);
        static void writeLog(const string & msg);
        // ��¼������־
        static void writeErrorLog(const string& msg, const string& LogFileName);
};


#endif // SVLOG_H
