#include <QVBoxLayout>
#include <QGroupBox>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QSpacerItem>

#include "yemimesettings.h"
#include "yemime.h"
#include "yeapplication.h"
#include "yefileutils.h"
#include "yesplitter.h"
//==============================================================================================================================

MimeSettings::MimeSettings(YeApplication *app, QWidget *parent)
	: QWidget(parent)
	, m_app(app)
	, m_mime(app->mime())
	, m_loaded(false)
{
	QWidget *mimeWidget = createMimeSettings();
	m_progress = createMimeProgress();

	m_stack = new QStackedLayout(this);
	m_stack->addWidget(m_progress);
	m_stack->addWidget(mimeWidget);
}

MimeSettings::~MimeSettings()
{

}
//==============================================================================================================================

QWidget *MimeSettings::createMimeSettings()
{
	QLabel *label = new QLabel(tr("Mime types"));
	m_mimeWidget = new QTreeWidget;

	QWidget *mimeWidget = new QWidget;
	QVBoxLayout *mimeLayout = new QVBoxLayout(mimeWidget);

	mimeLayout->setContentsMargins(0, 0, 0, 0);
	mimeLayout->setSpacing(4);
	mimeLayout->addWidget(label);
	mimeLayout->addWidget(m_mimeWidget, 1);

	m_mimeWidget->setAlternatingRowColors(true);
	m_mimeWidget->setRootIsDecorated(true);
	m_mimeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_mimeWidget->setColumnWidth(0, 220);

	QTreeWidgetItem *header = m_mimeWidget->headerItem();
	header->setText(0, tr("Mime"));
	header->setText(1, tr("Application"));
	//--------------------------------------------------------------------------------------------------------------------------

	label = new QLabel(tr("Applications"));
	m_appList = new QListWidget;
	QPushButton *btnRem = new QPushButton(tr("Remove"));
	QPushButton *btnUp = new QPushButton(tr("Move up"));
	QPushButton *btnDown = new QPushButton(tr("Move down"));
	QSpacerItem *sp = new QSpacerItem(1, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);

	m_appAssoc = new QWidget;
	m_appAssoc->setEnabled(false);
	QGridLayout *grid = new QGridLayout(m_appAssoc);

	grid->setContentsMargins(0, 0, 0, 0);
	grid->setSpacing(4);
	grid->addWidget(label    , 0, 0, 1, 2);
	grid->addWidget(m_appList, 1, 0, 4, 1);
	grid->addItem(sp       , 1, 1);
	grid->addWidget(btnRem   , 2, 1);
	grid->addWidget(btnUp    , 3, 1);
	grid->addWidget(btnDown  , 4, 1);
	//--------------------------------------------------------------------------------------------------------------------------

	Splitter *splitter = new Splitter;
	splitter->setDirection(SplitterDirection::Bottom, 120);
	splitter->setClient(m_appAssoc, mimeWidget);
	//--------------------------------------------------------------------------------------------------------------------------

	connect(m_mimeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
	        SLOT(onMimeSelected(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(btnRem, SIGNAL(clicked()), SLOT(removeAppAssoc()));
	connect(btnUp, SIGNAL(clicked()), SLOT(moveAppAssocUp()));
	connect(btnDown, SIGNAL(clicked()), SLOT(moveAppAssocDown()));

	return splitter;
}
//==============================================================================================================================

void MimeSettings::loadAppIcons()
{
	// Load application list
	QStringList apps = FileUtils::getApplicationNames();
	apps.replaceInStrings(".desktop", "");
	apps.sort();

	// Prepare source of icons
	QDir appIcons("/usr/share/pixmaps", "", 0, QDir::Files | QDir::NoDotAndDotDot);
	QStringList iconFiles = appIcons.entryList();
	QIcon defaultIcon = QIcon::fromTheme("application-x-executable");

	// Loads icon list
	QList<QIcon> icons;
	foreach (QString app, apps) {
		QPixmap temp = QIcon::fromTheme(app).pixmap(16, 16);
		if (!temp.isNull()) {
			icons.append(temp);
		} else {
			QStringList searchIcons = iconFiles.filter(app);
			if (searchIcons.count() > 0) {
				icons.append(QIcon("/usr/share/pixmaps/" + searchIcons.at(0)));
			} else {
				icons.append(defaultIcon);
			}
		}
	}
}

void MimeSettings::onMimeSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	updateMimeAssoc(previous);			// Store previously used associations
	m_appList->clear();					// Clear previously used associations

	if (current->childCount() > 0) {	// Check if current is editable
		m_appAssoc->setEnabled(false);
		return;
	}

	m_appAssoc->setEnabled(true);		// Enable editation

	QDir appIcons("/usr/share/pixmaps", "", 0, QDir::Files | QDir::NoDotAndDotDot);	// Prepare source of icons
	QStringList iconFiles = appIcons.entryList();
	QIcon defaultIcon = QIcon::fromTheme("application-x-executable");
	QStringList apps = m_mimeWidget->currentItem()->text(1).remove(" ").split(";");

	foreach (QString app, apps) {
		if (app.compare("") == 0) {		// Skip empty string
			continue;
		}

		QIcon temp = QIcon::fromTheme(app).pixmap(16, 16);		// Finds icon
		if (temp.isNull()) {
			QStringList searchIcons = iconFiles.filter(app);
			if (searchIcons.count() > 0) {
				temp = QIcon("/usr/share/pixmaps/" + searchIcons.at(0));
			} else {
				temp = defaultIcon;
			}
		}

		m_appList->addItem(new QListWidgetItem(temp, app, m_appList));		// Add application
	}
}

void MimeSettings::updateMimeAssoc(QTreeWidgetItem *item)
{
	if (item && item->childCount() == 0) {
		QStringList associations;
		for (int i = 0; i < m_appList->count(); i++) {
			associations.append(m_appList->item(i)->text());
		}
		item->setText(1, associations.join(";"));
	}
}

void MimeSettings::addDesktopItem(const QString &name)
{
	if (name.isEmpty() || !m_appAssoc->isEnabled()) {
		return;
	}

	for (int i = 0; i < m_appList->count(); i++) {			// If application with same name is already used, exit
		if (m_appList->item(i)->text().compare(name) == 0) {
			return;
		}
	}

	QIcon icon = QIcon::fromTheme(name).pixmap(16, 16);		// Add new launcher to the list of launchers
	m_appList->addItem(new QListWidgetItem(icon, name, m_appList));
	updateMimeAssoc(m_mimeWidget->currentItem());
}

void MimeSettings::removeAppAssoc()
{
	qDeleteAll(m_appList->selectedItems());
	updateMimeAssoc(m_mimeWidget->currentItem());
}

void MimeSettings::moveAppAssocUp()
{
	QListWidgetItem *current = m_appList->currentItem();
	int currIndex = m_appList->row(current);
	QListWidgetItem *prev = m_appList->item(m_appList->row(current) - 1);
	int prevIndex = m_appList->row(prev);
	QListWidgetItem *temp = m_appList->takeItem(prevIndex);
	m_appList->insertItem(prevIndex, current);
	m_appList->insertItem(currIndex, temp);
	updateMimeAssoc(m_mimeWidget->currentItem());
}

void MimeSettings::moveAppAssocDown()
{
	QListWidgetItem *current = m_appList->currentItem();
	int currIndex = m_appList->row(current);
	QListWidgetItem *next = m_appList->item(m_appList->row(current) + 1);
	int nextIndex = m_appList->row(next);
	QListWidgetItem *temp = m_appList->takeItem(nextIndex);
	m_appList->insertItem(currIndex, temp);
	m_appList->insertItem(nextIndex, current);
	updateMimeAssoc(m_mimeWidget->currentItem());
}
//==============================================================================================================================

QWidget *MimeSettings::createMimeProgress()
{
	m_progressMime = new QProgressBar;
	m_progressMime->setMinimumWidth(250);
	m_progressMime->setMaximumWidth(250);

	QWidget *widget = new QWidget(this);
	QGridLayout *layout = new QGridLayout(widget);

	layout->addWidget(new QLabel(tr("Loading mime types...")), 1, 1);
	layout->addWidget(m_progressMime, 2, 1);

	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 0, 4);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 2, 4);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
	                                QSizePolicy::MinimumExpanding), 0, 1);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
	                                QSizePolicy::MinimumExpanding), 3, 1);

	return widget;
}
//==============================================================================================================================

bool MimeSettings::save()
{
	for (int i = 0; i < m_mimeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *cathegory = m_mimeWidget->topLevelItem(i);
		QString cathegoryName = cathegory->text(0) + "/";
		for (int j = 0; j < cathegory->childCount(); j++) {
			QString mime = cathegoryName + cathegory->child(j)->text(0);
			QString appNames = cathegory->child(j)->text(1);
			if (!appNames.isEmpty()) {
				QStringList temps = appNames.split(";");
				for (int i = 0; i < temps.size(); i++) {
					temps[i] = temps[i] + ".desktop";
				}
				m_mime->setDefault(mime, temps);
			}
		}
	}
	m_mime->saveDefaults();

	return true;
}
//==============================================================================================================================

/*
 * This function run inside thread.
 */
void MimeSettings::loadMimes()
{
	// If mimes have been already loaded move to another section (mime config)
	if (m_loaded) {
		m_stack->setCurrentIndex(1);
		return;
	}

	m_stack->setCurrentIndex(0);

	// Load list of mimes
	QStringList mimes = m_mime->getMimeTypes();

	// Init process
	m_progressMime->setRange(1, mimes.size());

	// Default icon
	QIcon defaultIcon = QIcon::fromTheme("unknown");

	// Mime cathegories and their icons
	QMap<QString, QTreeWidgetItem *> categories;
	QMap<QTreeWidgetItem *, QIcon> genericIcons;

	// Load mime settings
	foreach (QString mime, mimes) {

		// Updates progress
		m_progressMime->setValue(m_progressMime->value() + 1);

		// Skip all 'inode' nodes including 'inode/directory'
		if (mime.startsWith("inode")) {
			continue;
		}

		// Skip all 'x-content' and 'message' nodes
		if (mime.startsWith("x-content") || mime.startsWith("message")) {
			continue;
		}

		// Parse mime
		QStringList splitMime = mime.split("/");

		// Retrieve cathegory
		QIcon icon;
		QString categoryName = splitMime.first();
		QTreeWidgetItem *category = categories.value(categoryName, NULL);
		if (!category) {
			category = new QTreeWidgetItem(m_mimeWidget);
			category->setText(0, categoryName);
			category->setFlags(Qt::ItemIsEnabled);
			categories.insert(categoryName, category);
			icon = FileUtils::searchGenericIcon(categoryName, defaultIcon);
			genericIcons.insert(category, icon);
		} else {
			icon = genericIcons.value(category);
		}

		// Load icon and default application for current mime
		// NOTE: if icon is not found generic icon is used
		icon = FileUtils::searchMimeIcon(mime, icon);
		QString appNames = m_mime->getDefault(mime).join(";");

		// Create item from current mime
		QTreeWidgetItem *item = new QTreeWidgetItem(category);
		item->setIcon(0, icon);
		item->setText(0, splitMime.at(1));
		item->setText(1, appNames.remove(".desktop"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	// Move to mimes
	m_stack->setCurrentIndex(1);
	m_loaded = true;
}

void MimeSettings::threadFunc(void *arg)
{
	MimeSettings *p = (MimeSettings *) arg;
	p->loadMimes();
}
//==============================================================================================================================

//void QTreeWidget::itemExpanded(QTreeWidgetItem *item)
