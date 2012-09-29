#include "JoinerListWindow.h"
#include "ui_JoinerListWindow.h"

JoinerListWindow::JoinerListWindow( USER_LIST list, QWidget *parent ) : list_(list),
	QDialog(parent),
	ui(new Ui::JoinerListWindow)
{
	ui->setupUi(this);

	for( size_t i = 0 ; i < list.size(); i++ )
	{
		ui->listJoiner->addItem( Util::toStringFromUtf8(list[i]->nickName()) );
	}
}

JoinerListWindow::~JoinerListWindow()
{
	delete ui;
}
