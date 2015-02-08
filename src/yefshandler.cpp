#include <QFile>
#include <QFileInfo>
#include <QModelIndex>
#include <QKeyEvent>
#include <QDebug>

#include "yefshandler.h"
#include "yefswidget.h"
#include "yefilepane.h"

#include "yefstreeview.h"
#include "yefslistview.h"
#include "yefsmodel.h"
#include "yefslistmodel.h"
#include "yefsactions.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"

#include "yefileutils.h"
//==============================================================================================================================

FsHandler::FsHandler(FsWidget *widget, QObject *parent)
	: QObject(parent)
	, m_app(widget->app())
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
	m_timer.start(R::data().hoverTime);
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
	AppData &d = R::data();

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

	AppData &d = R::data();

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

void dumpKey(int key, Qt::KeyboardModifiers mod)
{
	int shift = (mod & Qt::ShiftModifier)   ? 1 : 0;
	int ctrl  = (mod & Qt::ControlModifier) ? 1 : 0;
	int alt   = (mod & Qt::AltModifier)     ? 1 : 0;
	qDebug("key: 0x%x, Shift: %d, Ctrl: %d, Alt: %d", key, shift, ctrl, alt);
}

void FsHandler::handleKeyPress(QKeyEvent *event)
{
	int key = event->key();
	Qt::KeyboardModifiers mod = event->modifiers();
//	dumpKey(key, mod);

	if ((key == Qt::Key_Enter || key == Qt::Key_Return) && mod == Qt::NoModifier) {
		onItemActivated(m_widget->getCurrentIndex());
		event->accept();
		return;
	}

	if (key == Qt::Key_Control || key == Qt::Key_Shift ||
		key == Qt::Key_Alt || key == Qt::Key_Meta || key == Qt::Key_AltGr ||
		key == Qt::Key_unknown)
		return;

	if (mod & Qt::ShiftModifier)   key += Qt::SHIFT;
	if (mod & Qt::ControlModifier) key += Qt::CTRL;
	if (mod & Qt::AltModifier)     key += Qt::ALT;

	int action = m_app->getFileViewKeyAction(key);
	dispatchKeyAction(action);
}

void FsHandler::dispatchKeyAction(int action)
{
	switch (action) {
		case KeyAction::NewFile   : m_widget->handleNewFile();    break;
		case KeyAction::NewFolder : m_widget->handleNewFolder();  break;
		case KeyAction::Run       : m_widget->handleRun();        break;
		case KeyAction::Cut       : m_widget->handleCut();        break;
		case KeyAction::Copy      : m_widget->handleCopy();       break;
		case KeyAction::Paste     : m_widget->handlePaste();      break;
		case KeyAction::Delete    : m_widget->handleDelete();     break;
		case KeyAction::Rename    : m_widget->handleRename();     break;
		case KeyAction::Properties: m_widget->handleProperties(); break;
	}
}
//==============================================================================================================================

void FsHandler::showContextMenu(QContextMenuEvent *event)
{
	m_app->fsActions()->showContextMenu(m_widget, event);
}

void FsHandler::showStatusMessage(const QModelIndex &index)
{
	QFileInfo info = m_widget->getFileInfo(index);
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
		 << date.toString(R::data().statDateFormat)
		 << date.toString(R::data().statTimeFormat)
		 << name;

	m_app->win()->showStatusMessage(msgs, m_widget->pane()->index(), 12000);
}
