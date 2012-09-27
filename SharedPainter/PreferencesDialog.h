#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include "ui_PreferencesDialog.h"

class PreferencesDialog : public QDialog
{
	Q_OBJECT

public:
	PreferencesDialog(QWidget *parent = 0);
	~PreferencesDialog();

private:
	void loadGeneral( void );

private slots:
	void onOK( void );

private:
	Ui::PreferencesDialog ui;
};

#endif // PREFERENCESDIALOG_H
