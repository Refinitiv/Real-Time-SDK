/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2019 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#include "StaticDecoder.h"

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
#include "OmmError.h"
#include "OmmRmtes.h"
#include "OmmUtf8.h"
#include "OmmOpaque.h"
#include "OmmAnsiPage.h"
#include "OmmXml.h"

#include "OmmArray.h"
#include "ElementList.h"
#include "FieldList.h"
#include "FilterList.h"
#include "Map.h"
#include "Series.h"
#include "Vector.h"
#include "NoDataImpl.h"

#include "AckMsg.h"
#include "GenericMsg.h"
#include "PostMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "ReqMsg.h"

#include "Encoder.h"

#include "OmmQosDecoder.h"
#include "OmmStateDecoder.h"

#include "Utilities.h"

#include "rtr/rsslMsgDecoders.h"

#include <new>

using namespace rtsdk::ema::access;

void StaticDecoder::setRsslData( Data* pData, RsslMsg* pRsslMsg, UInt8 majVer, UInt8 minVer, const RsslDataDictionary* dictionary )
{
	DataType::DataTypeEnum dType = msgDataType[ pRsslMsg->msgBase.msgClass ];

	morph( pData, dType );

	//pData->getDecoder().setRsslData(majVer, minVer, &pRsslMsg->msgBase.encMsgBuffer, dictionary, NULL);

	if ( !pData->getDecoder().setRsslData( majVer, minVer, pRsslMsg, dictionary ) )
		StaticDecoder::setRsslData( pData, pData->getDecoder().getErrorCode(),  majVer, minVer, &pRsslMsg->msgBase.encMsgBuffer );
}

void StaticDecoder::setRsslData( Data* pData, RsslBuffer* pRsslBuffer, RsslDataType rsslType, UInt8 majVer, UInt8 minVer, const RsslDataDictionary* dictionary )
{
	DataType::DataTypeEnum dType;

	if ( rsslType == RSSL_DT_MSG )
	{
		RsslMsg rsslMsg;
		rsslClearMsg( &rsslMsg );

		RsslDecodeIterator decodeIter;
		rsslClearDecodeIterator( &decodeIter );
		
		RsslRet retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, majVer, minVer );
		if ( RSSL_RET_SUCCESS != retCode )
		{
			StaticDecoder::setRsslData( pData, OmmError::IteratorSetFailureEnum,  majVer, minVer, pRsslBuffer );
			return;
		}
	
		retCode = rsslSetDecodeIteratorBuffer( &decodeIter, pRsslBuffer );
		if ( RSSL_RET_SUCCESS != retCode )
		{
			StaticDecoder::setRsslData( pData, OmmError::IteratorSetFailureEnum,  majVer, minVer, pRsslBuffer );
			return;
		}

		dType = msgDataType[ rsslExtractMsgClass( &decodeIter ) ];
	}
	else
		dType = (DataType::DataTypeEnum)rsslType;

	morph( pData, dType );

	if ( !pData->getDecoder().setRsslData( majVer, minVer, pRsslBuffer, dictionary, 0 ) )
		StaticDecoder::setRsslData( pData, pData->getDecoder().getErrorCode(),  majVer, minVer, pRsslBuffer );
}

void StaticDecoder::setData( Data* pData, const RsslDataDictionary* dictionary )
{
	pData->getDecoder().setRsslData( pData->getEncoder().getMajVer(), pData->getEncoder().getMinVer(),
									&pData->getEncoder().getRsslBuffer(), dictionary, 0 );
}

void StaticDecoder::setRsslData( OmmQos* pData, RsslQos* pRsslQos )
{
	static_cast<OmmQosDecoder&>( pData->getDecoder() ).setRsslData( pRsslQos );
}

void StaticDecoder::setRsslData( OmmState* pData, RsslState* pRsslState )
{
	static_cast<OmmStateDecoder&>( pData->getDecoder() ).setRsslData( pRsslState );
}

void StaticDecoder::setRsslData( Data* pData, OmmError::ErrorCode errorCode, UInt8 majVer, UInt8 minVer, RsslBuffer* pRsslBuffer )
{
	if ( pData->getDataType() != DataType::ErrorEnum )
	{
		pData->~Data();

		create( pData, DataType::ErrorEnum );
	}

	pData->getDecoder().setRsslData( majVer, minVer, pRsslBuffer, (const RsslDataDictionary*)errorCode, 0 );
}

void StaticDecoder::morph( Data* data, DataType::DataTypeEnum dType )
{
	if ( data->getDataType() != dType )
	{
		data->~Data();

		StaticDecoder::create( data, dType );
	}
}

void StaticDecoder::create( Data* data, DataType::DataTypeEnum dType )
{
	switch ( dType )
	{
	case DataType::IntEnum :
		new (data) OmmInt();
		break;
	case DataType::UIntEnum :
		new (data) OmmUInt();
		break;
	case DataType::FloatEnum :
		new (data) OmmFloat();
		break;
	case DataType::DoubleEnum :
		new (data) OmmDouble();
		break;
	case DataType::BufferEnum :
		new (data) OmmBuffer();
		break;
	case DataType::AsciiEnum :
		new (data) OmmAscii();
		break;
	case DataType::Utf8Enum :
		new (data) OmmUtf8();
		break;
	case DataType::RmtesEnum :
		new (data) OmmRmtes();
		break;
	case DataType::RealEnum :
		new (data) OmmReal();
		break;
	case DataType::DateEnum :
		new (data) OmmDate();
		break;
	case DataType::TimeEnum :
		new (data) OmmTime();
		break;
	case DataType::DateTimeEnum :
		new (data) OmmDateTime();
		break;
	case DataType::QosEnum :
		new (data) OmmQos();
		break;
	case DataType::StateEnum :
		new (data) OmmState();
		break;
	case DataType::EnumEnum :
		new (data) OmmEnum();
		break;
	case DataType::ArrayEnum :
		new (data) OmmArray();
		break;
	case DataType::FieldListEnum :
		new (data) FieldList();
		break;
	case DataType::MapEnum :
		new (data) Map();
		break;
	case DataType::ElementListEnum :
		new (data) ElementList();
		break;
	case DataType::FilterListEnum :
		new (data) FilterList();
		break;
	case DataType::VectorEnum :
		new (data) Vector();
		break;
	case DataType::SeriesEnum :
		new (data) Series();
		break;
	case DataType::OpaqueEnum :
		new (data) OmmOpaque();
		break;
	case DataType::AnsiPageEnum :
		new (data) OmmAnsiPage();
		break;
	case DataType::XmlEnum :
		new (data) OmmXml();
		break;
	case DataType::ReqMsgEnum :
		new (data) ReqMsg();
		break;
	case DataType::RefreshMsgEnum :
		new (data) RefreshMsg();
		break;
	case DataType::StatusMsgEnum :
		new (data) StatusMsg();
		break;
	case DataType::UpdateMsgEnum :
		new (data) UpdateMsg();
		break;
	case DataType::AckMsgEnum :
		new (data) AckMsg();
		break;
	case DataType::PostMsgEnum :
		new (data) PostMsg();
		break;
	case DataType::GenericMsgEnum :
		new (data) GenericMsg();
		break;
	case DataType::NoDataEnum :
		new (data) NoDataImpl();
		break;
	case DataType::ErrorEnum :
		new (data) OmmError();
		break;
	}
}
