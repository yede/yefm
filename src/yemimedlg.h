#ifndef YE_MIMEAPPSELDLG_H
#define YE_MIMEAPPSELDLG_H

#include <QThread>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QLabel>
#include "yedesktopfile.h"
//==============================================================================================================================

class YeApplication;
class MimeSettings;
class Splitter;

typedef void (*MimeThreadFunc) (void *arg);
//==============================================================================================================================

class MimeThread: public QThread
{
	Q_OBJECT
public:
	MimeThread(QObject *parent = NULL);

	void startLoad(MimeThreadFunc func, void *arg);

protected:
	void run();

signals:
	void loadFinished();

private:
	MimeThreadFunc m_func;
	void          *m_arg;
};
//==============================================================================================================================

class MimeDlg : public QDialog
{
	Q_OBJECT
public:
	explicit MimeDlg(YeApplication *app, QWidget *parent = 0);

	bool showDialog(QString &resultApp, const QString &mimeType);
	void showDialog();

protected:
	void closeEvent(QCloseEvent *event);
	void showEvent(QShowEvent *event);

private:
	QWidget *createAppTree();
	void createCategories();

public slots:
	void accept();

protected slots:
	void updateCommand(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void onAppItemDoubleClicked(QTreeWidgetItem *item, int col);
	void initCategory(QTreeWidgetItem *category);

protected:
	YeApplication    *m_app;
	int               m_mode;
	int               m_sideWidth;
	int               m_width, m_height;
	bool              m_loaded;
	bool              m_busy;
	MimeSettings     *m_mimeSettings;
	QWidget          *m_sideWidget;
	Splitter         *m_splitter;
	QTreeWidget      *m_sideTree;
	QDialogButtonBox *m_buttons;
	QLabel           *m_guideLabel;
	QLineEdit        *m_edSelApp;
	QLabel           *m_titleLabel;
	QMap<QString, QStringList>      m_catNames;
	QMap<QString, QTreeWidgetItem*> m_categories;
	QMap<QString, QTreeWidgetItem*> m_applications;
	QTreeWidgetItem *findCategory(const DesktopFile &app);

private slots:
	void onLoadFinished();
	void loadItems();

private:
	void loadCategories();
	static void threadFunc(void *arg);

	MimeThread m_thread;
};
//==============================================================================================================================

#endif // YE_MIMEAPPSELDLG_H
