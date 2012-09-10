#pragma once

namespace Util
{
	static bool checkKeyPressed( int virtKey )
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

	static QString checkAndChangeSameFileName( const QString &path, const QString *baseName = NULL, int index = 1 )
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