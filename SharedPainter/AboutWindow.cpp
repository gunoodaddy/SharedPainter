/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "StdAfx.h"
#include "AboutWindow.h"
#include "UpgradeWindow.h"
#include "UIStyleSheet.h"

AboutWindow::AboutWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.labelIcon->setPixmap( QPixmap(":/SharedPainter/Resources/tray_connect.png") );

	QString version = "Ver ";
	version += VERSION_TEXT;
	ui.labelVersion->setText(version);

	version = "Protocol Ver ";
	version += PROTOCOL_VERSION_TEXT;
	ui.labelProtVersion->setText(version);

	ui.editCurrentPatch->document()->setDefaultStyleSheet(gStyleSheet_UpdateEdit);

	std::string ver, patchContents;
	if( UpgradeManagerPtr()->currentVersionContents( ver, patchContents ) )
	{
		UpgradeWindow::setContents( ui.editCurrentPatch, Util::toStringFromUtf8(ver), Util::toStringFromUtf8(patchContents) );

		ui.buttonUpgrade->setEnabled( UpgradeManagerPtr()->isAvailableUpgrade() );
	}
	else
	{
		ui.editCurrentPatch->append( "Not received version info");
		ui.buttonUpgrade->setEnabled(false);
	}
}

AboutWindow::~AboutWindow()
{

}

void AboutWindow::onUpgrade( void )
{
	UpgradeManagerPtr()->doUpgradeNow();
}
