#include <QSettings>
#include <QDebug>

#include "yestylesettings.h"
#include "ui_yestylesettings.h"
#include "yeapp.h"
#include "yeappdata.h"
#include "yeappresources.h"
#include "yeapplication.h"
//==============================================================================================================================

StyleSettings::StyleSettings(YeApplication *app, QWidget *parent)
	: QWidget(parent, Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
	, ui(new Ui::StyleSettings)
	, m_app(app)
{
	ui->setupUi(this);

	ui->stack->setCurrentIndex(0);
	ui->listWidget->setCurrentRow(0);
	ui->listWidget->setSpacing(1);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onReject()));
	connect(ui->btnTest, SIGNAL(clicked()), this, SLOT(onTest()));
	connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(onPageChanged(int)));
}

StyleSettings::~StyleSettings()
{
	delete ui;
}
//==============================================================================================================================

void StyleSettings::loadSettings()
{
	QSettings s(App::getStyleFile(), QSettings::IniFormat);
	FmStyle &m = *R::app()->fmStyle();

	s.beginGroup("file-view");
	m.fvStyle.fileColor     = QColor(s.value("fileColor"  , "#404040").toString());
	m.fvStyle.folderColor   = QColor(s.value("folderColor", "#404080").toString());
	m.fvStyle.symbolColor   = QColor(s.value("symbolColor", "#40a0a0").toString());
	m.fvStyle.execColor     = QColor(s.value("execColor"  , "#00a000").toString());
	m.fvStyle.hiddenColor   = QColor(s.value("hiddenColor", "#80a080").toString());
	m.fvStyle.cutsColor     = QColor(s.value("cutsColor"  , "#a08080").toString());
	m.fvStyle.strikeOutCuts = s.value("strikeOutCuts", true).toBool();
	m.fvStyle.spacing       = s.value("spacing", 2).toInt();
	s.endGroup();

	s.beginGroup("bookmark-list");
	m.bmStyle.sepFgColor = QColor(s.value("sepFgColor", "#80a0a0").toString());
	m.bmStyle.sepBgColor = QColor(s.value("sepBgColor", "#f0f0f8").toString());
	m.bmStyle.itemColor  = QColor(s.value("itemColor" , "#404040").toString());
	m.bmStyle.spacing    = s.value("spacing", 2).toInt();
	s.endGroup();

	s.beginGroup("status-bar");
	m.stStyle.fontName  = s.value("fontName", "Monospace").toString();
	m.stStyle.fontSize  = s.value("fontSize", 12).toInt();
	m.stStyle.fgColor   = QColor(s.value("fgColor",   "#404040").toString());
	m.stStyle.bgColor   = QColor(s.value("bgColor",   "#e8e8e8").toString());
	m.stStyle.useWinBg  = s.value("useWinBg", true).toBool();
	m.stStyle.pmsColor  = QColor(s.value("pmsColor" , "#800080").toString());
	m.stStyle.usrColor  = QColor(s.value("usrColor" , "#00a000").toString());
	m.stStyle.grpColor  = QColor(s.value("grpColor" , "#808000").toString());
	m.stStyle.sizeColor = QColor(s.value("sizeColor", "#0080a0").toString());
	m.stStyle.dateColor = QColor(s.value("dateColor", "#c08000").toString());
	m.stStyle.timeColor = QColor(s.value("timeColor", "#00a080").toString());
	m.stStyle.fileColor = QColor(s.value("fileColor", "#404040").toString());
	s.endGroup();
}

void StyleSettings::saveSettings()
{
	QSettings s(App::getStyleFile(), QSettings::IniFormat);
	FmStyle &m = *R::app()->fmStyle();

	s.beginGroup("file-view");
	s.setValue("fileColor",     m.fvStyle.fileColor.name());
	s.setValue("folderColor",   m.fvStyle.folderColor.name());
	s.setValue("symbolColor",   m.fvStyle.symbolColor.name());
	s.setValue("execColor",     m.fvStyle.execColor.name());
	s.setValue("hiddenColor",   m.fvStyle.hiddenColor.name());
	s.setValue("cutsColor",     m.fvStyle.cutsColor.name());
	s.setValue("strikeOutCuts", m.fvStyle.strikeOutCuts);
	s.setValue("spacing",       m.fvStyle.spacing);
	s.endGroup();

	s.beginGroup("bookmark-list");
	s.setValue("sepFgColor", m.bmStyle.sepFgColor.name());
	s.setValue("sepBgColor", m.bmStyle.sepBgColor.name());
	s.setValue("itemColor",  m.bmStyle.itemColor.name());
	s.setValue("spacing",    m.bmStyle.spacing);
	s.endGroup();

	s.beginGroup("status-bar");
	s.setValue("fontName",  m.stStyle.fontName);
	s.setValue("fontSize",  m.stStyle.fontSize);
	s.setValue("fgColor",   m.stStyle.fgColor.name());
	s.setValue("bgColor",   m.stStyle.bgColor.name());
	s.setValue("useWinBg",  m.stStyle.useWinBg);
	s.setValue("pmsColor",  m.stStyle.pmsColor.name());
	s.setValue("usrColor",  m.stStyle.usrColor.name());
	s.setValue("grpColor",  m.stStyle.grpColor.name());
	s.setValue("sizeColor", m.stStyle.sizeColor.name());
	s.setValue("dateColor", m.stStyle.dateColor.name());
	s.setValue("timeColor", m.stStyle.timeColor.name());
	s.setValue("fileColor", m.stStyle.fileColor.name());
	s.endGroup();
}
//==============================================================================================================================

void StyleSettings::showDialog()
{
	FmStyle &m = *R::app()->fmStyle();

	ui->edFvCuts->setText(m.fvStyle.cutsColor.name());
	ui->edFvExec->setText(m.fvStyle.execColor.name());
	ui->edFvFile->setText(m.fvStyle.fileColor.name());
	ui->edFvFolder->setText(m.fvStyle.folderColor.name());
	ui->edFvHidden->setText(m.fvStyle.hiddenColor.name());
	ui->edFvSymbol->setText(m.fvStyle.symbolColor.name());
	ui->edFvSpacing->setText(QString::number(m.fvStyle.spacing));
	ui->chkFvStrikeOut->setChecked(m.fvStyle.strikeOutCuts);

	ui->edBmkSepFg->setText(m.bmStyle.sepFgColor.name());
	ui->edBmkSepBg->setText(m.bmStyle.sepBgColor.name());
	ui->edBmkColor->setText(m.bmStyle.itemColor.name());
	ui->edBmkSpacing->setText(QString::number(m.bmStyle.spacing));

	ui->edStatFontName->setText(m.stStyle.fontName);
	ui->edStatFontSize->setText(QString::number(m.stStyle.fontSize));
	ui->edStatFgColor->setText(m.stStyle.fgColor.name());
	ui->edStatBgColor->setText(m.stStyle.bgColor.name());
	ui->chkStatUseWinBg->setChecked(m.stStyle.useWinBg);

	ui->edStatPmsColor->setText(m.stStyle.pmsColor.name());
	ui->edStatUsrColor->setText(m.stStyle.usrColor.name());
	ui->edStatGrpColor->setText(m.stStyle.grpColor.name());
	ui->edStatSizeColor->setText(m.stStyle.sizeColor.name());
	ui->edStatDateColor->setText(m.stStyle.dateColor.name());
	ui->edStatTimeColor->setText(m.stStyle.timeColor.name());
	ui->edStatFileColor->setText(m.stStyle.fileColor.name());

	show();
}

void StyleSettings::onTest()
{

}

void StyleSettings::onAccept()
{
	hide();
}

void StyleSettings::onReject()
{
	hide();
}
//==============================================================================================================================

void StyleSettings::onPageChanged(int index)
{
	ui->stack->setCurrentIndex(index);
}
