#include <QStackedLayout>
#include <QClipboard>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

#include "yefilepanedelegate.h"
#include "yefswidget.h"
#include "yefshandler.h"

#include "yefstreeview.h"
#include "yefslistview.h"
#include "yefsmodel.h"
#include "yefssortmodel.h"

#include "yeiconloader.h"
#include "yeappcfg.h"
#include "yeapp.h"

#include "yedefaultactions.h"
#include "yeactionmgr.h"

#include "yefileutils.h"
//==============================================================================================================================

FsWidget::FsWidget(TreeNode *tab, QWidget *parent)
	: QWidget(parent)
	, m_pane(NULL)
	, m_tab(tab)
	, m_hiddenVisible(false)
	, m_pathIsHidden(false)
	, m_editing(false)
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

	m_model = new FsModel(this);
	m_sortModel = new FsSortModel(m_model);

	m_fileList->setModel(m_sortModel);
	m_seleModel = m_fileList->selectionModel();
	m_idleModel = NULL;

	m_handler->lateStart();
	m_fileList->lateStart();

	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(updateIconTheme()));
	connect(App::app(), SIGNAL(settingsChanged()), this, SLOT(updateSettings()));

#if USE_node
	connect(m_model, SIGNAL(rootPathChanged(QModelIndex)), this, SLOT(rootPathChanged(QModelIndex)));
	connect(m_model, SIGNAL(goParent()), this, SLOT(handleGoParent()));
	connect(m_model, SIGNAL(invalidated()), this, SLOT(invalidated()));
	connect(m_model, SIGNAL(edit(QModelIndex)), this, SLOT(edit(QModelIndex)));
	connect(m_model, SIGNAL(select(QModelIndex,bool)), this, SLOT(select(QModelIndex,bool)));
#endif
}

FsWidget::~FsWidget()
{	
	clearHistory();

	delete m_sortModel;
	delete m_model;
	delete m_handler;
}
//==============================================================================================================================

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

void FsWidget::setPane(FilePaneDelegate *pane)
{
	m_pane = pane;
	setHiddenVisible(pane->isHiddenVisible());
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
//	qDebug() << "FsWidget::setWorkPath(): path=" << path;
	if (m_workPath == path) return;

	QFileInfo info(path);
	if (!info.isDir()) {
		QString text = QString("%1   \n\n%2   \n").arg(tr("Folder not exests:")).arg(path);
		App::app()->showPadMessage(text, "", 3000);
		return;
	}

	if (!info.isReadable() || !info.isExecutable()) {
		QString text = QString("%1   \n\n%2   \n").arg(tr("Can not enter folder:")).arg(path);
		App::app()->showPadMessage(text, "", 3000);
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

#if USE_qsys
	afterSetRootPath(path);
#endif
}

void FsWidget::rootPathChanged(const QModelIndex &index)
{
	QModelIndex dest = m_sortModel->mapFromSource(index);
	m_fileList->setRootIndex(dest);
	if (m_fileTree != NULL) m_fileTree->setRootIndex(dest);

#if USE_node
//	m_sortModel->sort(0);
#endif

	doSetWorkPath(m_model->rootPath());
	clearSelection();
	clearCurrentIndex();
	loadHistory();
}

void FsWidget::rootPathChanged(const QString &path)
{
//	qDebug() << "FsWidget::afterSetRootPath(): path=" << path;
	QModelIndex src = m_model->index(path);
	if (!src.isValid()) {
		qDebug("FsWidget::afterSetRootPath(): NOT valid, path=\"%s\"", qPrintable(path));
	//	return;
	}

	QModelIndex dest = m_sortModel->mapFromSource(src);
	if (!dest.isValid()) qDebug("FsWidget::afterSetRootPath(): mapFromSource(src) NOT valid");

	m_fileList->setRootIndex(dest);
	if (m_fileTree != NULL) m_fileTree->setRootIndex(dest);

	doSetWorkPath(path);
	clearSelection();
	clearCurrentIndex();
	loadHistory();
}

void FsWidget::setHiddenVisible(bool visible)
{
	bool flag = visible || m_pathIsHidden;
	if (m_hiddenVisible == flag) return;

	if (!flag && QFileInfo(m_workPath).isHidden()) m_seleModel->clear();

	m_model->setHiddenVisible(flag);
	m_hiddenVisible = flag;

#if USE_node
	m_sortModel->invalidate();
#endif
}
//==============================================================================================================================

void FsWidget::refresh()
{
	QApplication::clipboard()->clear();
//	m_seleModel->clear();
#if USE_qsys
	m_model->setRootPath("");	//changing rootPath forces reread, updates file sizes
	m_model->setRootPath(m_workPath);
#endif
#if USE_node
	m_sortModel->invalidate();
	m_sortModel->sort(0);
#endif
	m_model->clearCutItems();
}

void FsWidget::invalidated()
{
	qDebug() << "FsWidget::invalidated(): show invalidated status.";
	//	TODO: update tab-title, cut-list, path-button
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

void FsWidget::showTooltip(const QString &tips)
{
	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->setToolTip(tips); break;
		case FsWidgetMode::Tree: m_fileTree->setToolTip(tips); break;
	}
}

void FsWidget::validatePath()
{
#if USE_node
	if (m_workPath.isEmpty()) return;

	if (!QFileInfo(m_workPath).exists()) m_model->invalidate();
#endif
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

bool FsWidget::isHiddenVisible() const
{
	return m_pane->isHiddenVisible();
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

void FsWidget::edit(const QModelIndex &index)
{
	QModelIndex dest = m_sortModel->mapFromSource(index);
	m_seleModel->setCurrentIndex(dest, QItemSelectionModel::ClearAndSelect);

	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->edit(dest); break;
		case FsWidgetMode::Tree: m_fileTree->edit(dest); break;
	}
}

void FsWidget::selectSibling(int row, int col, const QModelIndex &sibling)
{
	QModelIndex index = m_sortModel->sibling(row, col, sibling);
	clearSelection();
	selectSorted(index, true);
	setCurrentIndex(index);
}

void FsWidget::selectSorted(const QModelIndex &index, bool ensureVisible)
{
	if (!index.isValid()) return;

	if (!m_seleModel->isSelected(index)) {
		m_seleModel->select(index, QItemSelectionModel::Select);
	}

	if (ensureVisible) {
		if (m_fileList != NULL) m_fileList->scrollTo(index, QAbstractItemView::EnsureVisible);
		if (m_fileTree != NULL) m_fileTree->scrollTo(index, QAbstractItemView::EnsureVisible);
	}
}

void FsWidget::select(const QModelIndex &index, bool ensureVisible)
{
//	qDebug() << "FsWidget::select()" << index.isValid();
	if (!index.isValid()) return;

	QModelIndex sorted = m_sortModel->mapFromSource(index);
//	qDebug() << "FsWidget::select()" << dest.isValid() << m_seleModel->isSelected(dest);
	selectSorted(sorted, ensureVisible);
}

void FsWidget::selectFile(const QString &path, bool ensureVisible)
{
	QModelIndex src = m_model->index(path);
	select(src, ensureVisible);
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

void FsWidget::dispatchDefaultAction(int actionId)
{
	switch (actionId) {
		case DefaultAction::Activate  : handleActivate();   break;
		case DefaultAction::GoParent  : handleGoParent();   break;
		case DefaultAction::NewFile   : handleNewFile();    break;
		case DefaultAction::NewFolder : handleNewFolder();  break;
		case DefaultAction::Cut       : handleCut();        break;
		case DefaultAction::Copy      : handleCopy();       break;
		case DefaultAction::Paste     : handlePaste();      break;
		case DefaultAction::Delete    : handleDelete();     break;
		case DefaultAction::Rename    : handleRename();     break;
		case DefaultAction::Properties: handleProperties(); break;
		case DefaultAction::Testing   : handleTest();       break;
	}
}

void FsWidget::handleActivate()
{
	if (m_editing) return;

	QFileInfo info;
	if (!getCurrentFileInfo(info)) return;

	if (info.isDir()) {
		setWorkPath(info.filePath());
		return;
	}

	if (info.isExecutable()) {
		QString cmd = info.filePath();
		QStringList args;
		QProcess::startDetached(cmd, args, m_workPath);
		return;
	}

	// TODO: open file
}

void FsWidget::handleGoParent()
{
	QChar sep = QChar('/');
	if (m_workPath.startsWith(sep) && m_workPath.size() > 1) {
		QString path = m_workPath.left(m_workPath.lastIndexOf(sep));
		setWorkPath(path.isEmpty() ? sep : path);
	}
}

void FsWidget::handleNewFile()
{
	if (!QFileInfo(m_workPath).isWritable()) {
		QString msg = QString("%1\n%2\n%3")
					  .arg(tr("Can not create new file."))
					  .arg(tr("Current Folder is not writable:"))
					  .arg(m_workPath);
		App::message(msg);
		return;
	}
#if USE_node
	m_model->createFile();
#else
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
#endif
}

void FsWidget::handleNewFolder()
{
	if (!QFileInfo(m_workPath).isWritable()) {
		QString msg = QString("%1\n%2\n%3")
					  .arg(tr("Can not create new folder."))
					  .arg(tr("Current Folder is not writable:"))
					  .arg(m_workPath);
		App::message(msg);
		return;
	}
#if USE_node
	m_model->createFolder();
#else
	QString name = tr("new_folder");
	QString path = FileUtils::buildRenamePath(m_workPath, name);
	QDir dir(m_workPath);

	if (!dir.mkpath(path)) {
		return;
	}

	QModelIndex src  = m_model->index(path);
	QModelIndex dest = m_sortModel->mapFromSource(src);
	m_seleModel->setCurrentIndex(dest, QItemSelectionModel::ClearAndSelect);

	switch (m_viewMode) {
		case FsWidgetMode::List: m_fileList->edit(dest); break;
		case FsWidgetMode::Tree: m_fileTree->edit(dest); break;
	}
#endif
}

void FsWidget::handleCut()      { m_model->handleCut(); }
void FsWidget::handleCopy()     { m_model->handleCopy(); }
void FsWidget::handlePaste()    { m_model->handlePaste(); }
void FsWidget::handleDelete()   { m_model->handleDelete(); }

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
	if (files.size() > 0) App::app()->showProperties(files);
}

void FsWidget::handleTest()
{
//	QStringList files;
//	files << "/home/nat/z.re" << "/home/nat/z.txt";
//	selectFiles(files);
}
//==============================================================================================================================

void FsWidget::selectLater(const QStringList &selFiles)
{
	if (selFiles.size() < 1) return;

	if (FsModel::isCutAction()) {
		FsModel::clearCutItems();
		QApplication::clipboard()->clear();
	}

	clearSelection();
	clearCurrentIndex();

	QModelIndex source = m_model->index(selFiles.at(0));
	if (source.isValid()) {
		QModelIndex sorted = m_sortModel->mapFromSource(source);
		setCurrentIndex(sorted);
	}
	selectFiles(selFiles);

#if USE_qsys
	QString root = m_model->rootPath();
	m_model->setRootPath("");					// changing rootPath forces reread, updates file sizes
	m_model->setRootPath(root);
#endif
}
//==============================================================================================================================

void FsWidget::updateColSizes() { if (m_fileTree != NULL) m_fileTree->updateColSizes(); }
void FsWidget::saveColSizes()   { if (m_fileTree != NULL) m_fileTree->saveColSizes(); }
