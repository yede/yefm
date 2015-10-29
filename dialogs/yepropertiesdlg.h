#ifndef YE_PROPERTIESDLG_H
#define YE_PROPERTIESDLG_H

#include <QThread>
#include <QDialog>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QHash>
#include <QIcon>
//==============================================================================================================================

class PropertiesDlg;

class PropertiesDlgThread : public QThread
{
	Q_OBJECT
public:
	PropertiesDlgThread(PropertiesDlg *parent);
	~PropertiesDlgThread();

	void ensureStop();

protected:
	void run();

private:
	PropertiesDlg *m_dlg;
};
//==============================================================================================================================

class PropertiesDlg : public QDialog
{
	Q_OBJECT
public:
	PropertiesDlg(const QStringList &files);
	~PropertiesDlg();

	static void showProperties(const QStringList &files);

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
	friend class PropertiesDlgThread;

	void init();
	void setQuit() { m_quit = true; }

	void showDialog();
	void showProperties();
	void recurseProperties(const QString &path);

	QHash<QString,QIcon> &m_folderIcons;
	QHash<QString,QIcon> &m_fileIcons;
	QStringList           m_fileList;

	PropertiesDlgThread  *m_thread;

	QToolButton *m_iconButton;
	QLabel *m_path;
	QLabel *m_sizeInfo;
	QLabel *m_containsInfo;
	QLabel *m_modifiedInfo;
	QLabel *m_driveInfo;

	QCheckBox * m_ownerRead;
	QCheckBox * m_ownerWrite;
	QCheckBox * m_ownerExec;

	QCheckBox * m_groupRead;
	QCheckBox * m_groupWrite;
	QCheckBox * m_groupExec;

	QCheckBox * m_otherRead;
	QCheckBox * m_otherWrite;
	QCheckBox * m_otherExec;

	QLineEdit * m_permissionsNumeric;

	QDialogButtonBox *m_buttons;

	QString m_pathName;
	QString m_permString;
	bool m_iconChanged;
	bool m_quit;
	bool dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;

	int m_type;           // 1=folder, 2=file, 3=multiple
	qint64 m_files;
	qint64 m_folders;
	qint64 m_totalSize;
};

#endif // YE_PROPERTIESDLG_H
