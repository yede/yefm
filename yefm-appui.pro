
TEMPLATE = app
TARGET   = yefm

CONFIG += thread
QT     += core gui dbus

# Include widgets in QT 5
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

include(app-build.pri)

#===============================================================================================================================

INCLUDEPATH += appui \
	fswidget actionmgr \
	dialogs pastemgr mimedlg dirmodel app udev global

equals(QT_MAJOR_VERSION, 4) {
INCLUDEPATH += mimetypes-include mimetypes mimetypes/inqt5
LIBS += -L. \
		-lfswidget -lactionmgr \
		-ldialogs -lpastemgr -lmimedlg -ldirmodel -lapp \
		-lyudev -lglobal -lqmimetype \
		-L/usr/lib -ludev
}

equals(QT_MAJOR_VERSION, 5) {
LIBS += -L. \
		-lfswidget -lactionmgr \
		-ldialogs -lpastemgr -lmimedlg -ldirmodel -lapp \
		-lyudev -ludev -lglobal
}

unix:!macx {
#	CONFIG += link_pkgconfig
#	PKGCONFIG += libudev
}
#===============================================================================================================================

include(src-appui.pri)

#===============================================================================================================================

RESOURCES += \
	data/yeimgs.qrc

#===============================================================================================================================

include(install.pri)
