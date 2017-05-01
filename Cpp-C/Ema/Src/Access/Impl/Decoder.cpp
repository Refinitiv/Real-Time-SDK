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

#define EMA_DECODER_TYPE_SIZE 271 

using namespace thomsonreuters::ema::access;

bool Decoder::setRsslData( Data* pData,
						RsslDataType rsslType,
						RsslDecodeIterator* pDecodeIter, RsslBuffer* pRsslBuffer,
						const RsslDataDictionary* pRsslDictionary, void* localDb ) const
{
	DataType::DataTypeEnum dType;

	if ( rsslType == RSSL_DT_MSG )
	{
		RsslMsg rsslMsg;
		rsslClearMsg( &rsslMsg );

		RsslDecodeIterator decodeIter;
		rsslClearDecodeIterator( &decodeIter );
		
		RsslRet retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, pDecodeIter->_majorVersion, pDecodeIter->_minorVersion );
	
		retCode = rsslSetDecodeIteratorBuffer( &decodeIter, pRsslBuffer );

		dType = msgDataType[ rsslExtractMsgClass( &decodeIter ) ];
	}
	else
		dType = (DataType::DataTypeEnum)rsslType;

	if ( pData->getDataType() != dType )
	{
		pData->~Data();

		create( pData, dType );
	}

	if ( (dType < DataType::NoDataEnum) || (dType == DataType::OpaqueEnum)  || (dType == DataType::XmlEnum) || (dType == DataType::AnsiPageEnum))
	{
		if ( !pData->getDecoder().setRsslData( pDecodeIter, pRsslBuffer ) )
			setRsslData( pData, pData->getDecoder().getErrorCode(), pDecodeIter, pRsslBuffer );
	}
	else
	{
		if ( !pData->getDecoder().setRsslData( pDecodeIter->_majorVersion, pDecodeIter->_minorVersion, pRsslBuffer, pRsslDictionary, localDb ) )
			setRsslData( pData, pData->getDecoder().getErrorCode(), pDecodeIter, pRsslBuffer );
	}

	return true;
}

Data* Decoder::setRsslData( Data** pLoadPool, RsslDataType rsslType, RsslDecodeIterator* pDecodeIter,
	RsslBuffer* pRsslBuffer, const RsslDataDictionary* pRsslDictionary, void* localDb ) const
{
	DataType::DataTypeEnum dType;

	if ( rsslType == RSSL_DT_MSG )
	{
		RsslMsg rsslMsg;
		rsslClearMsg( &rsslMsg );

		RsslDecodeIterator decodeIter;
		rsslClearDecodeIterator( &decodeIter );
		
		RsslRet retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, pDecodeIter->_majorVersion, pDecodeIter->_minorVersion );
	
		retCode = rsslSetDecodeIteratorBuffer( &decodeIter, pRsslBuffer );

		dType = msgDataType[ rsslExtractMsgClass( &decodeIter ) ];
	}
	else
		dType = (DataType::DataTypeEnum)rsslType;

	if ((dType < DataType::NoDataEnum) || (dType == DataType::OpaqueEnum) || (dType == DataType::XmlEnum) || (dType == DataType::AnsiPageEnum))
	{
		if ( !pLoadPool[dType]->getDecoder().setRsslData( pDecodeIter, pRsslBuffer ) )
		{
			setRsslData( pLoadPool[DataType::ErrorEnum], pLoadPool[dType]->getDecoder().getErrorCode(), pDecodeIter, pRsslBuffer );
			dType = DataType::ErrorEnum;
		}
	}
	else
	{
		if ( !pLoadPool[dType]->getDecoder().setRsslData( pDecodeIter->_majorVersion, pDecodeIter->_minorVersion, pRsslBuffer, pRsslDictionary, localDb ) )
		{
			setRsslData( pLoadPool[DataType::ErrorEnum], pLoadPool[dType]->getDecoder().getErrorCode(), pDecodeIter, pRsslBuffer );
			dType = DataType::ErrorEnum;
		}
	}

	return pLoadPool[dType];
}

Data* Decoder::setRsslData( Data* pData, OmmError::ErrorCode errorCode, RsslDecodeIterator* pDecodeIter, RsslBuffer* pRsslBuffer ) const
{
	if ( pData->getDataType() != DataType::ErrorEnum )
	{
		pData->~Data();

		create( pData, DataType::ErrorEnum );
	}

	pData->getDecoder().setRsslData( pDecodeIter->_majorVersion, pDecodeIter->_minorVersion, pRsslBuffer, (const RsslDataDictionary*)errorCode, 0 );

	return pData;
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

void Decoder::createLoadPool( Data**& pLoadPool )
{
	pLoadPool = new Data*[EMA_DECODER_TYPE_SIZE];

	for (UInt32 index = 0; index < EMA_DECODER_TYPE_SIZE; index++)
	{
		pLoadPool[index] = 0;
	}

	pLoadPool[DataType::ReqMsgEnum] = new ReqMsg;
	pLoadPool[DataType::RefreshMsgEnum] = new RefreshMsg;
	pLoadPool[DataType::UpdateMsgEnum] = new UpdateMsg;
	pLoadPool[DataType::StatusMsgEnum] = new StatusMsg;
	pLoadPool[DataType::PostMsgEnum] = new PostMsg;
	pLoadPool[DataType::AckMsgEnum] = new AckMsg;
	pLoadPool[DataType::GenericMsgEnum] = new GenericMsg;
	pLoadPool[DataType::FieldListEnum] = new FieldList;
	pLoadPool[DataType::ElementListEnum] = new ElementList;
	pLoadPool[DataType::MapEnum] = new Map;
	pLoadPool[DataType::VectorEnum] = new Vector;
	pLoadPool[DataType::SeriesEnum] = new Series;
	pLoadPool[DataType::FilterListEnum] = new FilterList;
	pLoadPool[DataType::OpaqueEnum] = new OmmOpaque;
	pLoadPool[DataType::XmlEnum] = new OmmXml;
	pLoadPool[DataType::AnsiPageEnum] = new OmmAnsiPage;
	pLoadPool[DataType::ArrayEnum] = new OmmArray;
	pLoadPool[DataType::IntEnum] = new OmmInt;
	pLoadPool[DataType::UIntEnum] = new OmmUInt;
	pLoadPool[DataType::RealEnum] = new OmmReal;
	pLoadPool[DataType::FloatEnum] = new OmmFloat;
	pLoadPool[DataType::DoubleEnum] = new OmmDouble;
	pLoadPool[DataType::DateEnum] = new OmmDate;
	pLoadPool[DataType::TimeEnum] = new OmmTime;
	pLoadPool[DataType::DateTimeEnum] = new OmmDateTime;
	pLoadPool[DataType::QosEnum] = new OmmQos;
	pLoadPool[DataType::StateEnum] = new OmmState;
	pLoadPool[DataType::EnumEnum] = new OmmEnum;
	pLoadPool[DataType::BufferEnum] = new OmmBuffer;
	pLoadPool[DataType::AsciiEnum] = new OmmAscii;
	pLoadPool[DataType::Utf8Enum] = new OmmUtf8;
	pLoadPool[DataType::RmtesEnum] = new OmmRmtes;
	pLoadPool[DataType::ErrorEnum] = new OmmError;
	pLoadPool[DataType::NoDataEnum] = new NoDataImpl;
}

void Decoder::destroyLoadPool( Data**& pLoadPool )
{
	if ( !pLoadPool ) return;

	for (UInt16 idx = 0; idx < EMA_DECODER_TYPE_SIZE; ++idx)
	{
		if (pLoadPool[idx] != 0)
		{
			delete pLoadPool[idx];
		}
	}

	delete [] pLoadPool;

	pLoadPool = 0;
}
