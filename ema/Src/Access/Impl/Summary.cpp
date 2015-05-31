/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Summary.h"
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

using namespace thomsonreuters::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Summary::Summary() :
 _pDecoder( 0 )
{
}

Summary::~Summary()
{
}

DataType::DataTypeEnum Summary::getDataType() const
{
	return _pDecoder->getSummaryData()->getDataType();
}

const ComplexType& Summary::getData() const
{
	return static_cast<const ComplexType&>( *_pDecoder->getSummaryData() );
}

const ReqMsg& Summary::getReqMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ReqMsg&>( summary );
}

const RefreshMsg& Summary::getRefreshMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const RefreshMsg&>( summary );
}

const UpdateMsg& Summary::getUpdateMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const UpdateMsg&>( summary );
}

const StatusMsg& Summary::getStatusMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const StatusMsg&>( summary );
}

const PostMsg& Summary::getPostMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const PostMsg&>( summary );
}

const AckMsg& Summary::getAckMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const AckMsg&>( summary );
}

const GenericMsg& Summary::getGenericMsg() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const GenericMsg&>( summary );
}

const FieldList& Summary::getFieldList() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FieldList&>( summary );
}

const ElementList& Summary::getElementList() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ElementList&>( summary );
}

const Map& Summary::getMap() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Map&>( summary );
}

const Vector& Summary::getVector() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Vector&>( summary );
}

const Series& Summary::getSeries() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Series&>( summary );
}

const FilterList& Summary::getFilterList() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FilterList&>( summary );
}

const OmmOpaque& Summary::getOpaque() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmOpaque&>( summary );
}

const OmmXml& Summary::getXml() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmXml&>( summary );
}

const OmmAnsiPage& Summary::getAnsiPage() const
{
	const Data& summary = *_pDecoder->getSummaryData();

	if ( summary.getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( summary.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmAnsiPage&>( summary );
}
