#include "StdAfx.h"
#include "sharedpainter.h"
#include "SharedPainterScene.h"
#include <QtGui/QApplication>
#include "ffmpegwrapper.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	qDebug() << "App path : " << qApp->applicationDirPath() << "Current Path : " << QDir::currentPath();

#if defined(Q_WS_WIN)

	HANDLE semaphore;
	semaphore = CreateSemaphore(NULL, 1, 1, L"gunoodaddy-0813-sharedpainter");
	BOOL alreadyExist = (GetLastError() == ERROR_ALREADY_EXISTS);
	if(alreadyExist)
	{
		QString f = qApp->applicationDirPath() + QDir::separator() + "MULTI_INSTANCE.opt";
		if( ! QFileInfo( f ).isFile() )
		{
			::MessageBoxA( NULL, "This program is running already.", PROGRAME_TEXT, MB_OK );
			return -1;
		}
		_multi_instance_mode = true;
	}
#endif
	CSingleton<CDefferedCaller>::Instance();
	CSingleton<CSharedPaintManager>::Instance();

	a.setOrganizationName(AUTHOR_TEXT);
	a.setApplicationName(PROGRAME_TEXT);
	a.setApplicationVersion(VERSION_TEXT);

	CSharedPainterScene *scene = new CSharedPainterScene;
	SharedPainter w(scene);
	w.show();

	int res = a.exec();

#if defined(Q_WS_WIN)
	ReleaseSemaphore(semaphore, 1, NULL);
	CloseHandle(semaphore);
#endif
	return res;

}
