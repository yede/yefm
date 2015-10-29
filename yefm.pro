
TEMPLATE = subdirs

equals(QT_MAJOR_VERSION, 4): SUBDIRS = qmimetype

SUBDIRS += \
	global \
	yudev \
	app \
	dirmodel \
	mimedlg \
	pastemgr \
	dialogs \
	actionmgr \
	fswidget \
	appui

qmimetype.file = yefm-mimetype.pro

global.file    = yefm-global.pro
yudev.file     = yefm-udev.pro

app.file       = yefm-app.pro
dirmodel.file  = yefm-dirmodel.pro
mimedlg.file   = yefm-mimedlg.pro
pastemgr.file  = yefm-pastemgr.pro
dialogs.file   = yefm-dialogs.pro
actionmgr.file = yefm-actionmgr.pro
fswidget.file  = yefm-fswidget.pro

appui.file     = yefm-appui.pro
appui.depends  =

CONFIG += ordered
