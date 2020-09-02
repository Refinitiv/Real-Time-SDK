/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Payload.h"
#include "FieldList.h"
#include "ElementList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "FilterList.h"
#include "OmmOpaque.h"
#include "OmmXml.h"
#include "OmmAnsiPage.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "MsgDecoder.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace rtsdk::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Payload::Payload() :
 _pPayload( 0 )
{
}

Payload::~Payload()
{
}

DataType::DataTypeEnum Payload::getDataType() const
{
	return _pPayload->getDataType();
}

const ComplexType& Payload::getData() const
{
	return static_cast<const ComplexType&>( *_pPayload );
}

const ReqMsg& Payload::getReqMsg() const
{
	if ( _pPayload->getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( *_pPayload );
}

const RefreshMsg& Payload::getRefreshMsg() const
{
	if ( _pPayload->getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( *_pPayload );
}

const UpdateMsg& Payload::getUpdateMsg() const
{
	if ( _pPayload->getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( *_pPayload );
}

const StatusMsg& Payload::getStatusMsg() const
{
	if ( _pPayload->getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( *_pPayload );
}

const PostMsg& Payload::getPostMsg() const
{
	if ( _pPayload->getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( *_pPayload );
}

const AckMsg& Payload::getAckMsg() const
{
	if ( _pPayload->getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( *_pPayload );
}

const GenericMsg& Payload::getGenericMsg() const
{
	if ( _pPayload->getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( *_pPayload );
}

const FieldList& Payload::getFieldList() const
{
	if ( _pPayload->getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( *_pPayload );
}

const ElementList& Payload::getElementList() const
{
	if ( _pPayload->getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( *_pPayload );
}

const Map& Payload::getMap() const
{
	if ( _pPayload->getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( *_pPayload );
}

const Vector& Payload::getVector() const
{
	if ( _pPayload->getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( *_pPayload );
}

const Series& Payload::getSeries() const
{
	if ( _pPayload->getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( *_pPayload );
}

const FilterList& Payload::getFilterList() const
{
	if ( _pPayload->getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( *_pPayload );
}

const OmmOpaque& Payload::getOpaque() const
{
	if ( _pPayload->getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( *_pPayload );
}

const OmmXml& Payload::getXml() const
{
	if ( _pPayload->getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( *_pPayload );
}

const OmmAnsiPage& Payload::getAnsiPage() const
{
	if ( _pPayload->getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( _pPayload->getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( *_pPayload );
}
