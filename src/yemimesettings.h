#ifndef MIMESETTINGS_H
#define MIMESETTINGS_H

#include <QWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QStackedLayout>
//==============================================================================================================================

class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;
class QListWidgetItem;

class YeApplication;
class Mime;

class MimeSettings : public QWidget
{
	Q_OBJECT
public:
	explicit MimeSettings(YeApplication *app, QWidget *parent = 0);
	~MimeSettings();

	void addDesktopItem(const QString &name);
	bool save();

	bool isLoaded() const { return m_loaded; }
	QTreeWidget *mimeTree() const { return m_mimeWidget; }

private:
	QWidget *createMimeSettings();
	QWidget *createMimeProgress();
	void loadAppIcons();

signals:

public slots:
	void loadMimes();
	void updateMimeAssoc(QTreeWidgetItem* item);
	void onMimeSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void removeAppAssoc();
	void moveAppAssocUp();
	void moveAppAssocDown();

private:
	YeApplication  *m_app;
	Mime           *m_mime;
	bool            m_loaded;
	QTreeWidget    *m_mimeWidget;
	QWidget        *m_progress;
	QStackedLayout *m_stack;
	QWidget        *m_appAssoc;
	QListWidget    *m_appList;
	QProgressBar   *m_progressMime;

public:
	static void threadFunc(void *arg);
};

#endif // MIMESETTINGS_H
