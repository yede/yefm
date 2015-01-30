#ifndef YE_PROPERTIESDLG_H
#define YE_PROPERTIESDLG_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFuture>
#include <QHash>
#include <QIcon>
//==============================================================================================================================

class YeApplication;

class PropertiesDlg: public QWidget
{
	Q_OBJECT

public:
	PropertiesDlg(YeApplication *app);
	~PropertiesDlg();

	void showDialog(const QStringList &paths);

public slots:
	void accept();
	void reject();
	void update();
	void checkboxesChanged();
	void changeIcon();
	void numericChanged(QString);
	void finished();

signals:
	void finishedSignal();
	void updateSignal();

private:
	void init(const QStringList &paths);
	void folderProperties(const QStringList &paths);
	void recurseProperties(const QString &path);
	int result() const { return m_result; }
	void setResult(int value) { m_result = value; }

	YeApplication *m_app;
	QHash<QString,QIcon> &folderIcons;
	QHash<QString,QIcon> &fileIcons;
	int m_result;

	QToolButton *iconButton;
	QLabel *path;
	QLabel *sizeInfo;
	QLabel *containsInfo;
	QLabel *modifiedInfo;
	QLabel *driveInfo;

	QCheckBox * ownerRead;
	QCheckBox * ownerWrite;
	QCheckBox * ownerExec;

	QCheckBox * groupRead;
	QCheckBox * groupWrite;
	QCheckBox * groupExec;

	QCheckBox * otherRead;
	QCheckBox * otherWrite;
	QCheckBox * otherExec;

	QLineEdit * permissionsNumeric;

	QDialogButtonBox *buttons;
	QFuture<void> thread;

	QString pathName;
	QString permString;
	bool iconChanged;

	int type;           // 1=folder, 2=file, 3=multiple
	qint64 files;
	qint64 folders;
	qint64 totalSize;
};

#endif // YE_PROPERTIESDLG_H
