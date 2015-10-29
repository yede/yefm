#===============================================================================================================================

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
#===============================================================================================================================
