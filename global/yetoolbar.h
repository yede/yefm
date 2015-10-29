#ifndef YE_TOOLBAR_H
#define YE_TOOLBAR_H

#include <QWidget>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QMargins>
//==============================================================================================================================

class ToolItem;
class ToolIcon;
class ToolList;

class ToolBar : public QWidget
{
    Q_OBJECT
public:
	explicit ToolBar(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = 0);
	~ToolBar();

	enum CheckMode {
		cmPressDown,
		cmIndicator
	};

	enum IndicatorEdge {
		ieLeft, ieTop, ieRight, ieBottom
	};

	void setupLayout();
	void setupTailLayout();

	void updateIconSize(int w, int h);
	void updateItemmSize();
	void updateOrientation();
	void updateLayout();

	void addSeparator(int size = 8);
	void addItem(QWidget *item, int stretch, Qt::Alignment alignment);
	void addItem(QWidget *item, int stretch = 0);
	void insertItem(QWidget *item, int pos, int stretch = 0);
	void removeItem(QWidget *item);

	ToolList *addToolList();
	ToolIcon *addToolIcon();
	ToolIcon *addToolIcon(const QIcon &icon);
	ToolIcon *addToolIcon(const QIcon &icon, const QString &tips);
	ToolIcon *addToolIcon(const QIcon &icon, const QObject *receiver, const char *method);
	ToolIcon *addToolIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method);
	ToolIcon *addToolIcon(const QString &tips);
	ToolIcon *addToolIcon(const QString &tips, const QObject *receiver, const char *method);

	void addTailSeparator(int size = 8);
	void addTailItem(QWidget *item, int stretch, Qt::Alignment alignment);
	void addTailItem(QWidget *item, int stretch = 0);
	void insertTailItem(QWidget *item, int pos, int stretch = 0);
	void removeTailItem(QWidget *item);

	ToolIcon *addTailIcon();
	ToolIcon *addTailIcon(const QIcon &icon);
	ToolIcon *addTailIcon(const QIcon &icon, const QString &tips);
	ToolIcon *addTailIcon(const QIcon &icon, const QObject *receiver, const char *method);
	ToolIcon *addTailIcon(const QIcon &icon, const QString &tips, const QObject *receiver, const char *method);
	ToolIcon *addTailIcon(const QString &tips);
	ToolIcon *addTailIcon(const QString &tips, const QObject *receiver, const char *method);

	void setOrientation(Qt::Orientation orientation) { m_orientation = orientation; }
	void setIconSize(int w, int h);
	void setBasePads(int left, int top, int right, int bottom);
	void setItemPads(int left, int top, int right, int bottom);
	void setItemSpacing(int spacing);
	void setHasSpacer(bool flag) { m_hasSpacer = flag; }

	const QSize          &itemSize()     const { return m_itemSize; }

	const QSize          &iconSize()     const { return m_iconSize; }
	const QRect          &iconRect()     const { return m_iconRect; }

	const QSize          &buttonSize()   const { return m_buttonSize; }
	const QRect          &buttonRect()   const { return m_buttonRect; }

	const QMargins       &basePads()     const { return m_basePads; }
	const QMargins       &itemPads()     const { return m_itemPads; }

	QBoxLayout           *itemLayout()   const { return m_itemLayout; }
	QBoxLayout           *tailLayout()   const { return m_tailLayout; }

	bool                  isHorizontal() const { return m_orientation == Qt::Horizontal; }
	Qt::Orientation       orientation()  const { return m_orientation; }
	QBoxLayout::Direction direction()    const { return isHorizontal() ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom; }
	Qt::Alignment         alignment()    const { return isHorizontal() ? Qt::AlignLeft : Qt::AlignTop; }

	void setCheckMode(CheckMode mode)         { m_checkMode = mode; }
	void setIndicatorEdge(IndicatorEdge edge) { m_indicatorEdge = edge; }
	void setIndicatorSide(int size)           { m_indicatorSize = size; }

	CheckMode     checkMode()     const { return m_checkMode; }
	IndicatorEdge indicatorEdge() const { return m_indicatorEdge; }
	int           indicatorSize() const { return m_indicatorSize; }

protected:

private:
	void doAddSeparator(QBoxLayout *box, int size);
	void doInsertItem(QBoxLayout *box, QWidget *item, int pos, int stretch);
	void doRemoveItem(QBoxLayout *box, QWidget *item);

signals:

public slots:

private:
	Qt::Orientation m_orientation;
	QRect           m_buttonRect;
	QSize           m_buttonSize;
	QRect           m_iconRect;
	QSize           m_iconSize;
	QSize           m_itemSize;		// itemSize > buttonSize > iconSize
	QMargins        m_basePads;
	QMargins        m_itemPads;
	int             m_itemSpacing;
	int             m_separatorSize;
	bool            m_hasSpacer;
	QSpacerItem    *m_baseSpacer;
	QBoxLayout     *m_baseLayout;
	QBoxLayout     *m_itemLayout;
	QBoxLayout     *m_tailLayout;
	CheckMode       m_checkMode;
	IndicatorEdge   m_indicatorEdge;
	int             m_indicatorSize;
};

#endif // YE_TOOLBAR_H
