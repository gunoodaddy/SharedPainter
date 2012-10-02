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
#include "SharedPainterScene.h"
#include <QMouseEvent>
#include <QDebug>
#include <QColor>
#include <QAbstractGraphicsShapeItem>

#ifndef MAX
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a, b) ( (a) < (b) ? (a) : (b) )
#endif

#define CURSOR_OFFSET_X -10
#define CURSOR_OFFSET_Y 9

#define DEFAULT_TIMEOUT_REMOVE_LAST_COVER_ITEM	2	//sec
#define DEFAULT_COVER_RECT_OFFSET				5

#define ITEM_SCALE_STEP	0.1f
#define ITEM_SCALE_MIN	0.1f
#define ITEM_SCALE_MAX	4.0f

#define ITEM_DATA_KEY_OWNER		0
#define ITEM_DATA_KEY_ITEMID	1

enum ItemBorderType {
	Border_Ellipse,
	Border_PainterPath,
	Border_Rect,
};

template<class T>
class CMyGraphicItem : public T
{
public:
	CMyGraphicItem( CSharedPainterScene *scene ) : scene_(scene) { }

	void setItemData( boost::weak_ptr<CPaintItem> data )
	{
		T::setAcceptHoverEvents( true );

		itemData_ = data;

		if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
		{
			T::setData( ITEM_DATA_KEY_OWNER, QString(r->owner().c_str()) );
			T::setData( ITEM_DATA_KEY_ITEMID, QString::number(r->itemId()) );

			r->setPos( T::scenePos().x(), T::scenePos().y() );
			r->setDrawingObject( this );
		}
	}

	QVariant itemChange( QGraphicsItem::GraphicsItemChange change, const QVariant & value )
	{
		if( change == QGraphicsItem::ItemPositionHasChanged )
		{
			if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
			{
				QPointF newPos = value.toPointF();

				scene_->onItemMoving( r, newPos );
				return newPos;
			}
		}
		return QGraphicsItem::itemChange(change, value);
	}

	void keyPressEvent( QKeyEvent * event )
	{
		if( event->key() == Qt::Key_Delete )
		{
			if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
			{
				scene_->onItemRemove( r );
			}
		}
		else if ( event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier )
		{
			if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
			{
				scene_->onItemClipboardCopy( r );
			}
		}

		T::keyPressEvent( event );
	}

	void hoverEnterEvent( QGraphicsSceneHoverEvent * event )
	{
		if( scene_->isFreePenMode() )
			return;

		scene_->setCursor( Qt::OpenHandCursor );
	}

	void hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
	{
		if( scene_->isFreePenMode() )
			return;

		scene_->setCursor( Qt::PointingHandCursor );
	}

	void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event )
	{
		if( scene_->isFreePenMode() )
			return;

		if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
		{
			r->execute();
		}
		T::mouseDoubleClickEvent( event );
	}

	void mousePressEvent( QGraphicsSceneMouseEvent * event )
	{
		scene_->setCursor( Qt::ClosedHandCursor );

		T::mousePressEvent( event );
	}

	void mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
	{
		scene_->setCursor( Qt::OpenHandCursor );

		T::mouseReleaseEvent( event );
	}

	void wheelEvent( QGraphicsSceneWheelEvent * event )
	{
		if( scene_->isFreePenMode() )
			return;

		if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
		{
			double sc = r->scale();
			if( event->delta() < 0 )
				sc -= ITEM_SCALE_STEP;
			else
				sc += ITEM_SCALE_STEP;

			if( sc > ITEM_SCALE_MAX )
				return;
			if( sc < ITEM_SCALE_MIN )
				return;

			r->setScale( sc );
			scene_->onItemUpdate( r );
		}
	}

private:
	CSharedPainterScene *scene_;
	boost::weak_ptr<CPaintItem> itemData_;
};


CSharedPainterScene::CSharedPainterScene(void )
: eventTarget_(NULL), freezeActionFlag_(false), drawFlag_(false), freePenMode_(false)
, hiqhQualityMoveItemMode_(false)
, currentZValue_(ZVALUE_NORMAL), gridLineSize_(0)
, lastCoverGraphicsItem_(NULL), timeoutRemoveLastCoverItem_(0), lastTempBlinkShowFlag_(false), showLastAddItemBorderFlag_(false)
{
	backgroundColor_ = Qt::white;
	penClr_ = Qt::blue;
	penWidth_ = 2;

	connect(this, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(sceneRectChanged(const QRectF &)));

	// Timer Setting
	timer_ = new QTimer(this);
	timer_->start(200);
	connect(timer_, SIGNAL(timeout()),this, SLOT(onTimer()));
}

CSharedPainterScene::~CSharedPainterScene()
{

}

void CSharedPainterScene::onTimer( void )
{
	if( NULL == lastCoverGraphicsItem_ || timeoutRemoveLastCoverItem_ <= 0 )
		return;

	int now = time(NULL);
	int elapsedSec =  now - lastTimeValue_;
	timeoutRemoveLastCoverItem_ -= elapsedSec;

	if( timeoutRemoveLastCoverItem_ > 0)
	{
		if( lastTempBlinkShowFlag_ )
			lastCoverGraphicsItem_->show();
		else
			lastCoverGraphicsItem_->hide();

		lastTempBlinkShowFlag_ = !lastTempBlinkShowFlag_;
	}

	if( timeoutRemoveLastCoverItem_ <= 0 )
	{
		clearLastItemBorderRect();
	}

	lastTimeValue_ = now;
}

void CSharedPainterScene::sceneRectChanged(const QRectF &rect)
{
	//qDebug() << "sceneRectChanged" << rect;
	resetBackground( rect );
}

void CSharedPainterScene::internalDrawGridLine( QPainter *painter, const QRectF &rect, int gridLineSize )
{
	painter->setPen(QPen(QColor(102, 102, 102), 2, Qt::SolidLine));

	int w = rect.width();
	int h = rect.height();

	// vertical line
	int x = gridLineSize;
	for( ; x < w; x += gridLineSize )
	{
		painter->drawLine( x, 0, x, h);
	}
	
	// horizontal line
	int y = gridLineSize;
	for( ; y < h; y += gridLineSize )
	{
		painter->drawLine( 0, y, w, y);
	}
}

boost::shared_ptr<CPaintItem> CSharedPainterScene::findPaintItem( QGraphicsItem *item )
{
	if( ! item )
		return boost::shared_ptr<CPaintItem>();

	std::string owner = item->data( ITEM_DATA_KEY_OWNER ).toString().toStdString();
	int itemId = item->data( ITEM_DATA_KEY_ITEMID ).toInt();

	boost::shared_ptr<CPaintItem> paintItem = eventTarget_->onICanvasViewEvent_FindItem( this, owner, itemId );
	return paintItem;
}


void CSharedPainterScene::resetBackground( const QRectF &rect )
{
	QImage newImage(rect.toRect().size(), QImage::Format_RGB32);
    newImage.fill(backgroundColor_);
    
	QPainter painter(&newImage);

	// draw image
	if( ! backgroundPixmap_.isNull() )
		painter.drawPixmap(QPoint(0, 0), backgroundPixmap_);

	// draw grid line
	if( gridLineSize_ > 0 )
	{
		internalDrawGridLine( &painter, rect, gridLineSize_ );
	}

	image_ = newImage;	

	invalidate( QRectF(), QGraphicsScene::BackgroundLayer );
}

void CSharedPainterScene::setScaleImageFileItem( boost::shared_ptr<CImageFileItem> image, QGraphicsPixmapItem *pixmapItem )
{
	QPixmap pixmap( image->path() );

	// basic size
	int newW = pixmap.width();
	int newH = pixmap.height();
	if( pixmap.width() > DEFAULT_PIXMAP_ITEM_SIZE_W )
	{
		newW = DEFAULT_PIXMAP_ITEM_SIZE_W;
		newH = (pixmap.height() * DEFAULT_PIXMAP_ITEM_SIZE_W) / pixmap.width();
	}

	newW *= image->scale();
	newH *= image->scale();

	pixmap = pixmap.scaled( newW, newH );
	pixmapItem->setPixmap( pixmap ); 
}

static QPainterPath createCoveringBorderPath( int borderType, QGraphicsItem *item )
{
	QRectF res = item->boundingRect();
	res = item->mapRectToScene ( res );

	double left = res.x();
	double top = res.y();
	double w = res.width();
	double h = res.height();

	res.setLeft( left - DEFAULT_COVER_RECT_OFFSET );
	res.setRight( left + w + DEFAULT_COVER_RECT_OFFSET );
	res.setTop( top - DEFAULT_COVER_RECT_OFFSET );
	res.setBottom( top + h + DEFAULT_COVER_RECT_OFFSET );

	QPainterPath path;
	path.addRoundedRect( res, 5, 5 );

	return path;
}

void CSharedPainterScene::clearLastItemBorderRect( void )
{
	if( lastCoverGraphicsItem_ )
	{
		QGraphicsScene::removeItem( lastCoverGraphicsItem_ );
		lastCoverGraphicsItem_ = NULL;
	}
	timeoutRemoveLastCoverItem_ = 0;
}

void CSharedPainterScene::drawLastItemBorderRect( void  )
{
	if( ! lastAddItem_ )
		return;

	if( ! lastAddItem_->drawingObject() )
		return;

	QAbstractGraphicsShapeItem* i = reinterpret_cast<QAbstractGraphicsShapeItem *>(lastAddItem_->drawingObject());
	if( ! i )
		return;

	// setting style..
	QPainterPath path = createCoveringBorderPath( lastItemBorderType_, i );
	if ( path.isEmpty() )
		return;

	clearLastItemBorderRect();

	QAbstractGraphicsShapeItem* lastBorderItem = addPath( path );
	lastBorderItem->setPen( QPen( Util::getComplementaryColor(backgroundColor_, penColor() ), 2) );
	lastBorderItem->setZValue( currentZValue() );
	lastCoverGraphicsItem_ = lastBorderItem;

	// clear
	lastTempBlinkShowFlag_ = true;
	lastTimeValue_ = time(NULL);
	timeoutRemoveLastCoverItem_ = DEFAULT_TIMEOUT_REMOVE_LAST_COVER_ITEM;
}

void CSharedPainterScene::startBlinkLastItem( void )
{
	if( !lastAddItem_ )
		return;

	if( lastAddItem_->type() == PT_LINE && lastAddItem_->isMyItem() )
	{
		// NOTHING TO DO..
	}
	else
	{
		clearLastItemBorderRect();

		if( showLastAddItemBorderFlag_ )
			drawLastItemBorderRect();
	}
}

void CSharedPainterScene::commonAddItem( boost::shared_ptr<CPaintItem> item, QGraphicsItem* drawingItem, int borderType )
{
	clearSelectedItemState();

	drawingItem->setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges );
	addItem( drawingItem );

	QString tooltip = eventTarget_->onICanvasViewEvent_GetToolTipText( this, item );
	if( tooltip.isEmpty() == false )
		drawingItem->setToolTip( tooltip );

	lastItemBorderType_ = borderType;
	lastAddItem_ = item;

	// Blink last item feature
	startBlinkLastItem();
}


QRectF CSharedPainterScene::itemBoundingRect( boost::shared_ptr<CPaintItem> item )
{
	if( ! item )
		return QRectF();

	if( ! item->drawingObject() )
		return QRectF();

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());
	return i->boundingRect();
}

void CSharedPainterScene::removeItem( CPaintItem * item )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	if( lastAddItem_.get() == item )
		clearLastItemBorderRect();

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());
	QGraphicsScene::removeItem( i );

	invalidate( i->boundingRect() );
}

void CSharedPainterScene::removeItem( boost::shared_ptr<CPaintItem> item )
{
	removeItem( item.get() );
}

void CSharedPainterScene::updateItem( boost::shared_ptr<CPaintItem> item )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	clearLastItemBorderRect();

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());

	if( item->isScalable() )
	{
		if( item->type() == PT_IMAGE_FILE )
			setScaleImageFileItem( boost::static_pointer_cast<CImageFileItem>(item), (QGraphicsPixmapItem *)i );
		else
			i->setScale( item->scale() );
	}
	i->setPos( item->posX(), item->posY() );
}

void CSharedPainterScene::moveItem( boost::shared_ptr<CPaintItem> item, double x, double y  )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	clearLastItemBorderRect();

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());

	// freeze move changes notify
	i->setFlag( QGraphicsItem::ItemSendsGeometryChanges, false );

	i->setPos( x, y );
	
	// thaw move changes notify
	i->setFlag( QGraphicsItem::ItemSendsGeometryChanges, true );
	invalidate( i->boundingRect() );
}


void CSharedPainterScene::drawSendingStatus( boost::shared_ptr<CPaintItem> item )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());
	Q_UNUSED(i);
	//qDebug() << "drawSendingStatus" << item->wroteBytes() << item->totalBytes();
	// TODO : SENDING PROGRESS
	// How make progress bar and handle it??
	// ....
}


void CSharedPainterScene::drawBackgroundGridLine( int size )
{
	gridLineSize_ = size;

	resetBackground( sceneRect() );
}


void CSharedPainterScene::drawFile( boost::shared_ptr<CFileItem> file )
{
	QFileInfo info( file->path() );
	QIcon icon = fileIconProvider_.icon(info);

	QPixmap pixmap = icon.pixmap(9999, 9999);
	CMyGraphicItem<QGraphicsPixmapItem> *item = new CMyGraphicItem<QGraphicsPixmapItem>( this );
	item->setPixmap( pixmap ); 
	if( file->isAvailablePosition() )
		item->setPos( file->posX(), file->posY() );
	item->setItemData( file );
	item->setZValue( ZVALUE_TOPMOST );
	commonAddItem( file, item, Border_Rect );
}

void CSharedPainterScene::drawImage( boost::shared_ptr<CImageItem> image )
{
	CMyGraphicItem<QGraphicsPixmapItem> *item = new CMyGraphicItem<QGraphicsPixmapItem>( this );
	item->setPixmap( image->createPixmap() ); 

	if( image->isAvailablePosition() )
		item->setPos( image->posX(), image->posY() );
	item->setItemData( image );
	item->setZValue( currentZValue() );
	commonAddItem( image, item, Border_Rect );
}

void CSharedPainterScene::drawImageFile( boost::shared_ptr<CImageFileItem> imageFile )
{
	CMyGraphicItem<QGraphicsPixmapItem> *item = new CMyGraphicItem<QGraphicsPixmapItem>( this );

	setScaleImageFileItem( imageFile, item );

	if( imageFile->isAvailablePosition() )
		item->setPos( imageFile->posX(), imageFile->posY() );
	item->setItemData( imageFile );
	item->setZValue( currentZValue() );
	commonAddItem( imageFile, item, Border_Rect );
}

void CSharedPainterScene::drawText( boost::shared_ptr<CTextItem> text )
{
	CMyGraphicItem<QGraphicsSimpleTextItem> *item = new CMyGraphicItem<QGraphicsSimpleTextItem>( this );
	item->setText( text->text() );
	item->setFont( text->font() );
	if( text->isAvailablePosition() )
		item->setPos( text->posX(), text->posY() );
	item->setItemData( text );
	item->setBrush ( QBrush(text->color()) );
	item->setZValue( currentZValue() );
	commonAddItem( text, item, Border_Rect );
}


void CSharedPainterScene::drawLine( boost::shared_ptr<CLineItem> line )
{
	if( line->pointCount() <= 0 )
		return;

	QPainterPath painterPath;

	painterPath.moveTo( *line->point( 0 ) );
	if( line->pointCount() > 1 )
	{
		for( size_t i = 1; i < line->pointCount(); i++ )
		{
			painterPath.lineTo( *line->point( i ) );
		}

		CMyGraphicItem<QGraphicsPathItem> *pathItem = new CMyGraphicItem<QGraphicsPathItem>( this );
		if( line->isAvailablePosition() )
			pathItem->setPos( line->posX(), line->posY() );
		pathItem->setPath( painterPath );
		pathItem->setPen( QPen(line->color(), line->width(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
		pathItem->setZValue( currentZValue() );
		pathItem->setItemData( line );
		commonAddItem( line, pathItem, Border_PainterPath );
	}
	else
	{
		QPointF to = *line->point( 0 );

		double x = to.x() - (line->width() / 2);
		double y = to.y() - (line->width() / 2);
		QRectF rect( x, y, line->width(), line->width() );

		CMyGraphicItem<QGraphicsEllipseItem> *ellipseItem = new CMyGraphicItem<QGraphicsEllipseItem>( this );
		if( line->isAvailablePosition() )
			ellipseItem->setPos( line->posX(), line->posY() );
		ellipseItem->setRect( rect );
		ellipseItem->setPen( QPen(line->color(), 1) );
		ellipseItem->setBrush( QBrush(line->color()) );
		ellipseItem->setZValue( currentZValue() );
		ellipseItem->setItemData( line );
		commonAddItem( line, ellipseItem, Border_Ellipse );
	}
}


void CSharedPainterScene::setBackgroundColor( int r, int g, int b, int a )
{
	backgroundColor_ = QColor(r, g, b, a);
	resetBackground( sceneRect () );	
}

void CSharedPainterScene::clearBackgroundImage( void )
{
	backgroundImageItem_ = boost::shared_ptr<CBackgroundImageItem>();

	backgroundPixmap_ = QPixmap();

	resetBackground( sceneRect() );
}


void CSharedPainterScene::drawBackgroundImage( boost::shared_ptr<CBackgroundImageItem> image )
{
	backgroundImageItem_ = image;

	if( image )
	{
		backgroundPixmap_ = image->createPixmap();

		int newSceneW = sceneRect().width();
		int newSceneH = sceneRect().height();

		QSize size = backgroundPixmap_.size();
		if( newSceneW < size.width() )
			newSceneW = size.width();
		if( newSceneH < size.height() )
			newSceneH = size.height();
		setSceneRect( 0, 0, newSceneW, newSceneH );
	}
	else
	{
		clearBackgroundImage();
		return;
	}

	resetBackground( sceneRect () );
}

void CSharedPainterScene::drawLineStart( const QPointF &pt, const QColor &clr, int width )
{
	double x = pt.x() - (double(width) / 2.f);
	double y = pt.y() - (double(width) / 2.f);
	QRectF rect( x, y, width, width);

	QGraphicsEllipseItem *item = addEllipse( rect, QPen(clr, 1), QBrush(clr) );
	item->setZValue( currentZValue() );

	tempLineItemList_.push_back( item );
}

void CSharedPainterScene::drawLineTo( const QPointF &pt1, const QPointF &pt2, const QColor &clr, int width )
{
	QGraphicsLineItem *item = addLine( pt1.x(), pt1.y(), pt2.x(), pt2.y(), QPen(clr, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
	item->setZValue( currentLineZValue_ );

	tempLineItemList_.push_back( item );
}


void CSharedPainterScene::resizeImage(QImage *image, const QSize &newSize)
{
     if (image->size() == newSize)
         return;

     QImage newImage(newSize, QImage::Format_RGB32);
     newImage.fill(qRgb(255, 255, 255));
   
	 QPainter painter(&newImage);
     painter.drawImage(QPoint(0, 0), *image);
     *image = newImage;
}

void CSharedPainterScene::addImageFileItem( const QPointF &pos, const QString &path )
{
	boost::shared_ptr<CImageFileItem> imageItem = boost::shared_ptr<CImageFileItem>(new CImageFileItem( path ));
	imageItem->setMyItem();
	imageItem->setPos( pos.x(), pos.y() );

	fireEvent_DrawItem( imageItem );
}

void CSharedPainterScene::addGeneralFileItem( const QPointF &pos, const QString &path )
{
	boost::shared_ptr<CFileItem> fileItem = boost::shared_ptr<CFileItem>(new CFileItem( path ));
	fileItem->setMyItem();
	fileItem->setPos( pos.x(), pos.y() );

	fireEvent_DrawItem( fileItem );
}

void CSharedPainterScene::drawBackground ( QPainter * painter, const QRectF & rect )
{
	//qDebug() << "drawBackground" << rect;
	painter->drawImage(rect, image_, rect);
}


void CSharedPainterScene::doLowQualityMoveItems( void )
{
	if( tempMovingItemList_.size() <= 0 )
		return;

	ITEM_SET::iterator it = tempMovingItemList_.begin();
	for( ; it != tempMovingItemList_.end(); it++ )
	{
		boost::shared_ptr<CPaintItem> item = *it;

		QAbstractGraphicsShapeItem* i = reinterpret_cast<QAbstractGraphicsShapeItem *>(item->drawingObject());
		if( ! i )
			continue;

		item->setPos( i->pos().x(), i->pos().y() );
		eventTarget_->onICanvasViewEvent_MoveItem( this, item );
	}

	tempMovingItemList_.clear();
}

void CSharedPainterScene::onItemMoving(boost::shared_ptr< CPaintItem > item, const QPointF & newPos)
{
	if( hiqhQualityMoveItemMode_ )
	{
		item->setPos( newPos.x(), newPos.y() );
		eventTarget_->onICanvasViewEvent_MoveItem( this, item );
	}
	else
	{
		tempMovingItemList_.insert( item );
	}
}

void CSharedPainterScene::onItemUpdate( boost::shared_ptr< CPaintItem > item )
{
	eventTarget_->onICanvasViewEvent_UpdateItem( this, item );
}

void CSharedPainterScene::onItemClipboardCopy( boost::shared_ptr< CPaintItem > item )
{
	item->copyToClipboard();
}

void CSharedPainterScene::onItemRemove( boost::shared_ptr< CPaintItem > item )
{
	eventTarget_->onICanvasViewEvent_RemoveItem( this, item );
}

void CSharedPainterScene::keyPressEvent( QKeyEvent * evt )
{
	if ( evt->key() == Qt::Key_A && evt->modifiers() == Qt::ControlModifier )
	{
		QList<QGraphicsItem *> list = items();
		for (int i = 0; i < list.size(); ++i)
		{
			list.at(i)->setSelected( true );
		}
	}
	else if ( evt->key() == Qt::Key_C && evt->modifiers() == Qt::ControlModifier )
	{
		QList<QGraphicsItem *> list = selectedItems();
		for (int i = 0; i < list.size(); ++i)
		{
			boost::shared_ptr<CPaintItem> paintItem = findPaintItem( list.at(i) );
			if( paintItem )
			{
				paintItem->copyToClipboard( i == 0 ? true : false );
			}
		}
	}
	else if ( evt->key() == Qt::Key_Delete )
	{
		QList<QGraphicsItem *> list = selectedItems();
		for (int i = 0; i < list.size(); ++i)
		{
			boost::shared_ptr<CPaintItem> paintItem = findPaintItem( list.at(i) );
			if( paintItem )
			{
				eventTarget_->onICanvasViewEvent_RemoveItem( this, paintItem );
			}
		}
	}
	QGraphicsScene::keyPressEvent( evt );
}

void CSharedPainterScene::dragEnterEvent( QGraphicsSceneDragDropEvent * evt )
{
	int cnt = 0;
	const QMimeData* mimeData = evt->mimeData();

	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList = mimeData->urls();
		for (int i = 0; i < urlList.size(); ++i)
		{
			QString path = urlList.at(i).toLocalFile();
			if( !QFileInfo(path).isFile() )
				continue;
			cnt++;
		}
	}
	if( cnt > 0 )
		evt->acceptProposedAction();
}

void CSharedPainterScene::dragLeaveEvent( QGraphicsSceneDragDropEvent * evt )
{
	evt->accept();
}

void CSharedPainterScene::dragMoveEvent( QGraphicsSceneDragDropEvent * evt )
{
	evt->acceptProposedAction();
}

void CSharedPainterScene::dropEvent( QGraphicsSceneDragDropEvent * evt )
{
	const QMimeData* mimeData = evt->mimeData();

	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size(); ++i)
		{
			QString path = urlList.at(i).toLocalFile();
			bool isImageFile = false;

			if( !QFileInfo(path).isFile() )
				continue;

			// TODO : Image check, is this the fastest way to check it?
			isImageFile = (QImage(path).isNull() == false);

			if( isImageFile )
				addImageFileItem( evt->scenePos(), path );
			else
				addGeneralFileItem( evt->scenePos(), path );
		}
	}
}


void CSharedPainterScene::mousePressEvent( QGraphicsSceneMouseEvent *evt )
{
	if( !freePenMode_)
	{
		QGraphicsScene::mousePressEvent( evt );
		return;
	}

	if( !drawFlag_ )
	{
		prevPos_ = evt->scenePos();
		prevPos_.setX( prevPos_.x() + CURSOR_OFFSET_X );
		prevPos_.setY( prevPos_.y() + CURSOR_OFFSET_Y );
		drawFlag_ = true;

		currLineItem_ = boost::shared_ptr<CLineItem>(new CLineItem( penClr_, penWidth_ ) );
		currLineItem_->addPoint( prevPos_ );
		currLineItem_->setMyItem();

		currentLineZValue_ = currentZValue();

		drawLineStart( prevPos_, currLineItem_->color(), currLineItem_->width() );
	}
}


void CSharedPainterScene::mouseMoveEvent( QGraphicsSceneMouseEvent *evt )
{
	if( !freePenMode_)
	{
		QGraphicsScene::mouseMoveEvent( evt );
		return;
	}

	if(!drawFlag_)
		return;

	QPointF to = evt->scenePos();
	to.setX( to.x() + CURSOR_OFFSET_X );
	to.setY( to.y() + CURSOR_OFFSET_Y );

	if( currLineItem_ )
	{
		drawLineTo( prevPos_, to, currLineItem_->color(), currLineItem_->width() );
		currLineItem_->addPoint( to );
	}

	prevPos_ = to;
}

void CSharedPainterScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *evt )
{
	if( !freePenMode_)
	{
		doLowQualityMoveItems();

		QGraphicsScene::mouseReleaseEvent( evt );
		return;
	}

	if( drawFlag_ )
	{
		drawFlag_ = false;

		fireEvent_DrawItem( currLineItem_ );

		currLineItem_ = boost::shared_ptr<CLineItem>();

		for( size_t i = 0; i < tempLineItemList_.size(); i++ )
		{
			QGraphicsScene::removeItem( tempLineItemList_[i] );
		}
		tempLineItemList_.clear();

		resetBackground( sceneRect () );
	}
}
