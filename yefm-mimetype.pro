
TEMPLATE = lib
TARGET   = qmimetype

CONFIG += staticlib
QT     += core

# Include widgets in QT 5
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

#DEPENDPATH  += .
INCLUDEPATH += \
	mimetypes-include \
	mimetypes \
	mimetypes/inqt5

#===============================================================================================================================

include(src-mimetype.pri)

RESOURCES += \
	mimetypes/mimetypes.qrc

#===============================================================================================================================
