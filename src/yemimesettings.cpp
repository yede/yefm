#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QSpacerItem>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDir>
#include <QDebug>

#include "yemimesettings.h"
#include "yemime.h"
#include "yemimeview.h"
#include "yemimeitem.h"

#include "yefileutils.h"
#include "yesplitter.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
#include "yeapp.h"
//==============================================================================================================================

MimeSettings::MimeSettings(YeApplication *app, QWidget *parent)
	: QWidget(parent)
	, m_app(app)
	, m_mime(app->mime())
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
	bt->setIcon(R::icon(icon, 16));
	bt->setToolTip(tips);
	connect(bt, SIGNAL(clicked()), this, method);
	return bt;
}

QWidget *MimeSettings::createMimeSettings()
{
	m_edFilter = new QLineEdit;
	QLabel *label = new QLabel(tr("Filter pattern:"));

	QHBoxLayout *filterLayout = new QHBoxLayout;
	filterLayout->setContentsMargins(0, 0, 0, 6);
	filterLayout->setSpacing(6);
	filterLayout->addWidget(label);
	filterLayout->addWidget(m_edFilter);
	//--------------------------------------------------------------------------------------------------------------------------

	label = new QLabel(tr("Mime types"));
	m_mimeView = new MimeView(m_app);

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

	connect(m_mimeView, SIGNAL(currentItemChanged(MimeItem *, MimeItem *)),
			SLOT(onMimeSelected(MimeItem *, MimeItem *)));

	return splitter;
}
//==============================================================================================================================

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

		QIcon icon = R::appIcon(app);
		m_appList->addItem(new QListWidgetItem(icon, app, m_appList));		// Add application
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

	QIcon icon = R::appIcon(name);
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
