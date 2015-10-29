#ifndef YE_HISTORY_MODEL_H
#define YE_HISTORY_MODEL_H

#include <QColor>
#include "yetreemodel.h"
//==============================================================================================================================

namespace HisModelAction {
	enum {
		None,
		Activate,
		Remove		// Directly, without confirmation
	};
}

class Style;

class HistoryModel : public TreeModel
{
	Q_OBJECT
public:
	explicit HistoryModel(QObject *parent = 0);
	~HistoryModel();

	void lateStart();

	void save();
	void load();

	void addPath(const QString &path);
	void startTemporarilySkip();

	QString getStatusMessage(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;

	void showContextMenu(UserView *view, QContextMenuEvent *event);
	bool handleKeyPress(UserView *view, QKeyEvent *event);

	static HistoryModel *instance() { return m_instance; }
	static void createInstance()    { if (!m_instance) m_instance = new HistoryModel; }
	static void deleteInstance()    { delete m_instance; m_instance = NULL; }
	static void setupShortcuts();

private:
	TreeNode *insertNode(const QString &title, const QString &path, int pos = -1);

	void removeSelection(UserView *view);
	void removeNode(TreeNode *node);

signals:

public slots:
	void updateIconTheme();
	void onRemove();
	void onClear();
	void onTimeout();

private:
	Style    *m_style;
	TreeNode *m_contextNode;
	QAction  *m_actRemove;
	QAction  *m_actClear;
	bool      m_skip, dummy_1, dummy_2, dummy_3, dummy_4, dummy_5, dummy_6, dummy_7;
	QTimer   *m_timer;

	static HistoryModel *m_instance;
};

#endif // YE_HISTORY_MODEL_H
