#include "StdAfx.h"
#include "UpgradeWindow.h"
#include "UIStyleSheet.h"

#define ADD_CHAT_VERTICAL_SPACE(space)	\
{	\
	QTextCharFormat fmt;	\
	fmt.setFontPointSize( space );	\
	ui.textEditUpgrade->setCurrentCharFormat( fmt );	\
	ui.textEditUpgrade->append( "" );	\
}

UpgradeWindow::UpgradeWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.textEditUpgrade->document()->setDefaultStyleSheet(gStyleSheet_UpdateEdit);

}

UpgradeWindow::~UpgradeWindow()
{

}


void UpgradeWindow::setContents( const QString &version, const QString &patchContents )
{
	QString temp = tr("Version : ");
	temp += version;

	ui.textEditUpgrade->append( "<html><div class=version>" + temp + "</div></html>" );
	ADD_CHAT_VERTICAL_SPACE( 10 );
	
	temp = tr("[Update]");
	ui.textEditUpgrade->append( "<html><div class=title>" + temp + "</div></html>" );
	ADD_CHAT_VERTICAL_SPACE( 5 );

	temp = patchContents;
	temp = temp.replace( "\r\n", "<br>" ).replace("\n", "<br>");
	ui.textEditUpgrade->append( "<html><div class=contents>" + temp + "</div></html>" );
}

void UpgradeWindow::onOK( void )
{
	UpgradeManagerPtr()->doUpgradeNow();
	accept();
}


void UpgradeWindow::onCancel( void )
{
	UpgradeManagerPtr()->stopVersionCheck();
	reject();

	QMessageBox::information( NULL, "", tr("You can upgrade manually by executing <b>Upgrader program</b>, after exit this application") ); 
}
