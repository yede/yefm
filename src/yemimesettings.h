#ifndef MIMESETTINGS_H
#define MIMESETTINGS_H

#include <QWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QStackedLayout>
//==============================================================================================================================

class QLineEdit;
class QListWidget;

class YeApplication;
class Mime;
class MimeView;
class MimeItem;

class MimeSettings : public QWidget
{
	Q_OBJECT
public:
	explicit MimeSettings(YeApplication *app, QWidget *parent = 0);
	~MimeSettings();

	void addDesktopItem(const QString &name);
	bool saveMimes();

	bool isLoaded() const;

private:
	QWidget *createToolButton(const QString &icon, const QString &tips, const char *method);
	QWidget *createMimeSettings();
	QWidget *createMimeProgress();

signals:

public slots:
	void loadMimes();
	void updateMimeAssoc(MimeItem* item);
	void onMimeSelected(MimeItem* current, MimeItem* previous);
	void removeAppAssoc();
	void moveAppAssocUp();
	void moveAppAssocDown();

private:
	YeApplication  *m_app;
	Mime           *m_mime;
	MimeView       *m_mimeView;
	QWidget        *m_progress;
	QStackedLayout *m_stack;
	QLineEdit      *m_edFilter;
	QWidget        *m_appAssoc;
	QListWidget    *m_appList;
	QProgressBar   *m_progressMime;

public:
	static void threadFunc(void *arg);
};

#endif // MIMESETTINGS_H
