#include "StdAfx.h"
#include "SharedPainterScene.h"
#include <QMouseEvent>
#include <QDebug>
#include <QColor>
#include <QAbstractGraphicsShapeItem>

template<class T>
class CMyGraphicItem : public T
{
public:
	CMyGraphicItem( CSharedPainterScene *scene ) : scene_(scene), moveFlag_(false) { }

	void setItemData( boost::weak_ptr<CPaintItem> data )
	{
		setAcceptHoverEvents( true );

		itemData_ = data;
	
		if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
		{
			r->setPos( scenePos().x(), scenePos().y() );
			r->setDrawingObject( this );
		}
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
		T::keyPressEvent( event );
	}

	void hoverEnterEvent( QGraphicsSceneHoverEvent * event )
	{
		scene_->setCursor( Qt::OpenHandCursor ); 
	}

	void hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
	{
		scene_->setCursor( Qt::PointingHandCursor ); 
	}

	//void hoverMoveEvent( QGraphicsSceneHoverEvent * event )
	//{
	//}

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

	void mouseMoveEvent( QGraphicsSceneMouseEvent * event ) 
	{
		if( !moveFlag_ )
		{
			if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
			{
				r->setPos( scenePos().x(), scenePos().y() );
				scene_->onItemMoveBegin( r );
			}
			moveFlag_ = true;
		}

		T::mouseMoveEvent( event );
	}

	void mousePressEvent( QGraphicsSceneMouseEvent * event ) 
	{
		scene_->setCursor( Qt::ClosedHandCursor ); 

		T::mousePressEvent( event );
	}

	void mouseReleaseEvent( QGraphicsSceneMouseEvent * event ) 
	{
		scene_->setCursor( Qt::OpenHandCursor ); 

		moveFlag_ = false;
		if( boost::shared_ptr<CPaintItem> r = itemData_.lock() )
		{
			r->setPos( scenePos().x(), scenePos().y() );
			scene_->onItemMoveEnd( r );
		}

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
				sc -= 0.5;
			else
				sc += 0.5;

			if( sc > 3 )
				return;
			if( sc < 0.3 )
				return;

			r->setScale( sc );
			scene_->onItemUpdate( r );
		}
	}

private:
	bool moveFlag_;
	CSharedPainterScene *scene_;
	boost::weak_ptr<CPaintItem> itemData_;
};


CSharedPainterScene::CSharedPainterScene(void )
: eventTarget_(NULL), drawFlag_(false), freePenMode_(false), currentZValue_(ZVALUE_NORMAL), gridLineSize_(0)
{
	backgroundColor_ = Qt::white;
	penClr_ = Qt::blue;
	penWidth_ = 2;

	connect(this, SIGNAL(sceneRectChanged(const QRectF &)), this, SLOT(sceneRectChanged(const QRectF &)));
}

CSharedPainterScene::~CSharedPainterScene()
{

}

void CSharedPainterScene::sceneRectChanged(const QRectF &rect)
{
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

void CSharedPainterScene::updateItem( boost::shared_ptr<CPaintItem> item )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

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


void CSharedPainterScene::commonAddItem( QGraphicsItem *item )
{
	item->setFlags( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable );
	addItem( item );
}


void CSharedPainterScene::removeItem( CPaintItem * item )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());
	QGraphicsScene::removeItem( i );

	invalidate( i->boundingRect() );
}

void CSharedPainterScene::removeItem( boost::shared_ptr<CPaintItem> item )
{
	removeItem( item.get() );
}


void CSharedPainterScene::moveItem( boost::shared_ptr<CPaintItem> item, double x, double y  )
{
	if( ! item )
		return;

	if( ! item->drawingObject() )
		return;

	QGraphicsItem* i = reinterpret_cast<QGraphicsItem *>(item->drawingObject());
	i->setPos( x, y );
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
	// TODO
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
	commonAddItem( item );
}

void CSharedPainterScene::drawImage( boost::shared_ptr<CImageFileItem> image )
{
	CMyGraphicItem<QGraphicsPixmapItem> *item = new CMyGraphicItem<QGraphicsPixmapItem>( this );

	setScaleImageFileItem( image, item );

	if( image->isAvailablePosition() )
		item->setPos( image->posX(), image->posY() );
	item->setItemData( image );
	item->setZValue( currentZValue() );
	commonAddItem( item );
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
	commonAddItem( item );
}


void CSharedPainterScene::drawLine( boost::shared_ptr<CLineItem> line )
{
	if( line->pointSize() <= 0 )
		return;

	QRectF invalidateRect;
	QPainterPath painterPath;

	painterPath.moveTo( *line->point( 0 ) );
	if( line->pointSize() > 1 )
	{
		for( size_t i = 1; i < line->pointSize(); i++ )
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
		commonAddItem( pathItem );

		//invalidateRect = pathItem->boundingRect();
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
		commonAddItem( ellipseItem );

		//invalidateRect = ellipseItem->boundingRect();
	}

	//invalidate( invalidateRect );
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

	resetBackground( sceneRect () );
}


void CSharedPainterScene::drawBackgroundImage( boost::shared_ptr<CBackgroundImageItem> image )
{
	backgroundImageItem_ = image;

	if( image )
		backgroundPixmap_ = image->createPixmap();
	else
	{
		clearBackgroundImage();
		return;
	}

	resetBackground( sceneRect () );
}

void CSharedPainterScene::drawLineStart( const QPointF &pt, const QColor &clr, int width )
{
	double x = pt.x() - (width / 2);
	double y = pt.y() - (width / 2);
	QRectF rect( x, y, width, width );

	QGraphicsEllipseItem *item = addEllipse( rect, QPen(clr, 1), QBrush(clr) );
	item->setZValue( currentZValue() );

	tempLineItemList_.push_back( item );
}

void CSharedPainterScene::drawLineTo( const QPointF &pt1, const QPointF &pt2, const QColor &clr, int width )
{
	QGraphicsLineItem *item = addLine( pt1.x(), pt1.y(), pt2.x(), pt2.y(), QPen(clr, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
	item->setZValue( currentLineZValue_ );

	tempLineItemList_.push_back( item );

	/*	QPainter painter(&image_);
	painter.setPen(QPen(clr, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter.drawLine(pt1, pt2);

	int rad = (width / 2) + 2;
	invalidate( QRectF(pt1, pt2).normalized().adjusted(-rad, -rad, +rad, +rad), QGraphicsScene::BackgroundLayer);
	*/
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

void CSharedPainterScene::onItemMoveBegin( boost::shared_ptr< CPaintItem > item)
{
	eventTarget_->onICanvasViewEvent_BeginMove( this, item );
}

void CSharedPainterScene::onItemMoveEnd( boost::shared_ptr< CPaintItem > item )
{
	//qDebug() << "onItemMovingEnd" << item->posX() << item->posY();
	eventTarget_->onICanvasViewEvent_EndMove( this, item );
}

void CSharedPainterScene::onItemUpdate( boost::shared_ptr< CPaintItem > item )
{
	eventTarget_->onICanvasViewEvent_UpdateItem( this, item );
}

void CSharedPainterScene::onItemRemove( boost::shared_ptr< CPaintItem > item )
{
	eventTarget_->onICanvasViewEvent_RemoveItem( this, item );
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

			// TODO : is this the fastest way to check it?
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
		drawFlag_ = true;

		currLineItem_ = boost::shared_ptr<CLineItem>(new CLineItem( penClr_, penWidth_ ) );
		currLineItem_->addPoint( evt->scenePos() );
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

	if( currLineItem_ )
	{
		drawLineTo( prevPos_, evt->scenePos(), currLineItem_->color(), currLineItem_->width() );

		currLineItem_->addPoint( evt->scenePos() );
	}

	prevPos_ = evt->scenePos();
}


void CSharedPainterScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *evt )
{
	if( !freePenMode_)
	{
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
