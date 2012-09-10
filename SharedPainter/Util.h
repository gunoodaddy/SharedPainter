#pragma once

namespace Util
{
	QColor getComplementaryColor( const QColor &clr, const QColor &lineClr = QColor() );

	inline bool checkKeyPressed( int virtKey )
	{
		bool res = false;
		// TODO : multiplatform issue!
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

	void HexDump(const void *ptr, int buflen);

	inline QString generateFileDownloadPath( const QString *path = 0 )
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