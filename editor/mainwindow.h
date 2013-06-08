/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow  <mjwybrow@users.sourceforge.net>
 * Copyright (C) 2006-2008  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 * 
*/

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <map>

class QApplication;
class QFileInfo;
class QDockWidget;
#include "libdunnartcanvas/ui/deviceinfodialog.h"
#include "libdunnartcanvas/ui/dlgscaninfo.h"
#include "plugins/ScanPlugin/CommonDef.h"
#include <string>
#include "image.h"
//#include "libdunnartcanvas/monitor.h"
using namespace std;

namespace dunnart {

class Application;
class CanvasView;
class Canvas;
class CanvasTabWidget;
class ZoomLevel;
class LayoutPropertiesDialog;
class ConnectorPropertiesDialog;
class CreateAlignmentDialog;
class CreateDistributionDialog;
class CreateSeparationDialog;
class CreateTemplateDialog;
class PropertiesEditorDialog;
class ShapePickerDialog;
class UndoHistoryDialog;
class CanvasOverviewDialog;

////设备基本属性数据 (TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)
//typedef struct {
//        std::string baseMac;//基本Mac地址
//        std::string snmpflag;//
//        std::string community_get;
//        std::string community_set;
//        std::string devType;//
//        std::string devFactory;//
//        std::string devModel;//
//        std::string devTypeName;//
//        std::string sysOid;//sysObjectID 1.3.6.1.2.1.1.2
//        std::string sysSvcs;//sysServices 1.3.6.1.2.1.1.7
//        std::string sysName;//sysName 1.3.6.1.2.1.1.5
//        std::vector<std::string> ips;//IP地址1.3.6.1.2.1.4.20.1.3
//        std::vector<std::string> infinxs;//IP地址对应的索引1.3.6.1.2.1.4.20.1.2
//        std::vector<std::string> msks;//MASK地址1.3.6.1.2.1.4.20.1.3
//        std::vector<std::string> macs;//MAC地址1.3.6.1.2.1.2.2.1.6
//}IDBody;

////边结构
//typedef struct{
//        std::string ip_left;//左端点ip
//        std::string pt_left;//左端点port
//        std::string inf_left;//左端点inf
//        std::string dsc_left;//左端点描述信息
//        std::string ip_right;//右端点ip
//        std::string pt_right;//右端点port
//        std::string inf_right;//右端点inf
//        std::string dsc_right;//右端点描述信息
//}EDGE;


////设备基本信息列表{devIP,(TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)}
//typedef std::map<std::string, IDBody> DEVID_LIST;

//typedef std::list<EDGE> EDGE_LIST;


static const int MAX_RECENT_FILES = 10;

class MainWindow : public QMainWindow
{
    Q_OBJECT;

    public:
        MainWindow(Application *app);
        virtual ~MainWindow() { }

        Canvas *canvas(void);
        CanvasView *view(void);
        QList<CanvasView *> views(void);
        void newCanvasTab(void);
        bool loadDiagram(const QString& filename);
    public slots:
        void showhidePc(bool isHide);
    private slots:
        void documentNew(void);
        void documentOpen(void);
        void documentOpenRecent();
        void documentExport(void);
        void documentPrint(void);
        void about(void);
        void openHomepage(void);
        void clearRecentFileMenu(void);
        void canvasChanged(Canvas *);
        void canvasFileInfoChanged(const QFileInfo& fileinfo);               
        void On_ActionScan_Config();
        void on_ActionDeviceInfo();
        void On_ActionStart();
        void ShowPhysicalDiagram(DEVID_LIST device_lst, EDGE_LIST edge_lst, IFPROP_LIST ifprop_list);

        void setAutoLayout(bool autoLayout);
        void timerShow();
        void setEnglish();
        void setChinese();
    protected:
        virtual void closeEvent(QCloseEvent *event);

    private:
        void updateRecentFileActions(void);
        QString strippedName(const QString& fullFileName);
        int langSwitchTip();

        CanvasTabWidget *m_tab_widget;

        QToolBar *m_edit_toolbar;

        QMenu *m_file_menu;
        QMenu *m_edit_menu;
        QMenu *m_view_menu;
        QMenu *m_scan_menu;
        QMenu *m_layout_menu;
        QMenu *m_language_menu;
        QMenu *m_help_menu;

        QAction *m_new_action;
        QAction *m_open_action;
        QAction *m_close_action;
        QAction *m_save_action;
        QAction *m_print_action;
        QAction *m_save_as_action;
        QAction *m_export_action;
        QAction *m_quit_action;
        QAction *m_action_show_zoom_level_dialog;
        QAction *m_action_show_layout_properties_dialog;
        QAction *m_action_show_connector_properties_dialog;
        QAction *m_action_show_create_alignment_dialog;
        QAction *m_action_show_create_distribution_dialog;
        QAction *m_action_show_create_separation_dialog;
        QAction *m_action_show_create_template_dialog;
        QAction *m_action_show_properties_editor_dialog;
        QAction *m_action_show_shape_picker_dialog;
        QAction *m_action_show_undo_history_dialog;
        QAction *m_action_show_canvas_overview_dialog;
        QAction *m_action_open_recent_file[MAX_RECENT_FILES];
        QAction *m_action_recent_file_separator;
        QAction *m_action_clear_recent_files;
        QAction *m_action_scan_config;
        QAction *m_action_device_info;
        QAction *m_action_start_scan;
        QAction *m_action_hidePc;
        QAction *m_action_autolayout;
        QAction *m_about_action;
        QAction *m_homepage_action;
        QAction *m_english_action;
        QAction *m_chinese_action;

        Application *m_application;

        ZoomLevel *m_dialog_zoomLevel;
        LayoutPropertiesDialog *m_dialog_layoutProps;
        ConnectorPropertiesDialog *m_dialog_connectorProps;
        CreateAlignmentDialog *m_dialog_alignment;
        CreateDistributionDialog *m_dialog_distribution;
        CreateSeparationDialog *m_dialog_separation;
        CreateTemplateDialog *m_dialog_template;
        PropertiesEditorDialog *m_dialog_properties_editor;
        ShapePickerDialog *m_dialog_shape_picker;
        UndoHistoryDialog *m_dialog_undo_history;
        CanvasOverviewDialog *m_dialog_canvas_overview;

        DeviceInfoDialog *deviceInfoDlg;


        map<string,int> map_devType;
        DEVID_LIST device_list;
        EDGE_LIST edge_list;
        IFPROP_LIST ifprop_list;

        map<QString, Image*> ipAndNode;
        map<Connector*, EDGE> edgeList;
        //monitor *devMonitor;

        //QList<monitor *> monitorList;

        QSettings *setLang;


};


}
#endif // MAINWINDOW_H_
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

