#ifndef YE_PASTEMGR_H
#define YE_PASTEMGR_H

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QMimeData>
#include <QStackedLayout>
//==============================================================================================================================

namespace PasteType {
	enum { Copy, Move, CopyStructure };
}

class PasteTask;
class PasteItem;
class ToolBar;
//==============================================================================================================================

class PasteMgr : public QDialog
{
    Q_OBJECT
public:
	explicit PasteMgr(QWidget *parent = 0);
	~PasteMgr();

	static PasteMgr *instance() { return m_instance; }
	static void launcher(const QMimeData *srcData, const QString &destFolder, int type, QWidget *view);

	void addTask(PasteTask *task);
	void removeTask(PasteTask *task);

	void setCurrentTask(PasteTask *task);
	void showPad();

	const QColor &lineColor() const { return m_lineColor; }
	bool isBusy() const;

protected:
	void closeEvent(QCloseEvent *event);

private:
	static bool isValidPaste(const QStringList &srcFiles, const QString &destFolder);
	static bool isDuplicate(const QString &destFolder, const QStringList &srcFiles);

signals:
	void iconThemeChanged();

public slots:
	void onHelp();
	void hidePad();

private:
	ToolBar        *m_toolbar;
	QStackedLayout *m_stack;

	QTimer  m_timer;
	QColor  m_lineColor;

	static PasteMgr *m_instance;
};
//==============================================================================================================================

#endif // YE_PASTEMGR_H
