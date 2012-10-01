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

#ifndef CSHAREDPAINTERSCENE_H
#define CSHAREDPAINTERSCENE_H

#include <QGraphicsScene>
#include "PaintItem.h"

class CSharedPainterScene;

class ICanvasViewEvent
{
public:
	virtual void onICanvasViewEvent_BeginMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item ) = 0;
	virtual void onICanvasViewEvent_EndMove( CSharedPainterScene *view, boost::shared_ptr< CPaintItem > item ) = 0;
	virtual void onICanvasViewEvent_DrawItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void onICanvasViewEvent_UpdateItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void onICanvasViewEvent_RemoveItem( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item ) = 0;
	virtual boost::shared_ptr<CPaintItem> onICanvasViewEvent_FindItem( CSharedPainterScene *view, const std::string &owner, int itemId ) = 0;
	virtual QString onICanvasViewEvent_GetToolTipText( CSharedPainterScene *view, boost::shared_ptr<CPaintItem> item ) = 0;
};

class CSharedPainterScene : public QGraphicsScene, public IGluePaintCanvas
{
	Q_OBJECT

public:
	static const int ZVALUE_NORMAL = 1;
	static const int ZVALUE_TOPMOST	= 9999;

	CSharedPainterScene(void);
	~CSharedPainterScene();

	void setEvent( ICanvasViewEvent *evt )
	{

		eventTarget_ = evt;
	}

	void setCursor( QCursor cursor )
	{
		QList<QGraphicsView *> list = views();
		for (int i = 0; i < list.size(); ++i)
		{
			list.at(i)->setCursor( cursor );
		}
	}

	bool freezeAction( void )
	{
		if( freezeActionFlag_ )
			return false;

		// TODO : CURSOR CHANGE ON DISABLE WIDGET
		// When this view become disabled state, the cursor has changed to default cursor automatically..
		// I don't know how to avoid this problem.
		// SO I call QApplication::setOverrideCursor()...

		//setCursor( QCursor(QPixmap(":/SharedPainter/Resources/draw_disabled.png")) );

		QList<QGraphicsView *> list = views();
		for (int i = 0; i < list.size(); ++i)
		{
			list.at(i)->setEnabled( false );
		}

		freezeActionFlag_ = true;
		return true;
	}

	bool thawAction( void )
	{
		if( ! freezeActionFlag_ )
			return false;

		freezeActionFlag_ = false;

		QList<QGraphicsView *> list = views();
		for (int i = 0; i < list.size(); ++i)
		{
			list.at(i)->setEnabled( true );
		}

		setFreePenMode( freePenMode_ );
		return true;
	}

	void setFreePenMode( bool enable ) 
	{
		freePenMode_ = enable; 

		if( freezeActionFlag_ )
			return;

		if( enable )
		{
			clearSelectedItemState();
			QList<QGraphicsView *> list = views();
			for (int i = 0; i < list.size(); ++i)
			{
				list.at(i)->setDragMode( QGraphicsView::NoDrag );
			}
			setCursor( QCursor(QPixmap(":/SharedPainter/Resources/draw_line.png")) );
		}
		else
		{
			QList<QGraphicsView *> list = views();
			for (int i = 0; i < list.size(); ++i)
			{
				list.at(i)->setDragMode( QGraphicsView::RubberBandDrag );
			}
			setCursor( Qt::PointingHandCursor );
		}
	}
	void setPenSetting( const QColor &clr, int width )
	{
		penClr_ = clr;
		penWidth_ = width;
	}

	bool isFreePenMode( void ) { return freePenMode_; }

	void updateBackground( void )
	{
		resetBackground( sceneRect() );
	}

	int backgroundGridLineSize( void ) { return gridLineSize_; }
	int penWidth( void ) { return penWidth_; }
	const QColor & penColor( void ) { return penClr_; }
	const QColor & backgroundColor( void ) { return backgroundColor_; }

	bool isSettingShowLastAddItemBorder( void ) { return showLastAddItemBorderFlag_; }
	void setSettingShowLastAddItemBorder( bool enable )
	{
		showLastAddItemBorderFlag_ = enable;
		if( enable )
			drawLastItemBorderRect();
		else
			clearLastItemBorderRect();
	}

	void drawLastItemBorderRect( void );
	void clearSelectedItemState( void )
	{
		QList<QGraphicsItem *> list = selectedItems();
		for (int i = 0; i < list.size(); ++i)
		{
			list.at(i)->setSelected( false );
		}
	}

public:
	// IGluePaintCanvas
	virtual QRectF itemBoundingRect( boost::shared_ptr<CPaintItem> item );
	virtual void moveItem( boost::shared_ptr<CPaintItem> item, double x, double y  );
	virtual void updateItem( boost::shared_ptr<CPaintItem> item );
	virtual void removeItem( CPaintItem * item );
	virtual void removeItem( boost::shared_ptr<CPaintItem> item );
	virtual void drawSendingStatus( boost::shared_ptr<CPaintItem> item );
	virtual void drawLine( boost::shared_ptr<CLineItem> line );
	virtual void drawFile( boost::shared_ptr<CFileItem> file );
	virtual void drawText( boost::shared_ptr<CTextItem> text );
	virtual void drawImage( boost::shared_ptr<CImageItem> image );
	virtual void drawImageFile( boost::shared_ptr<CImageFileItem> imageFile );
	virtual void drawBackgroundGridLine( int size );
	virtual void drawBackgroundImage( boost::shared_ptr<CBackgroundImageItem> image );
	virtual void clearBackgroundImage( void );
	virtual void clearScreen( void )
	{
		lastAddItem_ = boost::shared_ptr<CPaintItem>();
		gridLineSize_ = 0;
		backgroundColor_ = Qt::white;
		currentZValue_ = ZVALUE_NORMAL;
		clearLastItemBorderRect();
		clearBackgroundImage();
	}
	virtual void setBackgroundColor( int r, int g, int b, int a );


private slots:
	void sceneRectChanged(const QRectF &rect);
	void onTimer( void );

	// QGraphicsScene
private:
	void mouseMoveEvent( QGraphicsSceneMouseEvent *evt );
	void mousePressEvent( QGraphicsSceneMouseEvent *evt);
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *evt );
	void drawBackground ( QPainter * painter, const QRectF & rect );
	void dragEnterEvent( QGraphicsSceneDragDropEvent * evt );
	void dragLeaveEvent( QGraphicsSceneDragDropEvent * evt );
	void dragMoveEvent( QGraphicsSceneDragDropEvent * evt );
	void dropEvent( QGraphicsSceneDragDropEvent * evt );
	void keyPressEvent( QKeyEvent * evt );
	// for CMyGraphicItem
public:	
	void onItemMoveBegin( boost::shared_ptr< CPaintItem > );
	void onItemMoveEnd( boost::shared_ptr< CPaintItem > );
	void onItemUpdate( boost::shared_ptr< CPaintItem > );
	void onItemRemove( boost::shared_ptr< CPaintItem > );
	void onItemClipboardCopy( boost::shared_ptr< CPaintItem > );

private:
	qreal currentZValue( void )
	{
		currentZValue_ += 0.01;
		return currentZValue_;
	}

	void startBlinkLastItem( void );
	void resetBackground( const QRectF &rect );

	boost::shared_ptr<CPaintItem> findPaintItem( QGraphicsItem *item );
	void clearLastItemBorderRect( void );
	
	void addImageFileItem( const QPointF &pos, const QString &path );
	void addGeneralFileItem( const QPointF &pos, const QString &path );
	void resizeImage(QImage *image, const QSize &newSize);
	void drawLineStart( const QPointF &pt, const QColor &clr, int width );
	void drawLineTo( const QPointF &pt1, const QPointF &pt2, const QColor &clr, int width );
	void setScaleImageFileItem( boost::shared_ptr<CImageFileItem> image, QGraphicsPixmapItem *pixmapItem );
	void commonAddItem( boost::shared_ptr<CPaintItem> item, QGraphicsItem *drawingItem, int borderType );
	void internalDrawGridLine( QPainter *painter, const QRectF &rect, int gridLineSize );

	inline void fireEvent_DrawItem( boost::shared_ptr<CPaintItem> item )
	{
		if(eventTarget_)
			eventTarget_->onICanvasViewEvent_DrawItem( this, item );
	}

private:
	ICanvasViewEvent *eventTarget_;
	QColor penClr_;
	int penWidth_;
	QPointF prevPos_;
	bool freezeActionFlag_;
	bool drawFlag_;
	bool freePenMode_;
	QImage image_;
	QPixmap backgroundPixmap_;

	boost::shared_ptr<CBackgroundImageItem> backgroundImageItem_;
	boost::shared_ptr<CLineItem> currLineItem_;

	std::vector< QGraphicsItem * > tempLineItemList_;

	QFileIconProvider fileIconProvider_;
	qreal currentZValue_;
	qreal currentLineZValue_;
	QColor backgroundColor_;
	int gridLineSize_;

	// blink last item
	QTimer *timer_;
	QGraphicsItem *lastCoverGraphicsItem_;
	int lastTimeValue_;
	int timeoutRemoveLastCoverItem_;
	int lastItemBorderType_;
	boost::shared_ptr<CPaintItem> lastAddItem_;
	bool lastTempBlinkShowFlag_;
	bool showLastAddItemBorderFlag_;
};

#endif // CSHAREDPAINTERSCENE_H
