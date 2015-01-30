#ifndef PATHWIDGET_H
#define PATHWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
//==============================================================================================================================

class YeFilePane;
class PathButton;

class PathWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PathWidget(YeFilePane *pane, QWidget *parent = 0);
	~PathWidget();

	void showPath(const QString &path);
	void clear();

	void setWorkPath(PathButton *button);

	int buttonHeight();

private:
	void updateIconTheme();
	void showButtons();
	bool showDirs(const QStringList &dirs);

signals:

public slots:
	void onIconThemeChanged();

private:
	YeFilePane *m_pane;
	bool m_pathWithRoot;
	int m_buttonHeight;
	QWidget    *m_host;
	QHBoxLayout *m_box;
	QList<PathButton *> m_buttons;
};

#endif // PATHWIDGET_H
