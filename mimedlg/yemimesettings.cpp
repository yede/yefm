#include <QVBoxLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QSpacerItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QDebug>

#include "yemimesettings.h"
#include "yemime.h"
#include "yemimeview.h"
#include "yemimeitem.h"

#include "yesplitter.h"
#include "yeiconloader.h"
#include "yeappcfg.h"
//==============================================================================================================================

MimeSettings::MimeSettings(QWidget *parent)
	: QWidget(parent)
{
	QWidget *mimeWidget = createMimeSettings();
	m_progress = createMimeProgress();

	m_stack = new QStackedLayout(this);
	m_stack->addWidget(mimeWidget);
	m_stack->addWidget(m_progress);
}

MimeSettings::~MimeSettings()
{
}
//==============================================================================================================================

QWidget *MimeSettings::createToolButton(const QString &icon, const QString &tips, const char *method)
{
	QPushButton *bt = new QPushButton;
	bt->setIcon(IconLoader::icon(icon, 16));
	bt->setToolTip(tips);
	connect(bt, SIGNAL(clicked()), this, method);
	return bt;
}

QWidget *MimeSettings::createMimeSettings()
{
	m_mimeView = new MimeView();
	m_edFilter = new MimeFilterEdit(m_mimeView);

	QLabel *label = new QLabel(tr("Filter pattern:"));
	QHBoxLayout *filterLayout = new QHBoxLayout;
	filterLayout->setContentsMargins(0, 0, 0, 6);
	filterLayout->setSpacing(6);
	filterLayout->addWidget(label);
	filterLayout->addWidget(m_edFilter);
	//--------------------------------------------------------------------------------------------------------------------------

	label = new QLabel(tr("Mime types"));
	QWidget *mimeWidget = new QWidget;
	QVBoxLayout *mimeLayout = new QVBoxLayout(mimeWidget);

	mimeLayout->setContentsMargins(0, 0, 0, 0);
	mimeLayout->setSpacing(4);
	mimeLayout->addWidget(label);
	mimeLayout->addWidget(m_mimeView, 1);
	mimeLayout->addLayout(filterLayout);
	//--------------------------------------------------------------------------------------------------------------------------

	label = new QLabel(tr("Associated applications:"));
	m_appList = new QListWidget;
	QWidget *btnRem  = createToolButton("list-remove", tr("Remove"), SLOT(removeAppAssoc()));
	QWidget *btnUp   = createToolButton("go-up", tr("Move up"), SLOT(moveAppAssocUp()));
	QWidget *btnDown = createToolButton("go-down", tr("Move down"), SLOT(moveAppAssocDown()));
	QSpacerItem *sp = new QSpacerItem(1, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);

	m_appAssoc = new QWidget;
	m_appAssoc->setEnabled(false);
	QGridLayout *grid = new QGridLayout(m_appAssoc);

	grid->setContentsMargins(0, 0, 0, 0);
	grid->setSpacing(4);
	grid->addWidget(label    , 0, 0, 1, 2);
	grid->addWidget(m_appList, 1, 0, 4, 1);
	grid->addItem  (sp       , 1, 1);
	grid->addWidget(btnRem   , 2, 1);
	grid->addWidget(btnUp    , 3, 1);
	grid->addWidget(btnDown  , 4, 1);
	//--------------------------------------------------------------------------------------------------------------------------

	Splitter *splitter = new Splitter;
	splitter->setDirection(SplitterDirection::Bottom, 120);
	splitter->setClient(m_appAssoc, mimeWidget);
	//--------------------------------------------------------------------------------------------------------------------------

	connect(this, SIGNAL(iconThemeChanged()), m_mimeView, SLOT(updateIconTheme()));
	connect(m_mimeView, SIGNAL(currentItemChanged(MimeItem*,MimeItem *)), this, SLOT(onMimeSelected(MimeItem*,MimeItem*)));

	return splitter;
}
//==============================================================================================================================

void MimeSettings::updateItemSizeHint(QListWidgetItem *item)
{
	int sz = AppCfg::instance()->iconSize + 2;
	item->setSizeHint(QSize(sz, sz));
}

void MimeSettings::onMimeSelected(MimeItem *current, MimeItem *previous)
{
	Q_UNUSED(previous);
	m_appList->clear();					// Clear previously used associations

	if (current == NULL || current->type != MimeItemType::Mime) {	// Check if current is editable
		m_appAssoc->setEnabled(false);
		return;
	}

	m_appAssoc->setEnabled(true);		// Enable editation

	QStringList apps = current->apps.remove(" ").split(";");

	foreach (QString app, apps) {
		if (app.isEmpty()) continue;

		QIcon icon = IconLoader::appIcon(app);
		QListWidgetItem *item = new QListWidgetItem(icon, app, m_appList);
		updateItemSizeHint(item);
		m_appList->addItem(item);		// Add application
	}
}

void MimeSettings::updateMimeAssoc(MimeItem *item)
{
	if (item != NULL && item->type == MimeItemType::Mime) {
		QStringList associations;
		for (int i = 0; i < m_appList->count(); i++) {
			associations.append(m_appList->item(i)->text());
		}
		item->apps = associations.join(";");
		m_mimeView->updateApps(item);
	}
}

void MimeSettings::updateIconTheme()
{
	emit iconThemeChanged();
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

	QIcon icon = IconLoader::appIcon(name);
	m_appList->addItem(new QListWidgetItem(icon, name, m_appList));
	updateMimeAssoc(m_mimeView->currentItem());
}

void MimeSettings::removeAppAssoc()
{
	qDeleteAll(m_appList->selectedItems());
	updateMimeAssoc(m_mimeView->currentItem());
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
	updateMimeAssoc(m_mimeView->currentItem());
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
	updateMimeAssoc(m_mimeView->currentItem());
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

bool MimeSettings::saveMimes()
{
	return m_mimeView->saveMimes();
}
//==============================================================================================================================

bool MimeSettings::isLoaded() const
{
	return m_mimeView->isLoaded();
}

void MimeSettings::loadMimes()
{
//	m_stack->setCurrentIndex(1);
	m_mimeView->loadMimes();
//	m_stack->setCurrentIndex(0);
}

void MimeSettings::threadFunc(void *arg)
{
	MimeSettings *p = (MimeSettings *) arg;
	p->loadMimes();
}

//==============================================================================================================================
// class MimeFilterEdit
//==============================================================================================================================

MimeFilterEdit::MimeFilterEdit(MimeView *view, QWidget *parent)
	: QLineEdit(parent), m_view(view)
{
//	connect(m_edFilter, SIGNAL(editingFinished()), this, SLOT(updateMimeFilter()));
}

void MimeFilterEdit::updateMimeFilter()
{
	QString pattern = text().trimmed();
	m_view->setFilter(pattern);
}

void MimeFilterEdit::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	int mod = event->modifiers();

	if (key == Qt::Key_Return || key == Qt::Key_Enter) {
		updateMimeFilter();
		event->accept();
		return;
	}

	bool flag = (!hasSelectedText() && key == Qt::Key_X && mod == Qt::ControlModifier) ||
				(key == Qt::Key_Delete && mod == Qt::ControlModifier);
	if (flag) {
		clear();
		updateMimeFilter();
		event->accept();
		return;
	}

	QLineEdit::keyPressEvent(event);
}
//==============================================================================================================================
