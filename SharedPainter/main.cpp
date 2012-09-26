#include "StdAfx.h"
#include "sharedpainter.h"
#include "SharedPainterScene.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
#if defined(Q_WS_WIN)

	HANDLE semaphore;
	semaphore = CreateSemaphore(NULL, 1, 1, L"gunoodaddy-0813-sharedpainter");
	BOOL alreadyExist = (GetLastError() == ERROR_ALREADY_EXISTS);
	if(alreadyExist)
	{
		if( ! QFileInfo("MULTI_INSTANCE.opt").isFile() )
		{
			::MessageBoxA( NULL, "This program is running already.", PROGRAME_TEXT, MB_OK );
			return -1;
		}
	}
#endif

	CSingleton<CDefferedCaller>::Instance();
	CSingleton<CSharedPaintManager>::Instance();

	QApplication a(argc, argv);

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
