/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Decoder.h"

#include "OmmAnsiPage.h"
#include "OmmInt.h"
#include "OmmUInt.h"
#include "OmmFloat.h"
#include "OmmDouble.h"
#include "OmmReal.h"
#include "OmmDate.h"
#include "OmmDateTime.h"
#include "OmmTime.h"
#include "OmmState.h"
#include "OmmQos.h"
#include "OmmAscii.h"
#include "OmmBuffer.h"
#include "OmmEnum.h"
#include "OmmUtf8.h"
#include "OmmRmtes.h"
#include "OmmError.h"
#include "OmmOpaque.h"
#include "OmmXml.h"

#include "OmmArray.h"
#include "ElementList.h"
#include "FieldList.h"
#include "FilterList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "NoDataImpl.h"

#include "AckMsg.h"
#include "GenericMsg.h"
#include "PostMsg.h"
#include "RefreshMsg.h"
#include "StatusMsg.h"
#include "UpdateMsg.h"
#include "ReqMsg.h"

#include "Utilities.h"

#include "rtr/rsslMsgDecoders.h"

#include <new>

using namespace thomsonreuters::ema::access;

void Decoder::setRsslData( Data* pData,
						RsslDataType rsslType,
						RsslDecodeIterator* pDecodeIter, RsslBuffer* pRsslBuffer,
						const RsslDataDictionary* pRsslDictionary, void* localDb ) const
{
	DataType::DataTypeEnum dType;

	if ( rsslType == RSSL_DT_MSG )
		dType = msgDataType[ rsslExtractMsgClass( pDecodeIter ) ];
	else
		dType = dataType[rsslType];

	if ( pData->getDataType() != dType )
	{
		pData->~Data();

		create( pData, dType );
	}

	if ( dType >= DataType::OpaqueEnum && dType < DataType::ErrorEnum )
		pData->getDecoder().setRsslData( pDecodeIter, pRsslBuffer );
	else
		pData->getDecoder().setRsslData( pDecodeIter->_majorVersion, pDecodeIter->_minorVersion, pRsslBuffer, pRsslDictionary, localDb );
}

void Decoder::setRsslData( Data* pData, OmmError::ErrorCode errorCode, RsslDecodeIterator* pDecodeIter, RsslBuffer* pRsslBuffer ) const
{
	if ( pData->getDataType() != DataType::ErrorEnum )
	{
		pData->~Data();

		create( pData, DataType::ErrorEnum );
	}

	pData->getDecoder().setRsslData( pDecodeIter->_majorVersion, pDecodeIter->_minorVersion, pRsslBuffer, (const RsslDataDictionary*)errorCode, 0 );
}

void Decoder::create( Data* pData, DataType::DataTypeEnum dType ) const
{
	switch ( dType )
	{
	case DataType::IntEnum :
		new (pData) OmmInt();
		break;
	case DataType::UIntEnum :
		new (pData) OmmUInt();
		break;
	case DataType::FloatEnum :
		new (pData) OmmFloat();
		break;
	case DataType::DoubleEnum :
		new (pData) OmmDouble();
		break;
	case DataType::BufferEnum :
		new (pData) OmmBuffer();
		break;
	case DataType::AsciiEnum :
		new (pData) OmmAscii();
		break;
	case DataType::Utf8Enum :
		new (pData) OmmUtf8();
		break;
	case DataType::RmtesEnum :
		new (pData) OmmRmtes();
		break;
	case DataType::RealEnum :
		new (pData) OmmReal();
		break;
	case DataType::DateEnum :
		new (pData) OmmDate();
		break;
	case DataType::TimeEnum :
		new (pData) OmmTime();
		break;
	case DataType::DateTimeEnum :
		new (pData) OmmDateTime();
		break;
	case DataType::QosEnum :
		new (pData) OmmQos();
		break;
	case DataType::StateEnum :
		new (pData) OmmState();
		break;
	case DataType::EnumEnum :
		new (pData) OmmEnum();
		break;
	case DataType::ArrayEnum :
		new (pData) OmmArray();
		break;
	case DataType::FieldListEnum :
		new (pData) FieldList();
		break;
	case DataType::MapEnum :
		new (pData) Map();
		break;
	case DataType::OpaqueEnum :
		new (pData) OmmOpaque();
		break;
	case DataType::AnsiPageEnum :
		new (pData) OmmAnsiPage();
		break;
	case DataType::XmlEnum :
		new (pData) OmmXml();
		break;
	case DataType::NoDataEnum :
		new (pData) NoDataImpl();
		break;
	case DataType::ElementListEnum :
		new (pData) ElementList();
		break;
	case DataType::FilterListEnum :
		new (pData) FilterList();
		break;
	case DataType::VectorEnum :
		new (pData) Vector();
		break;
	case DataType::SeriesEnum :
		new (pData) Series();
		break;
	case DataType::ReqMsgEnum :
		new (pData) ReqMsg();
		break;
	case DataType::RefreshMsgEnum :
		new (pData) RefreshMsg();
		break;
	case DataType::StatusMsgEnum :
		new (pData) StatusMsg();
		break;
	case DataType::UpdateMsgEnum :
		new (pData) UpdateMsg();
		break;
	case DataType::AckMsgEnum :
		new (pData) AckMsg();
		break;
	case DataType::PostMsgEnum :
		new (pData) PostMsg();
		break;
	case DataType::GenericMsgEnum :
		new (pData) GenericMsg();
		break;
	case DataType::ErrorEnum :
		new (pData) OmmError();
		break;
	}
}
