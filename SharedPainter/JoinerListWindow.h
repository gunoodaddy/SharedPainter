#ifndef JOINERLISTWINDOW_H
#define JOINERLISTWINDOW_H

#include <QDialog>
#include "PaintUser.h"

namespace Ui {
class JoinerListWindow;
}

class JoinerListWindow : public QDialog
{
	Q_OBJECT
	
public:
	explicit JoinerListWindow( USER_LIST list, QWidget *parent = 0);
	~JoinerListWindow();

private:
	Ui::JoinerListWindow *ui;
	USER_LIST list_;
};

#endif // JOINERLISTWINDOW_H
