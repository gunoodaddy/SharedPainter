#include "StdAfx.h"
#include "SyncDataProgressDialog.h"

SyncDataProgressDialog::SyncDataProgressDialog(QWidget *parent)
	: QDialog(parent), cancel_(false)
{
	ui.setupUi(this);

	QMovie *movie = new QMovie(":/SharedPainter/Resources/syncprogress.gif"); 
	movie->start();
	ui.labelAnimation->setMovie(movie); 
}

SyncDataProgressDialog::~SyncDataProgressDialog()
{

}


void SyncDataProgressDialog::onCancel( void )
{
	cancel_ = true;
	reject();
}