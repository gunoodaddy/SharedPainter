#pragma once

class CPaintItemFactory
{
public:
	static boost::shared_ptr<CPaintItem> createItem( PaintItemType type )
	{
		boost::shared_ptr<CPaintItem> item;
		switch( type )
		{
		case PT_LINE:
			item = boost::shared_ptr<CLineItem>(new CLineItem);
			break;
		case PT_FILE:
			item = boost::shared_ptr<CFileItem>(new CFileItem);
			break;
		case PT_TEXT:
			item = boost::shared_ptr<CTextItem>(new CTextItem);
			break;
		case PT_IMAGE_FILE:
			item = boost::shared_ptr<CImageFileItem>(new CImageFileItem);
			break;
		case PT_IMAGE:
			item = boost::shared_ptr<CImageItem>(new CImageItem);
			break;
		default:
			assert( false && "not supported item type" );
		}
		return item;
	}
};