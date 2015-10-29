#ifndef PATHWIDGET_H
#define PATHWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
//==============================================================================================================================

class FilePane;
class PathButton;

class PathWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PathWidget(int paneIndex, QWidget *parent = 0);
	~PathWidget();

	void showPath(const QString &path);
	void clear();

	void changeWorkPath(PathButton *button);

	int buttonHeight();

private:
	void updateIconTheme();
	void showButtons();
	bool showDirs(const QStringList &dirs);

signals:
	void setWorkPath(const QString &path);

public slots:
	void onIconThemeChanged();

private:
	int m_paneIndex;
	bool m_pathWithRoot;
	int m_buttonHeight;
	QWidget    *m_host;
	QHBoxLayout *m_box;
	QList<PathButton *> m_buttons;
};

#endif // PATHWIDGET_H
