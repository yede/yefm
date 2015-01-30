#ifndef FSHANDLER_H
#define FSHANDLER_H

#include <QObject>
#include <QTimer>
#include <QModelIndex>
//==============================================================================================================================

class QKeyEvent;
class QMouseEvent;
class QContextMenuEvent;
class QItemSelectionModel;

class YeApplication;
class YeFilePane;
class FsWidget;
class FsModel;
class FsListModel;

class FsHandler : public QObject
{
	Q_OBJECT
public:
	explicit FsHandler(FsWidget *widget, QObject *parent = 0);
	~FsHandler();

	void lateStart();
	void stopHoverSelect();

	void handleKeyPress(QKeyEvent *event);
	void showContextMenu(QContextMenuEvent *event);
	void showStatusMessage(const QModelIndex &index);

private:
	void execHoverSelect(const QString &path);
	void startHoverSelect(const QModelIndex &sorted);

	void dispatchKeyAction(int action);

public:
	struct KeyAction {
		enum {
			None,
			NewFile,
			NewFolder,
			Run,
			Cut,
			Copy,
			Paste,
			Delete,
			Rename,
			Properties
		};
	};

signals:

public slots:
	void onTimeout();
	void onItemHovered(const QModelIndex &sorted);
	void onItemActivated(const QModelIndex &sorted);

private:
	YeApplication *m_app;
	FsWidget      *m_widget;
	bool           m_pressed;

	FsModel             *m_model;
	FsListModel         *m_sortModel;
	QItemSelectionModel *m_seleModel;

	QTimer m_timer;
	QString m_timerFile;

	QModelIndex m_hoverIndex;
};

#endif // FSHANDLER_H
