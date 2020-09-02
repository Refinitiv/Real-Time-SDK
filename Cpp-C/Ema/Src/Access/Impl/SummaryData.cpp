/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "SummaryData.h"
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
#include "StatusMsg.h"
#include "UpdateMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "MapDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace rtsdk::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

SummaryData::SummaryData() :
 _pDecoder( 0 )
{
}

SummaryData::~SummaryData()
{
}

DataType::DataTypeEnum SummaryData::getDataType() const
{
	return _pDecoder->getSummaryData()->getDataType();
}

const ComplexType& SummaryData::getData() const
{
	return static_cast<const ComplexType&>( *_pDecoder->getSummaryData() );
}

const ReqMsg& SummaryData::getReqMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( summaryData );
}

const RefreshMsg& SummaryData::getRefreshMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( summaryData );
}

const UpdateMsg& SummaryData::getUpdateMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( summaryData );
}

const StatusMsg& SummaryData::getStatusMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( summaryData );
}

const PostMsg& SummaryData::getPostMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( summaryData );
}

const AckMsg& SummaryData::getAckMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( summaryData );
}

const GenericMsg& SummaryData::getGenericMsg() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( summaryData );
}

const FieldList& SummaryData::getFieldList() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( summaryData );
}

const ElementList& SummaryData::getElementList() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( summaryData );
}

const Map& SummaryData::getMap() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( summaryData );
}

const Vector& SummaryData::getVector() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( summaryData );
}

const Series& SummaryData::getSeries() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( summaryData );
}

const FilterList& SummaryData::getFilterList() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( summaryData );
}

const OmmOpaque& SummaryData::getOpaque() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( summaryData );
}

const OmmXml& SummaryData::getXml() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( summaryData );
}

const OmmAnsiPage& SummaryData::getAnsiPage() const
{
	const Data& summaryData = *_pDecoder->getSummaryData();

	if ( summaryData.getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( summaryData.getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( summaryData );
}
