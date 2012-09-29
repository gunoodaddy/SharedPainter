#include "StdAfx.h"
#include "UpgradeWindow.h"
#include "UIStyleSheet.h"

#define ADD_CHAT_VERTICAL_SPACE(edit, space)	\
{	\
	QTextCharFormat fmt;	\
	fmt.setFontPointSize( space );	\
	edit->setCurrentCharFormat( fmt );	\
	edit->append( "" );	\
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


void UpgradeWindow::setContents( QTextEdit *edit, const QString &version, const QString &patchContents )
{
	QString temp = tr("Version : ");
	temp += version;

	edit->append( "<html><div class=version>" + temp + "</div></html>" );
	ADD_CHAT_VERTICAL_SPACE( edit, 10 );
	
	temp = tr("[Update]");
	edit->append( "<html><div class=title>" + temp + "</div></html>" );
	ADD_CHAT_VERTICAL_SPACE( edit, 5 );

	temp = patchContents;
	temp = temp.replace( "\r\n", "<br>" ).replace("\n", "<br>");
	edit->append( "<html><div class=contents>" + temp + "</div></html>" );

	edit->moveCursor( QTextCursor::Start );
}


void UpgradeWindow::setContents( const QString &version, const QString &patchContents )
{
	setContents( ui.textEditUpgrade, version, patchContents );
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
