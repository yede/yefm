#include <QDebug>

#include "yesidecmdmodel.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
#include "yestylesettings.h"
//==============================================================================================================================

SideCmdModel::SideCmdModel(YeApplication *app, QObject *parent)
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

