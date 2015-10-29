#include <sys/stat.h>

#include <QFileInfo>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QRegExpValidator>
#include <QPushButton>
#include <QDirIterator>
#include <QDebug>

#include "yepropertiesdlg.h"

#include "yefileutils.h"
#include "yemime.h"

#include "yeiconloader.h"
#include "yeiconcache.h"
#include "yeappcfg.h"
#include "yeapp.h"
//==============================================================================================================================

PropertiesDlgThread::PropertiesDlgThread(PropertiesDlg *parent)
	: QThread(parent)
	, m_dlg(parent)
{
}

PropertiesDlgThread::~PropertiesDlgThread()
{
}

void PropertiesDlgThread::run()
{
	m_dlg->showProperties();
}

void PropertiesDlgThread::ensureStop()
{
	m_dlg->setQuit();
	while (isRunning()) { msleep(10); }
}

//==============================================================================================================================
// class PropertiesDlg
//==============================================================================================================================

PropertiesDlg::PropertiesDlg(const QStringList &files)
	: QDialog(NULL)
	, m_folderIcons(IconCache::instance()->folderIcons())
	, m_fileIcons(IconCache::instance()->mimeIcons())
	, m_fileList(files)
{
	setAttribute(Qt::WA_DeleteOnClose, 1);
	setWindowTitle(tr("Properties"));

	m_thread = new PropertiesDlgThread(this);
}

PropertiesDlg::~PropertiesDlg()
{
	m_thread->ensureStop();
}

void PropertiesDlg::showProperties(const QStringList &files)
{
	if (files.size() < 1) return;

	PropertiesDlg *dlg = new PropertiesDlg(files);
	dlg->showDialog();
}

void PropertiesDlg::showDialog()
{
	init();
	show();
	m_thread->start();
}

void PropertiesDlg::showProperties()
{
	foreach (QString path, m_fileList) {
		recurseProperties(path);
		if (m_quit) return;
	}

	emit updateSignal();
	emit finishedSignal();
}
//==============================================================================================================================

void PropertiesDlg::init()
{
	m_pathName = m_fileList.at(0);
	QFileInfo file(m_pathName);

	m_quit = false;
	m_iconChanged = false;
	m_files = 0;
	m_folders = 0;
	m_totalSize = 0;

	QVBoxLayout *layout = new QVBoxLayout(this);
	QGroupBox *nameFrame = new QGroupBox(this);
	QGroupBox *fileFrame = new QGroupBox(this);
	QGroupBox *driveFrame = new QGroupBox(this);
	QGroupBox *permissions;
	QDateTime date = file.lastModified();

	m_path = new QLabel;
	m_path->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_sizeInfo = new QLabel;
	m_containsInfo = new QLabel;
	m_modifiedInfo = new QLabel(QString("%1 %2")
							  .arg(date.toString(AppCfg::instance()->propDateFormat))
							  .arg(date.toString(AppCfg::instance()->propTimeFormat)));
	m_permissionsNumeric = new QLineEdit;

	QHBoxLayout *nameLayout = new QHBoxLayout(nameFrame);
	nameLayout->addWidget(m_path, 1);

	QGridLayout *layoutPath = new QGridLayout(fileFrame);
//	layoutPath->addWidget(path, 0, 1);
//	layoutPath->setRowMinimumHeight(0, 30);

	layoutPath->addWidget(new QLabel(tr("Size:")), 1, 0);
	layoutPath->addWidget(m_sizeInfo, 1, 1, Qt::AlignRight);
	layoutPath->addWidget(m_containsInfo, 2, 1, Qt::AlignRight);

	if (m_fileList.count() == 1) {
		if (file.isDir()) {
			m_type = 1;

			m_iconButton = new QToolButton;
			m_iconButton->setToolTip(tr("Choose another icon..."));

			if (m_folderIcons.contains(file.fileName())) {
				m_iconButton->setIcon(m_folderIcons.value(file.fileName()));
			} else {
				m_iconButton->setIcon(QIcon::fromTheme("folder"));
			}

			m_iconButton->setIconSize(QSize(22, 22));
			m_iconButton->setAutoRaise(1);
			connect(m_iconButton, SIGNAL(clicked()), this, SLOT(changeIcon()));

			nameLayout->insertWidget(0, m_iconButton, 0, Qt::AlignLeft);
			layoutPath->addWidget(new QLabel(tr("Contains:")), 2, 0);
		}
		else {
			m_type = 2;

			QLabel *iconLabel = new QLabel();
			QIcon theIcon;

			if (file.suffix().isEmpty()) {
				theIcon = m_fileIcons.value(file.isExecutable() ? "exec" : "none");
			} else {
				theIcon = m_fileIcons.value(file.suffix());
			}

			iconLabel->setPixmap(theIcon.pixmap(22, 22));
			nameLayout->insertWidget(0, iconLabel, 0, Qt::AlignLeft);
			layoutPath->addWidget(new QLabel(tr("FileType:")), 2, 0);
			m_containsInfo->setText(Mime::instance()->getMimeType(m_pathName));
		}

		m_path->setWordWrap(1);
		m_path->setMinimumWidth(140);
		m_path->setText("<b>" + m_pathName);
		layoutPath->addWidget(new QLabel(tr("Modified:")), 3, 0);
		layoutPath->addWidget(m_modifiedInfo, 3, 1, Qt::AlignRight);

		//permissions
		permissions = new QGroupBox(this);
		QGridLayout *layoutPermissions = new QGridLayout(permissions);

		m_ownerRead = new QCheckBox;
		m_ownerWrite = new QCheckBox;
		m_ownerExec = new QCheckBox;

		m_groupRead = new QCheckBox;
		m_groupWrite = new QCheckBox;
		m_groupExec = new QCheckBox;

		m_otherRead = new QCheckBox;
		m_otherWrite = new QCheckBox;
		m_otherExec = new QCheckBox;

		connect(m_ownerRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_ownerWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_ownerExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		connect(m_groupRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_groupWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_groupExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		connect(m_otherRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_otherWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(m_otherExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		layoutPermissions->addWidget(new QLabel(tr("Read")), 1, 0);
		layoutPermissions->addWidget(new QLabel(tr("Write")), 2, 0);
		layoutPermissions->addWidget(new QLabel(tr("Execute")), 3, 0);

		layoutPermissions->addWidget(new QLabel(" "), 0, 1);   //blank column
		layoutPermissions->setColumnStretch(1, 1);

		layoutPermissions->addWidget(new QLabel(tr("Owner")), 0, 2);
		layoutPermissions->addWidget(new QLabel(tr("Group")), 0, 3);
		layoutPermissions->addWidget(new QLabel(tr("Other")), 0, 4);

		layoutPermissions->addWidget(m_ownerRead, 1, 2);
		layoutPermissions->addWidget(m_ownerWrite, 2, 2);
		layoutPermissions->addWidget(m_ownerExec, 3, 2);

		layoutPermissions->addWidget(m_groupRead, 1, 3);
		layoutPermissions->addWidget(m_groupWrite, 2, 3);
		layoutPermissions->addWidget(m_groupExec, 3, 3);

		layoutPermissions->addWidget(m_otherRead, 1, 4);
		layoutPermissions->addWidget(m_otherWrite, 2, 4);
		layoutPermissions->addWidget(m_otherExec, 3, 4);

		layoutPermissions->setVerticalSpacing(2);

		connect(m_permissionsNumeric, SIGNAL(textChanged(QString)), this, SLOT(numericChanged(QString)));
		layoutPermissions->addWidget(new QLabel(tr("Numeric")), 4, 0, 1, 1);
		layoutPermissions->addWidget(m_permissionsNumeric, 4, 2, 1, 3);

		struct stat perms;
		stat(m_pathName.toLocal8Bit(), &perms);
		m_permString = QString("%1%2%3").arg(((perms.st_mode & S_IRWXU) >> 6))
					 .arg(((perms.st_mode & S_IRWXG) >> 3))
					 .arg((perms.st_mode & S_IRWXO));

		m_permissionsNumeric->setText(m_permString);

		QRegExp input("^[0-7]*$");
		QValidator *permNumericValidator = new QRegExpValidator(input, this);
		m_permissionsNumeric->setValidator(permNumericValidator);
		m_permissionsNumeric->setMaxLength(3);

		int ret = chmod(m_pathName.toLocal8Bit(), m_permString.toInt(0, 8));
		if (ret) permissions->setDisabled(1);
	}
	else {
		m_type = 3;

		//calculate selected files and folders count
		foreach(QString selectedPaths, m_fileList) {
			if (QFileInfo(selectedPaths).isDir()) m_folders++;
			else m_files++;
		}

		if ((m_files) && (m_folders))
			m_path->setText(tr("<b>%1 files, %2 folders").arg(m_files).arg(m_folders));
		else if ((!m_files) && (m_folders)) //no files, folders only
			m_path->setText(tr("<b>%1 folders").arg(m_folders));
		else if ((m_files) && (!m_folders)) //no folders, files only
			m_path->setText(tr("<b>%1 files").arg(m_files));

		QLabel *iconLabel = new QLabel();
		iconLabel->setPixmap(QIcon::fromTheme("help-about").pixmap(22, 22));
		nameLayout->insertWidget(0, iconLabel, 0, Qt::AlignLeft);
		layoutPath->addWidget(new QLabel(tr("Total:")), 2, 0);
	}

	//drive info frame
	QLabel *driveIcon = new QLabel(this);
	driveIcon->setPixmap(QIcon::fromTheme("drive-harddisk").pixmap(22, 22));
	m_driveInfo = new QLabel();

	QGridLayout *layoutDrive = new QGridLayout(driveFrame);
	layoutDrive->addWidget(driveIcon, 0, 0);
	layoutDrive->addWidget(m_driveInfo, 0, 1, Qt::AlignRight);
	m_driveInfo->setText(FileUtils::getDriveInfo(m_pathName));

	//buttons
	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_buttons->button(QDialogButtonBox::Ok)->setEnabled(0);
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

	//main layout
	layout->addWidget(nameFrame);
	layout->addWidget(fileFrame);
	if (m_fileList.count() == 1) layout->addWidget(permissions);
	layout->addWidget(driveFrame);
	layout->addWidget(m_buttons);
	setLayout(layout);

	layout->setMargin(6);
	layout->setSpacing(4);

	connect(this, SIGNAL(updateSignal()), this, SLOT(update()));
	connect(this, SIGNAL(finishedSignal()), this, SLOT(finished()));

	this->adjustSize();
	QMainWindow *win = App::win();
	int posX = (win->width() > width()) ?
			   (win->width() - width()) / 2 : 0;
	int posY = (win->height() > height()) ?
			   (win->height() - height()) / 2 : 0;
	posX += win->x();
	posY += win->y();
	this->move(posX, posY);
}
//==============================================================================================================================

void PropertiesDlg::recurseProperties(const QString &path)
{
	QDirIterator it(path, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden,
					QDirIterator::Subdirectories);

	if (it.hasNext()) {
		while (it.hasNext()) {
			if (m_quit) return;		// user cancelled

			it.next();
			if (it.fileInfo().isDir()) {
				m_folders++;
				if (m_folders % 32 == 0) emit updateSignal();
			}
			else {
				m_files++;
				m_totalSize += it.fileInfo().size();
			}
		}
	}
	else {
		m_totalSize += QFileInfo(path).size();
	}
}

void PropertiesDlg::update()
{
	m_sizeInfo->setText(FileUtils::formatSize(m_totalSize));
	if (m_type != 2) m_containsInfo->setText(tr("%1 Files, %2 folders").arg(m_files).arg(m_folders));
}

void PropertiesDlg::finished()
{
	m_buttons->button(QDialogButtonBox::Ok)->setEnabled(1);
	this->activateWindow();
}

void PropertiesDlg::accept()
{
	m_thread->ensureStop();

	if (m_permissionsNumeric->text() != m_permString) {                             //has changed, set new permissions
		chmod(m_pathName.toLocal8Bit(), m_permissionsNumeric->text().toInt(0, 8));  //convert to octal
	}

	if (m_iconChanged) {
	//	App::addFolderIcon(QFileInfo(pathName).fileName(), iconButton->icon());
		m_folderIcons.insert(QFileInfo(m_pathName).fileName(), m_iconButton->icon());
	}

	done(1);
}

void PropertiesDlg::reject()
{
	m_thread->ensureStop();
	done(0);
}

void PropertiesDlg::checkboxesChanged()
{
	m_permissionsNumeric->setText(QString("%1%2%3")
								.arg(m_ownerRead->isChecked() * 4 + m_ownerWrite->isChecked() * 2 + m_ownerExec->isChecked())
								.arg(m_groupRead->isChecked() * 4 + m_groupWrite->isChecked() * 2 + m_groupExec->isChecked())
								.arg(m_otherRead->isChecked() * 4 + m_otherWrite->isChecked() * 2 + m_otherExec->isChecked()));
}

void PropertiesDlg::numericChanged(QString text)
{
	if (text.count() != 3) return;

	int owner = QString(text.at(0)).toInt();
	m_ownerRead->setChecked(owner / 4);
	m_ownerWrite->setChecked((owner - owner / 4 * 4 - owner % 2));
	m_ownerExec->setChecked(owner % 2);

	int group = QString(text.at(1)).toInt();
	m_groupRead->setChecked(group / 4);
	m_groupWrite->setChecked((group - group / 4 * 4 - group % 2));
	m_groupExec->setChecked(group % 2);

	int other = QString(text.at(2)).toInt();
	m_otherRead->setChecked(other / 4);
	m_otherWrite->setChecked((other - other / 4 * 4 - other % 2));
	m_otherExec->setChecked(other % 2);
}

void PropertiesDlg::changeIcon()
{
/*	IconDlg *dlg = IconDlg::dlg(this);
	if (dlg->exec() == 1) {
		m_iconChanged = true;
		iconButton->setIcon(QIcon::fromTheme(dlg->result));
	}*/
//	delete dlg;
}
//==============================================================================================================================
