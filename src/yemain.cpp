
#include <QApplication>

#include "yeapplication.h"
//==================================================================================

int main(int argc, char *argv[])
{
	YeApplication app(argc, argv);

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
