/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
 *|-----------------------------------------------------------------------------
 */

#include "FilterList.h"
#include "ExceptionTranslator.h"
#include "FilterListDecoder.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "OmmInvalidUsageException.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;

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
		g_pool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool.returnItem( _pDecoder );
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

const EmaString& FilterList::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	FilterList filterList;

	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	if (!_pEncoder)
		_pEncoder = g_pool.getFilterListEncoderItem();

	if (_pEncoder->isComplete())
	{
		RsslBuffer& rsslBuffer = _pEncoder->getRsslBuffer();

		StaticDecoder::setRsslData(&filterList, &rsslBuffer, RSSL_DT_FILTER_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictionary._pImpl->rsslDataDictionary());
		_toString.clear().append(filterList.toString());

		return _toString;
	}

	return _toString.clear().append("\nUnable to decode not completed FilterList data.\n");
}

const EmaString& FilterList::toString( UInt64 indent ) const
{
	FilterList filterList;

	if (!_pDecoder)
		return _toString.clear().append("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n");

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
		_pDecoder = g_pool.getFilterListDecoderItem();
		_entry._pDecoder = _pDecoder;
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

bool FilterList::hasDecoder() const
{
	return _pDecoder ? true : false;
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

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _entry;
}

const Encoder& FilterList::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFilterListEncoderItem();

	return *_pEncoder;
}

FilterList& FilterList::add( UInt8 filterId, FilterEntry::FilterAction action,
							const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFilterListEncoderItem();

	_pEncoder->add( filterId, action, value, permissionData );

	return *this;
}

FilterList& FilterList::add( UInt8 filterId, FilterEntry::FilterAction action,
	const EmaBuffer& permissionData )
{
	if (!_pEncoder)
		_pEncoder = g_pool.getFilterListEncoderItem();

	_pEncoder->add( filterId, action, permissionData );

	return *this;
}

const FilterList& FilterList::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFilterListEncoderItem();

	_pEncoder->complete();

	return *this;
}

FilterList& FilterList::totalCountHint( UInt32 totalCountHint )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFilterListEncoderItem();

	_pEncoder->totalCountHint( totalCountHint );

	return *this;
}
bool FilterList::hasEncoder() const
{
	return _pEncoder ? true : false;
}

