/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "StdAfx.h"
#include "sharedpainter.h"
#include "TextItemDialog.h"
#include "AboutWindow.h"
#include "UIStyleSheet.h"
#include "PreferencesDialog.h"

static const int DEFAULT_HIDE_POS_X = 9999;
static const int DEFAULT_HIDE_POS_Y = 9999;

#define ADD_CHAT_VERTICAL_SPACE(space)	\
{	\
	QTextCharFormat fmt;	\
	fmt.setFontPointSize( space );	\
	ui.editChat->setCurrentCharFormat( fmt );	\
	ui.editChat->append( "" );	\
}

#define ADD_CHAT_VERTICAL_SPACE_CHAT_SMALL()	ADD_CHAT_VERTICAL_SPACE(1)
#define ADD_CHAT_VERTICAL_SPACE_CHAT_BIG()	ADD_CHAT_VERTICAL_SPACE(3)

SharedPainter::SharedPainter(CSharedPainterScene *canvas, QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), canvas_(canvas), currPaintItemId_(1), currPacketId_(-1)
	, resizeFreezingFlag_(false), resizeSplitterFreezingFlag_(false), playbackSliderFreezingFlag_(false)
	, screenShotMode_(false), wroteProgressBar_(NULL)
	, lastTextPosX_(0), lastTextPosY_(0), status_(INIT), findingServerWindow_(NULL), syncProgressWindow_(NULL)
{
	fontBroadCastText_ = QFont( "Times" );
	fontBroadCastText_.setBold( true );
	fontBroadCastText_.setPixelSize( 20 );

	ui.setupUi(this);
	connect( ui.splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(splitterMoved(int, int)));
	ui.painterView->setScene( canvas );
	ui.painterView->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform );
	
	ui.editChat->document()->setDefaultStyleSheet(gStyleSheet_Chat);
	ui.editChat->setReadOnly( true );
	ui.editMsg->installEventFilter( this );

	setCursor( Qt::ArrowCursor ); 

	canvas_->setEvent( this );

	SharePaintManagerPtr()->registerObserver( this );
	SharePaintManagerPtr()->setCanvas( canvas_ );
	
	QMenuBar *menuBar = ui.menuBar;

	// Create Menu bar item
	{
		// File Menu
		QMenu* file = new QMenu( "&File", menuBar );
		file->addAction( "&Import from file", this, SLOT(actionImportFile()), Qt::CTRL+Qt::Key_I );
		file->addAction( "&Export to file", this, SLOT(actionExportFile()),  Qt::CTRL+Qt::Key_E );
		file->addAction( "&Save image", this, SLOT(actionSaveImageFile()),  Qt::CTRL+Qt::Key_S );
		file->addSeparator();
		file->addAction( "&About", this, SLOT(actionAbout()) );
		file->addSeparator();
		file->addAction( "E&xit", this, SLOT(actionExit()), Qt::CTRL+Qt::Key_Q );
		menuBar->addMenu( file );

		// Edit Menu
		QMenu* edit = new QMenu( "&Edit", menuBar );
		QMenu* penMenu = edit->addMenu( "Pen Setting" );
		penWidthAction_ = penMenu->addAction( "Pen &Width", this, SLOT(actionPenWidth()), Qt::ALT+Qt::Key_V );
		penMenu->addAction( "Pen &Color", this, SLOT(actionPenColor()), Qt::ALT+Qt::Key_C );
		penModeAction_ = edit->addAction( "Pen Mode", this, SLOT(actionPenMode()), Qt::ALT+Qt::Key_A );
		edit->addAction( "&Text", this, SLOT(actionAddText()), Qt::Key_Enter|Qt::Key_Return );
		gridLineAction_ = edit->addAction( "&Draw Grid Line", this, SLOT(actionGridLine()));
		edit->addAction( "&Background Color", this, SLOT(actionBGColor()), Qt::ALT+Qt::Key_B );
		edit->addAction( "&Screen Shot", this, SLOT(actionScreenShot()), Qt::ALT+Qt::Key_S );
		edit->addSeparator();
		edit->addAction( "Clear &Background", this, SLOT(actionClearBG()), Qt::CTRL+Qt::Key_B );
		edit->addAction( "Cl&ear Screen", this, SLOT(actionClearScreen()), Qt::CTRL+Qt::Key_X );
		edit->addSeparator();
		edit->addAction( "&Paste clipboard", this, SLOT(actionClipboardPaste()), Qt::CTRL+Qt::Key_V );
		edit->addSeparator();
		edit->addAction( "&Undo", this, SLOT(actionUndo()), Qt::CTRL+Qt::Key_Z );
		edit->addAction( "&Redo", this, SLOT(actionRedo()), Qt::CTRL+Qt::SHIFT+Qt::Key_Z );
		menuBar->addMenu( edit );

		// Network Menu
		QMenu* network = new QMenu( "&Network", menuBar );
		network->addAction( "&Connect to Relay Server", this, SLOT(actionConnectServer()) );
		network->addAction( "&Connect to Peer", this, SLOT(actionConnect()) );
		network->addSeparator();
		startFindServerAction_ = network->addAction( "Start &Find Server", this, SLOT(actionFindingServer()), Qt::CTRL+Qt::Key_1 );
		network->addSeparator();
		network->addAction( "Broadcast &Text Message", this, SLOT(actionBroadcastTextMessage()), Qt::CTRL+Qt::Key_M );
		network->addSeparator();
		network->addAction( "Close all connections", this, SLOT(actionCloseConnection()) );
		menuBar->addMenu( network );
		
		//  Menu
		QMenu* options = new QMenu( "&Options", menuBar );
		options->addAction( "&Nick Name", this, SLOT(actionNickName()) );
		options->addAction( "&Paint Channel", this, SLOT(actionPaintChannel()), Qt::CTRL+Qt::Key_H );
		options->addSeparator();
		showLastItemAction_ = options->addAction( "Blink &Last Item Always", this, SLOT(actionBlinkLastAddItem()), Qt::CTRL+Qt::Key_L );
		options->addSeparator();
		options->addAction( "&Preferences", this, SLOT(actionPreferences()) );
		menuBar->addMenu( options );

		gridLineAction_->setCheckable( true );
		penModeAction_->setCheckable( true );
		showLastItemAction_->setCheckable( true );
	}


	// create tool bar
	{
		ui.toolBar->setIconSize( QSize(32, 32) );
		toolBar_penColorButton_ = new QPushButton();
		toolBar_penColorButton_->connect( toolBar_penColorButton_, SIGNAL(clicked()), this, SLOT(actionPenColor()) );
		toolBar_penColorButton_->setToolTip( tr("Pen Color") );

		toolBar_bgColorButton_ = new QPushButton();
		toolBar_bgColorButton_->connect( toolBar_bgColorButton_, SIGNAL(clicked()), this, SLOT(actionBGColor()) );
		toolBar_bgColorButton_->setToolTip( tr("Background Color") );

		toolBar_MoveMode_ = ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/move_mode.png"), "Move", this, SLOT(actionMoveMode()) );
		toolBar_PenMode_ = ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/draw_line.png"), "Pen", this, SLOT(actionFreePenMode()) );
		ui.toolBar->addSeparator();
		ui.toolBar->addWidget( toolBar_penColorButton_ );
		
		QToolButton *penWidthButton = new QToolButton();
		QMenu *menuPenWidth = new QMenu();
		menuPenWidth->addAction( "Pen Width 20", this, SLOT(actionPenWidth20()) );
		menuPenWidth->addAction( "Pen Width 10", this, SLOT(actionPenWidth10()) );
		menuPenWidth->addAction( "Pen Width 6", this, SLOT(actionPenWidth6()) );
		menuPenWidth->addAction( "Pen Width 3", this, SLOT(actionPenWidth3()) );
		menuPenWidth->addAction( "Pen Width 1", this, SLOT(actionPenWidth1()) );
		menuPenWidth->addAction( penWidthAction_ );
		penWidthButton->setIcon( QIcon(":/SharedPainter/Resources/pen_width.png") );
		penWidthButton->setMenu( menuPenWidth );
		penWidthButton->connect( penWidthButton, SIGNAL(clicked()), penWidthButton, SLOT(showMenu()) );
		ui.toolBar->addWidget( penWidthButton );

		ui.toolBar->addSeparator();
		ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/screenshot.png"), "Screen Shot", this, SLOT(actionScreenShot()) );
		ui.toolBar->addWidget( toolBar_bgColorButton_ );
		toolBar_GridLine_ = ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/grid_line.png"), "Grid Line", this, SLOT(actionGridLine()) );
		ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/bg_clear.png"), "Clear Background", this, SLOT(actionClearBG()) );
		ui.toolBar->addSeparator();
		ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/clear_screen.png"), "Clear Screen", this, SLOT(actionClearScreen()) );
		ui.toolBar->addSeparator();
		ui.toolBar->addAction( QIcon(":/SharedPainter/Resources/last_item.png"), " Blink Last Added Item", this, SLOT(actionLastItem()) );
		toolBar_SliderPlayback_ = new QSlider(Qt::Horizontal);
		ui.toolBar->addWidget( toolBar_SliderPlayback_ );
		connect( toolBar_SliderPlayback_, SIGNAL(valueChanged(int)), this, SLOT(onPlaybackSliderValueChanged(int)) );

		toolBar_GridLine_->setCheckable( true );
		toolBar_MoveMode_->setCheckable( true );
		toolBar_PenMode_->setCheckable( true );

		toolBar_SliderPlayback_->setStyleSheet( gStyleSheet_Slider );
		toolBar_SliderPlayback_->setRange(0, 0);
		changeToobarButtonColor( toolBar_penColorButton_, canvas_->penColor() );
		changeToobarButtonColor( toolBar_bgColorButton_, QColor(Qt::white) );


	}


	// create status bar
	{
		statusBarLabel_ = new QLabel();
		broadCastTypeLabel_ = new QLabel();
		joinerCountLabel_ = new QLabel();
		playbackStatusLabel_ = new QLabel();
		wroteProgressBar_ = new QProgressBar();
		networkInfoLabel_ = new QLabel();
		ui.statusBar->addPermanentWidget( broadCastTypeLabel_ );
		ui.statusBar->addPermanentWidget( joinerCountLabel_ );
		ui.statusBar->addPermanentWidget( playbackStatusLabel_, 1 );
		ui.statusBar->addPermanentWidget( wroteProgressBar_ );
		ui.statusBar->addPermanentWidget( networkInfoLabel_ );
		ui.statusBar->addPermanentWidget( statusBarLabel_ );

		setStatusBar_BroadCastType( STR_NET_MODE_INIT );
		setStatusBar_JoinerCnt( 1 );	// my self 
		setStatusBar_PlaybackStatus( 0, 0 );
	}
	

	// create system tray
	{
		trayIconMenu_ = new QMenu(this);
		trayIconMenu_->addAction("&Open", this, SLOT(show()));
		trayIconMenu_->addAction("&About", this, SLOT(actionAbout()));
		trayIconMenu_->addSeparator();
		trayIconMenu_->addAction("E&xit", this, SLOT(actionExit()));

		trayIcon_ = new QSystemTrayIcon(this);
		trayIcon_->setContextMenu(trayIconMenu_);

		connect(trayIcon_, SIGNAL(messageClicked()), this, SLOT(onTrayMessageClicked()));
		connect(trayIcon_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayActivated(QSystemTrayIcon::ActivationReason)));
		trayIcon_->show();
	}

	// Setting applying
	SharePaintManagerPtr()->changeNickName( SettingManagerPtr()->nickName() );
	SharePaintManagerPtr()->setPaintChannel( SettingManagerPtr()->paintChannel() );
	setCheckShowLastAddItemAction( canvas_->isSettingShowLastAddItemBorder() );
	actionPenWidth3();

	// change status to "init"
	setStatus( INIT );

	// Pen mode activated..
	actionFreePenMode();

	// Key Hooking Timer 
	keyHookTimer_ = new QTimer(this);
	keyHookTimer_->start(20);
	connect(keyHookTimer_, SIGNAL(timeout()),this, SLOT(onTimer()));

	installEventFilter(this);

	// Title
	updateWindowTitle();

	// start server 
	SharePaintManagerPtr()->startServer();
	setStatusBar_NetworkInfo( Util::getMyIPAddress(), SharePaintManagerPtr()->acceptPort() );
}

SharedPainter::~SharedPainter()
{
	SharePaintManagerPtr()->unregisterObserver( this );
	SharePaintManagerPtr()->close();

	hideFindingServerWindow();
	hideSyncProgressWindow();

	delete keyHookTimer_;
}


// this logic is for fixing bug that the center "Enter key" don't work.
bool SharedPainter::eventFilter(QObject *object, QEvent *event)
{
	if( event->type() == QEvent::KeyPress )
	{
		QKeyEvent *keyEvt = (QKeyEvent*)event;

		if( keyEvt->key() == 0x1000004 || keyEvt->key() == Qt::Key_Enter )
		{
			if( object == ui.editMsg )
			{
				sendChatMessage();
				return true;
			}
			else if( ui.painterView->isActiveWindow() )
				actionAddText();
		}
		else if( keyEvt->key() == Qt::Key_Left || keyEvt->key() == Qt::Key_Right )
		{
			int step = keyEvt->key() == Qt::Key_Left ? -1 : +1;

			int newValue = toolBar_SliderPlayback_->value() + step;
			if( newValue >= 0 && newValue < toolBar_SliderPlayback_->maximum() )
				toolBar_SliderPlayback_->setValue( newValue );
		}
	}
	return QMainWindow::eventFilter(object,event);
}

void SharedPainter::onTimer( void )
{
	//if(!isActiveWindow())
	//	return;
	//
	//if( Util::checkKeyPressed( 'P' ) )
	//{
	//	penModeAction_->setChecked( !penModeAction_->isChecked() );
	//	actionPenMode();	
	//}
	//else if( Util::checkKeyPressed( 'T' ) )
	//{
	//	actionAddText();
	//}
}

void SharedPainter::onPlaybackSliderValueChanged( int value )
{
	if( playbackSliderFreezingFlag_ )
		return;

	SharePaintManagerPtr()->plabackTo( value - 1);

	setStatusBar_PlaybackStatus( value, toolBar_SliderPlayback_->maximum() );

	bool playback = SharePaintManagerPtr()->isPlaybackMode();

	if( playback )
	{
		if( canvas_->freezeAction() )
			qApp->setOverrideCursor(QCursor(QPixmap(":/SharedPainter/Resources/draw_disabled.png"))); 
	}
	else
	{
		qApp->restoreOverrideCursor(); 
		canvas_->thawAction();
	}
}

void SharedPainter::onTrayMessageClicked( void )
{
	show();
}

void SharedPainter::onTrayActivated( QSystemTrayIcon::ActivationReason reason )
{
	switch ( reason ) 
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		show();
		break;
	default:
		;
	}
}

void SharedPainter::updateWindowTitle( void )
{
	// Title
	QString newTitle = PROGRAME_TEXT;
	newTitle += " Ver ";
	newTitle += VERSION_TEXT;
	newTitle += ", ";
	newTitle += Util::toStringFromUtf8(SettingManagerPtr()->nickName());
	newTitle += " - Channel : ";
	newTitle += Util::toStringFromUtf8(SettingManagerPtr()->paintChannel());

	setWindowTitle( newTitle );
}

void SharedPainter::sendChatMessage( void )
{
	if( ! getNickNameString() )
		return;

	QString plainText = ui.editMsg->toPlainText().trimmed();
	std::string msg = Util::toUtf8StdString( plainText );

	SharePaintManagerPtr()->sendChatMessage( msg );

	ui.editMsg->setText( "" );
}

void SharedPainter::setCheckGridLineAction( bool checked )
{
	toolBar_GridLine_->setChecked( checked );
	gridLineAction_->setChecked( checked );
}


void SharedPainter::actionAbout( void )
{
	AboutWindow wnd(this);
	wnd.exec();
}

void SharedPainter::actionExit( void )
{
	trayIcon_->hide();
	close();
}

void SharedPainter::updateLastChatTime( void )
{
	QString str = tr("last message time : ");
	str += QDateTime::currentDateTime().toString();
	ui.labelLastMessageTime->setText( str );
}

void SharedPainter::addSystemMessage( const QString &msg )
{
	ADD_CHAT_VERTICAL_SPACE_CHAT_BIG();

	ui.editChat->append( "<html><div class=messageSystem>" + msg + "</div></html>" );

	lastChatUserId_ = "";
}

void SharedPainter::addMyChatMessage( const QString & userId, const QString &nickName, const QString &chatMsg )
{
	bool continuousChatFlag = false;
	if( lastChatUserId_ == userId )
		continuousChatFlag = true;

	if( ! continuousChatFlag )
	{
		ADD_CHAT_VERTICAL_SPACE_CHAT_BIG();
		ui.editChat->append( "<html><div class=nicknameMine>" + nickName + "</div></html>" );
	}
	else
		ADD_CHAT_VERTICAL_SPACE_CHAT_SMALL();

	ui.editChat->append( "<html><span class=chatMark> > </span><span class=messageMine>" + chatMsg + "</span></html>" );

	lastChatUserId_ = userId;
}

void SharedPainter::addYourChatMessage( const QString & userId, const QString &nickName, const QString &chatMsg )
{
	bool continuousChatFlag = false;
	if( lastChatUserId_ == userId )
		continuousChatFlag = true;

	if( ! continuousChatFlag )
	{
		ADD_CHAT_VERTICAL_SPACE_CHAT_BIG();
		ui.editChat->append( "<html><div class=nicknameOther>" + nickName + "</div></html>" );
	}
	else
		ADD_CHAT_VERTICAL_SPACE_CHAT_SMALL();

	ui.editChat->append( "<html><span class=chatMark> > </span><span class=messageOther>" + chatMsg + "</span></html>" );

	lastChatUserId_ = userId;

	updateLastChatTime();
}


void SharedPainter::addBroadcastChatMessage( const QString & channel, const QString & userId, const QString &nickName, const QString &chatMsg )
{
	bool continuousChatFlag = false;
	if( lastChatUserId_ == userId )
		continuousChatFlag = true;

	ADD_CHAT_VERTICAL_SPACE_CHAT_BIG();
	QString who;
	who = nickName + tr(" in \"") + channel + tr("\" channel");
	ui.editChat->append( "<html><div class=nicknameBroadcast>" + who + "</div></html>" );
	ui.editChat->append( "<html><div class=messageBroadcast>" + chatMsg + "</div></html>" );

	lastChatUserId_ = "";

	updateLastChatTime();
}


void SharedPainter::actionImportFile( void )
{
	QString path;

	path = QFileDialog::getOpenFileName( this, tr("Export to file"), "", tr("Shared Paint Data File (*.sp)") );

	if( path.isEmpty() )
		return;

	QFile f(path);
	if( !f.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, "", tr("cannot open file.") );
		return;
	}

	QByteArray byteArray;
	byteArray = f.readAll();

	SharePaintManagerPtr()->clearScreen( true );
	if( ! SharePaintManagerPtr()->deserializeData( byteArray.data(), byteArray.size() ) )
	{
		QMessageBox::critical( this, "", tr("cannot import this file. or this file is not compatible with this version.") );
	}
}

void SharedPainter::setCheckShowLastAddItemAction( bool checked )
{
	showLastItemAction_->setChecked( checked );
}

void SharedPainter::actionBlinkLastAddItem( void )
{
	canvas_->setSettingShowLastAddItemBorder( !canvas_->isSettingShowLastAddItemBorder() );
	setCheckShowLastAddItemAction( canvas_->isSettingShowLastAddItemBorder() );
}

void SharedPainter::actionLastItem( void )
{
	canvas_->drawLastItemBorderRect();
}

void SharedPainter::actionSaveImageFile( void )
{
	QString path;
	path = QFileDialog::getSaveFileName( this, tr("Save image"), "", tr("jpeg file (*.jpg)") );
	if( path.isEmpty() )
		return;

	QPixmap pixMap = QPixmap::grabWidget(ui.painterView);
	pixMap.save(path);

}

void SharedPainter::actionExportFile( void )
{
	std::string allData = SharePaintManagerPtr()->serializeData();

	QString path;
	path = QFileDialog::getSaveFileName( this, tr("Export to file"), "", tr("Shared Paint Data File (*.sp)") );
	if( path.isEmpty() )
		return;

	QFile f(path);
	if( !f.open( QIODevice::WriteOnly ) )
	{
		QMessageBox::warning( this, "", tr("cannot open file.") );
		return;
	}

	QDataStream out(&f);
	int ret = out.writeRawData( allData.c_str(), allData.size() );
	if( ret != allData.size() )
	{
		QMessageBox::warning( this, "", tr("failed to save.") );
		return;
	}
}

void SharedPainter::actionGridLine( void )
{
	setCheckGridLineAction( canvas_->backgroundGridLineSize() <= 0 );	// for next state..

	if( canvas_->backgroundGridLineSize() > 0 )
	{
		SharePaintManagerPtr()->setBackgroundGridLine( 0 );	// clear
	}
	else
	{
		SharePaintManagerPtr()->setBackgroundGridLine( DEFAULT_GRID_LINE_SIZE_W );	// draw
	}
}

void SharedPainter::actionBGColor( void )
{
	static QColor LAST_COLOR = Qt::white;
	QColor clr = QColorDialog::getColor(canvas_->backgroundColor(), this, tr("Background Color"));
	LAST_COLOR = clr;

	if( !clr.isValid() )
		return;

	SharePaintManagerPtr()->setBackgroundColor( clr.red(), clr.green(), clr.blue(), clr.alpha() );

	changeToobarButtonColor( toolBar_bgColorButton_, clr );
}

void SharedPainter::actionConnectServer( void )
{
	static std::string userId = Util::generateMyId();

	if( ! getNickNameString() )
		return;

	if( ! getPaintChannelString() )
		return;	

	QString errorMsg;

	do{
		bool ok = false;

		static QString lastAddress;

		QString addr = QInputDialog::getText( this, tr("Input relay server address"),
			tr("Address:Port"), QLineEdit::Normal, SettingManagerPtr()->relayServerAddress().c_str(), &ok);

		lastAddress = addr;

		if ( !ok )
			break;

		if ( addr.isEmpty())
		{
			errorMsg = tr("Your input addres is wrong format. (IP:PORT)");
			break;
		}

		QStringList list = addr.split(":");

		if( list.size() != 2 )
		{
			errorMsg = tr("Your input addres is wrong format. (IP:PORT)");
			break;
		}

		std::string ip = list.at(0).toStdString();
		int port = list.at(1).toInt();
	
		SettingManagerPtr()->setRelayServerAddress( addr.toStdString() );

		SharePaintManagerPtr()->requestJoinServer( ip, port, SettingManagerPtr()->paintChannel() );
		return;
	} while( false );

	if( ! errorMsg.isEmpty() )
		QMessageBox::warning(this, "", errorMsg);
}

void SharedPainter::actionConnect( void )
{
	QString errorMsg;

	do{
		bool ok = false;

		static QString lastAddress;

		QString addr = QInputDialog::getText( this, tr("Input peer address"),
			tr("Address:Port"), QLineEdit::Normal, SettingManagerPtr()->peerAddress().c_str(), &ok);

		lastAddress = addr;

		if ( !ok )
			break;

		if ( addr.isEmpty())
		{
			errorMsg = tr("Your input addres is wrong format. (IP:PORT)");
			break;
		}

		QStringList list = addr.split(":");

		if( list.size() != 2 )
		{
			errorMsg = tr("Your input addres is wrong format. (IP:PORT)");
			break;
		}

		std::string ip = list.at(0).toStdString();
		int port = list.at(1).toInt();

		// save 
		SettingManagerPtr()->setPeerAddress( addr.toStdString() );

		// start connecting
		if( ! SharePaintManagerPtr()->connectToPeer( ip, port ) )
		{
			errorMsg = tr("Could not connect to the peer.");
			break;
		}
		return;
	} while( false );

	if( ! errorMsg.isEmpty() )
		QMessageBox::warning(this, "", errorMsg);
}


void SharedPainter::actionAddText( void )
{
	TextItemDialog dlg(this);
	int res = dlg.exec();
	if( res != QDialog::Accepted )
		return;

	addTextItem( dlg.text(), dlg.font(), dlg.textColor() );
}


void SharedPainter::actionPenWidth( void )
{
	bool ok = false;

	int width = QInputDialog::getInt(this, tr("Pen Width"),
                                  tr("Width:"), canvas_->penWidth(), 0, 100, 1, &ok);

	if ( !ok )
		return;

	canvas_->setPenSetting( canvas_->penColor(), width );
}

void SharedPainter::actionPenWidth1( void )
{
	canvas_->setPenSetting( canvas_->penColor(), 1 );
}

void SharedPainter::actionPenWidth3( void )
{
	canvas_->setPenSetting( canvas_->penColor(), 3 );
}

void SharedPainter::actionPenWidth6( void )
{
	canvas_->setPenSetting( canvas_->penColor(), 6 );
}

void SharedPainter::actionPenWidth10( void )
{
	canvas_->setPenSetting( canvas_->penColor(), 10 );
}

void SharedPainter::actionPenWidth20( void )
{
	canvas_->setPenSetting( canvas_->penColor(), 20 );
}

void SharedPainter::actionPenColor( void )
{
	QColor clr = QColorDialog::getColor( canvas_->penColor(), this, tr("Pen Color") );

	if( !clr.isValid() )
		return;

	canvas_->setPenSetting( clr, canvas_->penWidth() );

	changeToobarButtonColor( toolBar_penColorButton_, clr );
}

void SharedPainter::actionFreePenMode( void )
{
	toolBar_MoveMode_->setChecked( false );
	toolBar_PenMode_->setChecked( true );

	penModeAction_->setChecked( true );
	canvas_->setFreePenMode( true );
}

void SharedPainter::actionMoveMode( void )
{
	toolBar_MoveMode_->setChecked( true );
	toolBar_PenMode_->setChecked( false );

	penModeAction_->setChecked( false );
	canvas_->setFreePenMode( false );
}


void SharedPainter::actionPenMode( void )
{
	if( penModeAction_->isChecked() )
	{
		actionFreePenMode();
	}
	else
	{
		actionMoveMode();
	}
}

void SharedPainter::actionScreenShot( void )
{
	screenShotMode_ = true;
	orgPos_ = pos();
	move(DEFAULT_HIDE_POS_X, DEFAULT_HIDE_POS_Y);
}


void SharedPainter::actionClearBG( void )
{
	SharePaintManagerPtr()->clearBackground();
}

void SharedPainter::actionClearScreen( void )
{
	int res = QMessageBox::question( this, "", tr("All items and playback data will be lost and can not be rolled back.\nWould you like to proceed?"), QMessageBox::Ok|QMessageBox::Cancel);
	if( res != QMessageBox::Ok )
	{
		return;
	}

	SharePaintManagerPtr()->clearScreen();
}

void SharedPainter::actionUndo( void )
{
	SharePaintManagerPtr()->undoCommand();
}

void SharedPainter::actionRedo( void )
{
	SharePaintManagerPtr()->redoCommand();
}

void SharedPainter::actionCloseConnection( void )
{
	SharePaintManagerPtr()->close();
}

void SharedPainter::actionPreferences( void )
{
	PreferencesDialog dlg(this);
	dlg.exec();
}

void SharedPainter::actionBroadcastTextMessage( void )
{
	bool ok;
	QString msg = QInputDialog::getText(this, tr("Broadcast Text Message"), tr("Message:"), QLineEdit::Normal, "", &ok);
	if( ! ok )
		return;

	SharePaintManagerPtr()->sendBroadCastTextMessage( SettingManagerPtr()->paintChannel(), Util::toUtf8StdString( msg ) );
}

void SharedPainter::actionNickName( void )
{
	getNickNameString( true );
}

void SharedPainter::actionPaintChannel( void )
{
	getPaintChannelString( true );
}

void SharedPainter::actionFindingServer( void )
{
	if( ! getNickNameString() )
		return;

	if( ! getPaintChannelString() )
		return;

	if( ! SharePaintManagerPtr()->isFindingServerMode() )
	{
		if( SharePaintManagerPtr()->startFindingServer() )
		{
			setStatusBar_BroadCastType( STR_NET_MODE_FINDING_SERVER );

			showFindingServerWindow();
		}
		else
			setStatusBar_BroadCastType( STR_NET_MODE_INIT );
	}
	else
	{
		SharePaintManagerPtr()->stopFindingServer();
		setStatusBar_BroadCastType( STR_NET_MODE_INIT );
	}
}

void SharedPainter::actionClipboardPaste( void )
{
	const QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *mimeData = clipboard->mimeData();

	 if (mimeData->hasImage()) 
	 {
		 int w = 0;
		 int h = 0;
		 boost::shared_ptr<CImageItem> item = boost::shared_ptr<CImageItem>(new CImageItem());
		 QPixmap pixmap( qvariant_cast<QPixmap>(mimeData->imageData()) );
		 item->setPixmap( pixmap );
		 item->setMyItem();

		 w = pixmap.width();
		 h = pixmap.height();

		 QPointF pos = Util::calculateNewItemPos( canvas_->sceneRect().width(), canvas_->sceneRect().height(), 
			 ui.painterView->mapFromGlobal(QCursor::pos()).x(),
			 ui.painterView->mapFromGlobal(QCursor::pos()).y(),
			 w, h );

		item->setPos( pos.x(), pos.y() );
		requestAddItem( item );
     } 
	 else if (mimeData->hasText())
	 {
		 QFont f(tr("Gulim"));
		 f.setPixelSize(10);
		 addTextItem( mimeData->text(), f, Util::getComplementaryColor( canvas_->backgroundColor() ) );
	 }
}


bool SharedPainter::getNickNameString( bool force )
{
	if( !force && SettingManagerPtr()->nickName().empty() == false )	// already setting
		return true;

	if( !force )
	{
		QMessageBox::warning(this, "", "Your need to set your nickname.");
	}

	bool ok;
	QString nick = QInputDialog::getText(this, tr("Nick Name"), tr("Name: any string")
		, QLineEdit::Normal, Util::toStringFromUtf8(SettingManagerPtr()->nickName()), &ok);

	if( ! ok )
	{
		if( SettingManagerPtr()->nickName().empty() )
			return getNickNameString( force );
		return true;
	}

	if( nick.isEmpty() )
	{
		QMessageBox::warning(this, "", "Invalid channel string.");
		return getNickNameString(force);
	}

	SettingManagerPtr()->setNickName( Util::toUtf8StdString(nick) );

	SharePaintManagerPtr()->changeNickName( SettingManagerPtr()->nickName() );

	updateWindowTitle();
	return true;
}

bool SharedPainter::getPaintChannelString( bool force )
{
	if( !force && SettingManagerPtr()->paintChannel().empty() == false )	// already setting
		return true;

	if( !force )
	{
		QMessageBox::warning(this, "", "Your need to set a channel first.");
	}

	bool ok;
	QString channel = QInputDialog::getText(this, tr("Paint Channel"), tr("Channel: any string"), QLineEdit::Normal, SettingManagerPtr()->paintChannel().c_str(), &ok);
	if( ! ok )
	{
		if( SettingManagerPtr()->paintChannel().empty() )
			return getPaintChannelString( force );
		return true;
	}

	if( channel.isEmpty() )
	{
		QMessageBox::warning(this, "", "Invalid channel string.");
		return getPaintChannelString( force );
	}

	std::string prevChannel = SettingManagerPtr()->paintChannel();

	bool reconnectFlag = false;
	if( prevChannel != "" && prevChannel != channel.toStdString() )
	{
		if( SharePaintManagerPtr()->isConnected() || SharePaintManagerPtr()->isConnecting() )
		{
			int res = QMessageBox::question( this, "", tr("If you change the channel, your connection will be lost.\nDo you change the channel and reconnect?"), QMessageBox::Ok|QMessageBox::Cancel);
			if( res != QMessageBox::Ok )
			{
				return false;
			}

			reconnectFlag = true;
		}
	}

	SettingManagerPtr()->setPaintChannel( channel.toStdString() );
	SharePaintManagerPtr()->setPaintChannel( SettingManagerPtr()->paintChannel() );

	if( reconnectFlag )
	{
		SharePaintManagerPtr()->close();
		SharePaintManagerPtr()->reconnect();
	}

	updateWindowTitle();
	return true;
}

void SharedPainter::keyPressEvent ( QKeyEvent * event )  
{ 
	QWidget::keyPressEvent(event); 
} 

void SharedPainter::checkSetting( void )
{
	// must be set..
	getNickNameString();
	getPaintChannelString();
}

void SharedPainter::showEvent( QShowEvent * evt )
{
	int w = ui.painterView->width();
	int h = ui.painterView->height();
	canvas_->setSceneRect(0, 0, w, h);

	static bool firstShow = true;
	if( firstShow )
	{
		QTimer::singleShot( 100, this, SLOT(checkSetting()) );

		QList<int> sz;
		sz.push_back( DEFAULT_INITIAL_CHATWINDOW_SIZE );
		sz.push_back( width() - DEFAULT_INITIAL_CHATWINDOW_SIZE );
		ui.splitter->setSizes(sz);
		firstShow = false;

		w = ui.painterView->width();
		h = ui.painterView->height();
		canvas_->setSceneRect(0, 0, w, h);

		SharePaintManagerPtr()->notifyResizingCanvas( w, h );
	}
}


void SharedPainter::closeEvent( QCloseEvent *evt )
{
	if (trayIcon_->isVisible()) {
		QMessageBox::information(this, tr("Systray"),
			tr("The program will keep running in the "
			"system tray. To terminate the program, "
			"choose <b>Exit</b> in the context menu "
			"of the system tray entry."));
		hide();
		evt->ignore();
		return;
	}

	SettingManagerPtr()->save();
	SharePaintManagerPtr()->clearScreen( false );

	QMainWindow::closeEvent( evt );
}
 

void SharedPainter::moveEvent( QMoveEvent * evt )
{
	// Screen Shot Action!
	if( screenShotMode_ )
	{
		if( evt->pos().x() == DEFAULT_HIDE_POS_X && evt->pos().y() == DEFAULT_HIDE_POS_Y )
		{
			// Screen Shot!
			QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());

			// Create Backgound Image Item
			boost::shared_ptr<CBackgroundImageItem> image = boost::shared_ptr<CBackgroundImageItem>( new CBackgroundImageItem );
			image->setPixmap( pixmap );
			image->setOwner( SharePaintManagerPtr()->myId() );
			image->setItemId( 0 );

			// Send to peers
			SharePaintManagerPtr()->sendBackgroundImage( image );

			// Restore original postion..
			move(orgPos_);
			screenShotMode_ = false;
		}
	}
}

void SharedPainter::resizeEvent( QResizeEvent *evt )
{
	int w = ui.painterView->width();
	int h = ui.painterView->height();
	int sw = canvas_->sceneRect().width();
	int sh = canvas_->sceneRect().height();

	if( w > sw )
		sw = w;
	if( h > sh )
		sh = h;
	canvas_->setSceneRect(0, 0, sw, sh);
	SharePaintManagerPtr()->notifyResizingCanvas( sw, sh );

	if( SettingManagerPtr()->isSyncWindowSize() )
	{
		if( !resizeFreezingFlag_ )
			SharePaintManagerPtr()->notifyResizingMainWindow( width(), height() );
	}

	QMainWindow::resizeEvent(evt);
}

void SharedPainter::splitterMoved( int pos, int index )
{
	std::vector<int> vec;
	for (int i = 0; i < ui.splitter->sizes().size(); ++i)
		vec.push_back( ui.splitter->sizes().at(i) );

	if( SettingManagerPtr()->isSyncWindowSize() )
	{
		if( !resizeSplitterFreezingFlag_ )
			SharePaintManagerPtr()->notifyResizingWindowSplitter( vec );
	}
}


void SharedPainter::requestAddItem( boost::shared_ptr<CPaintItem> item )
{
	item->setOwner( SharePaintManagerPtr()->myId() );
	item->setItemId( currPaintItemId_++ );

	SharePaintManagerPtr()->sendPaintItem( item );
}


void SharedPainter::onICanvasViewEvent_BeginMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item )
{
	// nothing to do
}

void SharedPainter::onICanvasViewEvent_EndMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item )
{
	SharePaintManagerPtr()->movePaintItem( item );
}

void SharedPainter::onICanvasViewEvent_DrawItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item  )
{
	requestAddItem( item );
}

void SharedPainter::onICanvasViewEvent_UpdateItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item )
{
	SharePaintManagerPtr()->updatePaintItem( item );
}

void SharedPainter::onICanvasViewEvent_RemoveItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item )
{
	SharePaintManagerPtr()->removePaintItem( item );
}
