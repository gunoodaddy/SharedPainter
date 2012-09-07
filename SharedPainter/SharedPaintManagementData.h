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

	void addItem( boost::shared_ptr<CPaintItem> item )
	{
		std::pair< ITEM_MAP::iterator, bool > ret = itemMap_.insert( ITEM_MAP::value_type(item->itemId(), item) );
		if( ! ret.second )
		{
			// overwrite
			ret.first->second = item;
		}
	}

	boost::shared_ptr<CPaintItem> findItem( int group )
	{
		ITEM_MAP::iterator it = itemMap_.find( group );
		if( it == itemMap_.end() )
			return boost::shared_ptr<CPaintItem>();

		return it->second;
	}

	void removeItem( int group )
	{
		ITEM_MAP::iterator it = itemMap_.find( group );
		if( it == itemMap_.end() )
			return;

		itemMap_.erase( it );
	}

	ITEM_MAP &itemMap()
	{
		return itemMap_;
	}

private:
	std::string owner_;
	ITEM_MAP itemMap_;
};
