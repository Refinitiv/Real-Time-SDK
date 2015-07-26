/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Vector.h"
#include "VectorDecoder.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"
#include "GlobalPool.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );
extern const EmaString& getVActionAsString( VectorEntry::VectorAction vAction );

Vector::Vector() :
 _toString(),
 _entry(),
 _summary(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

Vector::~Vector()
{
	if ( _pEncoder ) 
		g_pool._vectorEncoderPool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool._vectorDecoderPool.returnItem( _pDecoder );
}

Vector& Vector::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode Vector::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum Vector::getDataType() const
{
	return DataType::VectorEnum;
}

bool Vector::getSortable() const
{
	return _pDecoder->getSortable();
}

bool Vector::forth() const
{
	return !_pDecoder->getNextData();
}

void Vector::reset() const
{
	_pDecoder->reset();
}

const EmaString& Vector::toString() const
{
	return toString( 0 );
}

const EmaString& Vector::toString( UInt64 indent ) const
{
	VectorDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "Vector" );

	_toString.append( " sortable=\"" ).append( ( tempDecoder.getSortable() ? "true" : "false" ) ).append( "\"" );

	if ( tempDecoder.hasTotalCountHint() )
		_toString.append( " totalCountHint=\"" ).append( tempDecoder.getTotalCountHint() ).append( "\"" );
			
	if ( tempDecoder.hasSummary() )
	{
		++indent;
		addIndent( _toString.append( "\n" ), indent )
			.append( "SummaryData dataType=\"" ).append( getDTypeAsString( tempDecoder.getSummaryData()->getDataType() ) ).append( "\"\n" );

		++indent;
		_toString += tempDecoder.getSummaryData()->toString( indent );
		--indent;

		addIndent( _toString, indent )
			.append( "SummaryDataEnd" );
		--indent;
	}

	++indent;
		
	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "VectorEntry action=\"" ).append( getVActionAsString( tempDecoder.getAction() ) )
			.append( " index=\"" ).append( tempDecoder.getEntryIndex() );
		
		if ( tempDecoder.hasEntryPermissionData() )
		{
			_toString.append( "\" permissionData=\"" );
			hexToString( _toString, tempDecoder.getEntryPermissionData() );
		}

		_toString.append( "\" dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) ).append( "\"\n" );

		++indent;
		_toString += tempDecoder.getLoad().toString( indent );
		--indent;

		addIndent( _toString, indent ).append( "VectorEntryEnd" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "VectorEnd\n" );

	return _toString;
}

const EmaBuffer& Vector::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& Vector::getDecoder()
{
	if ( !_pDecoder )
	{
		_summary._pDecoder = _entry._pDecoder = _pDecoder = g_pool._vectorDecoderPool.getItem();
		_entry._pLoad = &_pDecoder->getLoad();
	}

	return *_pDecoder;
}

const Encoder& Vector::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool._vectorEncoderPool.getItem();

	return *_pEncoder;
}

const SummaryData& Vector::getSummaryData() const
{
	return _summary;
}

UInt32 Vector::getTotalCountHint() const
{
	return _pDecoder->getTotalCountHint();
}

bool Vector::hasTotalCountHint() const
{
	return _pDecoder->hasTotalCountHint();
}

const VectorEntry& Vector::getEntry() const
{
	if ( !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp );
	}

	return _entry;
}

Vector& Vector::add( UInt32 position, VectorEntry::VectorAction action,
					const ComplexType& value, const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._vectorEncoderPool.getItem();

	_pEncoder->add( position, action, value, permissionData );

	return *this;
}

const Vector& Vector::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool._vectorEncoderPool.getItem();

	_pEncoder->complete();

	return *this;
}

Vector& Vector::totalCountHint( UInt32 totalCountHint )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._vectorEncoderPool.getItem();

	_pEncoder->totalCountHint( totalCountHint );

	return *this;
}

Vector& Vector::summaryData( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._vectorEncoderPool.getItem();

	_pEncoder->summaryData( data );

	return *this;
}
