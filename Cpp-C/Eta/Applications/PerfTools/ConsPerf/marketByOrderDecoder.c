/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2018-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "marketByOrderDecoder.h"
#include "rtr/rsslGetTime.h"
#include "testUtils.h"

#define TIM_TRK_1_FID 3902
#define TIM_TRK_2_FID 3903
#define TIM_TRK_3_FID 3904

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


RsslRet decodeMBOUpdate(RsslDecodeIterator* pIter, RsslMsg* msg, ConsumerThread* pConsumerThread)
{
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry fEntry = RSSL_INIT_FIELD_ENTRY;
	RsslMap Map = RSSL_INIT_MAP;
	RsslMapEntry mEntry = RSSL_INIT_MAP_ENTRY;
	RsslRet ret = 0;
	
	RsslPrimitive key;
	RsslDataType dataType;
	RsslPrimitive primitive;
	RsslDictionaryEntry* dictionaryEntry = NULL;

	RsslUInt timeTracker = 0;
	RsslUInt postTimeTracker = 0;
	RsslUInt genMsgTimeTracker = 0;
	
	/* decode field list */
			
	if((ret = rsslDecodeMap(pIter, &Map)) == RSSL_RET_SUCCESS)
	{
		if(rsslMapCheckHasSetDefs(&Map) == RSSL_TRUE)
		{
			rsslClearLocalFieldSetDefDb(&pConsumerThread->fListSetDef);
			pConsumerThread->fListSetDef.entries.data = pConsumerThread->setDefMemory;
			pConsumerThread->fListSetDef.entries.length = sizeof(pConsumerThread->setDefMemory);
			if((ret = rsslDecodeLocalFieldSetDefDb(pIter, &pConsumerThread->fListSetDef)) != RSSL_RET_SUCCESS)
				return ret;
		}
		
		if(rsslMapCheckHasSummaryData(&Map) == RSSL_TRUE)
		{
			if((ret = rsslDecodeFieldList(pIter, &fList, &pConsumerThread->fListSetDef)) == RSSL_RET_SUCCESS)
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
						else if(fEntry.fieldId == TIM_TRK_2_FID)
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
		}

		while((ret = rsslDecodeMapEntry(pIter, &mEntry, &key)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (mEntry.action != RSSL_MPEA_DELETE_ENTRY)
			{
				if((ret = rsslDecodeFieldList(pIter, &fList, &pConsumerThread->fListSetDef)) == RSSL_RET_SUCCESS)
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
					}
				}
				else
				{
					return ret;
				}
			}
		}
	}
	else
	{
		return ret;
	}
	
	
	if (timeTracker)
		updateLatencyStats(pConsumerThread, timeTracker, RSSL_MC_UPDATE);
	if(postTimeTracker && checkPostUserInfo(msg))
		updateLatencyStats(pConsumerThread, postTimeTracker, RSSL_MC_POST);
	if(genMsgTimeTracker)
		updateLatencyStats(pConsumerThread, genMsgTimeTracker, RSSL_MC_GENERIC);

	return RSSL_RET_SUCCESS;
}

RsslRet decodeMBOUpdateJson(ConsumerThread* pConsumerThread, RsslMsgClasses rsslMsgClass, cJSON* json)
{
	return RSSL_RET_SUCCESS;
}
