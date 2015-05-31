/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Attrib.h"
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

Attrib::Attrib() :
 _pDecoder( 0 )
{
}

Attrib::~Attrib()
{
}

DataType::DataTypeEnum Attrib::getDataType() const
{
	return _pDecoder->getAttribData().getDataType();
}

const ComplexType& Attrib::getData() const
{
	return static_cast<const ComplexType&>( _pDecoder->getAttribData() );
}

const ReqMsg& Attrib::getReqMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ReqMsg&>( attrib );
}

const RefreshMsg& Attrib::getRefreshMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const RefreshMsg&>( attrib );
}

const UpdateMsg& Attrib::getUpdateMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const UpdateMsg&>( attrib );
}

const StatusMsg& Attrib::getStatusMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const StatusMsg&>( attrib );
}

const PostMsg& Attrib::getPostMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const PostMsg&>( attrib );
}

const AckMsg& Attrib::getAckMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const AckMsg&>( attrib );
}

const GenericMsg& Attrib::getGenericMsg() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const GenericMsg&>( attrib );
}

const FieldList& Attrib::getFieldList() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FieldList&>( attrib );
}

const ElementList& Attrib::getElementList() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const ElementList&>( attrib );
}

const Map& Attrib::getMap() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Map&>( attrib );
}

const Vector& Attrib::getVector() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Vector&>( attrib );
}

const Series& Attrib::getSeries() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const Series&>( attrib );
}

const FilterList& Attrib::getFilterList() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const FilterList&>( attrib );
}

const OmmOpaque& Attrib::getOpaque() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmOpaque&>( attrib );
}

const OmmXml& Attrib::getXml() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmXml&>( attrib );
}

const OmmAnsiPage& Attrib::getAnsiPage() const
{
	const Data& attrib = _pDecoder->getAttribData();

	if ( attrib.getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( attrib.getDataType() );
		throwIueException( temp );
	}

	return static_cast<const OmmAnsiPage&>( attrib );
}
