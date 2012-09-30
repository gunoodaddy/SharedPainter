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
	ui.checkBoxServerConnOnStart->setCheckState( SettingManagerPtr()->isRelayServerConnectOnStarting() ? Qt::Checked : Qt::Unchecked );
	ui.checkBoxBlinkLastItem->setCheckState( SettingManagerPtr()->isBlinkLastItem() ? Qt::Checked : Qt::Unchecked );
}

void PreferencesDialog::onOK( void )
{
	bool enable;

	enable = ui.checkBoxSyncWinSize->checkState() == Qt::Checked ? true : false;
	SettingManagerPtr()->setSyncWindowSize( enable );

	enable = ui.checkBoxServerConnOnStart->checkState() == Qt::Checked ? true : false;
	SettingManagerPtr()->setRelayServerConnectOnStarting( enable );

	enable = ui.checkBoxBlinkLastItem->checkState() == Qt::Checked ? true : false;
	SettingManagerPtr()->setBlinkLastItem( enable );

	accept();
}
