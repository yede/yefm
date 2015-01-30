#include <QGridLayout>
#include <QPushButton>
#include <QTimer>

#include "yeprogressdlg.h"
//==============================================================================================================================

ProgressDlg::ProgressDlg()
{
	filename = new QLabel("Initializing...");
	bar = new QProgressBar();
	button = new QPushButton("Cancel");

	//transfer info label
	transferInfo = new QLabel();
	transferInfo->setText(QString("<p><br></p>"));

	//remaining time timer
	remainingTimer = new QTime();
	oldSeconds = 0;

	runningTotal = 0;
	bar->setRange(0, 100);

	filename->setFixedWidth(300);
	bar->setFixedWidth(300);
	bar->setAlignment(Qt::AlignHCenter);
	connect(button, SIGNAL(clicked()), this, SLOT(accept()));

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(filename, 1, 1, 1, 4);
	layout->addWidget(bar, 2, 1, 1, 4);
	layout->addWidget(transferInfo, 3, 1, 1, 1, Qt::AlignBottom);
	layout->addWidget(button, 3, 4, 1, 1, Qt::AlignBottom);
	setLayout(layout);

	//start the timer
	remainingTimer->start();

	QTimer::singleShot(1000, this, SLOT(setShowing()));
}
//==============================================================================================================================

void ProgressDlg::setTitle(const QString &title)
{
	setWindowTitle(title);
}

void ProgressDlg::setShowing()
{
	if (bar->value() < 70) { open(); }
    return;
}

void ProgressDlg::update(qint64 bytes, qint64 total, QString name)
{
    //set file name
    filename->setText(name);

    //refresh the bar
    runningTotal += bytes;
    bar->setValue(runningTotal * 100 / total);

    //transfer info
    int currentSeconds = remainingTimer->elapsed() / 1000;  //convert to seconds

	if (currentSeconds != oldSeconds) {                     //enter here every second
		float cumulativeTransferRate = runningTotal / currentSeconds;
        float cumulativeTransferRateMB = cumulativeTransferRate / 1000000; //convert to megabytes
        int currentSecondsRemaining = (total - runningTotal) / cumulativeTransferRate;

        QString formattedTime;

		if (currentSecondsRemaining < 60) { formattedTime = QString("%1 seconds").arg(currentSecondsRemaining); }
		else { formattedTime = QString("%1 min %2 sec").arg(currentSecondsRemaining / 60).arg(currentSecondsRemaining % 60); }

        transferInfo->setText(QString("<p>Transfer rate: %2 MB/s<br>Time remaining: %3</p>")
							  .arg(cumulativeTransferRateMB, 0, 'f', 1).arg(formattedTime));

        oldSeconds = currentSeconds;
    }

    return;
}
