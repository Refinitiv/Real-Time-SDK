/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "MessageDataUtil.h"
#include "Access/Impl/StaticDecoder.h"

using namespace refinitiv::ema::access;

MessageDataUtil* MessageDataUtil::instance = NULL;

MessageDataUtil* MessageDataUtil::getInstance()
{
	if (instance == NULL)
	{
		instance = new MessageDataUtil();
	}
	return instance;
}

MessageDataUtil::MessageDataUtil(MessageDataUtil const& msgDataUtil)
{
	pMessageData = msgDataUtil.pMessageData;
	updateMsgMPIdx = msgDataUtil.updateMsgMPIdx;
	updateMsgMBOIdx = msgDataUtil.updateMsgMBOIdx;
	postMsgMPIdx = msgDataUtil.postMsgMPIdx;
	postMsgMBOIdx = msgDataUtil.postMsgMBOIdx;
	genericMsgMPIdx = msgDataUtil.genericMsgMPIdx;
	genericMsgMBOIdx = msgDataUtil.genericMsgMBOIdx;
}

MessageDataUtil& MessageDataUtil::operator=(MessageDataUtil const& msgDataUtil)
{
	pMessageData = msgDataUtil.pMessageData;
	updateMsgMPIdx = msgDataUtil.updateMsgMPIdx;
	updateMsgMBOIdx = msgDataUtil.updateMsgMBOIdx;
	postMsgMPIdx = msgDataUtil.postMsgMPIdx;
	postMsgMBOIdx = msgDataUtil.postMsgMBOIdx;
	genericMsgMPIdx = msgDataUtil.genericMsgMPIdx;
	genericMsgMBOIdx = msgDataUtil.genericMsgMBOIdx;

	return *this;
}


void MessageDataUtil::fillMarketPriceFieldList(FieldList& fieldList, MarketPriceMsg& mpMsg)
{
	fillFieldList(fieldList, mpMsg.fieldEntriesCount, mpMsg.fieldEntries);
}

void MessageDataUtil::fillMarketByOrderMap(Map& mapOrders, MarketByOrderMsg& mboMsg, UInt16 msgtype, PerfTimeValue latencyStartTime)
{
	FieldList fieldListSummary;
	FieldList fieldListOrder;

	// see how to use a summary data in EmaCppConsPerf.ConsumerThread
	// MarketByOrderClient::decodeMBOUpdate(), MarketByOrderClient::decodeFldList()
	switch (msgtype) {
	case DataType::RefreshMsgEnum:
		addRefereshLatency(fieldListSummary);
		break;
	case DataType::UpdateMsgEnum:
		if (latencyStartTime > 0)
			addUpdateLatency(fieldListSummary, latencyStartTime);
		break;
	case DataType::PostMsgEnum:
		if (latencyStartTime > 0)
			addPostLatency(fieldListSummary, latencyStartTime);
		break;
	case DataType::GenericMsgEnum:
		if (latencyStartTime > 0)
			addGenericLatency(fieldListSummary, latencyStartTime);
		break;
	}

	fieldListSummary.complete();
	mapOrders.summaryData(fieldListSummary);

	mapOrders.keyType(DataType::RmtesEnum);
	mapOrders.keyFieldId(ORDER_ID_FID);

	for (Int32 i = 0; i < mboMsg.orderCount; ++i)
	{
		MarketOrder* order = &(mboMsg.orders[i]);
		EmaBuffer keyBuffer(order->orderId.data, order->orderId.length);

		// now we skip local set definitions because EMA does not support
		//if (mboMsg.setDefDb && order->setId >= 0)
		//{
		//}

		fieldListOrder.clear();

		fillFieldList(fieldListOrder, order->fieldEntriesCount, order->fieldEntries);

		fieldListOrder.complete();

		// key: order->orderId, action: order->action, value: order->fieldEntries
		mapOrders.addKeyBuffer(keyBuffer, (MapEntry::MapAction)order->action, fieldListOrder);
	}
}

void MessageDataUtil::fillFieldList(FieldList& fieldList, Int32 fieldEntriesCount, const MarketField* fieldEntries)
{
	for (int fieldIdx = 0; fieldIdx < fieldEntriesCount; ++fieldIdx)
	{
		const MarketField* pField = &(fieldEntries[fieldIdx]);
		const RsslFieldEntry& fieldEntry = pField->fieldEntry;
		switch (fieldEntry.dataType)
		{
		case RSSL_DT_INT:
			if (!pField->isBlank)
				fieldList.addInt(fieldEntry.fieldId, pField->primitive.intType);
			else
				fieldList.addCodeInt(fieldEntry.fieldId);
			break;
		case RSSL_DT_UINT:
			if (!pField->isBlank)
				fieldList.addUInt(fieldEntry.fieldId, pField->primitive.uintType);
			else
				fieldList.addCodeUInt(fieldEntry.fieldId);
			break;
		case RSSL_DT_FLOAT:
			if (!pField->isBlank)
				fieldList.addFloat(fieldEntry.fieldId, pField->primitive.floatType);
			else
				fieldList.addCodeFloat(fieldEntry.fieldId);
			break;
		case RSSL_DT_DOUBLE:
			if (!pField->isBlank)
				fieldList.addDouble(fieldEntry.fieldId, pField->primitive.doubleType);
			else
				fieldList.addCodeDouble(fieldEntry.fieldId);
			break;
		case RSSL_DT_REAL:
			if (!pField->isBlank)
				fieldList.addReal(fieldEntry.fieldId, pField->primitive.realType.value, (OmmReal::MagnitudeType)pField->primitive.realType.hint);
			else
				fieldList.addCodeReal(fieldEntry.fieldId);
			break;
		case RSSL_DT_DATE:
			if (!pField->isBlank)
				fieldList.addDate(fieldEntry.fieldId, pField->primitive.dateType.year, pField->primitive.dateType.month, pField->primitive.dateType.day);
			else
				fieldList.addCodeDate(fieldEntry.fieldId);
			break;
		case RSSL_DT_TIME:
			if (!pField->isBlank)
				fieldList.addTime(fieldEntry.fieldId, pField->primitive.timeType.hour, pField->primitive.timeType.minute,
					pField->primitive.timeType.second, pField->primitive.timeType.millisecond, pField->primitive.timeType.microsecond, pField->primitive.timeType.nanosecond);
			else
				fieldList.addCodeTime(fieldEntry.fieldId);
			break;
		case RSSL_DT_DATETIME:
			if (!pField->isBlank)
				fieldList.addDateTime(fieldEntry.fieldId,
					pField->primitive.dateTimeType.date.year, pField->primitive.dateTimeType.date.month, pField->primitive.dateTimeType.date.day,
					pField->primitive.dateTimeType.time.hour, pField->primitive.dateTimeType.time.minute,
					pField->primitive.dateTimeType.time.second, pField->primitive.dateTimeType.time.millisecond,
					pField->primitive.dateTimeType.time.microsecond, pField->primitive.dateTimeType.time.nanosecond);
			else
				fieldList.addCodeDateTime(fieldEntry.fieldId);
			break;
		case RSSL_DT_QOS:
			if (!pField->isBlank)
				fieldList.addQos(fieldEntry.fieldId, pField->primitive.qosType.timeliness, pField->primitive.qosType.rate);
			else
				fieldList.addCodeQos(fieldEntry.fieldId);
			break;
		case RSSL_DT_STATE:
			if (!pField->isBlank)
				fieldList.addState(fieldEntry.fieldId,
					(OmmState::StreamState)pField->primitive.stateType.streamState,
					(OmmState::DataState)pField->primitive.stateType.dataState,
					pField->primitive.stateType.code,
					EmaString(pField->primitive.stateType.text.data, pField->primitive.stateType.text.length));
			else
				fieldList.addCodeState(fieldEntry.fieldId);
			break;
		case RSSL_DT_ENUM:
			if (!pField->isBlank)
				fieldList.addEnum(fieldEntry.fieldId, pField->primitive.enumType);
			else
				fieldList.addCodeEnum(fieldEntry.fieldId);
			break;
		case RSSL_DT_BUFFER:
			if (!pField->isBlank)
				fieldList.addBuffer(fieldEntry.fieldId, EmaBuffer(pField->primitive.bufferType.data, pField->primitive.bufferType.length));
			else
				fieldList.addCodeBuffer(fieldEntry.fieldId);
			break;
		case RSSL_DT_ASCII_STRING:
			if (!pField->isBlank)
				fieldList.addAscii(fieldEntry.fieldId, EmaString(pField->primitive.bufferType.data, pField->primitive.bufferType.length));
			else
				fieldList.addCodeAscii(fieldEntry.fieldId);
			break;
		case RSSL_DT_UTF8_STRING:
			if (!pField->isBlank)
				fieldList.addUtf8(fieldEntry.fieldId, EmaBuffer(pField->primitive.bufferType.data, pField->primitive.bufferType.length));
			else
				fieldList.addCodeUtf8(fieldEntry.fieldId);
			break;
		case RSSL_DT_RMTES_STRING:
			if (!pField->isBlank)
				fieldList.addRmtes(fieldEntry.fieldId, EmaBuffer(pField->primitive.bufferType.data, pField->primitive.bufferType.length));
			else
				fieldList.addCodeRmtes(fieldEntry.fieldId);
			break;
		default:
			printf("Error: Unhandled data type %s(%u) in field with ID %u.\n",
				rsslDataTypeToString(fieldEntry.dataType), fieldEntry.dataType, fieldEntry.fieldId);
			break;
		}  // end switch (fieldEntry.dataType)
	}  // for (fieldIdx)
}

void MessageDataUtil::fillMarketPriceFieldListRefreshMsg(FieldList& fieldList)
{
	fieldList.clear();
	if (pMessageData->getXmlMsgDataHasMarketPrice())
	{
		MarketPriceMsgList& mpMsgList = pMessageData->getMarketPriceMsgList();

		// template for the refresh-message
		MarketPriceMsg& mpMsg = mpMsgList.refreshMsg;

		fillMarketPriceFieldList(fieldList, mpMsg);

		// add the latency time fields into the fieldlist for refresh message
		addRefereshLatency(fieldList);
	}  // if (XmlMsgDataHasMarketPrice)
	fieldList.complete();
}

void MessageDataUtil::fillMarketByOrderMapRefreshMsg(Map& mapOrders)
{
	mapOrders.clear();
	if (pMessageData->getXmlMsgDataHasMarketByOrder())
	{
		MarketByOrderMsgList& mboMsgList = pMessageData->getMarketByOrderMsgList();

		// template for the refresh-message
		MarketByOrderMsg& mboMsg = mboMsgList.refreshMsg;

		fillMarketByOrderMap(mapOrders, mboMsg, DataType::RefreshMsgEnum);
	}  // if (XmlMsgDataHasMarketByOrder)
	mapOrders.complete();
}

void MessageDataUtil::addRefereshLatency(FieldList& fieldList)
{
	fieldList.addCodeUInt(TIM_TRK_1_FID);
	fieldList.addCodeUInt(TIM_TRK_2_FID);
	fieldList.addCodeUInt(TIM_TRK_3_FID);
}

void MessageDataUtil::addUpdateLatency(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.addUInt(TIM_TRK_1_FID, latencyStartTime);
}

void MessageDataUtil::addPostLatency(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.addUInt(TIM_TRK_2_FID, latencyStartTime);
}

void MessageDataUtil::addGenericLatency(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.addUInt(TIM_TRK_3_FID, latencyStartTime);
}

void MessageDataUtil::fillMarketPriceFieldListUpdateMsg(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.clear();
	if (pMessageData->getXmlMsgDataHasMarketPrice())
	{
		MarketPriceMsgList& mpMsgList = pMessageData->getMarketPriceMsgList();
		
		// template for the current update-message
		MarketPriceMsg& mpMsg = mpMsgList.updateMsgs[updateMsgMPIdx];

		// prepare next update-message's index
		if (++updateMsgMPIdx >= mpMsgList.updateMsgCount)
			updateMsgMPIdx = 0;

		fillMarketPriceFieldList(fieldList, mpMsg);

		// add the latency time fields into the fieldlist
		if (latencyStartTime > 0)
			addUpdateLatency(fieldList, latencyStartTime);
	}  // if (XmlMsgDataHasMarketPrice)
	fieldList.complete();
}

void MessageDataUtil::fillMarketPriceFieldListPostMsg(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.clear();
	if (pMessageData->getXmlMsgDataHasMarketPrice())
	{
		MarketPriceMsgList& mpMsgList = pMessageData->getMarketPriceMsgList();

		// template for the current post-message
		MarketPriceMsg& mpMsg = mpMsgList.postMsgs[postMsgMPIdx];

		// prepare next post-message's index
		if (++postMsgMPIdx >= mpMsgList.postMsgCount)
			postMsgMPIdx = 0;

		fillMarketPriceFieldList(fieldList, mpMsg);

		// add the latency time fields into the fieldlist
		if (latencyStartTime > 0)
			addPostLatency(fieldList, latencyStartTime);
	}  // if (XmlMsgDataHasMarketPrice)
	fieldList.complete();
}

void MessageDataUtil::fillMarketPriceFieldListGenericMsg(FieldList& fieldList, PerfTimeValue latencyStartTime)
{
	fieldList.clear();
	if (pMessageData->getXmlMsgDataHasMarketPrice())
	{
		MarketPriceMsgList& mpMsgList = pMessageData->getMarketPriceMsgList();

		// template for the current generic-message
		MarketPriceMsg& mpMsg = mpMsgList.genMsgs[genericMsgMPIdx];

		// prepare next generic-message's index
		if (++genericMsgMPIdx >= mpMsgList.genMsgCount)
			genericMsgMPIdx = 0;

		fillMarketPriceFieldList(fieldList, mpMsg);

		// add the latency time fields into the fieldlist
		if (latencyStartTime > 0)
			addGenericLatency(fieldList, latencyStartTime);
	}  // if (XmlMsgDataHasMarketPrice)
	fieldList.complete();
}

void MessageDataUtil::fillMarketByOrderMapUpdateMsg(Map& mapOrders, PerfTimeValue latencyStartTime)
{
	mapOrders.clear();
	if (pMessageData->getXmlMsgDataHasMarketByOrder())
	{
		MarketByOrderMsgList& mboMsgList = pMessageData->getMarketByOrderMsgList();

		// template for the current update-message
		MarketByOrderMsg& mboMsg = mboMsgList.updateMsgs[updateMsgMBOIdx];

		// prepare next update-message's index
		if (++updateMsgMBOIdx >= mboMsgList.updateMsgCount)
			updateMsgMBOIdx = 0;

		fillMarketByOrderMap(mapOrders, mboMsg, DataType::UpdateMsgEnum, latencyStartTime);
	}  // if (XmlMsgDataHasMarketByOrder)
	mapOrders.complete();
}

void MessageDataUtil::fillMarketByOrderMapPostMsg(Map& mapOrders, PerfTimeValue latencyStartTime)
{
	mapOrders.clear();
	if (pMessageData->getXmlMsgDataHasMarketByOrder())
	{
		MarketByOrderMsgList& mboMsgList = pMessageData->getMarketByOrderMsgList();

		// template for the current post-message
		MarketByOrderMsg& mboMsg = mboMsgList.postMsgs[postMsgMBOIdx];

		// prepare next post-message's index
		if (++postMsgMBOIdx >= mboMsgList.postMsgCount)
			postMsgMBOIdx = 0;

		fillMarketByOrderMap(mapOrders, mboMsg, DataType::PostMsgEnum, latencyStartTime);
	}  // if (XmlMsgDataHasMarketByOrder)
	mapOrders.complete();
}

void MessageDataUtil::fillMarketByOrderMapGenericMsg(Map& mapOrders, PerfTimeValue latencyStartTime)
{
	mapOrders.clear();
	if (pMessageData->getXmlMsgDataHasMarketByOrder())
	{
		MarketByOrderMsgList& mboMsgList = pMessageData->getMarketByOrderMsgList();

		// template for the current generic-message
		MarketByOrderMsg& mboMsg = mboMsgList.genMsgs[genericMsgMBOIdx];

		// prepare next generic-message's index
		if (++genericMsgMBOIdx >= mboMsgList.genMsgCount)
			genericMsgMBOIdx = 0;

		fillMarketByOrderMap(mapOrders, mboMsg, DataType::GenericMsgEnum, latencyStartTime);
	}  // if (XmlMsgDataHasMarketByOrder)
	mapOrders.complete();
}

bool MessageDataUtil::decodeUpdate(const FieldList& fldList, UInt16 msgtype, TimeTrack& timeTrack, EmaString& errText)
{
	Int64		intType;
	UInt64		uintType;
	float		floatType;
	double		doubleType;
	UInt16		enumType;
	EmaString	acsiiType;

	timeTrack.updTime = 0;
	timeTrack.postTime = 0;
	timeTrack.genTime = 0;

	while (fldList.forth())
	{
		const FieldEntry& fe = fldList.getEntry();
		if (fe.getCode() != Data::BlankEnum)
		{
			switch (fe.getLoadType())
			{
			case DataType::IntEnum:
				intType = fe.getInt();
				break;
			case DataType::UIntEnum:
				uintType = fe.getUInt();
				break;
			case DataType::FloatEnum:
				floatType = fe.getFloat();
				break;
			case DataType::DoubleEnum:
				doubleType = fe.getDouble();
				break;
			case DataType::RealEnum:
				doubleType = fe.getReal().getAsDouble();
				break;
			case DataType::DateEnum:
			{
				const OmmDate& ommDateType = fe.getDate();
				break;
			}
			case DataType::TimeEnum:
			{
				const OmmTime& ommTimeType = fe.getTime();
				break;
			}
			case DataType::DateTimeEnum:
			{
				const OmmDateTime& ommTDateimeType = fe.getDateTime();
				break;
			}
			case DataType::QosEnum:
			{
				const OmmQos& qosType = fe.getQos();
				break;
			}
			case DataType::StateEnum:
			{
				const OmmState& stateType = fe.getState();
				break;
			}
			case DataType::EnumEnum:
				enumType = fe.getEnum();
				break;
			case DataType::BufferEnum:
			{
				const 	EmaBuffer& bufferType = fe.getBuffer();
				break;
			}
			case DataType::AsciiEnum:
			{
				const 	EmaString& asciiType = fe.getAscii();
				break;
			}
			case DataType::Utf8Enum:
			{
				const 	EmaBuffer& bufUtf8Type = fe.getUtf8();
				break;
			}
			case DataType::RmtesEnum:
			{
				const 	RmtesBuffer& bufRmtesType = fe.getRmtes();
				break;
			}
			default:
			{
				errText = "Error: Unhandled data type ";
				errText.append((UInt32)fe.getLoadType());
				errText.append(" in field with ID ");
				errText.append((UInt32)fe.getFieldId());
				return false;
			}
			}  // switch()

			if (msgtype == DataType::UpdateMsgEnum)
			{
				if (fe.getFieldId() == TIM_TRK_1_FID)
					timeTrack.updTime = uintType;
				if (fe.getFieldId() == TIM_TRK_2_FID)
					timeTrack.postTime = uintType;
			}
			else if (msgtype == DataType::GenericMsgEnum)
			{
				if (fe.getFieldId() == TIM_TRK_3_FID)
					timeTrack.genTime = uintType;
			}
		}
	}

	return true;
}
