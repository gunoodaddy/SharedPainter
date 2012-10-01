#ifndef PAINTERLISTWINDOW_H
#define PAINTERLISTWINDOW_H

#include <QWidget>
#include "PaintUser.h"

namespace Ui {
class PainterListWindow;
}

class PainterListWindow : public QWidget
{
	Q_OBJECT
	
public:
	explicit PainterListWindow(QWidget *parent = 0);
	~PainterListWindow();
	
protected:
	void showEvent( QShowEvent * evt );

private slots:
	void listItemClicked(QListWidgetItem* item);

private:
	Ui::PainterListWindow *ui;
};

#endif // PAINTERLISTWINDOW_H
