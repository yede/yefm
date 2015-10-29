
TEMPLATE = app
TARGET   = yefm

include(src-actionmgr.pri)
include(src-dialogs.pri)
include(src-global.pri)
include(src-pastemgr.pri)
include(src-app.pri)
include(src-dirmodel.pri)
include(src-mimedlg.pri)
include(src-appui.pri)
include(src-fswidget.pri)
include(src-mimetype.pri)
include(src-udev.pri)

TRANSLATIONS = \
	data/translations/yefm_zh_CN.ts
