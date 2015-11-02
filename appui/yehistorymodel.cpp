#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QSettings>
#include <QMimeData>
#include <QAction>
#include <QTimer>
#include <QMenu>
#include <QDebug>

#include "yehistorymodel.h"
//#include "yehistorydlg.h"
#include "yeuserview.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
#include "yestyle.h"
#include "yeshortcuts.h"

#include "yefileutils.h"
//==============================================================================================================================

HistoryModel::HistoryModel(QObject *parent)
	: TreeModel(parent)
	, m_style(Style::instance())
	, m_contextNode(NULL)
	, m_actRemove(NULL)
	, m_skip(false)
{
	m_rootNode = new TreeNode(NodeType::Bookmark);

	m_timer = new QTimer;
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

HistoryModel::~HistoryModel()
{
	delete m_rootNode;
}

void HistoryModel::lateStart()
{
	updateIconTheme();
	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
}

void HistoryModel::updateIconTheme()
{
	if (m_actRemove == NULL) return;

	m_actRemove->setIcon(IconLoader::menuIcon("list-remove"));
}
//==============================================================================================================================

void HistoryModel::save()
{
	QStringList paths;
	QList<TreeNode *> &list = m_rootNode->children();

	foreach (TreeNode *node, list) {
		paths.append(node->path());
	}

	QSettings s(App::getHistoryFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	s.setValue("paths", paths);
	s.endGroup();
}

void HistoryModel::load()
{
	QSettings s(App::getHistoryFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	QStringList paths = s.value("paths").toStringList();
	s.endGroup();

	clear();
	int count = paths.size();

	beginInsertRows(QModelIndex(), 0, count - 1);
	for (int i = 0; i < count; i++) {
		QString path = paths.at(i);
		QString name = FileUtils::getFileName(path);
		insertNode(name, path);
	}
	endInsertRows();
}

void HistoryModel::addPath(const QString &path)
{
	if (path.isEmpty() || m_skip) return;

	QList<TreeNode *> &list = m_rootNode->children();
	int max, i = list.size();

	if (i > 0) {
		TreeNode *node = list.at(0);
		if (node->path() == path) return;		// no waste of time if present at 0
	}

	while (i > 1) {
		i--;
		TreeNode *node = list.at(i);
		if (node->path() == path) {
			beginRemoveRows(QModelIndex(), i, i);
			m_rootNode->removeChild(node);
			endRemoveRows();
		}
	}

	i = list.size();
	max = AppCfg::instance()->maxHistory;
//	if (max < 20) max = 20; else if (max > 2000) max = 2000;

	while (i > 0 && i >= max) {
		i--;
		TreeNode *node = list.at(i);
		beginRemoveRows(QModelIndex(), i, i);
		m_rootNode->removeChild(node);
		endRemoveRows();
	}

	QString name = FileUtils::getFileName(path);

	beginInsertRows(QModelIndex(), 0, 0);
	insertNode(name, path, 0);
	endInsertRows();

	save();
}
//==============================================================================================================================

void HistoryModel::onTimeout()
{
	m_skip = false;
}

void HistoryModel::startTemporarilySkip()
{
	m_skip = true;
	m_timer->start(2000);
}
//==============================================================================================================================

TreeNode *HistoryModel::insertNode(const QString &title, const QString &path, int pos)
{
	TreeNode *node = new TreeNode(NodeType::Bookmark, m_rootNode);
	node->setTitle(title);
	node->setPath(path);
//	if (path == "::") node->setSeparator(true);
	m_rootNode->insertChild(node, pos);

	return node;
}

void HistoryModel::removeNode(TreeNode *node)
{
	if (node == NULL) return;

	int pos = node->row();
	beginRemoveRows(QModelIndex(), pos, pos);
	m_rootNode->removeChild(node);
	endRemoveRows();

	save();
	delete node;
}

void HistoryModel::removeSelection(UserView *view)
{
	TreeNode *node = view->getSelectedNode();
	removeNode(node);
}

void HistoryModel::onRemove()
{
	removeNode(m_contextNode);
}

void HistoryModel::onClear()
{
	if (!App::confirm(tr("Be sure to clear all items?"))) return;

	beginResetModel();
	m_rootNode->clear();
	endResetModel();
}
//==============================================================================================================================

QString HistoryModel::getStatusMessage(const QModelIndex &index) const
{
	return index.isValid() ? getNode(index)->path() : QString();
}

void HistoryModel::showContextMenu(UserView *view, QContextMenuEvent *event)
{
	if (m_actRemove == NULL) {
		m_actRemove = new QAction("", this);
		m_actClear = new QAction(tr("Clear all items"), this);
		connect(m_actRemove, SIGNAL(triggered()), this, SLOT(onRemove()));
		connect(m_actClear, SIGNAL(triggered()), this, SLOT(onClear()));
		updateIconTheme();
	}

//	QModelIndex curr = view->currentIndex();
//	m_contextNode = curr.isValid() ? getNode(curr) : NULL;
	m_contextNode = view->getSelectedNode();
	bool flag = m_contextNode != NULL;

	QString txt2 = flag ? QString("%1 \"%2\"").arg(tr("Remove")).arg(m_contextNode->title())
						: tr("Remove item");

	m_actRemove->setText(txt2);
	m_actRemove->setEnabled(flag);
	m_actClear->setEnabled(m_rootNode->ownCount() > 0);

	QMenu menu;
	menu.addAction(m_actRemove);
	menu.addSeparator();
	menu.addAction(m_actClear);
	menu.exec(event->globalPos());

	m_contextNode = NULL;
}
//==============================================================================================================================

bool HistoryModel::handleKeyPress(UserView *view, QKeyEvent *event)
{
	int actionId = ShortcutMgr::instance()->findActionId("HistoryModel", event);
	if (actionId < 0) return false;

	switch (actionId) {
		case HisModelAction::Activate: view->activate();      break;
		case HisModelAction::Remove  : removeSelection(view); break;
	}

	return true;
}

void HistoryModel::setupShortcuts()
{
	ShortcutMgr *m = ShortcutMgr::instance();
	ShortcutGroup *grp = m->addGroup("HistoryModel");

	grp->add("Return",   "Activate", HisModelAction::Activate);
	grp->add("Ctrl+Del", "Remove",   HisModelAction::Remove);
}
//==============================================================================================================================

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeNode *node = getNode(index);
	bool sep = node->isSeparator();
	Style::BmkViewStyle &m = m_style->bmStyle;

	switch (role) {
		case Qt::DisplayRole   : return node->title();
		case Qt::DecorationRole: return sep ? QVariant() : IconLoader::icon("history-item");
	//	case Qt::ToolTipRole:    return node->path();
		case Qt::ForegroundRole: return sep ? QBrush(m.sepFgColor) : QBrush(m.itemColor);
		case Qt::BackgroundRole: return sep ? QBrush(m.sepBgColor) : QVariant();
	//	case Qt::TextAlignmentRole: return sep ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
	//										   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}

	return QVariant();
}
//==============================================================================================================================
