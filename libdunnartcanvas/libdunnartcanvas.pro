
TEMPLATE = lib
TARGET = dunnartcanvas

CONFIG += shared

DEPENDPATH +=  .. ../libogdf .
INCLUDEPATH += .. ../libogdf . ../plugins/ScanPlugin/ ../plugins/ScanPlugin/include

CONFIG += link_pkgconfig qt thread
QT += sql xml svg
PKGCONFIG += 

include(../common_options.qmake)

include(qtpropertybrowser/qtpropertybrowser.pri)

graphviz {
	PKGCONFIG += libgvc
}
!graphviz {
	DEFINES += NOGRAPHVIZ
}

win32 {
LIBS += -Wl,--export-all-symbols -Wl,--no-whole-archive
}
LIBS += -L$$DESTDIR -lavoid -lvpsc -ltopology -lcola  -lscanplugin -logdf -lsnmp_pp -lws2_32 -lboost_thread-mgw44-mt-1_34 -ldes


# Input
RESOURCES += \
	libdunnartcanvas.qrc \
    node.qrc \
    language.qrc \
    qss.qrc
FORMS = \
	ui/LayoutProperties.ui \
	ui/ZoomLevel.ui \
	ui/createalignment.ui \
	ui/createdistribution.ui \
	ui/createseparation.ui \
	ui/connectorproperties.ui \
    ui/createtemplate.ui \
    ui/shapepickerdialog.ui \
    ui/canvasoverview.ui \
    ui/seedsconfigwizardform.ui \
    ui/scanrangeconfigwizardform.ui \
    ui/scanperformanceconfigwizardform.ui \
    ui/excluderangeconfigwizardform.ui \
    ui/DlgScanInfo.ui \
    ui/deviceinfodialog.ui \
    ui/currentconfigwizardform.ui \
    ui/communityofrangeconfigwizardform.ui \
    ui/deviceportinformation.ui \
    ui/deviceinfo.ui \
    ui/interfaceinfo.ui

SOURCES += \ 
	FMMLayout.cpp \
	align3.cpp \
	oldcanvas.cpp \
	canvasitem.cpp \
	cluster.cpp \
	connector.cpp \
	distribution.cpp \
	expand_grid.cpp \
	freehand.cpp \
	graphdata.cpp \
	graphlayout.cpp \
	graphvizlayout.cpp \
	guideline.cpp \
	indicator.cpp \
	interferencegraph.cpp \
	nearestpoint.cpp \
	placement.cpp \
	polygon.cpp \
	separation.cpp \
	shape.cpp \
	textshape.cpp \
	undo.cpp \
	visibility.cpp \
	canvas.cpp \
	canvasview.cpp \
	utility.cpp \
	gmlgraph.cpp \
	template-constraints.cpp \
	templates.cpp \
	relationship.cpp \
	handle.cpp \
	svgshape.cpp \
	canvastabwidget.cpp \
	ui/layoutproperties.cpp \
	ui/zoomlevel.cpp \
	ui/createalignment.cpp \
	ui/createtemplate.cpp \
	ui/createdistribution.cpp \
	ui/createseparation.cpp \
	ui/connectorproperties.cpp \
	ui/propertieseditor.cpp \
    pluginshapefactory.cpp \
    pluginfileiofactory.cpp \
    ui/shapepickerdialog.cpp \
    connectorhandles.cpp \
    ui/undohistorydialog.cpp \
    stronglyconnectedcomponents.cpp \
    connectionpininfo.cpp \
    canvasapplication.cpp \
    pluginapplicationmanager.cpp \
    ui/canvasoverview.cpp \
    ui/wizard.cpp \
    ui/seedsconfigwizardform.cpp \
    ui/seedsconfigwizard.cpp \
    ui/scanrangeconfigwizardform.cpp \
    ui/scanrangeconfigwizard.cpp \
    ui/scanperformanceconfigwizardform.cpp \
    ui/scanperformanceconfigwizard.cpp \
    ui/excluderangeconfigwizardform.cpp \
    ui/excluderangeconfigwizard.cpp \
    ui/dlgscaninfo.cpp \
    ui/deviceinfodialog.cpp \
    ui/currentconfigwizardform.cpp \
    ui/currentconfigwizard.cpp \
    ui/communityofrangeconfigwizardform.cpp \
    ui/communityofrangeconfigwizard.cpp \
    image.cpp \
    CommonUtils/commonutil.cpp \
    ui/deviceportinformation.cpp \
    ui/deviceinfo.cpp \
    monitor.cpp \
    ui/interfaceinfo.cpp

HEADERS += \
	FMMLayout.h \
	align3.h \
	oldcanvas.h \
	canvasitem.h \
	cluster.h \
	connector.h \
	distribution.h \
	expand_grid.h \
	freehand.h \
	gmlgraph.h \
	graphdata.h \
	graphlayout.h \
	graphvizlayout.h \
	guideline.h \
	indicator.h \
	interferencegraph.h \
	nearestpoint.h \
	placement.h \
	polygon.h \
	separation.h \
	shape.h \
	shared.h \
	textshape.h \
	undo.h \
	visibility.h \
	canvas.h \
	canvasview.h \
	template-constraints.h \
	templates.h \
	utility.h \
	githash.h \
	relationship.h \
	handle.h \
	svgshape.h \
	canvastabwidget.h \
	ui/layoutproperties.h \
	ui/zoomlevel.h \
	ui/createalignment.h \
	ui/createtemplate.h \
	ui/createdistribution.h \
	ui/createseparation.h \
	ui/connectorproperties.h \
	ui/propertieseditor.h \
    pluginshapefactory.h \
    pluginfileiofactory.h \
    ui/shapepickerdialog.h \
    connectorhandles.h \
    ui/undohistorydialog.h \
    stronglyconnectedcomponents.h \
    connectionpininfo.h \
    shapeplugininterface.h \
    fileioplugininterface.h \
    applicationplugininterface.h \
    canvasapplication.h \
    pluginapplicationmanager.h \
    ui/canvasoverview.h \
    ui/wizard.h \
    ui/seedsconfigwizardform.h \
    ui/seedsconfigwizard.h \
    ui/scanrangeconfigwizardform.h \
    ui/scanrangeconfigwizard.h \
    ui/scanperformanceconfigwizardform.h \
    ui/scanperformanceconfigwizard.h \
    ui/excluderangeconfigwizardform.h \
    ui/excluderangeconfigwizard.h \
    ui/dlgscaninfo.h \
    ui/deviceinfodialog.h \
    ui/currentconfigwizardform.h \
    ui/currentconfigwizard.h \
    ui/communityofrangeconfigwizardform.h \
    ui/communityofrangeconfigwizard.h \
    image.h \
    CommonUtils/commonutil.cpp \
    ui/deviceportinformation.h \
    ui/deviceinfo.h \
    monitor.h \
    ui/interfaceinfo.h


displaygithash {
  githash.target = githash.h
  win32 {
    githash.commands = write_gitver.bat $$githash.target
  }
  else {
    githash.commands = @ver=`git show --abbrev-commit | grep "^commit" | cut -f2 -d\' \'`; echo \"`cat .hash`define GITHASH \\\"\$\$ver\\\"\" > $$githash.target;
  }
  githash.depends = dummy

  QMAKE_EXTRA_TARGETS += githash dummy
  
  PRE_TARGETDEPS += githash.h
}
