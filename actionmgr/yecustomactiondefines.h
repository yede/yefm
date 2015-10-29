#ifndef YE_CUSTOM_ACTION_DEINES_H
#define YE_CUSTOM_ACTION_DEINES_H

#include <QStringList>
//==============================================================================================================================

#define APP_TAG      "[app]"	// desktop file: *.desktop
#define EXE_TAG      "[exe]"	// command

#define FILE_TAG     "[-F]"
#define FOLDER_TAG   "[-D]"
#define ANY_FILE_TAG "[*F]"
#define ANY_TAG      "[**]"

namespace FsActionKind {
	enum { DesktopApp,			// name of desktop file: *.desktop
		   DirectExec			// executable program file directly
		 };
}

namespace FsActionData {
	enum { Kind = Qt::UserRole,
		   Name,
		   Exec,
		   Icon,
		   Match,
		   Types
		 };
}

namespace FsActionMatch {
	enum { None,
		   Folder,
		   File,
		   AnyFile,	// any file
		   Any		// any file or folder
		 };
}
//==============================================================================================================================

#endif // YE_CUSTOM_ACTION_DEINES_H
