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
class QFileInfo;

class FsWidget;
class FsModel;
class FsSortModel;

class FsHandler : public QObject
{
	Q_OBJECT
public:
	explicit FsHandler(FsWidget *widget, QObject *parent = 0);
	~FsHandler();

	void lateStart();
	void stopHoverSelect();

	bool handleKeyPress(QKeyEvent *event);
	void showContextMenu(QContextMenuEvent *event);
	void showStatusMessage(const QModelIndex &index);
	void showStatusMessage(const QFileInfo &info);

private:
	void execHoverSelect(const QString &path);
	void startHoverSelect(const QModelIndex &sorted);

signals:

public slots:
	void onTimeout();
	void onItemHovered(const QModelIndex &sorted);
	void onItemActivated(const QModelIndex &sorted);

private:
	FsWidget *m_widget;
	bool      m_pressed;

	FsModel             *m_model;
	FsSortModel         *m_sortModel;
	QItemSelectionModel *m_seleModel;

	QTimer m_timer;
	QString m_timerFile;

	QModelIndex m_hoverIndex;
};

#endif // FSHANDLER_H
