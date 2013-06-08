#-------------------------------------------------
#
# Project created by QtCreator 2012-09-06T13:42:23
#
#-------------------------------------------------

QT       += core gui

INCLUDEPATH += ./include \
                ./lib \

TEMPLATE = lib
CONFIG += shared

TARGET        = scanplugin #$$qtLibraryTarget(scanplugin)

include(../../common_options.qmake)
#include(./scan_plugin_options.pri)

#DESTDIR = $$[QT_INSTALL_PLUGINS]/styles

SOURCES += styleplugin.cpp \
    UnivDevice.cpp \
    TraceReader.cpp \
    TraceAnalyse.cpp \
    topoAnalyse.cpp \
    TelnetReader.cpp \
    SVPing.cpp \
    svlog.cpp \
    StreamData.cpp \
    SnmpPing.cpp \
    SnmpPara.cpp \
    SnmpDG.cpp \
    ScanUtils.cpp \
    ReadService.cpp \
    QConfig.cpp \
    PingFind.cpp \
    NetScan.cpp \
    HuaWeiDevice.cpp \
    H3CDevice.cpp \
    DeviceRegister.cpp \
    DeviceFactory.cpp \
    Device.cpp \
    CPUCount.cpp \
    CiscoDevice.cpp

HEADERS += styleplugin.h \
    UnivDevice.h \
    TraceReader.h \
    TraceAnalyse.h \
    topoAnalyse.h \
    threadpool.hpp \
    TelnetReader.h \
    SVPing.h \
    svlog.h \
    StreamData.h \
    stdafx.h \
    SnmpPing.h \
    SnmpPara.h \
    SnmpDG.h \
    ScanUtils.h \
    ReadService.h \
    QConfig.h \
    PingFind.h \
    NetScan.h \
    HuaWeiDevice.h \
    H3CDevice.h \
    DeviceRegister.h \
    DeviceFactory.h \
    Device.h \
    CPUCount.h \
    CommonDef.h \
    CiscoDevice.h \
    threadpool/task_adaptors.hpp \
    threadpool/subtask.hpp \
    threadpool/size_policies.hpp \
    threadpool/shutdown_policies.hpp \
    threadpool/scheduling_policies.hpp \
    threadpool/pool_adaptors.hpp \
    threadpool/pool.hpp \
    threadpool/future.hpp \
    threadpool/detail/worker_thread.hpp \
    threadpool/detail/subtask.hpp \
    threadpool/detail/sequential_task.hpp \
    threadpool/detail/scope_guard.hpp \
    threadpool/detail/pool_core.hpp \
    threadpool/detail/locking_ptr.hpp \
    threadpool/detail/future.hpp \
    threadpool/detail/countdown.hpp


LIBS += -Wl,--export-all-symbols -Wl,--no-whole-archive
win32 {
LIBS += -L./lib -lsnmp_pp  #don't change the position
LIBS += -lws2_32
LIBS += -L./lib -lboost_thread-mgw44-mt-1_34
LIBS += -L./lib -ldes
}

OTHER_FILES += scan_plugin_options.pri
