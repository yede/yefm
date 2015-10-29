#include <QDebug>

#include "yesidecmdmodel.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"
//==============================================================================================================================

SideCmdModel::SideCmdModel(Application *app, QObject *parent)
	: TreeModel(parent)
	, m_app(app)
{

	connect(app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
}

SideCmdModel::~SideCmdModel()
{

}

void SideCmdModel::onIconThemeChanged()
{
}
//==============================================================================================================================

void SideCmdModel::saveCommandList()
{

}

void SideCmdModel::loadCommandList()
{

}
//==============================================================================================================================

