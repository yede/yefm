//#include <QStackedWidget>
#include <QStackedLayout>
#include <QClipboard>
#include <QFileInfo>
#include <QDebug>

#include "yefilepane.h"
#include "yefswidget.h"
#include "yefshandler.h"

#include "yefstreeview.h"
#include "yefslistview.h"
#include "yefsmodel.h"
#include "yefslistmodel.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"

#include "yefileutils.h"
//==============================================================================================================================

FsWidget::FsWidget(YeFilePane *pane, TreeNode *tab, QWidget *parent)
	: QWidget(parent)
	, m_app(pane->app())
	, m_pane(NULL)
	, m_tab(tab)
	, m_hiddenVisible(false)
	, m_pathIsHidden(false)
	, m_fileTree(NULL)
	, m_fileList(NULL)
	, m_viewMode(FsWidgetMode::List)
{
	m_handler = new FsHandler(this);

	m_fileStack = new QStackedLayout(this);
	m_fileStack->setContentsMargins(0, 0, 0, 0);
	m_fileStack->setSpacing(0);

	m_fileList = new FsListView(this);
	m_fileStack->addWidget(m_fileList);

	m_model = new FsModel(m_app);
	m_model->setFsWidget(this);
	m_sortModel = new FsListModel(m_model);

	m_fileList->setModel(m_sortModel);
	m_seleModel = m_fileList->selectionModel();
	m_idleModel = NULL;

	m_handler->lateStart();
	m_fileList->lateStart();

	setDockPane(pane);

	connect(m_app, SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
	connect(m_app, SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
}

FsWidget::~FsWidget()
{	
	clearHistory();

	delete m_sortModel;
	delete m_model;
	delete m_handler;
}
//==============================================================================================================================

void FsWidget::setDockPane(YeFilePane *newPane)
{
	if (m_pane != NULL) {
		disconnect(m_pane, SIGNAL(toggleHidden(bool)), this, SLOT(updateHiddenState(bool)));
	}
	m_pane = newPane;
	setHiddenVisible(newPane->isHiddenVisible());
	connect(newPane, SIGNAL(toggleHidden(bool)), this, SLOT(updateHiddenState(bool)));
}

void FsWidget::updateIconTheme()
{
	if (m_fileList != NULL) m_fileList->updateIconTheme();
	if (m_fileTree != NULL) m_fileTree->updateIconTheme();

	if (!m_workPath.isEmpty()) refresh();
}

void FsWidget::updateSettings()
{
	if (m_fileList != NULL) m_fileList->updateSettings();
	if (m_fileTree != NULL) m_fileTree->updateSettings();
}

void FsWidget::updateHiddenState(bool visible)
{
	setHiddenVisible(visible);
}
//==============================================================================================================================

void FsWidget::clearHistory()
{
	foreach (History *his, m_history) { delete his; }
	m_history.clear();
}

FsWidget::History *FsWidget::findHistory(const QString &path)
{
	int cnt = m_history.size();
	for (int i = 0; i < cnt; i++) {
		if (m_history.at(i)->workPath == path) { return m_history.at(i); }
	}
	return NULL;
}

FsWidget::History *FsWidget::takeHistory(const QString &path)
{
	int pos = -1;
	int cnt = m_history.size();
	for (int i = 0; i < cnt; i++) {
		if (m_history.at(i)->workPath == path) { pos = i; break; }
	}
	return pos < 0 ? NULL : m_history.takeAt(pos);
}

void FsWidget::saveHistory()
{
	if (!m_workPath.isEmpty()) {
		QString file;
		if (getCurrentFilePath(file)) {
			int pos = 0;//horizontalScrollBar()->value();
			History *his = takeHistory(m_workPath);
			if (his != NULL) {
				his->update(pos, file);
			} else {
				his = new History(m_workPath, pos, file);
			}
			m_history.insert(0, his);
		}
	}
}

void FsWidget::loadHistory()
{
	const History *his = findHistory(m_workPath);
	if (his != NULL) {
	//	horizontalScrollBar()->setValue(his->scrollPos);
		if (!his->selectedFile.isEmpty()) {
			QModelIndex src = m_model->index(his->selectedFile);
			if (src.isValid()) {
				QModelIndex dest = m_sortModel->mapFromSource(src);
				m_seleModel->select(dest, QItemSelectionModel::Select);
				if (m_fileList != NULL) m_fileList->scrollTo(dest, QAbstractItemView::EnsureVisible);
				if (m_fileTree != NULL) m_fileTree->scrollTo(dest, QAbstractItemView::EnsureVisible);
			}
		}
	}
}
//==============================================================================================================================

void FsWidget::createFileTreeView()
{
	if (m_fileTree != NULL) return;

	m_fileTree = new FsTreeView(this);
	m_fileStack->addWidget(m_fileTree);

	m_fileTree->setModel(m_sortModel);
	m_fileTree->lateStart();

	if (!m_workPath.isEmpty()) {
		QModelIndex src = m_model->index(m_workPath);
		QModelIndex dest = m_sortModel->mapFromSource(src);
		m_fileTree->setRootIndex(dest);
	}

	m_idleModel = m_fileTree->selectionModel();
	m_fileTree->setSelectionModel(m_seleModel);
}

void FsWidget::setViewMode(int mode)
{
	if (m_viewMode != mode) {
		if (m_viewMode == FsWidgetMode::Tree) saveColSizes();
		switch (mode) {
			case FsWidgetMode::List:
				m_fileStack->setCurrentWidget(m_fileList);
				break;

			case FsWidgetMode::Tree:
				if (m_fileTree == NULL) createFileTreeView();
				m_fileStack->setCurrentWidget(m_fileTree);
				break;
		}
		m_viewMode = mode;
		if (m_viewMode == FsWidgetMode::Tree) updateColSizes();
	}

	switch (mode) {
		case FsWidgetMode::List: m_fileList->setFocus(); break;
		case FsWidgetMode::Tree: m_fileTree->setFocus(); break;
	}
}

void FsWidget::doSetWorkPath(const QString &path)
{
	m_workPath = path;
	m_pane->updateTab(m_tab, FileUtils::getTitleFromPath(path), path);
	m_pane->showPath(this, path);
}

void FsWidget::setWorkPath(const QString &path)
{
	if (m_workPath == path) return;

	if (!FileUtils::isDir(path)) {
		QString text = QString("%1   \n\n%2   \n").arg(tr("Path not exests:")).arg(path);
		m_pane->win()->showPadMessage(text, "", 3000);
		return;
	}

	m_pathIsHidden = FileUtils::isHidden(path);
	if (m_pathIsHidden) {
		if (!m_hiddenVisible) setHiddenVisible(true);		// force hidden to be visible
	} else {
		if (!m_pane->isHiddenVisible() && m_hiddenVisible) setHiddenVisible(false);
	}

	saveHistory();
	m_model->setRootPath(path);

	QModelIndex src = m_model->index(path);
	if (!src.isValid()) {
		qDebug() << "FsWidget::setWorkPath(): !src.isValid()" << path;
		return;
	}

	QModelIndex dest = m_sortModel->mapFromSource(src);
	m_fileList->setRootIndex(dest);
	if (m_fileTree != NULL) m_fileTree->setRootIndex(dest);

	doSetWorkPath(path);
	loadHistory();
}

void FsWidget::setHiddenVisible(bool visible)
{
	bool flag = visible || m_pathIsHidden;
	if (m_hiddenVisible == flag) return;

	if (!flag && QFileInfo(m_workPath).isHidden()) m_seleModel->clear();

	m_model->setHiddenVisible(flag);
	m_hiddenVisible = flag;
}
//==============================================================================================================================

void FsWidget::goUp()
{
	QChar sep = QChar('/');
	if (m_workPath.startsWith(sep) && m_workPath.size() > 1) {
		QString path = m_workPath.left(m_workPath.lastIndexOf(sep));
		setWorkPath(path.isEmpty() ? sep : path);
	}
}

void FsWidget::refresh()
{
	QApplication::clipboard()->clear();
	m_seleModel->clear();

	m_model->setRootPath("");	//changing rootPath forces reread, updates file sizes
	m_model->setRootPath(m_workPath);
	m_model->clearCutItems();
}
//==============================================================================================================================

int FsWidget::getHoverArea() const
{
	switch (m_viewMode) {
		case FsWidgetMode::List: return m_fileList->getHoverArea();
		case FsWidgetMode::Tree: return m_fileTree->getHoverArea();
	}
	return FsHoverArea::None;
}

bool FsWidget::canHoverSelect(bool isDir)
{
	switch (m_viewMode) {
		case FsWidgetMode::List: return m_fileList->canHoverSelect(isDir);
		case FsWidgetMode::Tree: return m_fileTree->canHoverSelect(isDir);
	}
	return false;
}

void FsWidget::clearHoverState()
{
	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->clearHoverState(); break;
		case FsWidgetMode::Tree: m_fileTree->clearHoverState(); break;
	}
}
//==============================================================================================================================

void FsWidget::setCurrentIndex(const QModelIndex &index)
{
	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->setCurrentIndex(index); break;
		case FsWidgetMode::Tree: m_fileTree->setCurrentIndex(index); break;
	}
}

QModelIndex FsWidget::getCurrentIndex() const
{
	switch (m_viewMode) {
		case FsWidgetMode::List: return m_fileList->currentIndex();
		case FsWidgetMode::Tree: return m_fileTree->currentIndex();
	}
	return QModelIndex();
}

QModelIndex FsWidget::getIndexAt(const QPoint &p) const
{
	switch (m_viewMode) {
		case FsWidgetMode::List: return m_fileList->indexAt(p);
		case FsWidgetMode::Tree: return m_fileTree->indexAt(p);
	}
	return QModelIndex();
}

QFileInfo FsWidget::getFileInfo(const QModelIndex &index) const
{
	if (!index.isValid()) return QFileInfo(m_workPath);

	QModelIndex src = m_sortModel->mapToSource(index);
	return m_model->fileInfo(src);
}

int FsWidget::selCount() const
{
	return getSelection().count();
}

bool FsWidget::hasSelection() const
{
	return m_seleModel->hasSelection();
}

QModelIndexList FsWidget::getSelection() const
{
	return m_seleModel->selectedIndexes();
}

QModelIndexList FsWidget::getSourceSelection() const
{
	QModelIndexList sels = m_seleModel->selectedIndexes();
	QModelIndexList srcs;
	foreach (QModelIndex index, sels) {
		QModelIndex src = m_sortModel->mapToSource(index);
		srcs.append(src);
	}
	return srcs;
}

bool FsWidget::getCurrentFilePath(QString &result) const
{
	QModelIndex curr = getCurrentIndex();
	if (curr.isValid()) {
		QModelIndex dest = m_sortModel->mapToSource(curr);
		result = m_model->filePath(dest);
		return true;
	}
	return false;
}

bool FsWidget::getCurrentFileInfo(QFileInfo &result) const
{
	QModelIndex curr = getCurrentIndex();
	if (curr.isValid()) {
		QModelIndex dest = m_sortModel->mapToSource(curr);
		result = m_model->fileInfo(dest);
		return true;
	}
	return false;
}

int FsWidget::getSelectedFiles(QStringList &files, QString &workDir) const
{
	QModelIndexList sels = getSourceSelection();
	int cnt = sels.count();
	if (cnt > 0) {
		foreach (QModelIndex item, sels) {
			files.append(m_model->filePath(item));
		}
		if (cnt == 1) {
			QFileInfo info = m_model->fileInfo(sels.at(0));
			workDir = info.isDir() ? info.filePath() : m_workPath;
		} else {
			workDir = m_workPath;
		}
	}
	return cnt;
}

int FsWidget::getSelectedFiles(QStringList &result) const
{
	QModelIndexList sels = getSourceSelection();
	foreach (QModelIndex item, sels) {
		result.append(m_model->filePath(item));
	}
	return sels.count();
}

QString FsWidget::getPastePath() const
{
	QModelIndex curr = getCurrentIndex();
	if (curr.isValid()) {
		QModelIndex dest = m_sortModel->mapToSource(curr);
		QFileInfo info = m_model->fileInfo(dest);
		if (info.isDir()) {
			return info.filePath();
		}
	}
	return m_workPath;
}
//==============================================================================================================================

void FsWidget::selectFile(const QString &path, bool ensureVisible)
{
	QModelIndex src = m_model->index(path);
	if (!src.isValid()) return;

	QModelIndex dest = m_sortModel->mapFromSource(src);
	if (!m_seleModel->isSelected(dest)) {
		m_seleModel->select(dest, QItemSelectionModel::Select);
	}

	if (ensureVisible) {
		if (m_fileList != NULL) m_fileList->scrollTo(dest, QAbstractItemView::EnsureVisible);
		if (m_fileTree != NULL) m_fileTree->scrollTo(dest, QAbstractItemView::EnsureVisible);
	}
}

void FsWidget::selectFiles(const QStringList &paths)
{
	foreach (const QString &path, paths) {
		selectFile(path, false);
	}
}

void FsWidget::clearSelection()
{
	m_seleModel->clear();
}

void FsWidget::clearCurrentIndex()
{
	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->setCurrentIndex(QModelIndex()); break;
		case FsWidgetMode::Tree: m_fileTree->setCurrentIndex(QModelIndex()); break;
	}
}
//==============================================================================================================================

void FsWidget::handleNewFile()
{
	if (!QFileInfo(m_workPath).isWritable()) {
		App::message(tr("Read only...can not create file"));
		return;
	}

	QString name = tr("new_file");
	QString path;
	QModelIndex fileIndex;
	int i = 0;

	do {
		path = i < 1 ? QString("%1/%2").arg(m_workPath).arg(name)
					 : QString("%1/%2_%3").arg(m_workPath).arg(name).arg(i);
		i++;
		fileIndex = m_model->index(path);
	}
	while (fileIndex.isValid());

	QFile newFile(path);
	newFile.open(QIODevice::WriteOnly);
	newFile.close();

	fileIndex = m_model->index(QFileInfo(newFile).filePath());
	QModelIndex dest = m_sortModel->mapFromSource(fileIndex);
	m_seleModel->setCurrentIndex(dest, QItemSelectionModel::ClearAndSelect);

	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->edit(dest); break;
		case FsWidgetMode::Tree: m_fileTree->edit(dest); break;
	}
}

void FsWidget::handleNewFolder()
{
	if (!QFileInfo(m_workPath).isWritable()) {
		App::message(tr("Read only...can not create folder"));
		return;
	}

	QString name = tr("new_folder");
	QString folder;
	QModelIndex newDir;
	QModelIndex parentDir = m_model->index(m_workPath);
	int i = 0;

	do {
		folder = i < 1 ? name : QString("%1_%2").arg(name).arg(i);
		i++;
		newDir = m_model->mkdir(parentDir, folder);
	}
	while (!newDir.isValid());

	QModelIndex dest = m_sortModel->mapFromSource(newDir);
	m_seleModel->setCurrentIndex(dest, QItemSelectionModel::ClearAndSelect);

	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->edit(dest); break;
		case FsWidgetMode::Tree: m_fileTree->edit(dest); break;
	}
}

void FsWidget::handleRun()
{
}

void FsWidget::handleCut()        { m_model->handleCut(); }
void FsWidget::handleCopy()       { m_model->handleCopy(); }
void FsWidget::handlePaste()      { m_model->handlePaste(); }
void FsWidget::handleDelete()     { m_model->handleDelete(); }

void FsWidget::handleRename()
{
	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->execRename(); break;
		case FsWidgetMode::Tree: m_fileTree->execRename(); break;
	}
}

void FsWidget::handleProperties()
{
	QStringList files;
	if (getSelectedFiles(files) < 1) files.append(m_workPath);
	m_app->showPropertiesDlg(files);
}

void FsWidget::handleTest()
{
//	QStringList files;
//	files << "/home/nat/z.re" << "/home/nat/z.txt";
//	selectFiles(files);
}
//==============================================================================================================================

void FsWidget::updateColSizes() { if (m_fileTree != NULL) m_fileTree->updateColSizes(); }
void FsWidget::saveColSizes()   { if (m_fileTree != NULL) m_fileTree->saveColSizes(); }
