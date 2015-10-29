#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QTimer>
#include <QCloseEvent>
#include <QDebug>

#include "yemimedlg.h"
#include "yemimesettings.h"
#include "yedesktopfile.h"
#include "yefileutils.h"
#include "yesplitter.h"

#include "yeiconloader.h"
#include "yeappcfg.h"
//==============================================================================================================================

#define DEF_W 800
#define MIN_W 320
#define MIN_H 480

namespace MimeDlgMode {
	enum { None, SeleApp, EditMimes };
}

MimeDlg::MimeDlg(QWidget *parent)
	: QDialog(parent)
	, m_mode(MimeDlgMode::None)
	, m_sideWidth(MIN_W)
	, m_width(DEF_W)
	, m_height(MIN_H)
	, m_loaded(false)
	, m_busy(false)
	, m_mimeSettings(NULL)
{
	m_sideWidget = createAppTree();
	m_mimeSettings = new MimeSettings();

	m_splitter = new Splitter;
	m_splitter->setDirection(SplitterDirection::Left, m_sideWidth);
	m_splitter->setClient(m_sideWidget, m_mimeSettings);

	m_guideLabel = new QLabel(tr("Double click to add into associated applications."));
	m_buttons = new QDialogButtonBox;
	m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->setSpacing(4);
	hbox->addWidget(m_guideLabel);
	hbox->addWidget(m_buttons);

	QVBoxLayout *box = new QVBoxLayout(this);
	box->setContentsMargins(6, 6, 6, 6);
	box->setSpacing(6);
	box->addWidget(m_splitter);
	box->addLayout(hbox);

	setMinimumSize(MIN_W, MIN_H);

	connect(m_sideTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
			this, SLOT(onAppItemDoubleClicked(QTreeWidgetItem*,int)));
	connect(&m_thread, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()));

	updateIconTheme();
}

void MimeDlg::closeEvent(QCloseEvent *event)
{
	if (m_busy) {
		event->setAccepted(false);
		return;
	}

	m_sideWidth = m_sideWidget->width();
	if (!isMaximized()) {
		m_height = height();
		if (m_mode == MimeDlgMode::EditMimes) m_width = width();
		if (m_width <= m_sideWidth) m_width = m_sideWidth + 160;
	}

	event->accept();
}

void MimeDlg::accept()
{
	if (m_mode == MimeDlgMode::EditMimes) {
		if (m_mimeSettings->saveMimes()) this->done(1);
	} else {
		QDialog::accept();
	}
}
//==============================================================================================================================

bool MimeDlg::showDialog(QString &resultApp, const QString &mimeType)
{
	setWindowTitle(tr("Select an application"));
	m_mode = MimeDlgMode::SeleApp;

	m_titleLabel->setText(tr("Launcher for mime-type: %1").arg(mimeType));
	m_edSelApp->setVisible(true);
	m_guideLabel->setVisible(false);
	m_mimeSettings->setVisible(false);
	if (!isMaximized()) resize(m_sideWidth, m_height);

	bool ok = exec() == QDialog::Accepted;

	if (ok) {
		QString app = m_edSelApp->text();
		ok = !app.isEmpty();
		if (ok) resultApp = app + ".desktop";
	}

	return ok;
}

void MimeDlg::showDialog()
{
	setWindowTitle(tr("Edit mime-open items"));
	m_mode = MimeDlgMode::EditMimes;

	m_titleLabel->setText(tr("Available applications"));
	m_edSelApp->setVisible(false);
	m_guideLabel->setVisible(true);
	m_mimeSettings->setVisible(true);
	m_splitter->setSideSize(m_sideWidth);
	if (!isMaximized()) resize(m_width, m_height);

	bool ok = exec() == QDialog::Accepted;

	if (ok) {
		//
	}
}

void MimeDlg::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);

	QTimer::singleShot(60, this, SLOT(loadItems()));
}
//==============================================================================================================================

void MimeDlg::updateItemSizeHint(QTreeWidgetItem *item)
{
	int sz = AppCfg::instance()->iconSize + 2;
	item->setSizeHint(0, QSize(sz, sz));
}

void MimeDlg::updateIconTheme()
{
//	m_delegate->updateRowHeight();
	int sz = AppCfg::instance()->iconSize;
	m_sideTree->setIconSize(QSize(sz, sz));
//	m_model->updateIconTheme();

	if (m_sideTree->topLevelItemCount() > 0) {
		//
	}

	m_mimeSettings->updateIconTheme();
}

QWidget *MimeDlg::createAppTree()
{

	// Creates app list view
	m_sideTree = new QTreeWidget;
	m_sideTree->setAlternatingRowColors(true);
	m_sideTree->headerItem()->setText(0, tr("Application"));

	// Command bar
	m_titleLabel = new QLabel;
	m_edSelApp = new QLineEdit;
	m_edSelApp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QWidget *widget = new QWidget;
	QVBoxLayout *box = new QVBoxLayout(widget);
	box->setContentsMargins(0, 0, 0, 0);
	box->setSpacing(4);
	box->addWidget(m_titleLabel);
	box->addWidget(m_edSelApp);
	box->addWidget(m_sideTree);

	// Synonyms for cathegory names
	m_catNames.clear();
	m_catNames.insert("Development", QStringList() << "Programming");
	m_catNames.insert("Games", QStringList() << "Game");
	m_catNames.insert("Graphics", QStringList());
	m_catNames.insert("Internet", QStringList() << "Network" << "WebBrowser");
	m_catNames.insert("Multimedia", QStringList() << "AudioVideo" << "Video");
	m_catNames.insert("Office", QStringList());
	m_catNames.insert("System", QStringList());
	m_catNames.insert("Settings", QStringList() << "preferences-desktop");
	m_catNames.insert("Utilities", QStringList() << "Utility");
	m_catNames.insert("Other", QStringList());

	// Create default application cathegories
	m_categories.clear();
	createCategories();

	// Signals
	connect(m_sideTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
			SLOT(updateCommand(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(m_sideTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(initCategory(QTreeWidgetItem*)));

	return widget;
}

void MimeDlg::createCategories()
{
	foreach (QString name, m_catNames.keys()) {

		QIcon icon;
		bool ok = IconLoader::findIcon(icon, "applications-" + name.toLower());

		if (!ok) {
			foreach (QString synonym, m_catNames.value(name)) {
				ok = IconLoader::findIcon(icon, "applications-" + synonym.toLower());
				if (ok) break;
			}

			if (!ok) {
				foreach (QString synonym, m_catNames.value(name)) {
					ok = IconLoader::findIcon(icon, synonym.toLower());
					if (ok) break;
				}
			}

			if (!ok) icon = IconLoader::defaultIcon("application-x-executable");
		}

		QTreeWidgetItem *category = new QTreeWidgetItem(m_sideTree);		// Create category
		category->setText(0, name);
		category->setIcon(0, icon);
		category->setFlags(Qt::ItemIsEnabled);
		updateItemSizeHint(category);

		m_categories.insert(name, category);
	}
}
//==============================================================================================================================

QTreeWidgetItem *MimeDlg::findCategory(const DesktopFile &app)
{
	QTreeWidgetItem *category = m_categories.value("Other");	// Default categoty is 'Other'

	foreach (QString name, m_catNames.keys()) {					// Try to find more suitable category

		if (app.getCategories().contains(name)) {				// Try cathegory name
			category = m_categories.value(name);
			break;
		}

		bool found = false;
		foreach (QString synonym, m_catNames.value(name)) {		// Try synonyms
			if (app.getCategories().contains(synonym)) {
				found = true;
				break;
			}
		}
		if (found) {
			category = m_categories.value(name);
			break;
		}
	}

	return category;
}
//==============================================================================================================================

struct CatApps {
	QList<DesktopFile *> items;
};

static CatApps *getCatInfo(QTreeWidgetItem *category)
{
	QVariant data = category->data(0, Qt::UserRole);
	if (data.isNull()) return NULL;

	void *p = data.value<void *>();
	return reinterpret_cast<CatApps *>(p);
}

static void setCatInfo(QTreeWidgetItem *category, DesktopFile *app)
{
	CatApps *apps;
	if (category->childCount() < 1) {
		apps = new CatApps;
		QVariant data = QVariant::fromValue<void *>(apps);
		category->setData(0, Qt::UserRole, data);
		new QTreeWidgetItem(category);				// create first child for showing "+"
	} else {
		apps = getCatInfo(category);
	}
	apps->items.append(app);
}
//==============================================================================================================================

void MimeDlg::initCategory(QTreeWidgetItem *category)
{
	CatApps *apps = getCatInfo(category);
	if (apps == NULL) return;

	bool isNew = false;
	foreach (DesktopFile *app, apps->items) {
		QTreeWidgetItem *item;
		if (isNew) {
			item = new QTreeWidgetItem(category);	// Create item from current mime
		} else {
			isNew = true;
			item = category->child(0);				// first child
		}
		updateItemSizeHint(item);
		item->setIcon(0, IconLoader::appIcon(*app));
		item->setText(0, app->getName());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		m_applications.insert(app->getPureFileName(), item);			// Register application
		delete app;
	}

	delete apps;
	category->setData(0, Qt::UserRole, QVariant());						// clear data
}

/*
 * This function run inside thread.
 */
void MimeDlg::loadCategories()
{
	QList<DesktopFile *> apps;
	FileUtils::getApplications(apps);					// Load applications and create category tree list

	foreach (DesktopFile *app, apps) {
		if (app->getName().compare("") == 0) {			// Check for name
			continue;
		}
		QTreeWidgetItem *category = findCategory(*app);	// Find category
		if (category != NULL) {
			setCatInfo(category, app);
		}
	}

//	QStringListModel *model = new QStringListModel;		// Create completer and its model for editation of command
//	model->setStringList(m_applications.keys());
//	QCompleter *completer = new QCompleter;
//	completer->setModel(model);
//	m_edtCommand->setCompleter(completer);

	m_loaded = true;
}

void MimeDlg::threadFunc(void *arg)
{
	MimeDlg *p = (MimeDlg *) arg;
	p->loadCategories();
}
//==============================================================================================================================

void MimeDlg::onLoadFinished()
{
	if (!m_mimeSettings->isLoaded() && m_mode == MimeDlgMode::EditMimes) {
		QTimer::singleShot(20, this, SLOT(loadItems()));
	}
}

void MimeDlg::loadItems()
{
	if (!m_loaded) {
		loadCategories();
		onLoadFinished();
	//	m_thread.startLoad(MimeDlg::threadFunc, (void *) this);
		return;
	}

	if (!m_mimeSettings->isLoaded() && m_mode == MimeDlgMode::EditMimes) {
	//	m_thread.startLoad(MimeSettings::threadFunc, (void *) m_mimeSettings);
		m_sideTree->setEnabled(false);
		m_buttons->setEnabled(false);
		m_busy = true;
		m_mimeSettings->loadMimes();
		m_sideTree->setEnabled(true);
		m_buttons->setEnabled(true);
		m_busy = false;
		return;
	}
}

//==============================================================================================================================

void MimeDlg::updateCommand(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous);

	m_edSelApp->setText(m_applications.key(current));
}

void MimeDlg::onAppItemDoubleClicked(QTreeWidgetItem *item, int col)
{
	Q_UNUSED(col);

	if (m_mode == MimeDlgMode::EditMimes) {
		m_mimeSettings->addDesktopItem(m_applications.key(item));
	}
}

//==============================================================================================================================
// class MimeThread
//==============================================================================================================================

MimeThread::MimeThread(QObject *parent)
	: QThread(parent)
{
}

void MimeThread::run()
{
	m_func(m_arg);
	emit loadFinished();
}

void MimeThread::startLoad(MimeThreadFunc func, void *arg)
{
	m_func = func;
	m_arg  = arg;
	start();
}
//==============================================================================================================================
