
target.path = /usr/bin

desktop.path  += /usr/share/applications
desktop.files += data/yefm.desktop

icon.path  += /usr/share/pixmaps
icon.files += data/imgs/yefm.png

docs.path  += /usr/share/doc/yefm
docs.files += README CHANGELOG COPYING

trans.path  += /usr/share/yefm/translations
trans.files += \
	data/translations/yefm_zh_CN.qm

INSTALLS += target desktop icon docs trans

OTHER_FILES += \
	TODO.txt
