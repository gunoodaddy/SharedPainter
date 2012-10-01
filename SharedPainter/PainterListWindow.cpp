#include "stdafx.h"
#include "PainterListWindow.h"
#include "ui_PainterListWindow.h"
#include "Util.h"
#include "SharedPaintManager.h"

PainterListWindow::PainterListWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::PainterListWindow)
{
	ui->setupUi(this);

	connect( ui->listPainter, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listItemClicked(QListWidgetItem*)) );
}

PainterListWindow::~PainterListWindow()
{
	delete ui;
}

void PainterListWindow::showEvent( QShowEvent * evt )
{
	USER_LIST list = SharePaintManagerPtr()->historyUserList();
	ui->listPainter->clear();

	for( size_t i = 0; i < list.size(); i++ )
	{
		QListWidgetItem *item = new QListWidgetItem( Util::toStringFromUtf8(list[i]->nickName()), ui->listPainter );

		if( SharePaintManagerPtr()->isAllowPainterToDraw( list[i]->userId() ) )
			item->setCheckState( Qt::Checked );
		else
			item->setCheckState( Qt::Unchecked );

		item->setData( 100, QString::fromStdString(list[i]->userId()) );
	}
}

void PainterListWindow::listItemClicked(QListWidgetItem* item)
{
	std::string userId = item->data( 100 ).toString().toStdString();
	qDebug() << userId.c_str() << item->checkState();

	SharePaintManagerPtr()->setAllowPainterToDraw( userId, ( item->checkState() == Qt::Checked ) );
}
