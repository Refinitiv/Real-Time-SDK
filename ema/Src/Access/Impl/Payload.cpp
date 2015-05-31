/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
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

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Payload::Payload() :
 _pDecoder( 0 )
{
}

Payload::~Payload()
{
}

DataType::DataTypeEnum Payload::getDataType() const
{
	return _pDecoder->getPayloadData().getDataType();
}

const ComplexType& Payload::getData() const
{
	return static_cast<const ComplexType&>( _pDecoder->getPayloadData() );
}

const ReqMsg& Payload::getReqMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ReqMsg&>( payload );
}

const RefreshMsg& Payload::getRefreshMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const RefreshMsg&>( payload );
}

const UpdateMsg& Payload::getUpdateMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const UpdateMsg&>( payload );
}

const StatusMsg& Payload::getStatusMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const StatusMsg&>( payload );
}

const PostMsg& Payload::getPostMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const PostMsg&>( payload );
}

const AckMsg& Payload::getAckMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const AckMsg&>( payload );
}

const GenericMsg& Payload::getGenericMsg() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const GenericMsg&>( payload );
}

const FieldList& Payload::getFieldList() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FieldList&>( payload );
}

const ElementList& Payload::getElementList() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ElementList&>( payload );
}

const Map& Payload::getMap() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Map&>( payload );
}

const Vector& Payload::getVector() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Vector&>( payload );
}

const Series& Payload::getSeries() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Series&>( payload );
}

const FilterList& Payload::getFilterList() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FilterList&>( payload );
}

const OmmOpaque& Payload::getOpaque() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmOpaque&>( payload );
}

const OmmXml& Payload::getXml() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmXml&>( payload );
}

const OmmAnsiPage& Payload::getAnsiPage() const
{
	const Data& payload = _pDecoder->getPayloadData();

	if ( payload.getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( payload.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmAnsiPage&>( payload );
}
