TEMPLATE = app
TARGET   = yefm

CONFIG += thread
QT     += core gui dbus

# Include widgets in QT 5
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent

CONFIG(debug, debug|release) {
	BUILD_TYPE = .d
	CONFIG += warn_on
	QMAKE_CFLAGS   += -pg
	QMAKE_CXXFLAGS += -pg
	QMAKE_LFLAGS   += -pg
} else {
	BUILD_TYPE =
	CONFIG += release warn_off
}
#===============================================================================================================================

exists(build-ide.pri) {
	include(build-ide.pri)
} else {
	include(build.pri)
}

#message("DESTDIR="\"$${DESTDIR}\")
#message("DATADIR="\"$${D_DATA_DIR}\")

DEFINES += D_BIN_DIR=\\\"$${DESTDIR}\\\"
DEFINES += D_RES_DIR=\\\"$${D_DATA_DIR}\\\"
#===============================================================================================================================

#OBJECTS_DIR = build
#MOC_DIR = build

DEPENDPATH  += . src src/mime-type
INCLUDEPATH += . src src/mime-type

#LIBS += -lmagic

unix:!macx {
#	CONFIG += link_pkgconfig
#	PKGCONFIG += glib-2.0
}
#===============================================================================================================================

HEADERS += \
	src/yeapp.h \
	src/yeapplication.h \
	src/yemainwindow.h \
	src/yefilepane.h \
	src/yeproperties.h \
	src/yefileutils.h \
	src/yemime.h \
	src/yefsmodelitem.h \
	src/yefsmodel.h \
	src/yedesktopfile.h \
	src/yefscache.h \
    src/yefslistmodel.h \
	src/yefstreemodel.h \
	src/yefslistview.h \
	src/yesplitter.h \
    src/yesplitterhandle.h \
    src/yepathwidget.h \
	src/yepathbutton.h \
	src/yetoolbar.h \
	src/yetoolitem.h \
	src/yeuserview.h \
	src/yetreenode.h \
	src/yetreemodel.h \
	src/yeusertabmodel.h \
	src/yeuserbmkmodel.h \
	src/yeuserbmkdlg.h \
	src/yeprogressdlg.h \
	src/yepastemgr.h \
    src/yestatusbar.h \
	src/yemimedlg.h \
	src/yemimesettings.h \
	src/yesettingsdlg.h \
	src/yepropertiesdlg.h \
	src/yedbus.h \
	src/yelineedit.h \
	src/yefsactionsdlg.h \
    src/yefsactions.h \
    src/yeinfopad.h \
    src/yeappresources.h \
    src/yeappdata.h \
    src/yestylesettings.h \
    src/yefswidget.h \
    src/yefstreeview.h \
    src/yefshandler.h \
    src/yesidesysview.h \
	src/yemount.h \
	src/yesidemntmodel.h \
	src/yesidecmdmodel.h

SOURCES += \
	src/yemain.cpp \
	src/yeapp.cpp \
	src/yeapplication.cpp \
	src/yemainwindow.cpp \
	src/yefilepane.cpp \
	src/yeproperties.cpp \
	src/yefileutils.cpp \
	src/yemime.cpp \
	src/yefsmodelitem.cpp \
	src/yefsmodel.cpp \
	src/yedesktopfile.cpp \
	src/yefscache.cpp \
    src/yefslistmodel.cpp \
	src/yefstreemodel.cpp \
	src/yefslistview.cpp \
	src/yesplitter.cpp \
    src/yesplitterhandle.cpp \
    src/yepathwidget.cpp \
	src/yepathbutton.cpp \
	src/yetoolbar.cpp \
	src/yetoolitem.cpp \
	src/yeuserview.cpp \
	src/yetreenode.cpp \
	src/yetreemodel.cpp \
	src/yeusertabmodel.cpp \
	src/yeuserbmkmodel.cpp \
	src/yeuserbmkdlg.cpp \
	src/yeprogressdlg.cpp \
	src/yepastemgr.cpp \
    src/yestatusbar.cpp \
	src/yemimedlg.cpp \
	src/yemimesettings.cpp \
	src/yesettingsdlg.cpp \
	src/yepropertiesdlg.cpp \
	src/yedbus.cpp \
	src/yelineedit.cpp \
	src/yefsactionsdlg.cpp \
    src/yefsactions.cpp \
    src/yeinfopad.cpp \
    src/yeappresources.cpp \
    src/yeappdata.cpp \
    src/yestylesettings.cpp \
    src/yefswidget.cpp \
    src/yefstreeview.cpp \
    src/yefshandler.cpp \
    src/yesidesysview.cpp \
	src/yemount.cpp \
	src/yesidemntmodel.cpp \
	src/yesidecmdmodel.cpp

HEADERS += \
#	src/mime-type/mime-type.h \
#	src/mime-type/mime-action.h \
#	src/mime-type/mime-cache.h

SOURCES += \
#	src/mime-type/mime-type.cpp \
#	src/mime-type/mime-action.cpp \
#	src/mime-type/mime-cache.cpp

FORMS += \
	src/yeuserbmkdlg.ui \
	src/yesettingsdlg.ui \
	src/yefsactionsdlg.ui \
    src/yestylesettings.ui

RESOURCES += \
	data/yeimgs.qrc

TRANSLATIONS = \
	data/translations/yefm_zh_CN.ts

#===============================================================================================================================
equals(QT_MAJOR_VERSION, 4) {

INCLUDEPATH += \
	mimetypes-include \
	mimetypes \
	mimetypes/inqt5

SOURCES += \
	mimetypes/qmimedatabase.cpp \
	mimetypes/qmimetype.cpp \
	mimetypes/qmimemagicrulematcher.cpp \
	mimetypes/qmimetypeparser.cpp \
	mimetypes/qmimemagicrule.cpp \
	mimetypes/qmimeglobpattern.cpp \
	mimetypes/qmimeprovider.cpp

the_includes.files += \
	mimetypes/qmime_global.h \
	mimetypes/qmimedatabase.h \
	mimetypes/qmimetype.h \

HEADERS += \
	$$the_includes.files \
	mimetypes/qmimemagicrulematcher_p.h \
	mimetypes/qmimetype_p.h \
	mimetypes/qmimetypeparser_p.h \
	mimetypes/qmimedatabase_p.h \
	mimetypes/qmimemagicrule_p.h \
	mimetypes/qmimeglobpattern_p.h \
	mimetypes/qmimeprovider_p.h

SOURCES += mimetypes/inqt5/qstandardpaths.cpp
win32: SOURCES += mimetypes/inqt5/qstandardpaths_win.cpp
unix: {
	macx-*: {
		SOURCES += mimetypes/inqt5/qstandardpaths_mac.cpp
		LIBS += -framework Carbon
	} else {
		SOURCES += mimetypes/inqt5/qstandardpaths_unix.cpp
	}
}

RESOURCES += \
	mimetypes/mimetypes.qrc

}
#===============================================================================================================================

include(install.pri)
