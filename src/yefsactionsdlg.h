#ifndef FSACTIONSDLG_H
#define FSACTIONSDLG_H

#include <QDialog>
#include <QListWidgetItem>
//==============================================================================================================================

namespace Ui {
class FsActionsDlg;
}

class YeApplication;

class FsActionsDlg : public QDialog
{
	Q_OBJECT

public:
	explicit FsActionsDlg(YeApplication *app, QWidget *parent = 0);
	~FsActionsDlg();

	void showDialog();

	static void loadCustomActions(QStringList &result);
	static void saveCustomActions(const QStringList &items);

	static void decodeCustomTypes(QStringList &result, QString &types);
	static bool decodeCustomItem(int &kind, QString &name, QString &exec, QString &icon,
								 int &match, QString &types, const QString &item);
	static void encodeCustomItem(QString &item, int kind, const QString &name, const QString &exec, const QString &icon,
								 int match, const QString &types);

	static bool matchDir(int match);
	static bool matchFile(const QStringList &types, const QString &ext);
	static bool matchAny(int match);
	static bool matchAnyFile(int match);

	static bool isDesktopApp(int kind);

private:
	void loadItems();
	void saveItems();
	void showItem(QListWidgetItem *item);
	void saveItem(QListWidgetItem *item);
	void setEditEnabled(bool enabled, int kind);

	QIcon searchIcon(int kind, const QString &name, const QString &icon);
	QListWidgetItem *addItem(int kind, const QString &name, const QString &exec, const QString &icon,
							 int match, const QString &types, int pos = -1);

	static int  decodeKind(QString &name);
	static int  decodeMatch(QString &types);
	static void insertTypeTag(QString &types, int match);

public slots:
	void accept();
	void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
	void onAdd();
	void onDel();
	void onMoveUp();
	void onMoveDown();
	void onTextModified(const QString &text);
	void onRadioModified(bool checked);
	void onKindChanged(bool checked);

private:
	Ui::FsActionsDlg *ui;
	YeApplication    *m_app;
	bool m_loaded;
	bool m_modified;
	bool m_itemModified;
};

#endif // FSACTIONSDLG_H
