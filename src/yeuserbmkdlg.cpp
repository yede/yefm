#include "yeuserbmkdlg.h"
#include "ui_yeuserbmkdlg.h"

UserBmkDlg::UserBmkDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::UserBmkDlg)
{
	ui->setupUi(this);
}

UserBmkDlg::~UserBmkDlg()
{
	delete ui;
}

bool UserBmkDlg::showDialog(QString &title, QString &path)
{
	ui->edName->setText(title);
	ui->edPath->setText(path);
	ui->edName->setFocus();

	ui->edPath->setEnabled(path != "::");

	int ok = exec();
	if (ok != QDialog::Accepted) return false;

	if (title == ui->edName->text() && path == ui->edPath->text())	// no changing
		return false;

	title = ui->edName->text();
	path  = ui->edPath->text();
	return true;
}
