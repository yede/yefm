#include <QMetaObject>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QModelIndex>
#include <QKeyEvent>
#include <QDebug>

#include "yefshandler.h"
#include "yefswidget.h"
#include "yefilepanedelegate.h"

#include "yefstreeview.h"
#include "yefslistview.h"
#include "yefsmodel.h"
#include "yefssortmodel.h"

#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"

#include "yefswidgetdelegate.h"
#include "yedefaultactions.h"
#include "yeactionmgr.h"
#include "yeshortcuts.h"

#include "yefileutils.h"
//==============================================================================================================================

FsHandler::FsHandler(FsWidget *widget, QObject *parent)
	: QObject(parent)
	, m_widget(widget)
	, m_pressed(false)
{
	m_timer.setSingleShot(true);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

FsHandler::~FsHandler()
{
}

void FsHandler::lateStart()
{
	m_model     = m_widget->model();
	m_sortModel = m_widget->sortModel();
	m_seleModel = m_widget->seleModel();
}
//==============================================================================================================================

void FsHandler::execHoverSelect(const QString &path)
{
	QModelIndex source = m_model->index(path);
	if (!source.isValid()) return;

	QModelIndex sorted = m_sortModel->mapFromSource(source);
	QModelIndex curr = m_widget->getCurrentIndex();

	if (!curr.isValid()) {
		m_widget->setCurrentIndex(sorted);
		m_seleModel->select(sorted, QItemSelectionModel::Select);
		return;
	}

	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	bool isSelectingCurrent = (sorted == curr);

	if (mods == Qt::ControlModifier) {
		if (isSelectingCurrent && m_seleModel->selectedIndexes().count() == 1) return;
		m_seleModel->select(sorted, QItemSelectionModel::Toggle);
		if (isSelectingCurrent && !m_seleModel->isSelected(curr)) {
			QModelIndexList idxs = m_seleModel->selectedIndexes();
			if (idxs.count() > 0) {
				m_widget->setCurrentIndex(idxs.at(0));
				m_seleModel->select(idxs.at(0), QItemSelectionModel::Select);
			} else {
				m_widget->setCurrentIndex(QModelIndex());
			}
		}
		return;
	}

	if (isSelectingCurrent) {
		if (!m_seleModel->isSelected(curr)) m_seleModel->select(curr, QItemSelectionModel::Select);
		return;
	}

	if (mods == Qt::ShiftModifier) {
		int r0 = curr.row();
		int r1 = sorted.row();
		QItemSelection sel = (r1 > r0) ? QItemSelection(curr, sorted) : QItemSelection(sorted, curr);
		m_seleModel->clear();
		m_widget->setCurrentIndex(curr);
		m_seleModel->select(sel, QItemSelectionModel::Select);
		return;
	}

	if (mods == Qt::NoModifier && !m_seleModel->isSelected(sorted)) {
		m_seleModel->clear();
		m_widget->setCurrentIndex(sorted);
		m_seleModel->select(sorted, QItemSelectionModel::Select);
		return;
	}
}

void FsHandler::startHoverSelect(const QModelIndex &sorted)
{
	QModelIndex source = m_sortModel->mapToSource(sorted);
	QFileInfo info = m_model->fileInfo(source);
	m_timerFile = info.filePath();
	m_timer.start(AppCfg::instance()->hoverTime);
}

void FsHandler::stopHoverSelect()
{
	if (m_timer.isActive()) m_timer.stop();
}

void FsHandler::onTimeout()
{
	execHoverSelect(m_timerFile);
}

void FsHandler::onItemHovered(const QModelIndex &sorted)
{
	bool isValid = sorted.isValid();
	AppCfg &d = *AppCfg::instance();

	switch (d.clickEnter) {
		case ClickEnter::DoubleClick: break;
		case ClickEnter::ClickIcon  : break;
		case ClickEnter::SingleClick:
			if (d.keyStopHover) {
				Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
				bool hasMods = (mods != Qt::NoModifier);
				if (isValid && !hasMods) startHoverSelect(sorted); else m_widget->clearHoverState();
			} else {
				if (isValid) startHoverSelect(sorted); else m_widget->clearHoverState();
			}
			break;
	}

	if (isValid) showStatusMessage(sorted);
}

void FsHandler::onItemActivated(const QModelIndex &sorted)
{
//	static int i = 0;
//	qDebug() << "onItemActivated" << i++;
	if (!sorted.isValid()) return;

	AppCfg &d = *AppCfg::instance();

	switch (d.clickEnter) {
		case ClickEnter::DoubleClick: break;
		case ClickEnter::ClickIcon  :
			if (m_widget->getHoverArea() != FsHoverArea::Icon) return;
			break;
		case ClickEnter::SingleClick:
			if (d.keyStopHover) {
				Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
				if (mods != Qt::NoModifier) return;
			}
			break;
	}

	QModelIndex source = m_sortModel->mapToSource(sorted);
	QFileInfo info = m_model->fileInfo(source);
	if (info.isDir()) {
		m_widget->setCurrentIndex(sorted);		// as history item
		m_widget->setWorkPath(info.filePath());
	}
}
//==============================================================================================================================

bool FsHandler::handleKeyPress(QKeyEvent *event)
{
	int actionId = ShortcutMgr::instance()->findActionId(DefaultActions::groupName(), event);
	if (actionId < 0) return false;

	m_widget->dispatchDefaultAction(actionId);

	return true;
}
//==============================================================================================================================

void FsHandler::showContextMenu(QContextMenuEvent *event)
{
	FsWidgetDelegate_ctx d;
	d.m_view                  = m_widget;
	d.m_event                 = event;
	d.m_workPath              = &FsWidget::workPath;
	d.m_getIndexAt            = &FsWidget::getIndexAt;
	d.m_getFileInfo           = &FsWidget::getFileInfo;
	d.m_getSelectedFiles      = &FsWidget::getSelectedFiles;
	d.m_hasSelection          = &FsWidget::hasSelection;
	d.m_clearSelection        = &FsWidget::clearSelection;
	d.m_clearCurrentIndex     = &FsWidget::clearCurrentIndex;
	d.m_dispatchDefaultAction = &FsWidget::dispatchDefaultAction;

	ActionMgr *m = ActionMgr::instance();
	m->showContextMenu(d);
}

void FsHandler::showStatusMessage(const QModelIndex &index)
{
	QFileInfo info = m_widget->getFileInfo(index);
	showStatusMessage(info);
}

void FsHandler::showStatusMessage(const QFileInfo &info)
{
	qint64 bytes = info.size();
	QString total = FileUtils::formatSize(bytes);
	QFile::Permissions pm = info.permissions();
	QDateTime date = info.lastModified();
	QString pms = QString("%1%2%3%4%5%6%7%8%9")
				  .arg((pm & QFile::ReadOwner)  ? 'r' : '-')
				  .arg((pm & QFile::WriteOwner) ? 'w' : '-')
				  .arg((pm & QFile::ExeOwner)   ? 'x' : '-')
				  .arg((pm & QFile::ReadGroup)  ? 'r' : '-')
				  .arg((pm & QFile::WriteGroup) ? 'w' : '-')
				  .arg((pm & QFile::ExeGroup)   ? 'x' : '-')
				  .arg((pm & QFile::ReadOther)  ? 'r' : '-')
				  .arg((pm & QFile::WriteOther) ? 'w' : '-')
				  .arg((pm & QFile::ExeOther)   ? 'x' : '-');
	QString name, link = tr("Link");
	if (info.isDir()) {
		name = info.isSymLink() ? QString("%1 --> %2").arg(link).arg(info.symLinkTarget()) : info.fileName();
	} else {
		name = info.isSymLink() ? QString("%1 --> %2").arg(link).arg(info.symLinkTarget()) : info.fileName();
	}

	QStringList msgs;
	msgs << pms << info.owner() << info.group() << total
		 << date.toString(AppCfg::instance()->statDateFormat)
		 << date.toString(AppCfg::instance()->statTimeFormat)
		 << name;

	App::app()->showStatusMessage(msgs, m_widget->pane()->index(), 12000);
//	m_widget->showTooltip(info.fileName());
}
