#ifndef MIMESETTINGS_H
#define MIMESETTINGS_H

#include <QWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QStackedLayout>
#include <QLineEdit>
//==============================================================================================================================

class QListWidget;
class QListWidgetItem;

class Mime;
class MimeView;
class MimeItem;

class MimeSettings : public QWidget
{
	Q_OBJECT
public:
	explicit MimeSettings(QWidget *parent = 0);
	~MimeSettings();

	void addDesktopItem(const QString &name);
	void updateIconTheme();
	bool saveMimes();

	bool isLoaded() const;

private:
	QWidget *createToolButton(const QString &icon, const QString &tips, const char *method);
	QWidget *createMimeSettings();
	QWidget *createMimeProgress();
	void updateItemSizeHint(QListWidgetItem *item);

signals:
	void iconThemeChanged();

public slots:
	void loadMimes();
	void updateMimeAssoc(MimeItem* item);
	void onMimeSelected(MimeItem* current, MimeItem* previous);
	void removeAppAssoc();
	void moveAppAssocUp();
	void moveAppAssocDown();

private:
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
//==============================================================================================================================

class MimeFilterEdit : public QLineEdit
{
	Q_OBJECT
public:
	MimeFilterEdit(MimeView *view, QWidget *parent = 0);

public slots:
	void updateMimeFilter();

protected:
	void keyPressEvent(QKeyEvent *event);

private:
	MimeView *m_view;
};

#endif // MIMESETTINGS_H
