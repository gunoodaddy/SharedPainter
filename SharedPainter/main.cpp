#include "StdAfx.h"
#include "sharedpainter.h"
#include "SharedPainterScene.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
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
	return res;

}
