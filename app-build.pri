
exists(build-ide.pri) {
	include(build-ide.pri)
} else {
	include(build.pri)
}
