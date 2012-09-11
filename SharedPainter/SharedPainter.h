#ifndef SHAREDPAINTER_H
#define SHAREDPAINTER_H

#include <qt_windows.h>
#include <QtGui/QMainWindow>
#include "ui_sharedpainter.h"
#include "SharedPainterScene.h"
#include "SharedPaintPolicy.h"

class SharedPainter : public QMainWindow, ICanvasViewEvent, ISharedPaintEvent
{
	Q_OBJECT

public:
	enum Status
	{
		INIT,
		CONNECTED,
		CONNECTFAILED,
		DISCONNECTED,
	};

	SharedPainter(CSharedPainterScene* canvas, QWidget *parent = 0, Qt::WFlags flags = 0);
	~SharedPainter();

	void setStatusBar_Network( const QString &str )
	{
		QString realStr = str + "  ";
		statusBarLabel_->setText( realStr );
	}
	void setStatusBar_BroadCastType( const QString &str )
	{
		QString realStr = str + "  ";
		broadCastTypeLabel_->setText( realStr );
	}
	void setStatusBar_JoinerCnt( int count )
	{
		QString str = tr("  Joiner Count : ");
		str += QString::number(count);
		joinerCountLabel_->setText( str );
	}
	void setStatus( Status status )
	{
		QString msg;
		switch( status )
		{
		case INIT:
			msg = tr("Not connected");
			break;
		case CONNECTED:
			msg = tr("Connected");
			break;
		case DISCONNECTED:
			msg = tr("Disconnected");
			break;
		case CONNECTFAILED:
			msg = tr("Connect Failure");
			break;
		}

		setStatusBar_Network( msg );
		setTrayIcon( status );
	}

	void setTrayIcon( Status status )
	{
		status_ = status;

		QIcon icon;
		switch( status )
		{
		case CONNECTED:
			icon = QIcon( QPixmap(":/SharedPainter/Resources/tray_connect.png") );
			break;
		default:
			icon = QIcon( QPixmap(":/SharedPainter/Resources/tray_disconnect.png") );
			break;
		}
		trayIcon_->setIcon( icon );
		setWindowIcon( icon );
	}

	void showTrayMessage( const QString &str )
	{
	     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;

	     trayIcon_->showMessage( PROGRAME_TEXT, str, icon, DEFAULT_TRAY_MESSAGE_DURATION_MSEC );
 	}

	void addTextItem( const QString &str, const QFont &font, const QColor &textColor )
	{
		boost::shared_ptr<CTextItem> textItem = boost::shared_ptr<CTextItem>(new CTextItem( str, font, textColor ));
		textItem->setMyItem();

		QPointF pos = _calculateTextPos( font.pixelSize() );

		textItem->setPos( pos.x(), pos.y() );

		_requestAddItem( textItem );
	}

protected:
	void closeEvent( QCloseEvent *evt );
	void moveEvent( QMoveEvent * evt );
	void showEvent ( QShowEvent * evt );
	void resizeEvent( QResizeEvent *evt );
	bool eventFilter(QObject *object, QEvent *event);
	void changeEvent ( QEvent * event )
	{
		if( event->type() == QEvent::ActivationChange )
		{
			if( isActiveWindow() )
			{
#ifdef Q_WS_WIN
				::FlashWindow( winId(), FALSE);
#endif
			}
		}
	}
	
protected slots:
	void onTimer( void );
	void onTrayMessageClicked( void );
	void onTrayActivated( QSystemTrayIcon::ActivationReason reason );
	void onPlaybackSliderValueChanged( int value  );

	void actionBroadcastChannel( void );
	void actionBroadcastTextMessage( void );
	void actionClearBG( void );
	void actionBGColor( void );
	void actionAddText( void );
	void actionClearScreen( void );
	void actionPenWidth( void );
	void actionPenWidth3( void );
	void actionPenWidth6( void );
	void actionPenWidth10( void );
	void actionPenWidth20( void );
	void actionPenColor( void );
	void actionPenMode( void );
	void actionFreePenMode( void );
	void actionMoveMode( void );
	void actionExit( void );
	void actionConnect( void );
	void actionScreenShot( void );
	void actionUndo( void );
	void actionRedo( void );
	void actionServerType( void );
	void actionClientType( void );
	void actionGridLine( void );
	void actionImportFile( void );
	void actionExportFile( void );
	void actionBlinkLastAddItem( void );
	void actionLastItem( void );

private:
	void setCheckGridLineAction( bool checked );
	void setCheckShowLastAddItemAction( bool checked );
	void _requestAddItem( boost::shared_ptr<CPaintItem> item );
	QPointF _calculateTextPos( int textSize );
	bool getBroadcastChannelString( bool force = false );
	void changeToobarButtonColor( QPushButton *button, const QColor &clr )
	{
		QString s;
		s += "background-color: " + clr.name() + ";";
		s += "width:20; height:20;";
		button->setStyleSheet(s);
	}

protected:
	// ICanvasViewEvent
	virtual void onICanvasViewEvent_BeginMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item );
	virtual void onICanvasViewEvent_EndMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item );
	virtual void onICanvasViewEvent_DrawItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item );
	virtual void onICanvasViewEvent_UpdateItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item );
	virtual void onICanvasViewEvent_RemoveItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item );

	// ISharedPaintEvent
	virtual void onISharedPaintEvent_Connected( CSharedPaintManager *self )
	{
		setStatus( CONNECTED );
	}

	virtual void onISharedPaintEvent_ConnectFailed( CSharedPaintManager *self )
	{
		setStatus( CONNECTFAILED );
	}

	virtual void onISharedPaintEvent_Disconnected( CSharedPaintManager *self )
	{
		setStatus( DISCONNECTED );
	}

	virtual void onISharedPaintEvent_ReceivedPacket( CSharedPaintManager *self )
	{
		if( isActiveWindow() == false )
		{
#ifdef Q_WS_WIN
			::FlashWindow( winId(), TRUE);
#endif
		}
	}

	virtual void onISharedPaintEvent_SendingPacket( CSharedPaintManager *self, int packetId, size_t wroteBytes, size_t totalBytes )
	{
		//qDebug() << "onISharedPaintEvent_SendingPacket" << packetId << wroteBytes << totalBytes;

		if( currPacketId_ != packetId )
		{
			wroteProgressBar_->setRange(0, totalBytes);
			currPacketId_ = packetId;
		}

		wroteProgressBar_->setValue( wroteBytes );

		/*
		// TODO
		ITEM_LIST list = self->findItem( packetId );
		for( size_t i = 0; i < list.size(); i++ )
		{
			list[i]->drawSendingStatus( wroteBytes, totalBytes );
		}
		*/
	}

	virtual void onISharedPaintEvent_AddPaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item )
	{
		item->setCanvas( canvas_ );
		item->draw();

		if( item->type() == PT_TEXT )
		{
			lastTextPosX_ = item->posX();
			lastTextPosY_ = item->posY();
		}

		qDebug() << toolBar_SliderPlayback_->maximum() << toolBar_SliderPlayback_->value() << self->historyTaskCount();
		bool lastPosFlag = false;
		if( toolBar_SliderPlayback_->value() == toolBar_SliderPlayback_->maximum() )
			lastPosFlag = true;
		toolBar_SliderPlayback_->setRange( 0, self->historyTaskCount() );

		playbackSliderFreezingFlag_ = true;
		if( lastPosFlag )
			toolBar_SliderPlayback_->setValue( toolBar_SliderPlayback_->maximum() );
		playbackSliderFreezingFlag_ = false;
	}

	virtual void onISharedPaintEvent_UpdatePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item )
	{
		item->update();
	}

	virtual void onISharedPaintEvent_RemovePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item )
	{
		item->remove();
	}

	virtual void onISharedPaintEvent_MovePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item, double x, double y )
	{
		item->move( x, y );
	}

	virtual void onISharedPaintEvent_ClearScreen( CSharedPaintManager *self )
	{
		setCheckGridLineAction( false );
	}

	virtual void onISharedPaintEvent_SetBackgroundGridLine( CSharedPaintManager *self, int size )
	{
		setCheckGridLineAction( size > 0 );
		canvas_->drawBackgroundGridLine( size );
	}

	virtual void onISharedPaintEvent_SetBackgroundImage( CSharedPaintManager *self, boost::shared_ptr<CBackgroundImageItem> image ) 
	{
		canvas_->drawBackgroundImage( image );
	}

	virtual void onISharedPaintEvent_SetBackgroundColor( CSharedPaintManager *self, int r, int g, int b, int a )
	{
		canvas_->setBackgroundColor( r, g, b, a );
	}

	virtual void onISharedPaintEvent_ClearBackground( CSharedPaintManager *self )
	{
		setCheckGridLineAction( false );
		canvas_->drawBackgroundGridLine( 0 );
		canvas_->clearBackgroundImage();
	}

	virtual void onISharedPaintEvent_ResizeMainWindow( CSharedPaintManager *self, int width, int height )
	{
		resizeFreezingFlag_ = true;
		resize( width, height );
		resizeFreezingFlag_ = false;
	}
	
	virtual void onISharedPaintEvent_UpdatePaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user )
	{
		setStatusBar_JoinerCnt( self->userCount() );
	}

	virtual void onISharedPaintEvent_GetServerInfo( CSharedPaintManager *self, const std::string &broadcastChannel, const std::string &addr, int port )
	{
		if( !self->isConnected() && !self->isServerMode() )
		{
			if( self->isConnecting() )
				return;

			self->connectToPeer( addr, port );
		}
	}
	
	virtual void onISharedPaintEvent_ReceivedTextMessage( CSharedPaintManager *self, const std::string &broadcastChannel, const std::string &message )
	{
		QString msg = QString::fromUtf8( message.c_str(), message.size() );
		showTrayMessage( msg );

		addTextItem( msg, fontBroadCastText_, Util::getComplementaryColor(canvas_->backgroundColor()) );
	}


private:
	Ui::SharedPainterClass ui;

	CSharedPainterScene* canvas_;

	int currPaintItemId_;
	int currPacketId_;
	bool resizeFreezingFlag_;
	bool playbackSliderFreezingFlag_;
	bool screenShotMode_;
	QPoint orgPos_;
	QLabel *broadCastTypeLabel_;
	QLabel *statusBarLabel_;
	QLabel *joinerCountLabel_;
	QAction *penWidthAction_;
	QAction *penModeAction_;
	QAction *gridLineAction_;
	QAction *showLastItemAction_;
	QAction *toolBar_MoveMode_;
	QAction *toolBar_PenMode_;
	QAction *toolBar_GridLine_;
	QSlider *toolBar_SliderPlayback_;
	QPushButton *toolBar_penColorButton_;
	QPushButton *toolBar_bgColorButton_;
	QProgressBar *wroteProgressBar_;
	QTimer *keyHookTimer_;

	double lastTextPosX_;
	double lastTextPosY_;

	Status status_;
	QSystemTrayIcon *trayIcon_;
	QMenu *trayIconMenu_;
	QFont fontBroadCastText_;
};

#endif // SHAREDPAINTER_H
