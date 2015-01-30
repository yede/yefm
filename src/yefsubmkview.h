#ifndef YE_FSUBMKVIEW_H
#define YE_FSUBMKVIEW_H

#include "yeuserview.h"
//==============================================================================================================================

namespace UbmkDragKind {
	enum { None = 0, Bookmark, FilePath };
}

class YeFilePane;

class FsUbmkView : public UserView
{
	Q_OBJECT
public:
	explicit FsUbmkView(YeFilePane *pane, QWidget *parent = 0);
	~FsUbmkView();

protected:
	void doDragEnter(QDragEnterEvent *event);
	void doDragMove(QDragMoveEvent *event);
	void doDrop(QDropEvent *event);

	int indicatorArea(const QModelIndex &index);

private:

signals:

public slots:

private:
};

#endif // YE_FSUBMKVIEW_H
