#pragma once

#include "PacketBuffer.h"
#include <boost/enable_shared_from_this.hpp>

class CPaintItem;
class CLineItem;
class CBackgroundImageItem;
class CFileItem;
class CTextItem;
class CImageFileItem;

static void HexDump(const void *ptr, int buflen)
{
	unsigned char *buf = (unsigned char*)ptr;
	int i,j;
	char line[1024];
	for (i=0; i<buflen; i+=16)
	{
		sprintf(line ,"%06x: ", i);
		
		for (j=0; j<16; j++) 
			if (i+j < buflen)
				sprintf(line, "%s%02x ", line, buf[i+j]);
			else
				strcat(line, "   ");
		
		strcat(line, " ");
		
		for (j=0; j<16; j++) 
			if (i+j < buflen)
				sprintf(line, "%s%c", line, isprint(buf[i+j]) ? buf[i+j] : '.');
		
		strcat(line, "\n");
		
		qDebug() << line;
	}
}

static QString generateFileDownloadPath( const QString *path = 0 )
{
	QString res;
	if( path )
		res += *path;
	else
		res = QDir::currentPath();

	res += QDir::separator();
	res += "Download";
	res += QDir::separator();

	QDir dir( res );
	if ( !dir.exists() )
		dir.mkpath( res );
	return res;
}

static std::string toUtf8StdString( const QString &str )
{
	std::string res;

	QByteArray a = str.toUtf8();

	res.assign( a.data(), a.size() );

	return res;
}

enum PaintItemType {
	PT_LINE = 0,
	PT_BG_IMAGE,
	PT_FILE,
	PT_IMAGE_FILE,
	PT_TEXT,
	PT_MAX 
};

typedef std::vector< boost::shared_ptr<CPaintItem> > ITEM_LIST;

class IGluePaintCanvas
{
public:
	virtual void drawSendingStatus( boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void drawLine( boost::shared_ptr<CLineItem> line ) = 0;
	virtual void drawFile( boost::shared_ptr<CFileItem> file ) = 0;
	virtual void drawText( boost::shared_ptr<CTextItem> text ) = 0;
	virtual void drawImage( boost::shared_ptr<CImageFileItem> image ) = 0;
	virtual void removeItem( CPaintItem * item ) = 0;
	virtual void removeItem( boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void moveItem( boost::shared_ptr<CPaintItem> item, double x, double y ) = 0;
	virtual void updateItem( boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void clearBackgroundImage( void ) = 0;
	virtual void clearScreen( void ) = 0;
	virtual void setBackgroundColor( int r, int g, int b, int a ) = 0;
	virtual void drawBackgroundGridLine( int size ) = 0;
	virtual void drawBackgroundImage( boost::shared_ptr<CBackgroundImageItem> line ) = 0;
};

struct SPaintData
{
	double posX;
	double posY;
	double scale;
	bool posSetFlag;
	int itemId;
	std::string owner;
};

class CPaintItem : public boost::enable_shared_from_this<CPaintItem> 
{
public:
	CPaintItem( void ) : canvas_(NULL)
		, object_(NULL), mine_(false)
		, packetId_(-1), wroteBytes_(0), totalBytes_(0) 
	{
		data_.posX = 0.f;
		data_.posY = 0.f;
		data_.scale = 1.f;
		data_.itemId = 0;
		data_.posSetFlag = false;

		prevData_ = data_;
	}
	virtual ~CPaintItem( void ) 
	{ 
		qDebug() << "CPaintItem deleted.. " << this; 
		remove(); 
	}

	void setCanvas( IGluePaintCanvas *canvas ) { canvas_ = canvas; }

	void setDrawingObject( void * obj ) { object_ = obj; }
	void * drawingObject( void ) { return object_; }

	struct SPaintData &data( void) { return data_; }
	struct SPaintData &prevData( void) { return prevData_; }
	
	void setData( const struct SPaintData &data ) { prevData_ = data_; data_ = data; }

	bool isAvailablePosition( void ) { return data_.posSetFlag; }
	double posX( void ) { return data_.posX; }
	double posY( void ) { return data_.posY; }
	
	void setPos( double x, double y ) 
	{
		prevData_.posX = data_.posX;
		prevData_.posY = data_.posY;
		prevData_.posSetFlag = data_.posSetFlag;
		data_.posX = x;
		data_.posY = y;
		data_.posSetFlag = true;
	}
	void setScale( double scale ) { prevData_.scale = data_.scale; data_.scale = scale; }
	double scale( void ) { return data_.scale; }

	void setOwner( const std::string &owner ) { data_.owner = prevData_.owner = owner; }
	const std::string & owner() const { return data_.owner; }

	void setItemId( int itemId ) { data_.itemId = prevData_.itemId = itemId; }
	int itemId() const { return data_.itemId; }
	void setMyItem( void ) { mine_ = true; }
	bool isMyItem( void ) { return mine_; }

	void setPacketId( int packetId ) { packetId_ = packetId; }
	int packetId( void ) { return packetId_; }
	size_t wroteBytes( void ) { return wroteBytes_; }
	size_t totalBytes( void ) { return totalBytes_; }

	static bool loadBasicPaintData( const std::string & data, struct SPaintData &res, int *readPos = NULL ) 
	{
		try
		{
			boost::int8_t f;
			int pos = readPos ? *readPos : 0;

			pos += CPacketBufferUtil::readString8( data, pos, res.owner );
			pos += CPacketBufferUtil::readInt32( data, pos, res.itemId, true );
			pos += CPacketBufferUtil::readInt8( data, pos, f );
			pos += CPacketBufferUtil::readDouble( data, pos, res.posX, true );
			pos += CPacketBufferUtil::readDouble( data, pos, res.posY, true );
			pos += CPacketBufferUtil::readDouble( data, pos, res.scale, true );
			res.posSetFlag = (f == 1 ? true : false);

			if( readPos )
				*readPos = pos;
		} catch(...)
		{
			return false;
		}
		return true;
	}

	static std::string generateBasicData( const struct SPaintData &data, int *writePos = NULL )
	{
		int pos = writePos ? *writePos : 0;
		std::string buf;

		pos += CPacketBufferUtil::writeString8( buf, pos, data.owner );
		pos += CPacketBufferUtil::writeInt32( buf, pos, data.itemId, true );
		pos += CPacketBufferUtil::writeInt8( buf, pos, data.posSetFlag ? 1 : 0 );
		pos += CPacketBufferUtil::writeDouble( buf, pos, data.posX, true );
		pos += CPacketBufferUtil::writeDouble( buf, pos, data.posY, true );
		pos += CPacketBufferUtil::writeDouble( buf, pos, data.scale, true );

		if( writePos )
			*writePos = pos;
		return buf;
	}

	// virtual methods
public:
	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		return loadBasicPaintData( data, data_, readPos );
	}

	virtual std::string serialize( int *writePos = NULL ) const
	{
		return generateBasicData( data_, writePos );
	}

	virtual PaintItemType type( void ) const = 0;

	virtual void move( double x, double y ) 
	{
		setPos( x, y );
		if( canvas_ )
			canvas_->moveItem( shared_from_this(), x, y );
	}
	virtual void update( void )
	{
		if( canvas_ )
			canvas_->updateItem( shared_from_this() );
	}
	virtual void draw( void ) = 0;
	virtual void execute( void ) { }
	virtual void remove( void )
	{
		if( canvas_ )
			canvas_->removeItem( this );
		setDrawingObject( NULL );
	}
	virtual void drawSendingStatus( size_t wroteBytes, size_t totalBytes ) 
	{
		wroteBytes_ = wroteBytes;
		totalBytes_ = totalBytes;
	}
	virtual bool isScalable( void ) { return false; }

protected:
	IGluePaintCanvas *canvas_;
	void *object_;
	bool mine_;
	int packetId_;
	size_t wroteBytes_;
	size_t totalBytes_;

	struct SPaintData data_;
	struct SPaintData prevData_;
};


class CBackgroundImageItem : public CPaintItem
{
public:
	CBackgroundImageItem( void ) : CPaintItem() { }
	virtual ~CBackgroundImageItem( void ) 
	{ 
		qDebug() << "CBackgroundImageItem deleted.. " << this; 
	}
	void setPixmap( const QPixmap &pixmap ) 
	{ 
		byteArray_.clear();
		QDataStream pixmapStream(&byteArray_, QIODevice::WriteOnly);
		pixmapStream << pixmap;
		pixmapStream.setByteOrder( QDataStream::LittleEndian ); 
		qDebug() << "########## setPixmap : " << pixmapStream << byteArray_.size();
	}
	
	QPixmap createPixmap() 
	{ 
		QPixmap pixmap;
		
		QDataStream pixmapStream(&byteArray_, QIODevice::ReadOnly);
		pixmapStream.setByteOrder( QDataStream::LittleEndian ); 

		qDebug() << "########## createPixmap : "  << byteArray_.size();

		pixmapStream >> pixmap;

		return pixmap; 
	}

	virtual PaintItemType type( void ) const
	{
		return PT_BG_IMAGE;
	}

	virtual void move( double x, double y ) 
	{
		//
	}
	virtual void remove( void )
	{
		if( canvas_ )
			canvas_->clearBackgroundImage();
	}
	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawBackgroundImage(  boost::static_pointer_cast<CBackgroundImageItem>(shared_from_this()) );		
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			int pos = 0;
			std::string pixmapBuf;
			QImage::Format format;

			if( ! CPaintItem::deserialize( data, &pos ) )
				return false;

			pos += CPacketBufferUtil::readString32( data, pos, pixmapBuf, true );
			
			QByteArray temp( pixmapBuf.c_str(), pixmapBuf.size() );
			byteArray_ = temp;

			//qDebug() << "########## !!!!!!!!!!!!! deserialize : " << byteArray_.size() << pixmapBuf.size() ;

			//HexDump(  pixmapBuf.c_str() + 10000, 50 );
			//HexDump(  byteArray_.data() + 10000, 50 );

		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
			return false;
		}
		return true;
	}

	std::string serialize( int *writePos = NULL ) const
	{
		int pos = 0;
		std::string data;

		data = CPaintItem::serialize( &pos );

		std::string pixmapBuf( byteArray_.data(), byteArray_.size() );
		pos += CPacketBufferUtil::writeString32( data, pos, pixmapBuf, true );

		qDebug() << "########## serialize pixmap : "<<  data_.owner.c_str() << data_.itemId << pixmapBuf.size();
		return data;
	}

private:
	QByteArray byteArray_;
};


class CLineItem : public CPaintItem
{
public:
	CLineItem( void ) : CPaintItem() { }
	CLineItem( const QColor &color, int width ) : CPaintItem(), clr_(color), w_(width) { }

	size_t pointSize( void) const { return listList_.size(); }
	const QPointF *point( size_t index ) const 
	{
		if( listList_.size() <= index )
			return NULL;
		return &listList_[index]; 
	}
	const QColor &color() const { return clr_; }
	int width() const { return w_; }

	void addPoint( const QPointF &pt ) 
	{
		listList_.push_back( pt );
	}

	virtual PaintItemType type( void ) const
	{
		return PT_LINE;
	}

	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawLine(  boost::static_pointer_cast<CLineItem>(shared_from_this()) );
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			double x1, y1, x2, y2;
			boost::int16_t r, g, b, a, w, ptCnt;
			int pos = 0;

			if( ! CPaintItem::deserialize( data, &pos ) )
				return false;

			pos += CPacketBufferUtil::readInt16( data, pos, r, true );
			pos += CPacketBufferUtil::readInt16( data, pos, g, true );
			pos += CPacketBufferUtil::readInt16( data, pos, b, true );
			pos += CPacketBufferUtil::readInt16( data, pos, a, true );
			pos += CPacketBufferUtil::readInt16( data, pos, w, true );
			pos += CPacketBufferUtil::readInt16( data, pos, ptCnt, true );
			for( boost::int16_t i = 0; i < ptCnt; i++ )
			{
				double x, y;
				pos += CPacketBufferUtil::readDouble( data, pos, x, true );
				pos += CPacketBufferUtil::readDouble( data, pos, y, true );
	
				listList_.push_back( QPointF( x, y ) );
			}

			clr_ = QColor( r, g, b, a );
			w_ = w;
		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
			return false;
		}

		return true;
	}

	std::string serialize( int *writePos = NULL ) const
	{
		int pos = 0;
		std::string data;

		data = CPaintItem::serialize( &pos );

		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.red(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.green(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.blue(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.alpha(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, w_, true );
		pos += CPacketBufferUtil::writeInt16( data, pos, listList_.size(), true );
		for( size_t i = 0; i < listList_.size(); i++ )
		{
			pos += CPacketBufferUtil::writeDouble( data, pos, listList_[i].x(), true );
			pos += CPacketBufferUtil::writeDouble( data, pos, listList_[i].y(), true );
		}
		return data;
	}

private:
	
	std::vector< QPointF > listList_;
	QColor clr_;
	int w_;
};


class CFileItem : public CPaintItem
{
public:
	CFileItem( void ) : CPaintItem() { }
	CFileItem( const QString &path ) : CPaintItem(), path_(path) { }
	virtual ~CFileItem( void ) 
	{ 
		qDebug() << "CFileItem deleted.. " << this; 
	}
	virtual PaintItemType type( void ) const
	{
		return PT_FILE;
	}

	const QString &path( void ) const { return path_; }

	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawFile( boost::static_pointer_cast<CFileItem>(shared_from_this()) );		
	}

	virtual void execute( void )
	{
		QString msg = QObject::tr("cannot execute file : ");
		msg += path_;

		if( ! QDesktopServices::openUrl( QUrl::fromLocalFile(path_) ) )
			QMessageBox::warning( NULL, "", msg );
	}

	virtual void drawSendingStatus( size_t wroteBytes, size_t totalBytes )
	{
		CPaintItem::drawSendingStatus( wroteBytes, totalBytes );
		if( canvas_ )
			canvas_->drawSendingStatus( shared_from_this() );
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			std::string tempName;
			std::string fileData;

			int pos = 0;

			if( ! CPaintItem::deserialize( data, &pos ) )
				return false;

			pos += CPacketBufferUtil::readString16( data, pos, tempName, true );
			pos += CPacketBufferUtil::readString32( data, pos, fileData, true );

			QString fileName = QString::fromUtf8( tempName.c_str(), tempName.size() );
			path_ = generateFileDownloadPath() + fileName;

			QFile f(path_);
			if( !f.open( QIODevice::WriteOnly ) )
			{
				return false;
			}

			QDataStream out(&f);
			int ret = out.writeRawData( fileData.c_str(), fileData.size() );
			if( ret != fileData.size() )
			{
				return false;
			}
		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
			return false;
		}
		return true;
	}

	std::string serialize( int *writePos = NULL ) const
	{
		QFileInfo pathInfo( path_ );
		QString fileName( pathInfo.fileName() );

		QFile f( path_ );
		if( !f.open( QIODevice::ReadOnly ) )
			return "";

		QByteArray byteArray;
		byteArray = f.readAll();

		int pos = 0;
		std::string data;
		data = CPaintItem::serialize( &pos );
	
		pos += CPacketBufferUtil::writeString16( data, pos, toUtf8StdString(fileName), true );
		pos += CPacketBufferUtil::writeInt32( data, pos, byteArray.size(), true );
		pos += CPacketBufferUtil::writeBinary( data, pos, byteArray.data(), byteArray.size() );
		return data;
	}

protected:
	QString path_;
};


class CImageFileItem : public CFileItem
{
public:
	CImageFileItem( void ) : CFileItem() { }
	CImageFileItem( const QString &path ) : CFileItem(path) { }
	virtual ~CImageFileItem( void ) 
	{ 
		qDebug() << "CImageFileItem deleted.. " << this; 
	}
	virtual PaintItemType type( void ) const
	{
		return PT_IMAGE_FILE;
	}

	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawImage(  boost::static_pointer_cast<CImageFileItem>(shared_from_this()) );		
	}
	virtual bool isScalable( void ) { return true; }
};


class CTextItem : public CPaintItem
{
public:
	CTextItem( void ) : CPaintItem() { }
	CTextItem( const QString &text, const QFont &font, const QColor &color ) : CPaintItem(), text_(text), font_(font), clr_(color) { }
	virtual ~CTextItem( void ) 
	{ 
		qDebug() << "CTextItem deleted.. " << this; 
	}
	virtual PaintItemType type( void ) const
	{
		return PT_TEXT;
	}

	const QString &text( void ) const { return text_; }
	const QFont &font( void ) const { return font_; }
	const QColor &color( void ) const { return clr_; }

	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawText(  boost::static_pointer_cast<CTextItem>(shared_from_this()) );		
	}

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			boost::int8_t bold;
			boost::int16_t r, g, b, a, s;
			std::string text, fontFamily;
			int pos = 0;

			if( ! CPaintItem::deserialize( data, &pos ) )
				return false;

			pos += CPacketBufferUtil::readInt16( data, pos, r, true );
			pos += CPacketBufferUtil::readInt16( data, pos, g, true );
			pos += CPacketBufferUtil::readInt16( data, pos, b, true );
			pos += CPacketBufferUtil::readInt16( data, pos, a, true );
			pos += CPacketBufferUtil::readInt16( data, pos, s, true );
			pos += CPacketBufferUtil::readString16( data, pos, text, true );
			pos += CPacketBufferUtil::readString16( data, pos, fontFamily, true );
			pos += CPacketBufferUtil::readInt8( data, pos, bold );

			// text 
			text_ = QString::fromUtf8( text.c_str(), text.size() );

			// font setting
			clr_ = QColor( r, g, b, a );
			QString n = QString::fromUtf8( fontFamily.c_str(), fontFamily.size() );
			font_.setFamily( n );
			font_.setPixelSize( s );
			font_.setBold( bold == 1 ? true : false );
		} catch(CPacketException &e) {
			(void)e;
			// nothing to do
			return false;
		}
		return true;
	}

	std::string serialize( int *writePos = NULL ) const
	{
		int pos = 0;
		std::string data;

		data = CPaintItem::serialize( &pos );

		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.red(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.green(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.blue(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, clr_.alpha(), true );
		pos += CPacketBufferUtil::writeInt16( data, pos, font_.pixelSize(), true );
		pos += CPacketBufferUtil::writeString16( data, pos, toUtf8StdString(text_), true );
		pos += CPacketBufferUtil::writeString16( data, pos, toUtf8StdString(font_.family()), true );
		pos += CPacketBufferUtil::writeInt8( data, pos, font_.bold() ? 1 : 0 );
	
		return data;
	}

private:
	QString text_;
	QFont font_;
	QColor clr_;
};
