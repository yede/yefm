#ifndef YE_PROGRESSDLG_H
#define YE_PROGRESSDLG_H


#include <QLabel>
#include <QTime>
#include <QProgressBar>
#include <QDialog>
//==============================================================================================================================

class ProgressDlg : public QDialog
{
    Q_OBJECT

public:
	ProgressDlg();
	void setTitle(const QString &title);

public slots:
    void setShowing();
    void update(qint64 bytes, qint64 total, QString name);

private:
    QLabel *filename;
    QLabel *transferInfo;
    QProgressBar *bar;
    QPushButton *button;

    QTime  *remainingTimer;

    qint64 runningTotal;
    int oldSeconds;
};

#endif // YE_PROGRESSDLG_H
