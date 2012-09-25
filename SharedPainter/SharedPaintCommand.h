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

#pragma once

#include "PaintItem.h"
#include "SharedPaintTask.h"

class CSharedPaintManager;
class CSharedPaintCommandManager;

class CSharedPaintCommand
{
public:
	CSharedPaintCommand( boost::shared_ptr<CPaintItem> item )
		: cmdMngr_(NULL), spMngr_(NULL), item_(item){ }
	virtual ~CSharedPaintCommand( void ) { }

private:
	virtual bool execute( void ) = 0;
	virtual bool undo( void ) = 0;

	boost::shared_ptr<CPaintItem> item( void ) { return item_; }
	void setCommandManager( CSharedPaintCommandManager *cmdManager ) { cmdMngr_ = cmdManager; }
	void setSharedPaintManager( CSharedPaintManager * spMngr ) { spMngr_ = spMngr; }

protected:
	friend class CSharedPaintCommandManager;
	CSharedPaintCommandManager *cmdMngr_;
	CSharedPaintManager *spMngr_;
	boost::shared_ptr<CPaintItem> item_;
};


class CAddItemCommand : public CSharedPaintCommand
{
public:
	CAddItemCommand( boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(item) { }
	~CAddItemCommand( void ) 
	{
		qDebug() << "~CAddItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );
};

class CRemoveItemCommand : public CSharedPaintCommand
{
public:
	CRemoveItemCommand( boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(item) { }
	~CRemoveItemCommand( void ) 
	{
		qDebug() << "~CRemoveItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );
};


class CUpdateItemCommand : public CSharedPaintCommand
{
public:
	CUpdateItemCommand( boost::shared_ptr<CPaintItem> item ) : CSharedPaintCommand(item)
		, prevData_(item->prevData()), data_(item->data()) { }
	~CUpdateItemCommand( void ) 
	{
		qDebug() << "~CUpdateItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );

private:
	struct SPaintData prevData_;
	struct SPaintData data_;
};


class CMoveItemCommand : public CSharedPaintCommand
{
public:
	CMoveItemCommand( boost::shared_ptr<CPaintItem> item )
		: CSharedPaintCommand(item)
		, prevX_(item_->prevData().posX), prevY_(item_->prevData().posY)
		, posX_(item->posX()), posY_(item->posY()) { }
	~CMoveItemCommand( void ) 
	{
		qDebug() << "~CMoveItemCommand";
	}

	virtual bool execute( void );
	virtual bool undo( void );

private:
	double prevX_;
	double prevY_;
	double posX_;
	double posY_;
};
