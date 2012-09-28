#ifndef UPGRADER_H
#define UPGRADER_H

#include <QtGui/QMainWindow>
#include "ui_upgrader.h"

class Upgrader : public QMainWindow
{
	Q_OBJECT

public:
	Upgrader(QWidget *parent = 0, Qt::WFlags flags = 0);
	~Upgrader();

private:
	Ui::UpgraderClass ui;
};

#endif // UPGRADER_H
