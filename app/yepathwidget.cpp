#include <QFileInfo>
#include <QDir>
#include <QSpacerItem>
#include <QDebug>

#include "yepathwidget.h"
#include "yepathbutton.h"
#include "yeapp.h"
#include "yeappcfg.h"
#include "yeiconloader.h"
//==============================================================================================================================

#define PADDING_TOP    1
#define PADDING_BOTTOM 0

PathWidget::PathWidget(int paneIndex, QWidget *parent)
	: QWidget(parent)
	, m_paneIndex(paneIndex)
	, m_pathWithRoot(false)
{
	QSpacerItem *spItem = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_host = new QWidget(this);

	m_box = new QHBoxLayout(m_host);
	m_box->setSpacing(4);
	m_box->setContentsMargins(4, PADDING_TOP, 4, PADDING_BOTTOM);
	m_box->addSpacerItem(spItem);

//	setPalette(QPalette(QColor(192, 192, 255, 192)));
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	updateIconTheme();
	connect(App::app(), SIGNAL(iconThemeChanged()), this, SLOT(onIconThemeChanged()));
}

PathWidget::~PathWidget()
{
}
//==============================================================================================================================

int PathWidget::buttonHeight()
{
	return m_buttonHeight;
}

void PathWidget::updateIconTheme()
{
	m_buttonHeight = AppCfg::instance()->iconSize + 6;
	int hostHeight = m_buttonHeight + PADDING_TOP + PADDING_BOTTOM;
	m_host->setFixedSize(9999, hostHeight);
	setFixedHeight(hostHeight);
}

void PathWidget::onIconThemeChanged()
{
	updateIconTheme();

	foreach (PathButton *button, m_buttons) {
		button->updateIconTheme();
	}
}
//==============================================================================================================================

void PathWidget::clear()
{
	foreach (PathButton *button, m_buttons) {
		m_box->removeWidget(button);
		delete button;
	}

	m_buttons.clear();
}

void PathWidget::showButtons()
{
	int cnt = m_buttons.size();

	for (int i = 0; i < cnt; i++) {
		m_buttons.at(i)->show();
	}
}

bool PathWidget::showDirs(const QStringList &dirs)
{
	int dirCnt = dirs.size();
	int btnCnt = m_buttons.size();
	if (dirCnt > btnCnt) return false;

	for (int i = 0; i < dirCnt; i++) {
		if (dirs.at(i) != m_buttons.at(i)->dir()) return false;
	}

	int actPos = dirCnt - 1;
	for (int i = 0; i < btnCnt; i++) {
		PathButton *bt = m_buttons.at(i);
		bt->setActive(i == actPos);
	}

	return true;
}

void PathWidget::showPath(const QString &path)
{
	QChar sep = QChar('/');

	QFileInfo info(path);
	if (!info.exists() || !info.isDir()) {
		clear();
		return;
	}

	m_pathWithRoot = path.startsWith(sep);
	QStringList dirs;

	dirs = path.split(sep);
	dirs.removeAll("");
	dirs.prepend(sep);
//	qDebug() << "PathWidget::showPath" << dirs;

	if (showDirs(dirs)) {
		return;
	}

	clear();

	int i = dirs.size();
	int activePos = i - 1;
	while (i > 0) {
		i--;
		PathButton *button = new PathButton(this, dirs.at(i), i == activePos);
		m_buttons.insert(0, button);
		m_box->insertWidget(0, button);
	}

	showButtons();
}

void PathWidget::changeWorkPath(PathButton *button)
{
	int cnt = m_buttons.size();
	if (cnt < 1) return;
	if (m_buttons.last() == button && button->isActive()) return;

	bool ok = false;
	QString path;
	QChar sep = QChar('/');

	for (int i = 0; i < cnt; i++) {
		PathButton *bt = m_buttons.at(i);
		if (i > 1) path.append(sep);
		path.append(bt->dir());
		if (button == bt) {
			ok = true;
			break;
		}
	}
//	qDebug() << "PathWidget::setWorkPath" << path;

	if (ok) {
		emit setWorkPath(path);
	}
}
