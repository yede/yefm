
#include <QDebug>

#include "yeappdelegate.h"
//==============================================================================================================================

bool AppDelegate::showMimeDlg(QString &resultApp, const QString &mimeType)
{
	return (m_app->*m_showMimeDlg)(resultApp, mimeType);
}
