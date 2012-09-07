#ifndef SHAREDPAINTER_H
#define SHAREDPAINTER_H

#include <QtGui/QMainWindow>
#include "ui_sharedpainter.h"
#include "SharedPainterScene.h"
#include "SharedPaintPolicy.h"

class SharedPainter : public QMainWindow, ICanvasViewEvent, ISharedPaintEvent
{
	Q_OBJECT

public:
	SharedPainter(CSharedPainterScene* canvas, QWidget *parent = 0, Qt::WFlags flags = 0);
	~SharedPainter();

	void setStatusBar_Network( const QString &str )
	{
		//ui.statusBar->showMessage( str );
		statusBarLabel_->setText( str );
	}
	void setStatusBar_BroadCastType( const QString &str )
	{
		//ui.statusBar->showMessage( str );
		broadCastTypeLabel_->setText( str );
	}
	void setStatusBar_JoinerCnt( int count )
	{
		QString str = tr("Joiner Count : ");
		str += QString::number(count);
		joinerCountLabel_->setText( str );
	}

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
	void actionBroadcastChannel( void );
	void actionClearBG( void );
	void actionBGColor( void );
	void actionAddText( void );
	void actionClearScreen( void );
	void actionPenWidth( void );
	void actionPenColor( void );
	void actionPenMode( void );
	void actionFreePenMode( void );
	void actionMoveMode( void );
	void actionExit( void );
	void actionConnect( void );
	void actionScreenShot( void );
	void actionUndo( void );
	void actionServerType( void );
	void actionClientType( void );
	void actionGridLine( void );

private:
	void _requestAddItem( boost::shared_ptr<CPaintItem> item );
	QPointF _calculateTextPos( int textSize );
	bool getBroadcastChannelString( bool force = false );

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
		qDebug() << "@@@@@@@@@@@@@@@@@@@ Connected";
		setStatusBar_Network("Connected");
	}

	virtual void onISharedPaintEvent_ConnectFailed( CSharedPaintManager *self )
	{
		qDebug() << "@@@@@@@@@@@@@@@@@@@ Connect Fail";
		setStatusBar_Network("Connect Failure");
	}

	virtual void onISharedPaintEvent_Disconnected( CSharedPaintManager *self )
	{
		qDebug() << "@@@@@@@@@@@@@@@@@@@ Disconnected";
		setStatusBar_Network("Disconnected");
	}

	virtual void onISharedPaintEvent_ReceivedPacket( CSharedPaintManager *self )
	{
#ifdef Q_WS_WIN
		::FlashWindow( winId(), TRUE);
#endif
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
	}

	virtual void onISharedPaintEvent_UpdatePaintItem( CSharedPaintManager *self, boost::shared_ptr<CPaintItem> item )
	{
		item->update();
	}

	virtual void onISharedPaintEvent_RemovePaintItem( CSharedPaintManager *self, const std::string &owner, int itemId )
	{
		self->removePaintItem( owner, itemId );
	}

	virtual void onISharedPaintEvent_MovePaintItem( CSharedPaintManager *self, const std::string &owner, int itemId, double x, double y )
	{
		boost::shared_ptr<CPaintItem> item = self->findItem( owner, itemId );
		if( item )
		{
			item->move( x, y );
		}
	}

	virtual void onISharedPaintEvent_ClearScreen( CSharedPaintManager *self )
	{
		self->clearAllItems();
	}

	virtual void onISharedPaintEvent_SetBackgroundGridLine( CSharedPaintManager *self, int size )
	{
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
		canvas_->drawBackgroundGridLine( 0 );
		canvas_->setBackgroundColor( 255, 255, 255, 255 );
		canvas_->clearBackgroundImage();
	}

	virtual void onISharedPaintEvent_ResizeMainWindow( CSharedPaintManager *self, int width, int height )
	{
		resizeFreezingFlag_ = true;
		resize( width, height );
		resizeFreezingFlag_ = false;
	}
	
	virtual void onISharedPaintEvent_GetServerInfo( CSharedPaintManager *self, const std::string &broadcastChannel, const std::string &addr, int port )
	{
		if( broadcastChannel != SettingManagerPtr()->broadCastChannel() )
			return;

		if( !self->isConnected() && !self->isServerMode() )
		{
			if( self->isConnecting() )
				return;

			self->connectToPeer( addr, port );
		}
	}

	virtual void onISharedPaintEvent_UpdatePaintUser( CSharedPaintManager *self, boost::shared_ptr<CPaintUser> user )
	{
		setStatusBar_JoinerCnt( self->userCount() );
	}


private:
	Ui::SharedPainterClass ui;

	CSharedPainterScene* canvas_;

	int currPaintItemId_;
	int currPacketId_;
	bool resizeFreezingFlag_;
	bool screenShotMode_;
	QPoint orgPos_;
	QLabel *broadCastTypeLabel_;
	QLabel *statusBarLabel_;
	QLabel *joinerCountLabel_;
	QAction *penModeAction_;
	QAction *toolBar_MoveMode_;
	QAction *toolBar_PenMode_;
	QPushButton *toolBar_penColorButton_;
	QPushButton *toolBar_bgColorButton_;
	QProgressBar *wroteProgressBar_;
	QTimer *keyHookTimer_;
};

#endif // SHAREDPAINTER_H
