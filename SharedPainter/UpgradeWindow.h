#ifndef UPGRADEWINDOW_H
#define UPGRADEWINDOW_H

#include <QWidget>
#include "ui_UpgradeWindow.h"

class UpgradeWindow : public QDialog
{
	Q_OBJECT

public:
	UpgradeWindow(QWidget *parent = 0);
	~UpgradeWindow();

public:
	void setContents( const QString &version, const QString &patchContents );
	static void setContents( QTextEdit *edit, const QString &version, const QString &patchContents );

private slots:
	void onOK( void );
	void onCancel( void );

private:
	Ui::UpgradeWindow ui;
};

#endif // UPGRADEWINDOW_H
