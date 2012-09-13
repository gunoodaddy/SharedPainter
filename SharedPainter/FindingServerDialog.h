#ifndef FINDINGSERVERDIALOG_H
#define FINDINGSERVERDIALOG_H

#include <QDialog>
#include "ui_FindingServerDialog.h"

class FindingServerDialog : public QDialog
{
	Q_OBJECT

public:
	FindingServerDialog(QWidget *parent = 0);
	~FindingServerDialog();

	bool isCanceled( void ) { return cancel_; }
	void setCanceled( void )
	{
		cancel_ = true;
		reject();
	}

	void setRemainCount( int cnt )
	{
		QString msg;
		msg = tr("Remain count : ");
		msg += QString::number(cnt);
		ui.labelSentCount->setText( msg );
	}

private slots:
	void onCancel( void );

private:
	Ui::FindingServerDialog ui;
	bool cancel_;
};

#endif // FINDINGSERVERDIALOG_H
