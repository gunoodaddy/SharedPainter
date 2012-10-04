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

#include <QtGui>

namespace Util
{
	std::string generateMyId( void );

	std::string getMyIPAddress( void );

	QColor getComplementaryColor( const QColor &clr, const QColor &lineClr = QColor() );

	bool executeProgram( const QString &path );

	inline bool stringTokenizer(const std::string &strSrc, const std::string& strDelimiter, std::vector<std::string> &strList)
	{
		size_t offsetPrev = 0;
		size_t offsetNext = 0;
		while( true )   
		{   
			offsetNext = strSrc.find(strDelimiter, offsetPrev);
			if(std::string::npos == offsetNext )
			{
				if(offsetPrev != 0)
				{
					std::string sub = strSrc.substr(offsetPrev);
					strList.push_back(sub);
				}
				break;
			}

			std::string sub = strSrc.substr(offsetPrev, offsetNext - offsetPrev);
			strList.push_back(sub);

			offsetPrev = offsetNext + strDelimiter.length();
		}

		return strList.size() ? true : false;
	}

	inline bool parseVersionString( const std::string &version, int &major, int &minor, int &revision )
	{
		std::vector<std::string> strList;
		if( ! stringTokenizer( version, ".", strList ) )
			return false;

		if( strList.size() < 3 )
			return false;

		major = atoi(strList[0].c_str());
		minor = atoi(strList[1].c_str());
		revision = atoi(strList[2].c_str());
		return true;
	}

	inline int compareVersion( const std::string &version1, const std::string &version2 )
	{
		int major1, minor1, rev1;
		int major2, minor2, rev2;
		if( !parseVersionString( version1, major1, minor1, rev1 ) )
			return 0;	// exception
		if( !parseVersionString( version2, major2, minor2, rev2 ) )
			return 0;	// exception

		if( major1 == major2 && minor1 == minor2 && rev1 == rev2 )	// most case
			return 0;

		if( major1 > major2 )
			return 1;
		else if( major1 < major2 )
			return -1;

		if( minor1 > minor2 )
			return 1;
		else if( minor1 < minor2 )
			return -1;

		if( rev1 > rev2 )
			return 1;
		else if( rev1 < rev2 )
			return -1;

		return 0;
	}

	inline bool checkKeyPressed( int virtKey )
	{
		bool res = false;
		// TODO : key press check, multiplatform issue!
#ifdef Q_WS_WIN
		static WORD pressKey[256] = {0, };

		WORD state = ::GetAsyncKeyState( virtKey );
		if( state & 0x1 )
		{
			if( pressKey[ virtKey ] == 0 )
			{
				res = true;
			}
		}
		pressKey[ virtKey ] = state;
#endif
		return res;
	}

	QPointF calculateNewItemPos( int sceneWidth, int sceneHeight, int mouseX, int mouseY, int targetWidth, int targetHeight, double *lastItemPosX = NULL, double *lastItemPosY = NULL );
	QPointF calculateNewTextPos( int sceneWidth, int sceneHeight, int mouseX, int mouseY, int textSize, double *lastTextPosX = NULL, double *lastTextPosY = NULL );

	void HexDump(const void *ptr, int buflen);

	inline QString generateFileDownloadPath( const QString *path = 0 )
	{
		QString res;
		if( path )
			res += *path;
		else
			res = qApp->applicationDirPath();

		res += QDir::separator();
		res += "Download";
		res += QDir::separator();

		QDir dir( res );
		if ( !dir.exists() )
			dir.mkpath( res );
		return res;
	}

	inline QString toStringFromUtf8( const std::string &str )
	{
		QString res = QString::fromUtf8(str.c_str());
		return res;
	}

	inline std::string toUtf8StdString( const QString &str )
	{
		std::string res;

		QByteArray a = str.toUtf8();

		res.assign( a.data(), a.size() );

		return res;
	}

	inline QString checkAndChangeSameFileName( const QString &path, const QString *baseName = NULL, int index = 1 )
	{
		QFileInfo pathInfo( path );
		if( !pathInfo.exists() )
			return path;

		QString baseNameOnly = baseName ? *baseName : pathInfo.baseName();
		QString newName(baseNameOnly);
		newName += ("(" + QString::number(index) + ")");

		return checkAndChangeSameFileName( QDir::toNativeSeparators (pathInfo.path() + QDir::separator() + newName + + "." +pathInfo.completeSuffix() ), &baseNameOnly, ++index );
	}
};
