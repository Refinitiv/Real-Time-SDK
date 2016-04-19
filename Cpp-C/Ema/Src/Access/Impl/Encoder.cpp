/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Encoder.h"
#include "GlobalPool.h"

using namespace thomsonreuters::ema::access;

Encoder::Encoder() :
 _pEncodeIter( 0 ),
 _iteratorOwner( 0 ),
 _containerComplete( false )
{
}

Encoder::~Encoder()
{
	releaseEncIterator();
}

void Encoder::acquireEncIterator()
{
	if ( !_pEncodeIter )
	{
		_pEncodeIter = g_pool._encodeIteratorPool.getItem();
	
		_iteratorOwner = this;

		_pEncodeIter->clear();
	}
}

void Encoder::releaseEncIterator()
{
	if ( _pEncodeIter )
	{
		if ( _iteratorOwner == this )
			g_pool._encodeIteratorPool.returnItem( _pEncodeIter );
	
		_pEncodeIter = 0;
		_iteratorOwner = 0;
		_containerComplete = false;
	}
}

bool Encoder::ownsIterator() const
{
	return _iteratorOwner == this ? true : false;
}

void Encoder::passEncIterator( Encoder& other )
{
	other._pEncodeIter = _pEncodeIter;
	other._iteratorOwner = this;
}

void Encoder::clear()
{
	if ( _pEncodeIter )
		_pEncodeIter->clear();
}

RsslBuffer& Encoder::getRsslBuffer() const
{
	return _pEncodeIter->_rsslEncBuffer1.data ? _pEncodeIter->_rsslEncBuffer1 : _pEncodeIter->_rsslEncBuffer2;
}

RsslUInt8 Encoder::getMajVer() const
{
	return _pEncodeIter->_rsslMajVer;
}

RsslUInt8 Encoder::getMinVer() const
{
	return _pEncodeIter->_rsslMinVer;
}

bool Encoder::isComplete() const
{
	return _containerComplete;
}

bool Encoder::hasEncIterator() const
{
	return _pEncodeIter != 0 ? true : false;
}

UInt8 Encoder::convertDataType( DataType::DataTypeEnum dType ) const
{
	switch ( dType )
	{
	case DataType::ReqMsgEnum :
	case DataType::RefreshMsgEnum :
	case DataType::UpdateMsgEnum :
	case DataType::StatusMsgEnum :
	case DataType::PostMsgEnum :
	case DataType::AckMsgEnum :
	case DataType::GenericMsgEnum :
		return RSSL_DT_MSG;
	case DataType::FieldListEnum :
		return RSSL_DT_FIELD_LIST;
	case DataType::ElementListEnum :
		return RSSL_DT_ELEMENT_LIST;
	case DataType::MapEnum :
		return RSSL_DT_MAP;
	case DataType::VectorEnum :
		return RSSL_DT_VECTOR;
	case DataType::SeriesEnum :
		return RSSL_DT_SERIES;
	case DataType::FilterListEnum :
		return RSSL_DT_FILTER_LIST;
	case DataType::OpaqueEnum :
		return RSSL_DT_OPAQUE;
	case DataType::XmlEnum :
		return RSSL_DT_XML;
	case DataType::AnsiPageEnum :
		return RSSL_DT_ANSI_PAGE;
	case DataType::ArrayEnum :
		return RSSL_DT_ARRAY;
	case DataType::IntEnum :
		return RSSL_DT_INT;
	case DataType::UIntEnum :
		return RSSL_DT_UINT;
	case DataType::RealEnum :
		return RSSL_DT_REAL;
	case DataType::FloatEnum :
		return RSSL_DT_FLOAT;
	case DataType::DoubleEnum :
		return RSSL_DT_DOUBLE;
	case DataType::DateEnum :
		return RSSL_DT_DATE;
	case DataType::TimeEnum :
		return RSSL_DT_TIME;
	case DataType::DateTimeEnum :
		return RSSL_DT_DATETIME;
	case DataType::QosEnum :
		return RSSL_DT_QOS;
	case DataType::StateEnum :
		return RSSL_DT_STATE;
	case DataType::EnumEnum :
		return RSSL_DT_ENUM;
	case DataType::BufferEnum :
		return RSSL_DT_BUFFER;
	case DataType::AsciiEnum :
		return RSSL_DT_ASCII_STRING;
	case DataType::Utf8Enum :
		return RSSL_DT_UTF8_STRING;
	case DataType::RmtesEnum :
		return RSSL_DT_RMTES_STRING;
	case DataType::NoDataEnum :
		return RSSL_DT_NO_DATA;
	default :
		return RSSL_DT_NO_DATA;
	}
}
