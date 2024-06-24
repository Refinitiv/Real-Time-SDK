/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "MapEntry.h"
#include "ExceptionTranslator.h"
#include "MapDecoder.h"
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
#include "OmmOpaque.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString AddString( "Add" );
const EmaString UpdateString( "Update" );
const EmaString DeleteString( "Delete" );
EmaString TempMAString;

const EmaString& getMActionAsString( MapEntry::MapAction mAction )
{
	switch ( mAction )
	{
	case MapEntry::AddEnum :
		return AddString;
	case MapEntry::UpdateEnum :
		return UpdateString;
	case MapEntry::DeleteEnum :
		return DeleteString;
	default :
		return TempMAString.set( "Unknown MapAction value " ).append( (Int64)mAction );
	}
}

MapEntry::MapEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _key(),
 _toString()
{
}

MapEntry::~MapEntry()
{
}

const EmaString& MapEntry::getMapActionAsString() const
{
	return getMActionAsString( getAction() );
}

MapEntry::MapAction MapEntry::getAction() const
{
	return _pDecoder->getAction();
}

DataType::DataTypeEnum MapEntry::getLoadType() const
{
	return _pLoad->getDataType();
}

const Data& MapEntry::getLoad() const
{
	return *_pLoad;
}

const OmmError& MapEntry::getError() const
{
	if ( _pLoad->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( *_pLoad );
}

const FieldList& MapEntry::getFieldList() const
{
	if ( _pLoad->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( *_pLoad );
}

const ElementList& MapEntry::getElementList() const
{
	if ( _pLoad->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( *_pLoad );
}

const Map& MapEntry::getMap() const
{
	if ( _pLoad->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( *_pLoad );
}

const Vector& MapEntry::getVector() const
{
	if ( _pLoad->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( *_pLoad );
}

const Series& MapEntry::getSeries() const
{
	if ( _pLoad->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( *_pLoad );
}

const FilterList& MapEntry::getFilterList() const
{
	if ( _pLoad->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( *_pLoad );
}

const ReqMsg& MapEntry::getReqMsg() const
{
	if ( _pLoad->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( *_pLoad );
}

const RefreshMsg& MapEntry::getRefreshMsg() const
{
	if ( _pLoad->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( *_pLoad );
}

const UpdateMsg& MapEntry::getUpdateMsg() const
{
	if ( _pLoad->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( *_pLoad );
}

const StatusMsg& MapEntry::getStatusMsg() const
{
	if ( _pLoad->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( *_pLoad );
}

const PostMsg& MapEntry::getPostMsg() const
{
	if ( _pLoad->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( *_pLoad );
}

const AckMsg& MapEntry::getAckMsg() const
{
	if ( _pLoad->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( *_pLoad );
}

const GenericMsg& MapEntry::getGenericMsg() const
{
	if ( _pLoad->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( *_pLoad );
}

const OmmOpaque& MapEntry::getOpaque() const
{
	if ( _pLoad->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( *_pLoad );
}

const OmmXml& MapEntry::getXml() const
{
	if ( _pLoad->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( *_pLoad );
}

const OmmAnsiPage& MapEntry::getAnsiPage() const
{
	if ( _pLoad->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( *_pLoad );
}

const Key& MapEntry::getKey() const
{
	return _key;
}

bool MapEntry::hasPermissionData() const
{
	return _pDecoder->hasEntryPermissionData();
}

const EmaBuffer& MapEntry::getPermissionData() const
{
	if ( !_pDecoder->hasEntryPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pDecoder->getEntryPermissionData();
}

const EmaString& MapEntry::toString() const
{
	_toString.clear()
		.append( "MapEntry " )
		.append( " action=\"" ).append( getMActionAsString( _pDecoder->getAction() ) ).append( "\"" )
		.append( " key dataType=\"" ).append( getDTypeAsString( _pDecoder->getKeyData().getDataType() ) ).append( "\"" )
		.append( " value=\"" ).append( _pDecoder->getKeyData().toString() );

	if ( _pDecoder->hasEntryPermissionData() )
	{
		_toString.append( "\" permissionData=\"" ).append( _pDecoder->getEntryPermissionData() ).append( "\"" );
		hexToString( _toString, _pDecoder->getEntryPermissionData() );
	}

	_toString.append( "\" dataType=\"" ).append( getDTypeAsString( _pDecoder->getLoad().getDataType() ) ).append( "\"\n" );
	_toString += _pDecoder->getLoad().toString( 1 );
	addIndent( _toString, 0 ).append( "MapEntryEnd\n" );

	return _toString;
}
