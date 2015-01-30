#include <QSettings>
#include <QDebug>

#include "yefsactions.h"
#include "yefsactionsdlg.h"
#include "ui_yefsactionsdlg.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
#include "yefileutils.h"
//==============================================================================================================================

FsActionsDlg::FsActionsDlg(YeApplication *app, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::FsActionsDlg)
	, m_app(app)
	, m_loaded(false)
	, m_modified(false)
{
	ui->setupUi(this);

	ui->btAdd->setIcon(R::icon("list-add"));
	ui->btDel->setIcon(R::icon("list-remove"));
	ui->btUp->setIcon(R::icon("go-up"));
	ui->btDown->setIcon(R::icon("go-down"));

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

FsActionsDlg::~FsActionsDlg()
{
	delete ui;
}
//==============================================================================================================================

void FsActionsDlg::showDialog()
{
	if (!m_loaded) loadItems();

	this->exec();
}

void FsActionsDlg::accept()
{
	saveItems();
	QDialog::accept();
}

void FsActionsDlg::loadCustomActions(QStringList &result)
{
	QSettings s(App::getCustomActionsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	result = s.value("items").toStringList();
	s.endGroup();
}

void FsActionsDlg::saveCustomActions(const QStringList &items)
{
	QSettings s(App::getCustomActionsFile(), QSettings::IniFormat);

	s.beginGroup("settings");
	s.setValue("items", items);
	s.endGroup();
}
//==============================================================================================================================

#define APP_TAG      "[app]"	// desktop file: *.desktop
#define EXE_TAG      "[exe]"	// command

#define FILE_TAG     "[-F]"
#define FOLDER_TAG   "[-D]"
#define ANY_FILE_TAG "[*F]"
#define ANY_TAG      "[**]"

namespace FsActionKind {
	enum { DesktopApp,			// name of desktop file: *.desktop
		   DirectExec			// executable program file directly
		 };
}

namespace FsActionData {
	enum { Kind = Qt::UserRole,
		   Name,
		   Exec,
		   Icon,
		   Match,
		   Types
		 };
}

namespace FsActionMatch {
	enum { None,
		   Folder,
		   File,
		   AnyFile,	// any file
		   Any		// any file or folder
		 };
}
//==============================================================================================================================

void FsActionsDlg::decodeCustomTypes(QStringList &result, QString &types)
{
	QChar sep = QChar(',');
	result = types.split(sep);
}

bool FsActionsDlg::decodeCustomItem(int &kind, QString &name, QString &exec, QString &icon,
									int &match, QString &types, const QString &item)
{
	QChar sep = QChar('|');

	int i = item.indexOf(sep, 0);      if (i <  0) return false;
	int j = item.indexOf(sep, i + 1);  if (j <= i) return false;
	int k = item.indexOf(sep, j + 1);  if (k <= j) return false;

	name  = item.left(i);
	exec  = item.mid(i + 1, j - i - 1);
	icon  = item.mid(j + 1, k - j - 1);
	types = item.mid(k + 1);

	kind  = decodeKind(name);
	match = decodeMatch(types);

	return true;
}

void FsActionsDlg::encodeCustomItem(QString &item, int kind, const QString &name, const QString &exec, const QString &icon,
									int match, const QString &types)
{
	QChar sep = QChar('|');
	QString tmp = types;
	insertTypeTag(tmp, match);
	item = (kind == FsActionKind::DesktopApp) ? QString(APP_TAG) : QString(EXE_TAG);
	item.append(name + sep + exec + sep + icon + sep + tmp);
}
//==============================================================================================================================

bool FsActionsDlg::matchFile(const QStringList &types, const QString &ext)
{
	int cnt = types.size();
	for (int i = 0; i < cnt; i++) {
		const QString &type = types.at(i);
		if (type == ext) return true;
	}
	return false;
}

bool FsActionsDlg::matchDir(int match)     { return match == FsActionMatch::Folder;   }
bool FsActionsDlg::matchAny(int match)     { return match == FsActionMatch::Any;      }
bool FsActionsDlg::matchAnyFile(int match) { return match == FsActionMatch::AnyFile;  }
bool FsActionsDlg::isDesktopApp(int kind)  { return kind == FsActionKind::DesktopApp; }

int FsActionsDlg::decodeKind(QString &name)
{
	int n = QString(APP_TAG).length();
	QString kind = name.left(n);
	name = name.mid(n);

	return kind == QString(APP_TAG) ? FsActionKind::DesktopApp : FsActionKind::DirectExec;
}

int FsActionsDlg::decodeMatch(QString &types)
{
	int n = QString(FILE_TAG).length();
	QString match = types.left(n);
	types = types.mid(n);

	if (match == QString(FOLDER_TAG))   return FsActionMatch::Folder;
	if (match == QString(ANY_FILE_TAG)) return FsActionMatch::AnyFile;
	if (match == QString(ANY_TAG))      return FsActionMatch::Any;

	return FsActionMatch::File;
}

void FsActionsDlg::insertTypeTag(QString &types, int match)
{
	switch (match) {
		case FsActionMatch::Folder : types.prepend(QString(FOLDER_TAG  )); break;
		case FsActionMatch::File   : types.prepend(QString(FILE_TAG    )); break;
		case FsActionMatch::AnyFile: types.prepend(QString(ANY_FILE_TAG)); break;
		case FsActionMatch::Any    : types.prepend(QString(ANY_TAG     )); break;
	}
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

void FsActionsDlg::loadItems()
{
	if (ui->listWidget->count() > 0)
		ui->listWidget->clear();

	QStringList items;
	FsActionsDlg::loadCustomActions(items);
//	qDebug() << "FsActionsDlg::loadItems()" << items;

	QString name, exec, icon, types;
	int kind, match;

	foreach (QString item, items) {
		if (FsActionsDlg::decodeCustomItem(kind, name, exec, icon, match, types, item)) {
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

void FsActionsDlg::saveItems()
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
		encodeCustomItem(appItem, kind, name, exec, icon, match, types);
		items.append(appItem);
	}

	saveCustomActions(items);
	m_app->fsActions()->resetCustomActions();
	m_modified = false;
//	qDebug() << "saved";
}
//==============================================================================================================================

void FsActionsDlg::setEditEnabled(bool enabled, int kind)
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

void FsActionsDlg::showItem(QListWidgetItem *item)
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

void FsActionsDlg::saveItem(QListWidgetItem *item)
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

void FsActionsDlg::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (previous != NULL) {
		if (m_itemModified) saveItem(previous);
	}

	if (current != NULL) {
		showItem(current);
	}
}
//==============================================================================================================================

QIcon FsActionsDlg::searchIcon(int kind, const QString &name, const QString &icon)
{
	if (kind == FsActionKind::DesktopApp && !name.isEmpty()) {
		DesktopFile df = DesktopFile("/usr/share/applications/" + name + ".desktop");
		return FileUtils::searchAppIcon(df);
	}

	return FileUtils::searchAppIcon(icon);
}

QListWidgetItem *FsActionsDlg::addItem(int kind, const QString &name, const QString &exec, const QString &icon,
									   int match, const QString &types, int pos)
{
	QListWidgetItem *item = new QListWidgetItem(searchIcon(kind, name, icon), name);

	if (pos < 0) pos = ui->listWidget->count();
	ui->listWidget->insertItem(pos, item);

	int h = R::data().iconSize + 2;
	item->setSizeHint(QSize(h, h));

	setItemKind (item, kind);
	setItemName (item, name);
	setItemExec (item, exec);
	setItemIcon (item, icon);
	setItemMatch(item, match);
	setItemTypes(item, types);

	return item;
}

void FsActionsDlg::onAdd()
{
	QString none;
	QListWidgetItem *curr = ui->listWidget->currentItem();
	int pos = curr != NULL ? ui->listWidget->row(curr) + 1 : ui->listWidget->count();
	QListWidgetItem *item = addItem(FsActionKind::DesktopApp, none, none, none, FsActionMatch::File, none, pos);
	ui->listWidget->setCurrentItem(item);
	ui->edName->setFocus();

	m_modified = true;
}

void FsActionsDlg::onDel()
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

void FsActionsDlg::onMoveUp()
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

void FsActionsDlg::onMoveDown()
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

void FsActionsDlg::onTextModified(const QString &text)
{
	Q_UNUSED(text);
	m_itemModified = true;
}

void FsActionsDlg::onRadioModified(bool checked)
{
	Q_UNUSED(checked);
	m_itemModified = true;
	ui->edAsso->setEnabled(ui->rbFile->isChecked());
}

void FsActionsDlg::onKindChanged(bool checked)
{
	Q_UNUSED(checked);
	m_itemModified = true;
	int kind = ui->rbApp->isChecked() ? FsActionKind::DesktopApp : FsActionKind::DirectExec;
	setEditEnabled(true, kind);
}
