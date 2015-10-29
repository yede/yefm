#ifndef YE_CUSTOM_ACTIONS_DLG_H
#define YE_CUSTOM_ACTIONS_DLG_H

#include <QDialog>
#include <QListWidgetItem>
//==============================================================================================================================

namespace Ui {
class CustomActionsDlg;
}

class CustomActionsDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CustomActionsDlg(QWidget *parent = 0);
	~CustomActionsDlg();

	void showDialog();

private:
	void loadItems();
	void saveItems();
	void showItem(QListWidgetItem *item);
	void saveItem(QListWidgetItem *item);
	void setEditEnabled(bool enabled, int kind);

	QIcon searchIcon(int kind, const QString &name, const QString &icon);
	QListWidgetItem *addItem(int kind, const QString &name, const QString &exec, const QString &icon,
							 int match, const QString &types, int pos = -1);

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
	Ui::CustomActionsDlg *ui;
	bool m_loaded;
	bool m_modified;
	bool m_itemModified;
	bool m_dummy_3, m_dummy_4, m_dummy_5, m_dummy_6, m_dummy_7;
};

#endif // YE_CUSTOM_ACTIONS_DLG_H
