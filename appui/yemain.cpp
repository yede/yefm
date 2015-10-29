
#include <QApplication>
#include <QDebug>

#include "yeapplication.h"
//#include "mountmanager.h"
//==================================================================================
#include "yeucs2table.h"

int main(int argc, char *argv[])
{
//	Ucs2Table::testChars(); return 0;

	Application app(argc, argv);

	if (app.hasPriorInstance()) {
		app.callPriorInstance(argc, argv);
		return 0;
	}

	int ret = 0;

	if (app.startSession(argc, argv)) {
		ret = app.exec();
		app.stopSession();
	}

	return ret;
}
