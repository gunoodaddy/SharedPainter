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

#pragma once

#include "Util.h"
#include "PacketBuffer.h"
#include <boost/enable_shared_from_this.hpp>
#include <set>

extern int _debug_paint_item_cnt;

class CPaintItem;
class CLineItem;
class CBackgroundImageItem;
class CFileItem;
class CTextItem;
class CImageItem;
class CImageFileItem;

enum PaintItemType {
	PT_LINE = 0,
	PT_IMAGE,
	PT_BG_IMAGE,
	PT_FILE,
	PT_IMAGE_FILE,
	PT_TEXT,
	PT_MAX 
};

typedef std::vector< boost::shared_ptr<CPaintItem> > ITEM_LIST;
typedef std::set< boost::shared_ptr<CPaintItem> > ITEM_SET;

class IGluePaintCanvas
{
public:
	virtual void drawSendingStatus( boost::shared_ptr<CPaintItem> item ) = 0;
	virtual QRectF itemBoundingRect( boost::shared_ptr<CPaintItem> item ) = 0;
	virtual void drawLine( boost::shared_ptr<CLineItem> line ) = 0;
	virtual void drawFile( boost::shared_ptr<CFileItem> file ) = 0;
	virtual void drawText( boost::shared_ptr<CTextItem> text ) = 0;
	virtual void drawImage( boost::shared_ptr<CImageItem> image ) = 0;
	virtual void drawImageFile( boost::shared_ptr<CImageFileItem> imageFile ) = 0;
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

		_debug_paint_item_cnt++;
	}
	virtual ~CPaintItem( void ) 
	{ 
		--_debug_paint_item_cnt;
		qDebug() << "CPaintItem deleted.. " << this << _debug_paint_item_cnt; 
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
		mutex_.lock();
		prevData_.posX = data_.posX;
		prevData_.posY = data_.posY;
		prevData_.posSetFlag = data_.posSetFlag;
		data_.posX = x;
		data_.posY = y;
		data_.posSetFlag = true;
		mutex_.unlock();
	}
	void setScale( double scale ) { prevData_.scale = data_.scale; data_.scale = scale; }
	double scale( void ) { return data_.scale; }

	void setOwner( const std::string &owner ) { data_.owner = prevData_.owner = owner; }
	const std::string & owner() const { return data_.owner; }

	void setItemId( int itemId ) { data_.itemId = prevData_.itemId = itemId; }
	int itemId() const { return data_.itemId; }
	void setMyItem( void ) { mine_ = true; }
	bool isMyItem( void ) { return mine_; }

	QRectF boundingRect( void )
	{
		if( canvas_ )
			return canvas_->itemBoundingRect( shared_from_this() );

		return QRect();
	}

	// TODO : sending packet work is not complete.
	//void setPacketId( int packetId ) { packetId_ = packetId; }
	//int packetId( void ) { return packetId_; }
	size_t wroteBytes( void ) { return wroteBytes_; }
	size_t totalBytes( void ) { return totalBytes_; }

	static bool deserializeBasicData( const std::string & data, struct SPaintData &res, int *readPos = NULL ) 
	{
		try
		{
			boost::uint8_t f;
			boost::uint32_t t;
			int pos = readPos ? *readPos : 0;

			pos += CPacketBufferUtil::readString8( data, pos, res.owner );
			pos += CPacketBufferUtil::readInt32( data, pos, t, true );
			pos += CPacketBufferUtil::readInt8( data, pos, f );
			pos += CPacketBufferUtil::readDouble( data, pos, res.posX, true );
			pos += CPacketBufferUtil::readDouble( data, pos, res.posY, true );
			pos += CPacketBufferUtil::readDouble( data, pos, res.scale, true );
			res.posSetFlag = (f == 1 ? true : false);
			res.itemId = t;
			if( readPos )
				*readPos = pos;
		} catch(...)
		{
			return false;
		}
		return true;
	}

	static std::string serializeBasicData( const struct SPaintData &data, int *writePos = NULL )
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

protected:
	static void copyTextToClipBoard( QString str, bool firstItem )
	{
		if( firstItem )
			QApplication::clipboard()->setText( "" );

		QString cbText = QApplication::clipboard()->text(); 
		if( cbText.isEmpty() == false )
			cbText += NATIVE_NEWLINE_STR;
		cbText += str;
		QApplication::clipboard()->setText( cbText );
	}

	// virtual methods
public:
	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		return deserializeBasicData( data, data_, readPos );
	}

	virtual std::string serialize( int *writePos = NULL ) const
	{
		return serializeBasicData( data_, writePos );
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
	virtual void copyToClipboard( bool firstItem = true ) 
	{
		if( firstItem )
			QApplication::clipboard()->setText( "" );	// init clipboard
	}

protected:
	IGluePaintCanvas *canvas_;
	void *object_;
	bool mine_;
	int packetId_;
	size_t wroteBytes_;
	size_t totalBytes_;
	boost::recursive_mutex mutex_;

	struct SPaintData data_;
	struct SPaintData prevData_;
};

class CImageItem : public CPaintItem
{
public:
	CImageItem( void ) : CPaintItem() { }
	virtual ~CImageItem( void ) 
	{ 
		qDebug() << "CImageItem deleted.. " << this; 
	}
	void setPixmap( const QPixmap &pixmap ) 
	{ 
		byteArray_.clear();
		QDataStream pixmapStream(&byteArray_, QIODevice::WriteOnly);
		pixmapStream << pixmap;
		pixmapStream.setByteOrder( QDataStream::LittleEndian ); 
	}
	
	QPixmap createPixmap() 
	{ 
		QPixmap pixmap;
		
		QDataStream pixmapStream(&byteArray_, QIODevice::ReadOnly);
		pixmapStream.setByteOrder( QDataStream::LittleEndian ); 

		pixmapStream >> pixmap;
		return pixmap; 
	}

	virtual PaintItemType type( void ) const
	{
		return PT_IMAGE;
	}

	virtual void draw( void )
	{
		if( canvas_ )
			canvas_->drawImage(  boost::static_pointer_cast<CImageItem>(shared_from_this()) );		
	}

	virtual bool isScalable( void ) { return true; }

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
		return data;
	}

	virtual void copyToClipboard( bool firstItem = true )
	{
		CPaintItem::copyToClipboard( firstItem );

		QPixmap px = createPixmap();
		QApplication::clipboard()->setPixmap( px );
	}

private:
	QByteArray byteArray_;
};


class CBackgroundImageItem : public CImageItem
{
public:
	CBackgroundImageItem( void ) : CImageItem() { }
	virtual ~CBackgroundImageItem( void ) 
	{ 
		qDebug() << "CBackgroundImageItem deleted.. " << this; 
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
	virtual bool isScalable( void ) { return false; }
};


class CLineItem : public CPaintItem
{
public:
	CLineItem( void ) : CPaintItem() { }
	CLineItem( const QColor &color, int width ) : CPaintItem(), clr_(color), w_(width) { }

	size_t pointCount( void) const { return listList_.size(); }
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
			boost::uint16_t r, g, b, a, w, ptCnt;
			int pos = 0;

			if( ! CPaintItem::deserialize( data, &pos ) )
				return false;

			pos += CPacketBufferUtil::readInt16( data, pos, r, true );
			pos += CPacketBufferUtil::readInt16( data, pos, g, true );
			pos += CPacketBufferUtil::readInt16( data, pos, b, true );
			pos += CPacketBufferUtil::readInt16( data, pos, a, true );
			pos += CPacketBufferUtil::readInt16( data, pos, w, true );
			pos += CPacketBufferUtil::readInt16( data, pos, ptCnt, true );
			for( boost::uint16_t i = 0; i < ptCnt; i++ )
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
			path_ = Util::generateFileDownloadPath() + fileName;

			path_ = Util::checkAndChangeSameFileName( path_ );

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
	
		pos += CPacketBufferUtil::writeString16( data, pos, Util::toUtf8StdString(fileName), true );
		pos += CPacketBufferUtil::writeInt32( data, pos, byteArray.size(), true );
		pos += CPacketBufferUtil::writeBinary( data, pos, byteArray.data(), byteArray.size() );
		return data;
	}

	virtual void copyToClipboard( bool firstItem = true )
	{
		CPaintItem::copyToClipboard( firstItem );

		CPaintItem::copyTextToClipBoard( path_, firstItem );
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
			canvas_->drawImageFile(  boost::static_pointer_cast<CImageFileItem>(shared_from_this()) );		
	}
	virtual bool isScalable( void ) { return true; }

	virtual void copyToClipboard( bool firstItem = true )
	{
		CPaintItem::copyToClipboard( firstItem );

		QPixmap pixmap( path() );
		QApplication::clipboard()->setPixmap( pixmap );
	}
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

	virtual bool isScalable( void ) { return true; }

	virtual bool deserialize( const std::string & data, int *readPos = NULL )
	{
		try
		{
			boost::uint8_t bold;
			boost::uint16_t r, g, b, a, s;
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
		pos += CPacketBufferUtil::writeString16( data, pos, Util::toUtf8StdString(text_), true );
		pos += CPacketBufferUtil::writeString16( data, pos, Util::toUtf8StdString(font_.family()), true );
		pos += CPacketBufferUtil::writeInt8( data, pos, font_.bold() ? 1 : 0 );
	
		return data;
	}

	virtual void copyToClipboard( bool firstItem = true )
	{
		CPaintItem::copyToClipboard( firstItem );

		CPaintItem::copyTextToClipBoard( text_, firstItem );
	}

private:
	QString text_;
	QFont font_;
	QColor clr_;
};
