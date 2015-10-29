
TEMPLATE = lib
TARGET   = app

CONFIG += thread staticlib
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

include(app-dirs.pri)

#message(pwd=$(PWD))
#message("DATADIR="\"$${D_DATA_DIR}\")

DEFINES += D_RES_DIR=\\\"$${D_DATA_DIR}\\\"
#===============================================================================================================================

INCLUDEPATH += app global

#===============================================================================================================================

include(src-app.pri)

#===============================================================================================================================

RESOURCES += \
	data/yeimgs.qrc

#===============================================================================================================================
