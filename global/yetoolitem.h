#ifndef YE_TOOLITEM_H
#define YE_TOOLITEM_H

#include <QWidget>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QHash>
#include <QList>
//==============================================================================================================================

class ToolBar;

class ToolItem : public QWidget
{
    Q_OBJECT
public:
	explicit ToolItem(QWidget *parent = 0);
	explicit ToolItem(ToolBar *toolBar);

	void setDefaultSize(QSize size)   { m_defaultSize = size; }
	void setDefaultSize(int w, int h) { m_defaultSize = QSize(w, h); }
	QSize sizeHint() const;

	QAction *addMenuItem(const QIcon &icon, const QString &title, const QObject *receiver, const char *method);
	QAction *addMenuItem(const QString &title, const QObject *receiver, const char *method);
	QAction *insertMenuItem(QAction *before, const QString &title, const QObject *receiver, const char *method);
	void addMenuSeparator();

	QMenu *menu() const { return m_menu; }
	QMenu *createMenu();
	void setMenu(QMenu *menu);

	void setToolBar(ToolBar *toolbar) { m_toolBar = toolbar; }
	void setAutoCheck(bool flag)      { m_autoCheck = flag; }
	void setChecked(bool flag)        { m_checked = flag; }

	ToolBar *toolBar() const { return m_toolBar; }
	bool isChecked() const { return m_checked; }

private:
	void init();

protected:
	void paintEvent(QPaintEvent *event);
//	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

	void showMenu(QMenu *menu);

signals:
	void clicked();

public slots:
	void aboutToHide();
	void onTimeout();

protected:
	ToolBar *m_toolBar;
	QTimer  *m_timer;
	QMenu   *m_menu;
	QSize    m_defaultSize;
	bool     m_autoCheck, m_checked;
	bool     m_hovering, m_pressed, m_popup;
};
//==============================================================================================================================

class ToolIcon : public ToolItem
{
	Q_OBJECT
public:
	explicit ToolIcon(ToolBar *parent);
	explicit ToolIcon(const QIcon &icon, ToolBar *parent);

	void setIcon(const QIcon &icon);
	void updateContent();

protected:
	void paintEvent(QPaintEvent *event);

private:
	QPixmap m_pixmap;
	QRect m_iconRect;
};
//==============================================================================================================================

namespace ToolListInsertModus {
	enum { AfterCurrent = 0,
		   GroupTailing,
		   LastPosition };
}

struct ToolListItemData;

class ToolList : public ToolItem
{
	Q_OBJECT
public:
	explicit ToolList(ToolBar *parent);
	~ToolList();

	int currentItemIndex() const { return m_currentItemIndex; }
	void *currentGroupKey() const;
	void *currentItemKey() const;
	void setCurrentItem(void *itemKey);
	void setCurrentItem(int index);
	void setInsertModus(int modus);

	QAction *addItem(void *itemKey, const QString &label, void *groupKey = 0);
	void setItemLabel(void *itemKey, const QString &label);
	void removeItem(void *itemKey);

	void addGroup(void *groupKey, const QString &label);
	void setGroupLabel(void *groupKey, const QString &label);
	void removeGroup(void *groupKey);

	void clear();

protected:
	void paintEvent(QPaintEvent *event);

private:
	ToolListItemData *currentItem() const;
	ToolListItemData *findItem(int &retIndex, void *itemKey);
	ToolListItemData *findActionItem(int &retIndex, QAction *action);
	int getGroupTail(void *groupKey);
	void deleteItem(ToolListItemData *item);
	QString getGroupLabel(void *key);

signals:
	void itemClicked(int itemIndex, void *itemKey, void *groupKey);

public slots:
	void onItemClicked();

private:
	int m_currentItemIndex;
	int m_modus;
	QHash<void *, QString>    m_groups;
	QList<ToolListItemData *> m_items;
};

#endif // YE_TOOLITEM_H
