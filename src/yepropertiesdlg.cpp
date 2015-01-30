#include <sys/stat.h>

#include <QFileInfo>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QRegExpValidator>
#include <QPushButton>
#include <QDirIterator>
#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
# include <QtConcurrent>
#else
# include <QtCore>
#endif

#include "yepropertiesdlg.h"
#include "yefscache.h"
#include "yefileutils.h"
#include "yemime.h"

#include "yemainwindow.h"
#include "yeapplication.h"
#include "yeappresources.h"
#include "yeappdata.h"
//==============================================================================================================================

PropertiesDlg::PropertiesDlg(YeApplication *app)
	: QWidget(NULL)
	, m_app(app)
	, folderIcons(app->cache()->folderIcons())
	, fileIcons(app->cache()->mimeIcons())
	, m_result(0)
{
	setAttribute(Qt::WA_DeleteOnClose, 1);
	setWindowTitle(tr("Properties"));
}

PropertiesDlg::~PropertiesDlg()
{
//	qDebug() << "~PropertiesDlg()";
}

void PropertiesDlg::showDialog(const QStringList &paths)
{
	if (paths.size() < 1) {
		this->deleteLater();
		return;
	}

	init(paths);
	show();
	thread = QtConcurrent::run(this, &PropertiesDlg::folderProperties, paths);
}

void PropertiesDlg::folderProperties(const QStringList &paths)
{
	foreach (QString path, paths)
		recurseProperties(path);

	emit updateSignal();
	emit finishedSignal();
}
//==============================================================================================================================

void PropertiesDlg::init(const QStringList &paths)
{
	pathName = paths.at(0);
	QFileInfo file(pathName);

	iconChanged = false;
	files = 0;
	folders = 0;
	totalSize = 0;

	QVBoxLayout *layout = new QVBoxLayout(this);
	QGroupBox *nameFrame = new QGroupBox(this);
	QGroupBox *fileFrame = new QGroupBox(this);
	QGroupBox *driveFrame = new QGroupBox(this);
	QGroupBox *permissions;
	QDateTime date = file.lastModified();

	path = new QLabel;
	path->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	sizeInfo = new QLabel;
	containsInfo = new QLabel;
	modifiedInfo = new QLabel(QString("%1 %2")
							  .arg(date.toString(R::data().propDateFormat))
							  .arg(date.toString(R::data().propTimeFormat)));
	permissionsNumeric = new QLineEdit;

	QHBoxLayout *nameLayout = new QHBoxLayout(nameFrame);
	nameLayout->addWidget(path, 1);

	QGridLayout *layoutPath = new QGridLayout(fileFrame);
//	layoutPath->addWidget(path, 0, 1);
//	layoutPath->setRowMinimumHeight(0, 30);

	layoutPath->addWidget(new QLabel("Size:"), 1, 0);
	layoutPath->addWidget(sizeInfo, 1, 1, Qt::AlignRight);
	layoutPath->addWidget(containsInfo, 2, 1, Qt::AlignRight);

	if (paths.count() == 1) {
		if (file.isDir()) {
			type = 1;

			iconButton = new QToolButton;
			iconButton->setToolTip(tr("Choose another icon..."));

			if (folderIcons.contains(file.fileName())) {
				iconButton->setIcon(folderIcons.value(file.fileName()));
			} else {
				iconButton->setIcon(QIcon::fromTheme("folder"));
			}

			iconButton->setIconSize(QSize(22, 22));
			iconButton->setAutoRaise(1);
			connect(iconButton, SIGNAL(clicked()), this, SLOT(changeIcon()));

			nameLayout->insertWidget(0, iconButton, 0, Qt::AlignLeft);
			layoutPath->addWidget(new QLabel("Contains:"), 2, 0);
		}
		else {
			type = 2;

			QLabel *iconLabel = new QLabel();
			QIcon theIcon;

			if (file.suffix().isEmpty()) {
				theIcon = fileIcons.value(file.isExecutable() ? "exec" : "none");
			} else {
				theIcon = fileIcons.value(file.suffix());
			}

			iconLabel->setPixmap(theIcon.pixmap(22, 22));
			nameLayout->insertWidget(0, iconLabel, 0, Qt::AlignLeft);
			layoutPath->addWidget(new QLabel("FileType:"), 2, 0);
			containsInfo->setText(m_app->mime()->getMimeType(pathName));
		}

		path->setWordWrap(1);
		path->setMinimumWidth(140);
		path->setText("<b>" + pathName);
		layoutPath->addWidget(new QLabel("Modified:"), 3, 0);
		layoutPath->addWidget(modifiedInfo, 3, 1, Qt::AlignRight);

		//permissions
		permissions = new QGroupBox(this);
		QGridLayout *layoutPermissions = new QGridLayout(permissions);

		ownerRead = new QCheckBox;
		ownerWrite = new QCheckBox;
		ownerExec = new QCheckBox;

		groupRead = new QCheckBox;
		groupWrite = new QCheckBox;
		groupExec = new QCheckBox;

		otherRead = new QCheckBox;
		otherWrite = new QCheckBox;
		otherExec = new QCheckBox;

		connect(ownerRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(ownerWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(ownerExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		connect(groupRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(groupWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(groupExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		connect(otherRead, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(otherWrite, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));
		connect(otherExec, SIGNAL(clicked(bool)), this, SLOT(checkboxesChanged()));

		layoutPermissions->addWidget(new QLabel("Read"), 1, 0);
		layoutPermissions->addWidget(new QLabel("Write"), 2, 0);
		layoutPermissions->addWidget(new QLabel("Execute"), 3, 0);

		layoutPermissions->addWidget(new QLabel(" "), 0, 1);   //blank column
		layoutPermissions->setColumnStretch(1, 1);

		layoutPermissions->addWidget(new QLabel("Owner"), 0, 2);
		layoutPermissions->addWidget(new QLabel("Group"), 0, 3);
		layoutPermissions->addWidget(new QLabel("Other"), 0, 4);

		layoutPermissions->addWidget(ownerRead, 1, 2);
		layoutPermissions->addWidget(ownerWrite, 2, 2);
		layoutPermissions->addWidget(ownerExec, 3, 2);

		layoutPermissions->addWidget(groupRead, 1, 3);
		layoutPermissions->addWidget(groupWrite, 2, 3);
		layoutPermissions->addWidget(groupExec, 3, 3);

		layoutPermissions->addWidget(otherRead, 1, 4);
		layoutPermissions->addWidget(otherWrite, 2, 4);
		layoutPermissions->addWidget(otherExec, 3, 4);

		layoutPermissions->setVerticalSpacing(2);

		connect(permissionsNumeric, SIGNAL(textChanged(QString)), this, SLOT(numericChanged(QString)));
		layoutPermissions->addWidget(new QLabel("Numeric"), 4, 0, 1, 1);
		layoutPermissions->addWidget(permissionsNumeric, 4, 2, 1, 3);

		struct stat perms;
		stat(pathName.toLocal8Bit(), &perms);
		permString = QString("%1%2%3").arg(((perms.st_mode & S_IRWXU) >> 6))
					 .arg(((perms.st_mode & S_IRWXG) >> 3))
					 .arg((perms.st_mode & S_IRWXO));

		permissionsNumeric->setText(permString);

		QRegExp input("^[0-7]*$");
		QValidator *permNumericValidator = new QRegExpValidator(input, this);
		permissionsNumeric->setValidator(permNumericValidator);
		permissionsNumeric->setMaxLength(3);

		int ret = chmod(pathName.toLocal8Bit(), permString.toInt(0, 8));
		if (ret) permissions->setDisabled(1);
	}
	else {
		type = 3;

		//calculate selected files and folders count
		foreach(QString selectedPaths, paths) {
			if (QFileInfo(selectedPaths).isDir()) folders++;
			else files++;
		}

		if ((files) && (folders))
			path->setText(QString("<b>%1 files, %2 folders").arg(files).arg(folders));
		else if ((!files) && (folders)) //no files, folders only
			path->setText(QString("<b>%1 folders").arg(folders));
		else if ((files) && (!folders)) //no folders, files only
			path->setText(QString("<b>%1 files").arg(files));

		QLabel *iconLabel = new QLabel();
		iconLabel->setPixmap(QIcon::fromTheme("help-about").pixmap(22, 22));
		nameLayout->insertWidget(0, iconLabel, 0, Qt::AlignLeft);
		layoutPath->addWidget(new QLabel("Total:"), 2, 0);
	}

	//drive info frame
	QLabel *driveIcon = new QLabel(this);
	driveIcon->setPixmap(QIcon::fromTheme("drive-harddisk").pixmap(22, 22));
	driveInfo = new QLabel();

	QGridLayout *layoutDrive = new QGridLayout(driveFrame);
	layoutDrive->addWidget(driveIcon, 0, 0);
	layoutDrive->addWidget(driveInfo, 0, 1, Qt::AlignRight);
	driveInfo->setText(FileUtils::getDriveInfo(pathName));

	//buttons
	buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttons->button(QDialogButtonBox::Ok)->setEnabled(0);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	//main layout
	layout->addWidget(nameFrame);
	layout->addWidget(fileFrame);
	if (paths.count() == 1) layout->addWidget(permissions);
	layout->addWidget(driveFrame);
	layout->addWidget(buttons);
	setLayout(layout);

	layout->setMargin(6);
	layout->setSpacing(4);

	connect(this, SIGNAL(updateSignal()), this, SLOT(update()));
	connect(this, SIGNAL(finishedSignal()), this, SLOT(finished()));

	this->adjustSize();
	YeMainWindow *win = m_app->win();
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
	if (it.hasNext())
		while (it.hasNext()) {
			if (this->result()) return;     //user cancelled

			it.next();
			if (it.fileInfo().isDir()) {
				folders++;
				if (folders % 32 == 0) emit updateSignal();
			}
			else {
				files++;
				totalSize += it.fileInfo().size();
			}
		}
	else
		totalSize += QFileInfo(path).size();
}

void PropertiesDlg::update()
{
	sizeInfo->setText(FileUtils::formatSize(totalSize));
	if (type != 2) containsInfo->setText(QString("%1 Files, %2 folders").arg(files).arg(folders));
}

void PropertiesDlg::finished()
{
	buttons->button(QDialogButtonBox::Ok)->setEnabled(1);
	this->activateWindow();
}

void PropertiesDlg::accept()
{
	this->setResult(1);
	thread.waitForFinished();

	if (permissionsNumeric->text() != permString) {                             //has changed, set new permissions
		chmod(pathName.toLocal8Bit(), permissionsNumeric->text().toInt(0, 8));  //convert to octal
	}

	if (iconChanged) {
	//	App::addFolderIcon(QFileInfo(pathName).fileName(), iconButton->icon());
		folderIcons.insert(QFileInfo(pathName).fileName(), iconButton->icon());
	}

//	this->done(1);
	this->close();
}

void PropertiesDlg::reject()
{
	this->setResult(1);
	thread.waitForFinished();
//	this->done(0);
	this->close();
}

void PropertiesDlg::checkboxesChanged()
{
	permissionsNumeric->setText(QString("%1%2%3")
								.arg(ownerRead->isChecked() * 4 + ownerWrite->isChecked() * 2 + ownerExec->isChecked())
								.arg(groupRead->isChecked() * 4 + groupWrite->isChecked() * 2 + groupExec->isChecked())
								.arg(otherRead->isChecked() * 4 + otherWrite->isChecked() * 2 + otherExec->isChecked()));
}

void PropertiesDlg::numericChanged(QString text)
{
	if (text.count() != 3) return;

	int owner = QString(text.at(0)).toInt();
	ownerRead->setChecked(owner / 4);
	ownerWrite->setChecked((owner - owner / 4 * 4 - owner % 2));
	ownerExec->setChecked(owner % 2);

	int group = QString(text.at(1)).toInt();
	groupRead->setChecked(group / 4);
	groupWrite->setChecked((group - group / 4 * 4 - group % 2));
	groupExec->setChecked(group % 2);

	int other = QString(text.at(2)).toInt();
	otherRead->setChecked(other / 4);
	otherWrite->setChecked((other - other / 4 * 4 - other % 2));
	otherExec->setChecked(other % 2);
}

void PropertiesDlg::changeIcon()
{
/*	IconDlg *dlg = IconDlg::dlg(this);
	if (dlg->exec() == 1) {
		iconChanged = true;
		iconButton->setIcon(QIcon::fromTheme(dlg->result));
	}*/
//	delete dlg;
}
//==============================================================================================================================
