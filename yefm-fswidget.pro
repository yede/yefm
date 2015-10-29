
TEMPLATE = lib
TARGET   = fswidget

CONFIG += staticlib
QT     += core gui

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

INCLUDEPATH += global app \
	dirmodel actionmgr pastemgr \
	fswidget

#===============================================================================================================================

include(src-fswidget.pri)

#===============================================================================================================================

RESOURCES += \
	data/yeimgs.qrc

#===============================================================================================================================
