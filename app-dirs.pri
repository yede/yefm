
exists(build-ide.pri) {
	TOP_DIR    = /home/nat/ydev/yede
	D_DATA_DIR = $${TOP_DIR}/projects/yefm-src/data
} else {
	D_DATA_DIR = /usr/share/yefm
}
