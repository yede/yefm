#ifndef YE_SIDECMDMODEL_H
#define YE_SIDECMDMODEL_H

#include <QObject>
#include "yetreemodel.h"
//==============================================================================================================================

class Application;

class SideCmdModel : public TreeModel
{
	Q_OBJECT
public:
	explicit SideCmdModel(Application *app, QObject *parent = 0);
	~SideCmdModel();

	void saveCommandList();
	void loadCommandList();

signals:

public slots:
	void onIconThemeChanged();

private:
	Application *m_app;
};

#endif // YE_SIDECMDMODEL_H
