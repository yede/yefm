#ifndef YEUSERBMKDLG_H
#define YEUSERBMKDLG_H

#include <QDialog>

namespace Ui {
class UserBmkDlg;
}

class UserBmkDlg : public QDialog
{
	Q_OBJECT

public:
	explicit UserBmkDlg(QWidget *parent = 0);
	~UserBmkDlg();

	bool showDialog(QString &title, QString &path);

private:
	Ui::UserBmkDlg *ui;
};

#endif // YEUSERBMKDLG_H
