#include <QSettings>
#include <QDebug>

#include "yestylesettings.h"
#include "ui_yestylesettings.h"

#include "yestyle.h"
#include "yeapp.h"
//==============================================================================================================================

StyleSettings::StyleSettings(QWidget *parent)
	: QWidget(parent, Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
	, ui(new Ui::StyleSettings)
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

void StyleSettings::showDialog()
{
	Style *m = Style::instance();

	ui->edFvCuts->setText(m->fvStyle.cutsColor.name());
	ui->edFvExec->setText(m->fvStyle.execColor.name());
	ui->edFvFile->setText(m->fvStyle.fileColor.name());
	ui->edFvFolder->setText(m->fvStyle.folderColor.name());
	ui->edFvHidden->setText(m->fvStyle.hiddenColor.name());
	ui->edFvSymbol->setText(m->fvStyle.symbolColor.name());
	ui->edFvSpacing->setText(QString::number(m->fvStyle.spacing));
	ui->chkFvStrikeOut->setChecked(m->fvStyle.strikeOutCuts);

	ui->edBmkSepFg->setText(m->bmStyle.sepFgColor.name());
	ui->edBmkSepBg->setText(m->bmStyle.sepBgColor.name());
	ui->edBmkColor->setText(m->bmStyle.itemColor.name());
	ui->edBmkSpacing->setText(QString::number(m->bmStyle.spacing));

	ui->edStatFontName->setText(m->stStyle.fontName);
	ui->edStatFontSize->setText(QString::number(m->stStyle.fontSize));
	ui->edStatFgColor->setText(m->stStyle.fgColor.name());
	ui->edStatBgColor->setText(m->stStyle.bgColor.name());
	ui->chkStatUseWinBg->setChecked(m->stStyle.useWinBg);

	ui->edStatPmsColor->setText(m->stStyle.pmsColor.name());
	ui->edStatUsrColor->setText(m->stStyle.usrColor.name());
	ui->edStatGrpColor->setText(m->stStyle.grpColor.name());
	ui->edStatSizeColor->setText(m->stStyle.sizeColor.name());
	ui->edStatDateColor->setText(m->stStyle.dateColor.name());
	ui->edStatTimeColor->setText(m->stStyle.timeColor.name());
	ui->edStatFileColor->setText(m->stStyle.fileColor.name());

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
