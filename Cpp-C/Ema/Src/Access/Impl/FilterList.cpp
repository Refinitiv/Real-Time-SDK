/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "FilterList.h"
#include "ExceptionTranslator.h"
#include "FilterListDecoder.h"
#include "Utilities.h"
#include "GlobalPool.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getFActionAsString( FilterEntry::FilterAction action );
extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

FilterList::FilterList() :
 _toString(),
 _entry(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

FilterList::~FilterList()
{
	if ( _pEncoder ) 
		g_pool._filterListEncoderPool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool._filterListDecoderPool.returnItem( _pDecoder );
}

FilterList& FilterList::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode FilterList::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum FilterList::getDataType() const
{
	return DataType::FilterListEnum;
}

bool FilterList::forth() const
{
	return !_pDecoder->getNextData();
}

bool FilterList::forth( UInt8 filterId ) const
{
	return !_pDecoder->getNextData( filterId );
}

void FilterList::reset() const
{
	_pDecoder->reset();
}

const EmaString& FilterList::toString() const
{
	return toString( 0 );
}

const EmaString& FilterList::toString( UInt64 indent ) const
{
	FilterListDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "FilterList" );

	if ( tempDecoder.hasTotalCountHint() )
		_toString.append( " totalCountHint=\"" ).append( tempDecoder.getTotalCountHint() ).append( "\"" );

	++indent;
		
	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "FilterEntry action=\"" ).append( getFActionAsString( tempDecoder.getAction() ) )
			.append( "\" filterId=\"" ).append( tempDecoder.getFilterId() );

		if ( tempDecoder.hasEntryPermissionData() )
		{
			_toString.append( "\" permissionData=\"" );
			hexToString( _toString, tempDecoder.getEntryPermissionData() );
		}

		_toString.append( "\" dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) ).append( "\"\n" );
		
		++indent;
		_toString += tempDecoder.getLoad().toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "FilterEntryEnd" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "FilterListEnd\n" );

	return _toString;
}

const EmaBuffer& FilterList::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& FilterList::getDecoder()
{
	if ( !_pDecoder )
	{
		_pDecoder = g_pool._filterListDecoderPool.getItem();
		_entry._pDecoder = _pDecoder;
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

UInt32 FilterList::getTotalCountHint() const
{
	return _pDecoder->getTotalCountHint();
}

bool FilterList::hasTotalCountHint() const
{
	return _pDecoder->hasTotalCountHint();
}

const FilterEntry& FilterList::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp );
	}

	return _entry;
}

const Encoder& FilterList::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool._filterListEncoderPool.getItem();

	return *_pEncoder;
}

FilterList& FilterList::add( UInt8 filterId, FilterEntry::FilterAction action,
							const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._filterListEncoderPool.getItem();

	_pEncoder->add( filterId, action, value, permissionData );

	return *this;
}

const FilterList& FilterList::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._filterListEncoderPool.getItem();

	_pEncoder->complete();

	return *this;
}

FilterList& FilterList::totalCountHint( UInt32 totalCountHint )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._filterListEncoderPool.getItem();

	_pEncoder->totalCountHint( totalCountHint );

	return *this;
}
