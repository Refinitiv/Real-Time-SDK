/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "SeriesEntry.h"
#include "ExceptionTranslator.h"
#include "SeriesDecoder.h"
#include "FieldList.h"
#include "ElementList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "FilterList.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "OmmError.h"
#include "OmmAnsiPage.h"
#include "OmmXml.h"
#include "OmmJson.h"
#include "OmmOpaque.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

SeriesEntry::SeriesEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

SeriesEntry::~SeriesEntry()
{
}

DataType::DataTypeEnum SeriesEntry::getLoadType() const
{
	return _pLoad->getDataType();
}

const Data& SeriesEntry::getLoad() const
{
	return *_pLoad;
}

const OmmError& SeriesEntry::getError() const
{
	if ( _pLoad->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( *_pLoad );
}

const ElementList& SeriesEntry::getElementList() const
{
	if ( _pLoad->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( *_pLoad );
}

const FieldList& SeriesEntry::getFieldList() const
{
	if ( _pLoad->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( *_pLoad );
}

const FilterList& SeriesEntry::getFilterList() const
{
	if ( _pLoad->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( *_pLoad );
}

const Map& SeriesEntry::getMap() const
{
	if ( _pLoad->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( *_pLoad );
}

const Series& SeriesEntry::getSeries() const
{
	if ( _pLoad->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( *_pLoad );
}

const Vector& SeriesEntry::getVector() const
{
	if ( _pLoad->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( *_pLoad );
}

const ReqMsg& SeriesEntry::getReqMsg() const
{
	if ( _pLoad->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( *_pLoad );
}

const RefreshMsg& SeriesEntry::getRefreshMsg() const
{
	if ( _pLoad->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( *_pLoad );
}

const UpdateMsg& SeriesEntry::getUpdateMsg() const
{
	if ( _pLoad->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( *_pLoad );
}

const StatusMsg& SeriesEntry::getStatusMsg() const
{
	if ( _pLoad->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( *_pLoad );
}

const PostMsg& SeriesEntry::getPostMsg() const
{
	if ( _pLoad->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( *_pLoad );
}

const AckMsg& SeriesEntry::getAckMsg() const
{
	if ( _pLoad->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( *_pLoad );
}

const GenericMsg& SeriesEntry::getGenericMsg() const
{
	if ( _pLoad->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( *_pLoad );
}

const OmmOpaque& SeriesEntry::getOpaque() const
{
	if ( _pLoad->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( *_pLoad );
}

const OmmXml& SeriesEntry::getXml() const
{
	if ( _pLoad->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( *_pLoad );
}

const OmmJson& SeriesEntry::getJson() const
{
	if ( _pLoad->getDataType() != DataType::JsonEnum )
	{
		EmaString temp( "Attempt to getJson() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmJson&>( *_pLoad );
}

const OmmAnsiPage& SeriesEntry::getAnsiPage() const
{
	if ( _pLoad->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( *_pLoad );
}

const EmaString& SeriesEntry::toString() const
{
	_toString.clear().append( "SeriesEntry dataType=\"" ).append( getDTypeAsString( _pDecoder->getLoad().getDataType() ) ).append( "\"\n" );
	_toString += _pDecoder->getLoad().toString( 1 );
	addIndent( _toString, 0 ).append( "SeriesEntryEnd\n" );

	return _toString;
}
