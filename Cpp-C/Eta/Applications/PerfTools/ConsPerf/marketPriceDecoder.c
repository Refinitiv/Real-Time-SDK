/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2022 Refinitiv. All rights reserved.
*/

#include "marketPriceDecoder.h"
#include "rtr/rsslGetTime.h"
#include "testUtils.h"

#define TIM_TRK_1_FID 3902
#define TIM_TRK_2_FID 3903
#define TIM_TRK_3_FID 3904

#define TIM_TRK_1_NAME "TIM_TRK_1"
#define TIM_TRK_2_NAME "TIM_TRK_2"
#define TIM_TRK_3_NAME "TIM_TRK_3"

#define TIM_TRK_1_LEN 9
#define TIM_TRK_2_LEN 9
#define TIM_TRK_3_LEN 9

RTR_C_INLINE RsslBool checkPostUserInfo(RsslMsg *pMsg)
{
	/* If post user info is present, make sure it matches our info.
	 * Otherwise, assume any posted information present came from us anyway(return true). */
	switch(pMsg->msgBase.msgClass)
	{
		case RSSL_MC_REFRESH:
			return (!(pMsg->refreshMsg.flags & RSSL_RFMF_HAS_POST_USER_INFO)
			 || pMsg->refreshMsg.postUserInfo.postUserAddr == postUserInfo.postUserAddr
			 && pMsg->refreshMsg.postUserInfo.postUserId == postUserInfo.postUserId);
		case RSSL_MC_UPDATE:
			return (!(pMsg->updateMsg.flags & RSSL_UPMF_HAS_POST_USER_INFO)
			 || pMsg->updateMsg.postUserInfo.postUserAddr == postUserInfo.postUserAddr
			 && pMsg->updateMsg.postUserInfo.postUserId == postUserInfo.postUserId);
		default:
			return RSSL_TRUE;
	}
}


RTR_C_INLINE void updateLatencyStats(ConsumerThread *pConsumerThread, RsslTimeValue timeTracker, RsslMsgClasses msgClass)
{
	RsslTimeValue currentTime;
	RsslTimeValue unitsPerMicro;

	currentTime = consPerfConfig.nanoTime ? rsslGetTimeNano() : rsslGetTimeMicro();
	unitsPerMicro = consPerfConfig.nanoTime ? 1000 : 1;

	switch(msgClass)
	{
	case RSSL_MC_UPDATE:
		timeRecordSubmit(&pConsumerThread->latencyRecords, timeTracker, currentTime, unitsPerMicro);
		break;
	case RSSL_MC_GENERIC:
		timeRecordSubmit(&pConsumerThread->genMsgLatencyRecords, timeTracker, currentTime, unitsPerMicro);
		break;
	case RSSL_MC_POST:
		timeRecordSubmit(&pConsumerThread->postLatencyRecords, timeTracker, currentTime, unitsPerMicro);
		break;
	}
}


RsslRet decodeMPUpdate(RsslDecodeIterator *pIter, RsslMsg *msg, ConsumerThread* pConsumerThread)
{
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslRet ret = 0;

	RsslDataType dataType;
	RsslPrimitive primitive;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	RsslUInt timeTracker = 0;
	RsslUInt postTimeTracker = 0;
	RsslUInt genMsgTimeTracker = 0;

	// Zero out the primitive before decoding 
	memset(&primitive, 0, sizeof(RsslPrimitive));

	/* decode field list */
	if ((ret = rsslDecodeFieldList(pIter, &fList, 0)) == RSSL_RET_SUCCESS)
	{
		while ((ret = rsslDecodeFieldEntry(pIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
		{	
			if (ret != RSSL_RET_SUCCESS) return ret;

			/* get dictionary entry */
			dictionaryEntry = getDictionaryEntry(pConsumerThread->pDictionary,
					fEntry.fieldId);

			if  (!dictionaryEntry)
			{
				printf("Error: Decoded field ID %d not present in dictionary.\n", fEntry.fieldId);
				return RSSL_RET_FAILURE;
			}

			/* decode and print out fid value */
			dataType = dictionaryEntry->rwfType;
			switch (dataType)
			{
				case RSSL_DT_INT:
					if((ret = rsslDecodeInt(pIter, &primitive.intType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_UINT:
					if((ret = rsslDecodeUInt(pIter, &primitive.uintType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_FLOAT:
					if ((ret = rsslDecodeFloat(pIter, &primitive.floatType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_DOUBLE:
					if ((ret = rsslDecodeDouble(pIter, &primitive.doubleType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_REAL:
					if ((ret = rsslDecodeReal(pIter, &primitive.realType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_DATE:
					if ((ret = rsslDecodeDate(pIter, &primitive.dateType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_TIME:
					if ((ret = rsslDecodeTime(pIter, &primitive.timeType)) < RSSL_RET_SUCCESS )
						return ret;
					break;
				case RSSL_DT_DATETIME:
					if ((ret = rsslDecodeDateTime(pIter, &primitive.dateTimeType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_QOS:
					if ((ret = rsslDecodeQos(pIter, &primitive.qosType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_STATE:
					if ((ret = rsslDecodeState(pIter, &primitive.stateType)) < RSSL_RET_SUCCESS)
						return ret;
					break;
				case RSSL_DT_ENUM:
				{
					RsslEnumType *pEnumTypeInfo;
					if ((ret = rsslDecodeEnum(pIter, &primitive.enumType)) < RSSL_RET_SUCCESS )
						return ret;

					if (ret == RSSL_RET_BLANK_DATA)
						break;

					pEnumTypeInfo = getFieldEntryEnumType(dictionaryEntry, primitive.enumType);
					if (pEnumTypeInfo)
						primitive.bufferType = pEnumTypeInfo->display;
					break;
				}
				case RSSL_DT_BUFFER:
				case RSSL_DT_ASCII_STRING:
				case RSSL_DT_UTF8_STRING:
				case RSSL_DT_RMTES_STRING:
					if ((ret = rsslDecodeBuffer(pIter, &primitive.bufferType)) < RSSL_RET_SUCCESS )
						return ret;
					break;
				default:
					printf("Error: Unhandled data type %s(%u) in field with ID %u.\n", 
							rsslDataTypeToString(dataType), dataType, fEntry.fieldId);
					return RSSL_RET_FAILURE;
			}

			if (msg->msgBase.msgClass == RSSL_MC_UPDATE && ret != RSSL_RET_BLANK_DATA)
			{
				if(fEntry.fieldId == TIM_TRK_1_FID)
					timeTracker = primitive.uintType;
				if(fEntry.fieldId == TIM_TRK_2_FID)
					postTimeTracker = primitive.uintType;
			}
			else if (msg->msgBase.msgClass == RSSL_MC_GENERIC && ret != RSSL_RET_BLANK_DATA)
			{
				if(fEntry.fieldId == TIM_TRK_3_FID)
					genMsgTimeTracker = primitive.uintType;
			}
		}
	}
	else
	{
		return ret;
	}
	
	if (timeTracker)
		updateLatencyStats(pConsumerThread, timeTracker, RSSL_MC_UPDATE);
	if (postTimeTracker && checkPostUserInfo(msg))
		updateLatencyStats(pConsumerThread, postTimeTracker, RSSL_MC_POST);
	if (genMsgTimeTracker)
		updateLatencyStats(pConsumerThread, genMsgTimeTracker, RSSL_MC_GENERIC);

	return RSSL_RET_SUCCESS;
}


RTR_C_INLINE RsslBool checkPostUserInfoJson(cJSON* json)
{
	/* If post user info is present, make sure it matches our info.
	 * Otherwise, assume any posted information present came from us anyway(return true). */
	return RSSL_TRUE;
}


RsslRet decodeMPUpdateJson(ConsumerThread* pConsumerThread, RsslMsgClasses rsslMsgClass, cJSON* json)
{
	cJSON *jsonFieldsNode, *jsonEntry;

	const char* fieldName = NULL;
	double numberValue = 0.;
	const char* stringValue = NULL;
	RsslBool nullValue = RSSL_FALSE;

	RsslUInt timeTracker = 0;
	RsslUInt postTimeTracker = 0;
	RsslUInt genMsgTimeTracker = 0;

	/* decode field list */
	jsonFieldsNode = cJSON_GetObjectItem(json, "Fields");
	if (jsonFieldsNode == NULL || !cJSON_IsObject(jsonFieldsNode))
		return RSSL_RET_FAILURE;

	cJSON_ArrayForEach(jsonEntry, jsonFieldsNode)
	{
		fieldName = jsonEntry->string;
		if (fieldName == NULL || strlen(fieldName) == 0)
		{
			printf("Error: Cannot get JSON field name.\n");
			return RSSL_RET_FAILURE;
		}

		if (cJSON_IsNumber(jsonEntry))
		{
			numberValue = cJSON_GetNumberValue(jsonEntry);
		}
		else if (cJSON_IsString(jsonEntry))
		{
			stringValue = cJSON_GetStringValue(jsonEntry);
		}
		else if (cJSON_IsNull(jsonEntry))
		{
			nullValue = RSSL_TRUE;
		}
		else
		{
			printf("Error: Unhandled JSON data type %i in JSON field %s.\n",
				jsonEntry->type, fieldName);
			return RSSL_RET_FAILURE;
		}

		if (rsslMsgClass == RSSL_MC_UPDATE && nullValue != RSSL_TRUE)
		{
			if (strncmp(fieldName, TIM_TRK_1_NAME, TIM_TRK_1_LEN) == 0)
				timeTracker = (RsslUInt)numberValue;
			if (strncmp(fieldName, TIM_TRK_2_NAME, TIM_TRK_2_LEN) == 0)
				postTimeTracker = (RsslUInt)numberValue;
		}
		else if (rsslMsgClass == RSSL_MC_GENERIC && nullValue != RSSL_TRUE)
		{
			if (strncmp(fieldName, TIM_TRK_3_NAME, TIM_TRK_3_LEN) == 0)
				genMsgTimeTracker = (RsslUInt)numberValue;
		}
	}

	if (timeTracker)
		updateLatencyStats(pConsumerThread, timeTracker, RSSL_MC_UPDATE);
	if (postTimeTracker && checkPostUserInfoJson(json))
		updateLatencyStats(pConsumerThread, postTimeTracker, RSSL_MC_POST);
	if (genMsgTimeTracker)
		updateLatencyStats(pConsumerThread, genMsgTimeTracker, RSSL_MC_GENERIC);

	return RSSL_RET_SUCCESS;
}
