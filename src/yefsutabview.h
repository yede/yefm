#ifndef YE_FSUTABVIEW_H
#define YE_FSUTABVIEW_H

#include "yeuserview.h"
//==============================================================================================================================

class YeFilePane;

class FsUtabView : public UserView
{
	Q_OBJECT
public:
	explicit FsUtabView(YeFilePane *pane, QWidget *parent = 0);
	~FsUtabView();

protected:

private:

signals:

public slots:

private:
};

#endif // YE_FSUTABVIEW_H
