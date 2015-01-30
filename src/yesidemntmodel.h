#ifndef YE_SIDEMNTMODEL_H
#define YE_SIDEMNTMODEL_H

#include <QTimer>
#include "yetreemodel.h"
//==============================================================================================================================

class QSocketNotifier;

class YeApplication;
class YeFilePane;
class UserView;

class Mount;
class MountItems;

class SideMntModel : public TreeModel
{
	Q_OBJECT
public:
	explicit SideMntModel(YeApplication *app, QObject *parent = 0);
	~SideMntModel();

	void startReadMounts();

	QString getStatusMessage(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;

private:
	TreeNode *insertNode(const QString &title, const QString &path, int pos = -1);

signals:

public slots:
	void onIconThemeChanged();
	void onTimeout();
	void showMounts(const MountItems &mounts);

private:
	YeApplication *m_app;
	YeFilePane *m_pane;
	Mount *m_mnt;

	QTimer m_timer;
};

#endif // YE_SIDEMNTMODEL_H
