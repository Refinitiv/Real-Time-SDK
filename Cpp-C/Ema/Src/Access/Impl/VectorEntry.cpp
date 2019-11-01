/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "VectorEntry.h"
#include "ExceptionTranslator.h"
#include "VectorDecoder.h"
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

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString SetString( "Set" );
const EmaString UpdateString( "Update" );
const EmaString ClearString( "Clear" );
const EmaString InsertString( "Insert" );
const EmaString DeleteString( "Delete" );
EmaString TempVAString;

const EmaString& getVActionAsString( VectorEntry::VectorAction vAction )
{
	switch ( vAction )
	{
	case VectorEntry::SetEnum :
		return SetString;
	case VectorEntry::UpdateEnum :
		return UpdateString;
	case VectorEntry::ClearEnum:
		return ClearString;
	case VectorEntry::InsertEnum:
		return InsertString;
	case VectorEntry::DeleteEnum :
		return DeleteString;
	default :
		return TempVAString.set( "Unknown VectorAction value " ).append( (Int64)vAction );
	}
}

VectorEntry::VectorEntry() :
 _pDecoder( 0 ),
 _pLoad( 0 ),
 _toString()
{
}

VectorEntry::~VectorEntry()
{
}

const EmaString& VectorEntry::getVectorActionAsString() const
{
	return getVActionAsString( getAction() );
}

VectorEntry::VectorAction VectorEntry::getAction() const
{
	return _pDecoder->getAction();
}

UInt32 VectorEntry::getPosition() const
{
	return _pDecoder->getEntryIndex();
}

DataType::DataTypeEnum VectorEntry::getLoadType() const
{
	return _pLoad->getDataType();
}

const Data& VectorEntry::getLoad() const
{
	return *_pLoad;
}

const OmmError& VectorEntry::getError() const
{
	if ( _pLoad->getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( *_pLoad );
}

const ElementList& VectorEntry::getElementList() const
{
	if ( _pLoad->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( *_pLoad );
}

const FieldList& VectorEntry::getFieldList() const
{
	if ( _pLoad->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( *_pLoad );
}

const FilterList& VectorEntry::getFilterList() const
{
	if ( _pLoad->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( *_pLoad );
}

const Map& VectorEntry::getMap() const
{
	if ( _pLoad->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( *_pLoad );
}

const Series& VectorEntry::getSeries() const
{
	if ( _pLoad->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( *_pLoad );
}

const Vector& VectorEntry::getVector() const
{
	if ( _pLoad->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( *_pLoad );
}

const ReqMsg& VectorEntry::getReqMsg() const
{
	if ( _pLoad->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( *_pLoad );
}

const RefreshMsg& VectorEntry::getRefreshMsg() const
{
	if ( _pLoad->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( *_pLoad );
}

const UpdateMsg& VectorEntry::getUpdateMsg() const
{
	if ( _pLoad->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( *_pLoad );
}

const StatusMsg& VectorEntry::getStatusMsg() const
{
	if ( _pLoad->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( *_pLoad );
}

const PostMsg& VectorEntry::getPostMsg() const
{
	if ( _pLoad->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getRespMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( *_pLoad );
}

const AckMsg& VectorEntry::getAckMsg() const
{
	if ( _pLoad->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( *_pLoad );
}

const GenericMsg& VectorEntry::getGenericMsg() const
{
	if ( _pLoad->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( *_pLoad );
}

const OmmOpaque& VectorEntry::getOpaque() const
{
	if ( _pLoad->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( *_pLoad );
}

const OmmXml& VectorEntry::getXml() const
{
	if ( _pLoad->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( *_pLoad );
}

const OmmAnsiPage& VectorEntry::getAnsiPage() const
{
	if ( _pLoad->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual data type is " );
		temp += getDTypeAsString( _pLoad->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( *_pLoad );
}

bool VectorEntry::hasPermissionData() const
{
	return _pDecoder->hasEntryPermissionData();
}

const EmaBuffer& VectorEntry::getPermissionData() const
{
	if ( !_pDecoder->hasEntryPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pDecoder->getEntryPermissionData();
}

const EmaString& VectorEntry::toString() const
{
	_toString.clear()
		.append( "VectorEntry " )
		.append( " action=\"" ).append( VectorAction( _pDecoder->getAction() ) ).append( "\"" )
		.append( " index=\"" ).append( _pDecoder->getEntryIndex() );

	if ( _pDecoder->hasEntryPermissionData() )
	{
		_toString.append( "\" permissionData=\"" ).append( _pDecoder->getEntryPermissionData() ).append( "\"" );
		hexToString( _toString, _pDecoder->getEntryPermissionData() );
	}

	_toString.append( "\" dataType=\"" ).append( getDTypeAsString( _pDecoder->getLoad().getDataType() ) ).append( "\"\n" );
	_toString += _pDecoder->getLoad().toString( 1 );
	addIndent( _toString, 0 ).append( "VectorEntryEnd\n" );

	return _toString;
}
