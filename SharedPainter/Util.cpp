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

#include "stdafx.h"
#include "Util.h"

std::string Util::generateMyId( void )
{
	static std::string _gId;
	if( _gId.empty() == false )
		return _gId;

	std::string id;

	foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	{
		if ( (!(interface.flags() & QNetworkInterface::IsLoopBack)) 
			&& interface.flags() & QNetworkInterface::IsRunning )
		{
			id = interface.hardwareAddress().toStdString();
			break;
		}
	}
	qint64 msecs = QDateTime::currentMSecsSinceEpoch();
	QString temp = QString::number( msecs );
	id += temp.toStdString();

	_gId = id;
	return id;
}

std::string Util::getMyIPAddress( void )
{
	std::string ip;

	// TODO : my active ip address
	foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	{
		if ( (!(interface.flags() & QNetworkInterface::IsLoopBack)) 
			&& interface.flags() & QNetworkInterface::IsRunning )
		{
			foreach(QNetworkAddressEntry entry, interface.addressEntries())
			{
				if( entry.ip().protocol() != QAbstractSocket::IPv4Protocol )
					continue;

				return entry.ip().toString().toStdString();
			}
			break;
		}
	}

	if( ip.empty() )
		return "127.0.0.1";
	return ip;
}

void Util::HexDump(const void *ptr, int buflen)
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

QColor Util::getComplementaryColor( const QColor &clr, const QColor &lineClr )
{
	// calcualte opposite color
	int r = 255 - clr.red();
	int g = 255 - clr.green();
	int b = 255 - clr.blue();

	int r2 = lineClr.red();
	int g2 = lineClr.green();
	int b2 = lineClr.blue();

	int dr = abs(r2 - r);
	int dg = abs(g2 - g);
	int db = abs(b2 - b);

	static const int COLOR_DIFF_OFFSET = 0x20;
	int cnt = 0, bits = 0;
	if( dr < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x1; }
	if( dg < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x2; }
	if( db < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x4; }

	if( dr + dg + db <= 128 )
	{
		if( !(bits & 0x1) ) r = r > 128 ? 0 : 255;
		if( !(bits & 0x2) ) g = g > 128 ? 0 : 255;
		if( !(bits & 0x4) ) b = b > 128 ? 0 : 255;
	}
	//qDebug() << dr << dg << db << "--" << r << g << b << "---(2)---" << r2 << g2 << b2 << bits << cnt;
	QColor res(r, g, b);
	return res;
}


QPointF Util::calculateNewTextPos( int sceneWidth, int sceneHeight, int mouseX, int mouseY, int textSize, double *lastItemPosX, double *lastItemPosY )
{
	return calculateNewItemPos( sceneWidth, sceneHeight, mouseX, mouseY, textSize, textSize, lastItemPosX, lastItemPosY );
}


QPointF Util::calculateNewItemPos( int sceneWidth, int sceneHeight, int mouseX, int mouseY, int targetWidth, int targetHeight, double *lastItemPosX, double *lastItemPosY )
{
	static double lastMX = 0;
	static double lastMY = 0;

	int sW = sceneWidth;
	int sH = sceneHeight;
	int w = DEFAULT_TEXT_ITEM_POS_REGION_W; if( w > sW ) w = sW;
	int h = DEFAULT_TEXT_ITEM_POS_REGION_H; if( h > sH ) h = sH;

	double mX = mouseX;
	double mY = mouseY;
	double rX = qrand() % w;
	double rY = qrand() % h;
	if( mX <= 0 || mY <= 0 ) mX = mY = -1.f;

	double x = 0;
	double y = 0;

	if( lastItemPosY == 0 )
	{
		// initial position #1
		x = mX - targetWidth;
		y = mY - targetHeight;
	}
	else if( mX != lastMX || mY != lastMY )
	{
		// initial position #2
		x = mX - targetWidth;
		y = mY - targetHeight;
	}
	else if( lastItemPosX && lastItemPosY )
	{
		// continuous position
		x = *lastItemPosX;
		y = *lastItemPosY + targetHeight;
	}
	else
	{
		// random position
		x = rX;
		y = rY;
	}

	if( x <= 0 || y <= 0 || (y >= sH - targetHeight) || x >= sW )
	{
		// exception postion
		x = rX;
		y = rY;
	}

	qDebug() << "calculateNewItemPos() : item pos" << x << y << mX << mY << rX << rY;
	if( lastItemPosX ) *lastItemPosX = x;
	if( lastItemPosY ) *lastItemPosY = y;
	lastMX = mX;
	lastMY = mY;

	return QPointF(x, y);
}
