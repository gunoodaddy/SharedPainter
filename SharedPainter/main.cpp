#include "StdAfx.h"
#include "sharedpainter.h"
#include "SharedPainterScene.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	CSingleton<CDefferedCaller>::Instance();
	CSingleton<CSharedPaintManager>::Instance();

	QApplication a(argc, argv);

	CSharedPainterScene *scene = new CSharedPainterScene;
	SharedPainter w(scene);
	w.show();

	int res = a.exec();
	return res;

}
