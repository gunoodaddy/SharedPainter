#include "StdAfx.h"
#include "PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	loadGeneral();
}

PreferencesDialog::~PreferencesDialog()
{

}

void PreferencesDialog::loadGeneral( void )
{
	ui.checkBoxSyncWinSize->setCheckState( SettingManagerPtr()->isSyncWindowSize() ? Qt::Checked : Qt::Unchecked );
}

void PreferencesDialog::onOK( void )
{
	bool enable = ui.checkBoxSyncWinSize->checkState() == Qt::Checked ? true : false;
	SettingManagerPtr()->setSyncWindowSize( enable );

	accept();
}