#ifndef YE_SIDECMDMODEL_H
#define YE_SIDECMDMODEL_H

#include <QObject>
#include "yetreemodel.h"
//==============================================================================================================================

class YeApplication;

class SideCmdModel : public TreeModel
{
	Q_OBJECT
public:
	explicit SideCmdModel(YeApplication *app, QObject *parent = 0);
	~SideCmdModel();

	void saveCommandList();
	void loadCommandList();

signals:

public slots:
	void onIconThemeChanged();

private:
	YeApplication *m_app;
};

#endif // YE_SIDECMDMODEL_H
