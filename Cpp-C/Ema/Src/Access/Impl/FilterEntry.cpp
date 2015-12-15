/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "FilterEntry.h"
#include "ExceptionTranslator.h"
#include "FilterListDecoder.h"
#include "OmmOpaque.h"
#include "OmmXml.h"
#include "OmmAnsiPage.h"
#include "FieldList.h"
#include "FilterList.h"
#include "ElementList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "OmmError.h"
#include "Utilities.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString UpdateString( "Update" );
const EmaString SetString( "Set" );
const EmaString ClearString( "Clear" );
EmaString TempFAString;

const EmaString& getFActionAsString( FilterEntry::FilterAction action )
{
	switch ( action )
	{
	case FilterEntry::SetEnum :
		return SetString;
	case FilterEntry::UpdateEnum :
		return UpdateString;
	case FilterEntry::ClearEnum :
		return ClearString;
	default :
		{
			TempFAString.set( "Unknown FilterAction value " );
			TempFAString.append( (Int64)action );
		}
		return TempFAString;
	}
}

FilterEntry::FilterEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

FilterEntry::~FilterEntry()
{
}

const EmaString& FilterEntry::getFilterActionAsString() const
{
	return getFActionAsString( _pDecoder->getAction() );
}

FilterEntry::FilterAction FilterEntry::getAction() const
{
	return _pDecoder->getAction();
}

DataType::DataTypeEnum FilterEntry::getLoadType() const
{
	return _pLoad->getDataType();
}

UInt8 FilterEntry::getFilterId() const
{
	return _pDecoder->getFilterId();
}

const Data& FilterEntry::getLoad() const
{
	return *_pLoad;
}

const OmmError& FilterEntry::getError() const
{
	if ( _pLoad->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmError&>( *_pLoad );
}

const FieldList& FilterEntry::getFieldList() const
{
	if ( _pLoad->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FieldList&>( *_pLoad );
}

const ElementList& FilterEntry::getElementList() const
{
	if ( _pLoad->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ElementList&>( *_pLoad );
}

const Map& FilterEntry::getMap() const
{
	if ( _pLoad->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Map&>( *_pLoad );
}

const Vector& FilterEntry::getVector() const
{
	if ( _pLoad->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Vector&>( *_pLoad );
}

const Series& FilterEntry::getSeries() const
{
	if ( _pLoad->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Series&>( *_pLoad );
}

const FilterList& FilterEntry::getFilterList() const
{
	if ( _pLoad->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FilterList&>( *_pLoad );
}

const OmmOpaque& FilterEntry::getOpaque() const
{
	if ( _pLoad->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmOpaque&>( *_pLoad );
}

const OmmXml& FilterEntry::getXml() const
{
	if ( _pLoad->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmXml&>( *_pLoad );
}

const OmmAnsiPage& FilterEntry::getAnsiPage() const
{
	if ( _pLoad->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmAnsiPage&>( *_pLoad );
}

const ReqMsg& FilterEntry::getReqMsg() const
{
	if ( _pLoad->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ReqMsg&>( *_pLoad );
}

const RefreshMsg& FilterEntry::getRefreshMsg() const
{
	if ( _pLoad->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const RefreshMsg&>( *_pLoad );
}

const UpdateMsg& FilterEntry::getUpdateMsg() const
{
	if ( _pLoad->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const UpdateMsg&>( *_pLoad );
}

const StatusMsg& FilterEntry::getStatusMsg() const
{
	if ( _pLoad->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const StatusMsg&>( *_pLoad );
}

const PostMsg& FilterEntry::getPostMsg() const
{
	if ( _pLoad->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const PostMsg&>( *_pLoad );
}

const AckMsg& FilterEntry::getAckMsg() const
{
	if ( _pLoad->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const AckMsg&>( *_pLoad );
}

const GenericMsg& FilterEntry::getGenericMsg() const
{
	if ( _pLoad->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual entry data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp );
	}

	return static_cast<const GenericMsg&>( *_pLoad );
}

bool FilterEntry::hasPermissionData() const
{
	return _pDecoder->hasEntryPermissionData();
}

const EmaBuffer& FilterEntry::getPermissionData() const
{
	if ( !_pDecoder->hasEntryPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp );
	}

	return _pDecoder->getEntryPermissionData();
}

const EmaString& FilterEntry::toString() const
{
	_toString.clear()
		.append( "FilterEntry " )
		.append( " action=\"" ).append( getFActionAsString( _pDecoder->getAction() ) ).append( "\"" )
		.append( " filterId=\"" ).append( _pDecoder->getFilterId() );

	if ( _pDecoder->hasEntryPermissionData() )
	{
		_toString.append( "\" permissionData=\"" ).append( _pDecoder->getEntryPermissionData() ).append( "\"" );
		hexToString( _toString, _pDecoder->getEntryPermissionData() );
	}

	_toString.append( "\" dataType=\"" ).append( getDTypeAsString( _pDecoder->getLoad().getDataType() ) ).append( "\"\n" );
	_toString += _pDecoder->getLoad().toString( 1 );
	addIndent( _toString, 0 ).append( "FilterEntryEnd\n" );

	return _toString;
}
