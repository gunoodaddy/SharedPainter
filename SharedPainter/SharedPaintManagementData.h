#pragma once

class CSharedPaintItemList
{
public:
	typedef std::map< int, boost::shared_ptr<CPaintItem> > ITEM_MAP;

public:
	CSharedPaintItemList( const std::string &owner ) : owner_(owner) { }
	~CSharedPaintItemList( void )
	{
		qDebug() << "~CSharedPaintItemList";
	}

	bool addItem( boost::shared_ptr<CPaintItem> item )
	{
		std::pair< ITEM_MAP::iterator, bool > ret = itemMap_.insert( ITEM_MAP::value_type(item->itemId(), item) );
		if( ! ret.second )
		{
			// TODO : check this right~
			//// overwrite
			//ret.first->second->lo
			//ret.first->second->remove();
			//ret.first->second = item;
			return false;
		}

		return true;
	}

	boost::shared_ptr<CPaintItem> findItem( int itemId )
	{
		ITEM_MAP::iterator it = itemMap_.find( itemId );
		if( it == itemMap_.end() )
			return boost::shared_ptr<CPaintItem>();

		return it->second;
	}

	void removeItem( int itemId )
	{
		ITEM_MAP::iterator it = itemMap_.find( itemId );
		if( it == itemMap_.end() )
			return;

		itemMap_.erase( it );
	}

	size_t itemCount( void ) { return itemMap_.size(); }

	ITEM_MAP &itemMap()
	{
		return itemMap_;
	}

private:
	std::string owner_;
	ITEM_MAP itemMap_;
};
