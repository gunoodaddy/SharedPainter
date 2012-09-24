#ifndef SYNCDATAPROGRESSDIALOG_H
#define SYNCDATAPROGRESSDIALOG_H

#include <QDialog>
#include "ui_SyncDataProgressDialog.h"

class SyncDataProgressDialog : public QDialog
{
	Q_OBJECT

public:
	SyncDataProgressDialog(QWidget *parent = 0);
	~SyncDataProgressDialog();

	bool isCanceled( void ) { return cancel_; }
	void setCanceled( void )
	{
		cancel_ = true;
		reject();
	}

private slots:
	void onCancel( void );

private:
	Ui::SyncDataProgressDialog ui;
	bool cancel_;
};

#endif // SYNCDATAPROGRESSDIALOG_H
