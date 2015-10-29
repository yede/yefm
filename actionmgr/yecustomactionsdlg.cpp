#include <QSettings>
#include <QDebug>

#include "yecustomactiondefines.h"
#include "yecustomactions.h"
#include "yecustomactionsdlg.h"
#include "ui_yecustomactionsdlg.h"

#include "yeactionmgr.h"

#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"

#include "yedesktopfile.h"
//==============================================================================================================================

CustomActionsDlg::CustomActionsDlg(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::CustomActionsDlg)
	, m_loaded(false)
	, m_modified(false)
{
	ui->setupUi(this);

	ui->btAdd->setIcon(IconLoader::icon("list-add"));
	ui->btDel->setIcon(IconLoader::icon("list-remove"));
	ui->btUp->setIcon(IconLoader::icon("go-up"));
	ui->btDown->setIcon(IconLoader::icon("go-down"));

	connect(ui->btAdd,  SIGNAL(clicked()), this, SLOT(onAdd()));
	connect(ui->btDel,  SIGNAL(clicked()), this, SLOT(onDel()));
	connect(ui->btUp,   SIGNAL(clicked()), this, SLOT(onMoveUp()));
	connect(ui->btDown, SIGNAL(clicked()), this, SLOT(onMoveDown()));

	connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
			this, SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

	connect(ui->edName, SIGNAL(textChanged(QString)), this, SLOT(onTextModified(QString)));
	connect(ui->edExec, SIGNAL(textChanged(QString)), this, SLOT(onTextModified(QString)));
	connect(ui->edAsso, SIGNAL(textChanged(QString)), this, SLOT(onTextModified(QString)));

	connect(ui->rbAny,     SIGNAL(toggled(bool)), this, SLOT(onRadioModified(bool)));
	connect(ui->rbAnyFile, SIGNAL(toggled(bool)), this, SLOT(onRadioModified(bool)));
	connect(ui->rbFolder,  SIGNAL(toggled(bool)), this, SLOT(onRadioModified(bool)));
	connect(ui->rbFile,    SIGNAL(toggled(bool)), this, SLOT(onRadioModified(bool)));

	connect(ui->rbApp, SIGNAL(toggled(bool)), this, SLOT(onKindChanged(bool)));
	connect(ui->rbExe, SIGNAL(toggled(bool)), this, SLOT(onKindChanged(bool)));
}

CustomActionsDlg::~CustomActionsDlg()
{
	delete ui;
}
//==============================================================================================================================

void CustomActionsDlg::showDialog()
{
	if (!m_loaded) loadItems();

	this->exec();
}

void CustomActionsDlg::accept()
{
	saveItems();
	QDialog::accept();
}
//==============================================================================================================================

static int getItemKind(QListWidgetItem *item)
{
	return item == NULL ? 0 : item->data(FsActionData::Kind).toInt();
}

static QString getItemName(QListWidgetItem *item)
{
	return item == NULL ? QString() : item->data(FsActionData::Name).toString();
}

static QString getItemExec(QListWidgetItem *item)
{
	return item == NULL ? QString() : item->data(FsActionData::Exec).toString();
}

static QString getItemIcon(QListWidgetItem *item)
{
	return item == NULL ? QString() : item->data(FsActionData::Icon).toString();
}

static int getItemMatch(QListWidgetItem *item)
{
	if (item != NULL) {
		int m = item->data(FsActionData::Match).toInt();
		switch (m) {
			case FsActionMatch::Folder : return FsActionMatch::Folder;
			case FsActionMatch::File   : return FsActionMatch::File;
			case FsActionMatch::AnyFile: return FsActionMatch::AnyFile;
			case FsActionMatch::Any    : return FsActionMatch::Any;
		}
	}
	return FsActionMatch::None;
}

static QString getItemTypes(QListWidgetItem *item)
{
	return item == NULL ? QString() : item->data(FsActionData::Types).toString();
}

static void setItemKind (QListWidgetItem *item, int kind)             { item->setData(FsActionData::Kind , kind ); }
static void setItemName (QListWidgetItem *item, const QString &name)  { item->setData(FsActionData::Name , name ); }
static void setItemExec (QListWidgetItem *item, const QString &exec)  { item->setData(FsActionData::Exec , exec ); }
static void setItemIcon (QListWidgetItem *item, const QString &icon)  { item->setData(FsActionData::Icon , icon ); }
static void setItemMatch(QListWidgetItem *item, int match)            { item->setData(FsActionData::Match, match); }
static void setItemTypes(QListWidgetItem *item, const QString &types) { item->setData(FsActionData::Types, types); }
//==============================================================================================================================

void CustomActionsDlg::loadItems()
{
	if (ui->listWidget->count() > 0)
		ui->listWidget->clear();

	QStringList items;
	CustomActions::load(items);
//	qDebug() << "CustomActionsDlg::loadItems()" << items;

	QString name, exec, icon, types;
	int kind, match;

	foreach (QString item, items) {
		if (CustomActions::decode(kind, name, exec, icon, match, types, item)) {
			addItem(kind, name, exec, icon, match, types);
		}
	}

	if (ui->listWidget->count() < 1) {
		addItem(FsActionKind::DirectExec, tr("Extract here"), "tar xf %f", "package-x-generic",
				FsActionMatch::File, "tgz,gz,bz2");
		m_modified = true;
	}

	ui->listWidget->setCurrentRow(0);
//	setEditEnabled(true);

	m_loaded = true;
}

void CustomActionsDlg::saveItems()
{
	if (m_itemModified) {
		QListWidgetItem *curr = ui->listWidget->currentItem();
		if (curr != NULL) saveItem(curr);
	}

	if (!m_modified) return;

	QStringList items;
	QString appItem;
	QString name, exec, icon, types;
	int kind, match;
	int cnt = ui->listWidget->count();

	for (int i = 0; i < cnt; i++) {
		QListWidgetItem *item = ui->listWidget->item(i);
		kind  = getItemKind(item);
		name  = getItemName(item);
		exec  = getItemExec(item);
		icon  = getItemIcon(item);
		match = getItemMatch(item);
		types = getItemTypes(item);
		if (kind == FsActionKind::DesktopApp && name.isEmpty()) continue;
		if (kind == FsActionKind::DirectExec && (name.isEmpty() || exec.isEmpty())) continue;
		CustomActions::encode(appItem, kind, name, exec, icon, match, types);
		items.append(appItem);
	}

	CustomActions::save(items);
	ActionMgr::instance()->resetCustomActions();
	m_modified = false;
//	qDebug() << "saved";
}
//==============================================================================================================================

void CustomActionsDlg::setEditEnabled(bool enabled, int kind)
{
	bool isExec = kind == FsActionKind::DirectExec;

	ui->btSelApp->setEnabled(enabled && !isExec);
	ui->btSelCmd->setEnabled(enabled && isExec);
	ui->btSelIcon->setEnabled(enabled && isExec);

	ui->edName->setEnabled(enabled);
	ui->edExec->setEnabled(enabled && isExec);
	ui->edIcon->setEnabled(enabled && isExec);
	ui->edAsso->setEnabled(enabled);

	ui->kindFrame->setEnabled(enabled);
	ui->radioFrame->setEnabled(enabled);

	ui->lbName->setText(isExec ? tr("Title:") : QString("%1 (*.desktop)").arg(tr("Launcher:")));
}

void CustomActionsDlg::showItem(QListWidgetItem *item)
{
	ui->edName->setText(getItemName(item));
	ui->edExec->setText(getItemExec(item));
	ui->edIcon->setText(getItemIcon(item));
	ui->edAsso->setText(getItemTypes(item));

	switch (getItemMatch(item)) {
		case FsActionMatch::Folder : ui->rbFolder->setChecked(true);  break;
		case FsActionMatch::AnyFile: ui->rbAnyFile->setChecked(true); break;
		case FsActionMatch::Any    : ui->rbAny->setChecked(true);     break;
		case FsActionMatch::File   :
		default                    : ui->rbFile->setChecked(true);
	}

	int kind = getItemKind(item);
	if (kind == FsActionKind::DesktopApp) {
		ui->rbApp->setChecked(true);
	} else {
		ui->rbExe->setChecked(true);
	}

	setEditEnabled(true, kind);
	ui->edAsso->setEnabled(ui->rbFile->isChecked());

	m_itemModified = false;
}

void CustomActionsDlg::saveItem(QListWidgetItem *item)
{
	int kind, match;

	kind = ui->rbApp->isChecked() ? FsActionKind::DesktopApp : FsActionKind::DirectExec;

	if      (ui->rbFolder->isChecked())  match = FsActionMatch::Folder;
	else if (ui->rbAnyFile->isChecked()) match = FsActionMatch::AnyFile;
	else if (ui->rbAny->isChecked())     match = FsActionMatch::Any;
	else                                 match = FsActionMatch::File;

	setItemKind (item, kind);
	setItemName (item, ui->edName->text().trimmed());
	setItemExec (item, ui->edExec->text().trimmed());
	setItemIcon (item, ui->edIcon->text().trimmed());
	setItemMatch(item, match);
	setItemTypes(item, ui->edAsso->text().trimmed());

	QString name = getItemName(item);
	item->setText(name);
	item->setIcon(searchIcon(kind, name, getItemIcon(item)));

	m_itemModified = false;
	m_modified = true;
}

void CustomActionsDlg::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (previous != NULL) {
		if (m_itemModified) saveItem(previous);
	}

	if (current != NULL) {
		showItem(current);
	}
}
//==============================================================================================================================

QIcon CustomActionsDlg::searchIcon(int kind, const QString &name, const QString &icon)
{
	if (kind == FsActionKind::DesktopApp && !name.isEmpty()) {
		DesktopFile df = DesktopFile("/usr/share/applications/" + name + ".desktop");
		return IconLoader::appIcon(df);
	}

	return IconLoader::appIcon(icon);
}

QListWidgetItem *CustomActionsDlg::addItem(int kind, const QString &name, const QString &exec, const QString &icon,
									   int match, const QString &types, int pos)
{
	QListWidgetItem *item = new QListWidgetItem(searchIcon(kind, name, icon), name);

	if (pos < 0) pos = ui->listWidget->count();
	ui->listWidget->insertItem(pos, item);

	int h = AppCfg::instance()->iconSize + 2;
	item->setSizeHint(QSize(h, h));

	setItemKind (item, kind);
	setItemName (item, name);
	setItemExec (item, exec);
	setItemIcon (item, icon);
	setItemMatch(item, match);
	setItemTypes(item, types);

	return item;
}

void CustomActionsDlg::onAdd()
{
	QString none;
	QListWidgetItem *curr = ui->listWidget->currentItem();
	int pos = curr != NULL ? ui->listWidget->row(curr) + 1 : ui->listWidget->count();
	QListWidgetItem *item = addItem(FsActionKind::DesktopApp, none, none, none, FsActionMatch::File, none, pos);
	ui->listWidget->setCurrentItem(item);
	ui->edName->setFocus();

	m_modified = true;
}

void CustomActionsDlg::onDel()
{
	QListWidgetItem *curr = ui->listWidget->currentItem();
	if (curr == NULL) return;

	int i = ui->listWidget->row(curr);
	int n = i < (ui->listWidget->count() - 1) ? (i + 1) : (i - 1);
	QListWidgetItem *next = n < 0 ? NULL : ui->listWidget->item(n);

	delete curr;

	if (next != NULL) {
		ui->listWidget->setCurrentItem(next);
	} else {
		setEditEnabled(false, FsActionKind::DesktopApp);
	}

	m_modified = true;
}

void CustomActionsDlg::onMoveUp()
{
	QListWidgetItem *curr = ui->listWidget->currentItem();
	if (curr == NULL) return;

	int i = ui->listWidget->row(curr);
	if (i < 1) return;

	ui->listWidget->takeItem(i);
	ui->listWidget->insertItem(i - 1, curr);
	ui->listWidget->setCurrentItem(curr);

	m_modified = true;
}

void CustomActionsDlg::onMoveDown()
{
	QListWidgetItem *curr = ui->listWidget->currentItem();
	if (curr == NULL) return;

	int i = ui->listWidget->row(curr);
	int n = ui->listWidget->count() - 1;
	if (i >= n) return;

	ui->listWidget->takeItem(i);
	ui->listWidget->insertItem(i + 1, curr);
	ui->listWidget->setCurrentItem(curr);

	m_modified = true;
}
//==============================================================================================================================

void CustomActionsDlg::onTextModified(const QString &text)
{
	Q_UNUSED(text);
	m_itemModified = true;
}

void CustomActionsDlg::onRadioModified(bool checked)
{
	Q_UNUSED(checked);
	m_itemModified = true;
	ui->edAsso->setEnabled(ui->rbFile->isChecked());
}

void CustomActionsDlg::onKindChanged(bool checked)
{
	Q_UNUSED(checked);
	m_itemModified = true;
	int kind = ui->rbApp->isChecked() ? FsActionKind::DesktopApp : FsActionKind::DirectExec;
	setEditEnabled(true, kind);
}
