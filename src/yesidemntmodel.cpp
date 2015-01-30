#include <QDebug>

#include "yesidemntmodel.h"
#include "yemount.h"
#include "yefileutils.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
#include "yestylesettings.h"
//==============================================================================================================================

SideMntModel::SideMntModel(YeApplication *app, QObject *parent)
	: TreeModel(parent)
	, m_app(app)
{
	m_rootNode = new TreeNode(NodeType::Device);
	m_mnt = new Mount(this);

	connect(m_mnt, SIGNAL(afterMounts(const MountItems &)), this, SLOT(showMounts(const MountItems &)));
	connect(app, SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));

	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

SideMntModel::~SideMntModel()
{
}

void SideMntModel::startReadMounts()
{
	m_timer.start(200);
}

void SideMntModel::onIconThemeChanged()
{
}

void SideMntModel::onTimeout()
{
	m_mnt->readMounts();
	m_timer.start(3000);
}
//==============================================================================================================================

TreeNode *SideMntModel::insertNode(const QString &title, const QString &path, int pos)
{
	TreeNode *node = new TreeNode(NodeType::Device, m_rootNode);
	node->setTitle(title);
	node->setPath(path);
	if (path == "::") node->setSeparator(true);
	m_rootNode->insertChild(node, pos);

	return node;
}

void SideMntModel::showMounts(const MountItems &mounts)
{
	clear();
	int cnt = mounts.count();
	beginInsertRows(QModelIndex(), 0, cnt - 1);

	for (int i = 0; i < cnt; ++i) {
		if (!mounts.isValid()) break;
		const MountItem &m = mounts.at(i);
		if (m.path[0] == '/' && m.path[1] == '\0') continue;
		insertNode(FileUtils::getTitleFromPath(m.path), m.path, i);
	}

	endInsertRows();
}

QString SideMntModel::getStatusMessage(const QModelIndex &index) const
{
	return index.isValid() ? getNode(index)->path() : QString();
}
//==============================================================================================================================

QVariant SideMntModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode *node = getNode(index);
	bool sep = node->isSeparator();
	FmStyle::BmkViewStyle &m = R::app()->fmStyle()->bmStyle;

	switch (role) {
		case Qt::DisplayRole   : return node->title();
		case Qt::DecorationRole: return sep ? QVariant() : R::icon("device");
	//	case Qt::ToolTipRole:    return node->path();
		case Qt::ForegroundRole: return sep ? QBrush(m.sepFgColor) : QBrush(m.itemColor);
		case Qt::BackgroundRole: return sep ? QBrush(m.sepBgColor) : QVariant();
	//	case Qt::TextAlignmentRole: return sep ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
	//										   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}

	return QVariant();
}
