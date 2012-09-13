#include "StdAfx.h"
#include "FindingServerDialog.h"

FindingServerDialog::FindingServerDialog(QWidget *parent)
	: QDialog(parent), cancel_(false)
{
	ui.setupUi(this);

	QMovie *movie = new QMovie(":/SharedPainter/Resources/findingserver.gif"); 
	movie->start();
	ui.labelAnimation->setMovie(movie); 
}

FindingServerDialog::~FindingServerDialog()
{

}


void FindingServerDialog::onCancel( void )
{
	cancel_ = true;
	reject();
}