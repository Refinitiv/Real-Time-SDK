/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
*/

#include "xmlMsgDataParser.h"
#include "rtr/rsslMemoryBuffer.h"
#include <assert.h>
#include <stdlib.h>

#define XML_ERR_STR "XML Msg Build Error: "

#define XML_ROOT_NODE_NAME "msgFormat"
#define XML_MBO_ROOT_NODE_NAME "marketByOrderMsgList"
#define XML_MP_ROOT_NODE_NAME "marketPriceMsgList"

#define ORDER_BID 1
#define ORDER_ASK 2

RsslBool xmlMsgDataLoaded = RSSL_FALSE;

/* Defines the payload of MarketPrice responses */
RsslBool xmlMsgDataHasMarketPrice = RSSL_FALSE;
MarketPriceMsgList xmlMarketPriceMsgs;
static RsslBuffer mpMsgMemory, mpMsgMemoryOrig;

/* Defines the payload of the MarketByOrder response */
RsslBool xmlMsgDataHasMarketByOrder = RSSL_FALSE;
MarketByOrderMsgList xmlMarketByOrderMsgs;
static RsslBuffer mboMsgMemory, mboMsgMemoryOrig;

static const char *fieldDictionaryFileName = "RDMFieldDictionary";			/* File name to load the field dictionary from. */
static const char *enumTypeDictionaryFileName = "enumtype.def";				/* File name to load the enumerated types dictionary from. */

/* Contains the globally-used dictionary. */
RsslDataDictionary dictionary; 


/*** String to Enum functions */

typedef struct
{
	const char *string;
	RsslInt32 value;
} StringToEnum;

static StringToEnum primitiveTypeTable[] =
{
	{ "RSSL_DT_INT", RSSL_DT_INT },
	{ "RSSL_DT_UINT", RSSL_DT_UINT },
	{ "RSSL_DT_FLOAT", RSSL_DT_FLOAT },
	{ "RSSL_DT_DOUBLE", RSSL_DT_DOUBLE },
	{ "RSSL_DT_REAL", RSSL_DT_REAL },
	{ "RSSL_DT_DATE", RSSL_DT_DATE },
	{ "RSSL_DT_TIME", RSSL_DT_TIME },
	{ "RSSL_DT_DATETIME", RSSL_DT_DATETIME },
	{ "RSSL_DT_QOS", RSSL_DT_QOS },
	{ "RSSL_DT_STATE", RSSL_DT_STATE },
	{ "RSSL_DT_ENUM", RSSL_DT_ENUM },
	{ "RSSL_DT_BUFFER", RSSL_DT_BUFFER },
	{ "RSSL_DT_ASCII_STRING", RSSL_DT_ASCII_STRING },
	{ "RSSL_DT_UTF8_STRING", RSSL_DT_UTF8_STRING },
	{ "RSSL_DT_RMTES_STRING", RSSL_DT_RMTES_STRING },
};
static RsslInt32 primitiveTypeTableSize = sizeof(primitiveTypeTable)/sizeof(StringToEnum);

static StringToEnum setTypeTable[] =
{
	{"RSSL_DT_INT", RSSL_DT_INT},					
	{"RSSL_DT_UINT", RSSL_DT_UINT},				
	{"RSSL_DT_FLOAT", RSSL_DT_FLOAT},				
	{"RSSL_DT_DOUBLE", RSSL_DT_DOUBLE},				
	{"RSSL_DT_REAL", RSSL_DT_REAL},				
	{"RSSL_DT_DATE", RSSL_DT_DATE},				
	{"RSSL_DT_TIME", RSSL_DT_TIME},				
	{"RSSL_DT_DATETIME", RSSL_DT_DATETIME},			
	{"RSSL_DT_QOS", RSSL_DT_QOS},					
	{"RSSL_DT_STATE", RSSL_DT_STATE},				
	{"RSSL_DT_ENUM", RSSL_DT_ENUM},				
	{"RSSL_DT_BUFFER", RSSL_DT_BUFFER},				
	{"RSSL_DT_ASCII_STRING", RSSL_DT_ASCII_STRING},		
	{"RSSL_DT_UTF8_STRING", RSSL_DT_UTF8_STRING},			
	{"RSSL_DT_RMTES_STRING", RSSL_DT_RMTES_STRING},		
	{"RSSL_DT_INT_1", RSSL_DT_INT_1},				
	{"RSSL_DT_UINT_1", RSSL_DT_UINT_1},				
	{"RSSL_DT_INT_2", RSSL_DT_INT_2},				
	{"RSSL_DT_UINT_2", RSSL_DT_UINT_2},				
	{"RSSL_DT_INT_4", RSSL_DT_INT_4},				
	{"RSSL_DT_UINT_4", RSSL_DT_UINT_4},				
	{"RSSL_DT_INT_8", RSSL_DT_INT_8},				
	{"RSSL_DT_UINT_8", RSSL_DT_UINT_8},				
	{"RSSL_DT_FLOAT_4", RSSL_DT_FLOAT_4},				
	{"RSSL_DT_DOUBLE_8", RSSL_DT_DOUBLE_8},			
	{"RSSL_DT_REAL_4RB", RSSL_DT_REAL_4RB},			
	{"RSSL_DT_REAL_8RB", RSSL_DT_REAL_8RB},			
	{"RSSL_DT_DATE_4", RSSL_DT_DATE_4},				
	{"RSSL_DT_TIME_3", RSSL_DT_TIME_3},				
	{"RSSL_DT_TIME_5", RSSL_DT_TIME_5},				
	{"RSSL_DT_DATETIME_7", RSSL_DT_DATETIME_7},			
	{"RSSL_DT_DATETIME_9", RSSL_DT_DATETIME_9},			
};
static RsslInt32 setTypeTableSize = sizeof(setTypeTable)/sizeof(StringToEnum);

static StringToEnum mapActionTable[] =
{
	{ "RSSL_MPEA_UPDATE_ENTRY", RSSL_MPEA_UPDATE_ENTRY },
	{ "RSSL_MPEA_ADD_ENTRY", RSSL_MPEA_ADD_ENTRY },
	{ "RSSL_MPEA_DELETE_ENTRY", RSSL_MPEA_DELETE_ENTRY },
};
static RsslInt32 mapActionTableSize = sizeof(mapActionTable)/sizeof(StringToEnum);

static StringToEnum qosTimeTable[] =
{
	{ "RSSL_QOS_TIME_UNSPECIFIED",		RSSL_QOS_TIME_UNSPECIFIED },
	{ "RSSL_QOS_TIME_REALTIME",			RSSL_QOS_TIME_REALTIME },
	{ "RSSL_QOS_TIME_DELAYED_UNKNOWN",	RSSL_QOS_TIME_DELAYED_UNKNOWN },
	{ "RSSL_QOS_TIME_DELAYED",			RSSL_QOS_TIME_DELAYED }
};
static RsslInt32 qosTimeTableSize = sizeof(qosTimeTable)/sizeof(StringToEnum);

static StringToEnum qosRateTable[] =
{
	{"RSSL_QOS_RATE_UNSPECIFIED",		RSSL_QOS_RATE_UNSPECIFIED},
	{"RSSL_QOS_RATE_TICK_BY_TICK",		RSSL_QOS_RATE_TICK_BY_TICK},
	{"RSSL_QOS_RATE_JIT_CONFLATED",		RSSL_QOS_RATE_JIT_CONFLATED},
	{"RSSL_QOS_RATE_TIME_CONFLATED",	RSSL_QOS_RATE_TIME_CONFLATED}
};
static RsslInt32 qosRateTableSize = sizeof(qosRateTable)/sizeof(StringToEnum);

static StringToEnum streamStateTable[] =
{
	{"RSSL_STREAM_UNSPECIFIED",     RSSL_STREAM_UNSPECIFIED},
	{"RSSL_STREAM_OPEN",    		RSSL_STREAM_OPEN},
	{"RSSL_STREAM_NON_STREAMING",   RSSL_STREAM_NON_STREAMING},
	{"RSSL_STREAM_CLOSED_RECOVER",  RSSL_STREAM_CLOSED_RECOVER},
	{"RSSL_STREAM_CLOSED",  		RSSL_STREAM_CLOSED},
	{"RSSL_STREAM_REDIRECTED",      RSSL_STREAM_REDIRECTED}
};
static RsslInt32 streamStateTableSize = sizeof(streamStateTable)/sizeof(StringToEnum);

static StringToEnum dataStateTable[] =
{
	{"RSSL_DATA_NO_CHANGE", RSSL_DATA_NO_CHANGE},
	{"RSSL_DATA_OK",        RSSL_DATA_OK},
	{"RSSL_DATA_SUSPECT",   RSSL_DATA_SUSPECT},
};
static RsslInt32 dataStateTableSize = sizeof(dataStateTable)/sizeof(StringToEnum);

static StringToEnum stateCodeTable[] =
{
	{"RSSL_SC_NONE", RSSL_SC_NONE},
	{"RSSL_SC_NOT_FOUND", RSSL_SC_NOT_FOUND},
	{"RSSL_SC_TIMEOUT", RSSL_SC_TIMEOUT},
	{"RSSL_SC_NOT_ENTITLED", RSSL_SC_NOT_ENTITLED},
	{"RSSL_SC_INVALID_ARGUMENT", RSSL_SC_INVALID_ARGUMENT},
	{"RSSL_SC_USAGE_ERROR", RSSL_SC_USAGE_ERROR},
	{"RSSL_SC_PREEMPTED", RSSL_SC_PREEMPTED},
	{"RSSL_SC_JIT_CONFLATION_STARTED", RSSL_SC_JIT_CONFLATION_STARTED},
	{"RSSL_SC_REALTIME_RESUMED", RSSL_SC_REALTIME_RESUMED},
	{"RSSL_SC_FAILOVER_STARTED", RSSL_SC_FAILOVER_STARTED},
	{"RSSL_SC_FAILOVER_COMPLETED", RSSL_SC_FAILOVER_COMPLETED},
	{"RSSL_SC_GAP_DETECTED", RSSL_SC_GAP_DETECTED},
	{"RSSL_SC_NO_RESOURCES", RSSL_SC_NO_RESOURCES},
	{"RSSL_SC_TOO_MANY_ITEMS", RSSL_SC_TOO_MANY_ITEMS},
	{"RSSL_SC_ALREADY_OPEN", RSSL_SC_ALREADY_OPEN},
	{"RSSL_SC_SOURCE_UNKNOWN", RSSL_SC_SOURCE_UNKNOWN},
	{"RSSL_SC_NOT_OPEN", RSSL_SC_NOT_OPEN},
	{"RSSL_SC_NON_UPDATING_ITEM", RSSL_SC_NON_UPDATING_ITEM},
	{"RSSL_SC_UNSUPPORTED_VIEW_TYPE", RSSL_SC_UNSUPPORTED_VIEW_TYPE},
	{"RSSL_SC_INVALID_VIEW", RSSL_SC_INVALID_VIEW},
	{"RSSL_SC_FULL_VIEW_PROVIDED", RSSL_SC_FULL_VIEW_PROVIDED},
	{"RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH", RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH},
	{"RSSL_SC_MAX_RESERVED", RSSL_SC_MAX_RESERVED},
	{"RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ", RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ},
	{"RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER", RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER},
	{"RSSL_SC_ERROR", RSSL_SC_ERROR},
	{"RSSL_SC_DACS_DOWN", RSSL_SC_DACS_DOWN},
	{"RSSL_SC_USER_UNKNOWN_TO_PERM_SYS", RSSL_SC_USER_UNKNOWN_TO_PERM_SYS},
	{"RSSL_SC_DACS_MAX_LOGINS_REACHED", RSSL_SC_DACS_MAX_LOGINS_REACHED},
	{"RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED", RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED},
	{"RSSL_SC_GAP_FILL", RSSL_SC_GAP_FILL},
	{"RSSL_SC_APP_AUTHORIZATION_FAILED", RSSL_SC_APP_AUTHORIZATION_FAILED}
};
RsslInt32 stateCodeTableSize = sizeof(stateCodeTable)/sizeof(StringToEnum);

/* stringToEnum
 *   Converts a string to the appropriate enumeration in the given table.  */
static RsslInt32 stringToEnum(StringToEnum *table, RsslInt32 tableSize, char *keyword)
{
	int i;
	int scanfRet, scanfLen;
	RsslUInt32 value;

	/* Match string */
	for(i = tableSize-1; i >= 0; --i)
	{
		if (0 == strcmp(keyword, table[i].string))
			return table[i].value;
	}

	/* Failed to match to enum, try to match it to integer value */
	scanfRet = sscanf(keyword, "%u%n", &value, &scanfLen);
	if (scanfLen == strlen(keyword) && scanfRet == 1) return value;

	/* Failed */
	printf("Failed to parse value: %s\n", keyword);
	return -1;
}

static RsslRet initPrimitive(RsslPrimitive *primitive, RsslDataType dataType, xmlNode *pXmlElement, RsslBuffer *pMemoryBuffer)
{
	int scanfRet, scanfLen;
	char *pXmlAttrib;
	RsslInt32 stringToEnumRet;
	RsslBuffer dataBuffer;
	size_t stringLength;

	RsslRet ret = RSSL_RET_FAILURE;

	// Zero out the primitive before encoding
	memset(primitive, 0, sizeof(RsslPrimitive));

	switch(dataType)
	{
		case RSSL_DT_INT: 
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib))) 
					ret = RSSL_RET_BLANK_DATA;
				else
				{
					scanfRet = sscanf(pXmlAttrib, "%lld%n", &primitive->intType, &scanfLen);
					if (scanfLen == stringLength && scanfRet == 1) ret = RSSL_RET_SUCCESS;
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_UINT: 
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib))) 
					ret = RSSL_RET_BLANK_DATA;
				else
				{
					scanfRet = sscanf(pXmlAttrib, "%llu%n", &primitive->uintType, &scanfLen);
					if (scanfLen == stringLength && scanfRet == 1) ret = RSSL_RET_SUCCESS;
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_FLOAT: 
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib))) 
					ret = RSSL_RET_BLANK_DATA;
				else
				{
					scanfRet = sscanf(pXmlAttrib, "%f%n", &primitive->floatType, &scanfLen);
					if (scanfLen == stringLength && scanfRet == 1) ret = RSSL_RET_SUCCESS;
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_DOUBLE: 
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				{
					scanfRet = sscanf(pXmlAttrib, "%lf%n", &primitive->doubleType, &scanfLen);
					if (scanfLen == stringLength && scanfRet == 1) ret = RSSL_RET_SUCCESS;
				}
			}
			xmlFree(pXmlAttrib);
			break;

		case RSSL_DT_REAL:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				{
					dataBuffer.length = (RsslUInt32)stringLength; dataBuffer.data = pXmlAttrib;
					ret = rsslNumericStringToReal(&primitive->realType, &dataBuffer);
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_DATE:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
				   	ret = RSSL_RET_BLANK_DATA;
				else
				{
					dataBuffer.length = (RsslUInt32)stringLength; dataBuffer.data = pXmlAttrib;
					ret = rsslDateStringToDate(&primitive->dateType, &dataBuffer);
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_TIME:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				else
				{
					dataBuffer.length = (RsslUInt32)stringLength; dataBuffer.data = pXmlAttrib;
					ret = rsslTimeStringToTime(&primitive->timeType, &dataBuffer);
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_DATETIME:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				{
					dataBuffer.length = (RsslUInt32)stringLength; dataBuffer.data = pXmlAttrib;
					ret = rsslDateTimeStringToDateTime(&primitive->dateTimeType, &dataBuffer);
				}
			}
			xmlFree(pXmlAttrib);
			break;
		case RSSL_DT_QOS: 
			{
				rsslClearQos(&primitive->qosType);

				pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
				if (pXmlAttrib)
				{
					if (!(stringLength = strlen(pXmlAttrib)))
						ret = RSSL_RET_BLANK_DATA;
					else
						printf("Error: QoS \"data\" element is for indicating blank data and must be empty.\n");

					xmlFree(pXmlAttrib);
					break;
				}

				pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"qosTimeliness");
				if (pXmlAttrib)
				{
					stringToEnumRet = stringToEnum(qosTimeTable, qosTimeTableSize, pXmlAttrib);
					if (stringToEnumRet < 0) break;
					primitive->qosType.timeliness = stringToEnumRet;
				}
				xmlFree(pXmlAttrib);


				pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"qosRate");
				if (pXmlAttrib)
				{
					stringToEnumRet = stringToEnum(qosRateTable, qosRateTableSize, pXmlAttrib);
					if (stringToEnumRet < 0) break;
					primitive->qosType.rate = stringToEnumRet;
				}
				xmlFree(pXmlAttrib);

				pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"qosDynamic");
				if (pXmlAttrib)
				{
					RsslUInt8 qosDynamic;
					scanfRet = sscanf(pXmlAttrib, "%hhu%n", &qosDynamic, &scanfLen);
					if (scanfLen != strlen(pXmlAttrib) && scanfRet != 1)
					{
						printf("Error parsing value: %s", pXmlAttrib);
						break;
					}
					primitive->qosType.rate = stringToEnumRet;
				}
				xmlFree(pXmlAttrib);

				pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"qosTimeInfo");
				if (pXmlAttrib)
				{
					scanfRet = sscanf(pXmlAttrib, "%hu%n", &primitive->qosType.timeInfo, &scanfLen);
					if (scanfLen != strlen(pXmlAttrib) && scanfRet != 1)
					{
						printf("Error parsing value: %s", pXmlAttrib);
						break;
					}
				}
				xmlFree(pXmlAttrib);

				pXmlAttrib= (char*)xmlGetProp(pXmlElement, (xmlChar*)"qosRateInfo");
				if (pXmlAttrib)
				{
					scanfRet = sscanf(pXmlAttrib, "%hu%n", &primitive->qosType.rateInfo, &scanfLen);
					if (scanfLen != strlen(pXmlAttrib) && scanfRet != 1)
					{
						printf("Error parsing value: %s", pXmlAttrib);
						break;
					}
				}
				xmlFree(pXmlAttrib);

				ret = RSSL_RET_SUCCESS;
				break;
			}
		case RSSL_DT_STATE: 
			rsslClearState(&primitive->stateType);

			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				else
					printf("Error: State \"data\" element is for indicating blank data and must be empty.\n");

				xmlFree(pXmlAttrib);
				break;
			}

			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"streamState");
			if (pXmlAttrib)
			{
				stringToEnumRet = stringToEnum(streamStateTable, streamStateTableSize, pXmlAttrib);
				if (stringToEnumRet < 0) break;
				primitive->stateType.streamState = stringToEnumRet;
			}
			xmlFree(pXmlAttrib);

			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"dataState");
			if (pXmlAttrib)
			{
				stringToEnumRet = stringToEnum(dataStateTable, dataStateTableSize, pXmlAttrib);
				if (stringToEnumRet < 0) break;
				primitive->stateType.dataState = stringToEnumRet;
			}
			xmlFree(pXmlAttrib);

			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"code");
			if (pXmlAttrib)
			{
				stringToEnumRet = stringToEnum(stateCodeTable, stateCodeTableSize, pXmlAttrib);
				if (stringToEnumRet < 0) break;
				primitive->stateType.code = stringToEnumRet;
			}
			xmlFree(pXmlAttrib);

			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"text");
			if (pXmlAttrib)
			{
				primitive->stateType.text.length = (RsslUInt32)strlen(pXmlAttrib); 
				primitive->stateType.text.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, primitive->stateType.text.length + 1, sizeof(char));
				if (!primitive->stateType.text.data)
					return RSSL_RET_BUFFER_TOO_SMALL;
				strncpy(primitive->stateType.text.data, pXmlAttrib, (primitive->stateType.text.length + 1));
			}
			xmlFree(pXmlAttrib);

			ret = RSSL_RET_SUCCESS;

			break;
		case RSSL_DT_ENUM:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				if (!(stringLength = strlen(pXmlAttrib)))
					ret = RSSL_RET_BLANK_DATA;
				else
				{
					scanfRet = sscanf(pXmlAttrib, "%hu%n", &primitive->enumType, &scanfLen);
					if (scanfLen == stringLength && scanfRet == 1) ret = RSSL_RET_SUCCESS;
				}
			}
			xmlFree(pXmlAttrib);
			break;

		case RSSL_DT_BUFFER:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			pXmlAttrib = (char*)xmlGetProp(pXmlElement, (xmlChar*)"data");
			if (pXmlAttrib)
			{
				primitive->bufferType.length = (RsslUInt32)strlen(pXmlAttrib); 
				primitive->bufferType.data = (char*)rsslReserveAlignedBufferMemory(pMemoryBuffer, primitive->bufferType.length + 1, sizeof(char));
				if (!primitive->bufferType.data)
					return RSSL_RET_BUFFER_TOO_SMALL;
				strncpy(primitive->bufferType.data, pXmlAttrib,(primitive->bufferType.length + 1) );
				ret = RSSL_RET_SUCCESS;
			}
			xmlFree(pXmlAttrib);
			break;
		default: break;

	}

	return ret;
}

/* Parses the XML representation of field set definitions into a set definition database. */
static RsslRet parseFieldSetDefs( xmlNode *pXmlSetDefDb, RsslLocalFieldSetDefDb *pSetDefDb, RsslBuffer *pMemoryBuffer)
{
	RsslInt32 fieldEntryIndex = 0, setDefIndex = 0;
	xmlNode *pXmlSetDef;

	rsslClearLocalFieldSetDefDb(pSetDefDb);

	for (pXmlSetDef = pXmlSetDefDb->children; pXmlSetDef; pXmlSetDef = pXmlSetDef->next)
	{
		RsslInt32 setDefEntryIndex = 0, setDefEntryCount = 0;
		char *pXmlAttrib;
		xmlNode *pXmlSetDefEntry;
		RsslFieldSetDef *pSetDef;
		RsslFieldSetDefEntry *pEntries;
		RsslUInt16 setId;

		if (pXmlSetDef->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp((char*)pXmlSetDef->name, "fieldSetDef")) return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlSetDef->name), RSSL_RET_FAILURE);

		pXmlAttrib = (char*)xmlGetProp(pXmlSetDef, (xmlChar*)"setId");
		if (!pXmlAttrib) return(printf(XML_ERR_STR "missing attribute 'setId'\n"), RSSL_RET_FAILURE);
		sscanf(pXmlAttrib, "%hu", &setId);
		xmlFree(pXmlAttrib);

		assert(setId <= 15);
		pSetDef = &pSetDefDb->definitions[setId];
		pSetDef->setId = setId;
		pSetDef->count = 0;

		for (pXmlSetDefEntry = pXmlSetDef->children; pXmlSetDefEntry; pXmlSetDefEntry = pXmlSetDefEntry->next)
		{
			if (!strcmp((char*)pXmlSetDefEntry->name, "fieldSetDefEntry"))
				++setDefEntryCount;
		}

		pSetDef->pEntries = pEntries = (RsslFieldSetDefEntry*)rsslReserveAlignedBufferMemory(pMemoryBuffer, setDefEntryCount, sizeof(RsslFieldSetDefEntry));
		if (!pSetDef->pEntries)
			return RSSL_RET_BUFFER_TOO_SMALL;

		for (pXmlSetDefEntry = pXmlSetDef->children; pXmlSetDefEntry; pXmlSetDefEntry = pXmlSetDefEntry->next)
		{
			RsslInt32 enumRet;

			if (pXmlSetDefEntry->type != XML_ELEMENT_NODE)
				continue;

			if (!pEntries) return( printf("Error: Memory Buffer out of space.\n"), RSSL_RET_FAILURE );

			if (strcmp((char*)pXmlSetDefEntry->name, "fieldSetDefEntry")) 
				return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlSetDefEntry->name), RSSL_RET_FAILURE);

			pXmlAttrib = (char*)xmlGetProp(pXmlSetDefEntry, (xmlChar*)"fieldId");
			if (!pXmlAttrib) return(printf(XML_ERR_STR "missing attribute 'fieldId'\n"), RSSL_RET_FAILURE);
			sscanf(pXmlAttrib, "%hu", &pEntries->fieldId);
			xmlFree(pXmlAttrib);

			pXmlAttrib = (char*)xmlGetProp(pXmlSetDefEntry, (xmlChar*)"dataType");
			if (!pXmlAttrib) return(printf(XML_ERR_STR "missing attribute 'dataType'\n"), RSSL_RET_FAILURE);
			enumRet = stringToEnum(setTypeTable, setTypeTableSize, pXmlAttrib);
			if (enumRet < 0) return(printf(XML_ERR_STR "dataType '%s' not recognized\n", pXmlAttrib), RSSL_RET_FAILURE);
			pEntries->dataType = enumRet;
			xmlFree(pXmlAttrib);

			++pEntries;
			++pSetDef->count;
		}

	}
	return RSSL_RET_SUCCESS;
}

/* Parses fields in a field list. */
static RsslRet parseFieldList( xmlNode *pXmlFieldList, MarketField *pFieldEntries, RsslInt32 *pFieldEntriesCount, RsslBuffer *pMemoryBuffer)
{
	RsslInt32 fieldEntryIndex = 0;
	xmlNode *pXmlFieldEntry;
	RsslRet ret;

	*pFieldEntriesCount = 0;

	for(pXmlFieldEntry = pXmlFieldList->children; pXmlFieldEntry; pXmlFieldEntry = pXmlFieldEntry->next)
	{
		char *pAttr;
		MarketField *fieldEntryData = &pFieldEntries[*pFieldEntriesCount];
		RsslInt32 enumRet;
		const RsslDictionaryEntry *entry;

		if (pXmlFieldEntry->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp((char*)pXmlFieldEntry->name, "fieldEntry")) return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlFieldEntry->name), RSSL_RET_FAILURE);

		rsslClearFieldEntry(&fieldEntryData->fieldEntry);

		pAttr = (char*)xmlGetProp(pXmlFieldEntry, (xmlChar*)"fieldId");
		if (!pAttr) return(printf(XML_ERR_STR "missing attribute 'fieldId'\n"), RSSL_RET_FAILURE);
		sscanf(pAttr, "%d", &enumRet);
		fieldEntryData->fieldEntry.fieldId = enumRet;
		xmlFree(pAttr);

		/* Check for reserved fields. */
		switch(fieldEntryData->fieldEntry.fieldId)
		{
			case TIM_TRK_1_FID:
			case TIM_TRK_2_FID:
			case TIM_TRK_3_FID:
				return(printf(XML_ERR_STR "Field ID %u is reserved and not allowed in message data.\n",
							fieldEntryData->fieldEntry.fieldId), RSSL_RET_FAILURE);
			default:
				break;
		}

		pAttr = (char*)xmlGetProp(pXmlFieldEntry, (xmlChar*)"dataType");
		if (!pAttr) return(printf(XML_ERR_STR "missing attribute 'dataType'\n"), RSSL_RET_FAILURE);

		enumRet = stringToEnum(primitiveTypeTable, primitiveTypeTableSize, pAttr);
		if (enumRet < 0) return(printf(XML_ERR_STR "dataType '%s' not recognized\n", pAttr), RSSL_RET_FAILURE);
		fieldEntryData->fieldEntry.dataType = enumRet;
		xmlFree(pAttr);

		/* Make sure that the field exists in the dictionary and has the correct type. */
		entry = getDictionaryEntry(&dictionary, fieldEntryData->fieldEntry.fieldId);
		if (!entry)
			return (printf(XML_ERR_STR "Field %d not found in dictionary.\n", fieldEntryData->fieldEntry.fieldId), RSSL_RET_FAILURE);
		else if (entry->rwfType != fieldEntryData->fieldEntry.dataType) 
		{
			printf(XML_ERR_STR "Field with ID %d has dataType mismatch with dictionary(%s instead of %s) .\n", 
					fieldEntryData->fieldEntry.fieldId, 
					rsslDataTypeToString(fieldEntryData->fieldEntry.dataType), 
					rsslDataTypeToString(entry->rwfType));
			return RSSL_RET_FAILURE;
		}

		if ((ret = initPrimitive(&fieldEntryData->primitive, fieldEntryData->fieldEntry.dataType, pXmlFieldEntry, pMemoryBuffer)) != RSSL_RET_SUCCESS)
		{
			if (ret == RSSL_RET_BLANK_DATA)
				fieldEntryData->isBlank = RSSL_TRUE;
			else
			{
				printf("Error parsing primitive data for field with ID:%d and dataType %s\n", fieldEntryData->fieldEntry.fieldId, rsslDataTypeToString(fieldEntryData->fieldEntry.dataType));
				return ret;
			}
		}
		else
			fieldEntryData->isBlank = RSSL_FALSE;

		++(*pFieldEntriesCount);


	}
	return RSSL_RET_SUCCESS;
}

/* Attempts to estimate the size needed to encode the given data. */
static RsslUInt32 getEstimatedFieldListContentLength(MarketField *pFieldList, RsslInt32 fieldCount)
{
	RsslInt32 i, estimatedContentLength = 1;

	for(i = 0; i < fieldCount; ++i)
	{
		MarketField *pField = &pFieldList[i];

		estimatedContentLength += 2;

		switch(pField->fieldEntry.dataType)
		{
			case RSSL_DT_INT: 
				estimatedContentLength += sizeof(RsslInt); 
				break;
			case RSSL_DT_UINT: 
				estimatedContentLength += sizeof(RsslUInt); 
				break;
			case RSSL_DT_FLOAT: 
				estimatedContentLength += sizeof(RsslFloat); 
				break;
			case RSSL_DT_DOUBLE: 
				estimatedContentLength += sizeof(RsslDouble); 
				break;
			case RSSL_DT_REAL: 
				estimatedContentLength += sizeof(RsslReal); 
				break;
			case RSSL_DT_DATE: 
				estimatedContentLength += sizeof(RsslDate); 
				break;
			case RSSL_DT_TIME: 
				estimatedContentLength += sizeof(RsslTime); 
				break;
			case RSSL_DT_DATETIME: 
				estimatedContentLength += sizeof(RsslDateTime); 
				break;
			case RSSL_DT_QOS: 
				estimatedContentLength += sizeof(RsslQos); 
				break;
			case RSSL_DT_STATE: 
				estimatedContentLength += sizeof(RsslState); 
				estimatedContentLength += pField->primitive.stateType.text.length;
				break;
			case RSSL_DT_ENUM:
				estimatedContentLength += sizeof(RsslEnum); 
				break;
			case RSSL_DT_BUFFER:
			case RSSL_DT_ASCII_STRING:
			case RSSL_DT_UTF8_STRING:
			case RSSL_DT_RMTES_STRING:
				estimatedContentLength += sizeof(RsslBuffer); 
				estimatedContentLength += pField->primitive.bufferType.length;
				break;
			default: break;

		}
	}

	return estimatedContentLength;
}


RsslRet parseXMLMarketByOrder(xmlNode *pXmlMsgList)
{
	xmlNode *pXmlMsg;
	int scanfRet, scanfLen;
	RsslRet ret;

	RsslInt32 msgIndex = 0;
	RsslBool refreshMsgFound = RSSL_FALSE;
	xmlNode *iNode;
	RsslInt32 updateMsgCount = 0;
	RsslInt32 postMsgCount = 0;
	RsslInt32 genMsgCount = 0;

	memset(&xmlMarketByOrderMsgs, 0, sizeof(xmlMarketByOrderMsgs));

	for(iNode = pXmlMsgList->children; iNode; iNode = iNode->next)
	{
		if (!strcmp((char*)iNode->name, "updateMsg")) 
			++xmlMarketByOrderMsgs.updateMsgCount;
		if (!strcmp((char*)iNode->name, "postMsg")) 
			++xmlMarketByOrderMsgs.postMsgCount;
		if (!strcmp((char*)iNode->name, "genMsg")) 
			++xmlMarketByOrderMsgs.genMsgCount;
	}
	xmlMarketByOrderMsgs.updateMsgs = (MarketByOrderMsg*)rsslReserveAlignedBufferMemory(&mboMsgMemory, xmlMarketByOrderMsgs.updateMsgCount, sizeof(MarketByOrderMsg));
	if (!xmlMarketByOrderMsgs.updateMsgs)
		return RSSL_RET_BUFFER_TOO_SMALL;
	xmlMarketByOrderMsgs.postMsgs = (MarketByOrderMsg*)rsslReserveAlignedBufferMemory(&mboMsgMemory, xmlMarketByOrderMsgs.postMsgCount, sizeof(MarketByOrderMsg));
	if (!xmlMarketByOrderMsgs.postMsgs )
		return RSSL_RET_BUFFER_TOO_SMALL;
	xmlMarketByOrderMsgs.genMsgs = (MarketByOrderMsg*)rsslReserveAlignedBufferMemory(&mboMsgMemory, xmlMarketByOrderMsgs.genMsgCount, sizeof(MarketByOrderMsg));
	if (!xmlMarketByOrderMsgs.genMsgs )
		return RSSL_RET_BUFFER_TOO_SMALL;


	/* msgs */
	for(pXmlMsg = pXmlMsgList->children; pXmlMsg; pXmlMsg = pXmlMsg->next)
	{
		xmlNode *pContainerNode, *pDataBodyNode, *pXmlMap, *pXmlMapEntry;
		RsslInt32 mapEntryIndex = 0, mapEntryCount = 0;
		RsslBool refreshMsgFound = RSSL_FALSE;
		RsslBool setDefsFound = RSSL_FALSE;
		MarketByOrderMsg *orderBookMsgData;

		if (pXmlMsg->type != XML_ELEMENT_NODE)
			continue;

		if (!strcmp((char*)pXmlMsg->name, "refreshMsg")) 
		{
			if (refreshMsgFound)
				return(printf(XML_ERR_STR "duplicate refreshMsg found.\n"), RSSL_RET_FAILURE);
			refreshMsgFound = RSSL_TRUE;
			orderBookMsgData = &xmlMarketByOrderMsgs.refreshMsg;
			pContainerNode = pXmlMsg;
		}
		else if (!strcmp((char*)pXmlMsg->name, "updateMsg")) 
		{
			orderBookMsgData = &xmlMarketByOrderMsgs.updateMsgs[updateMsgCount++];
			pContainerNode = pXmlMsg;
		}
		else if (!strcmp((char*)pXmlMsg->name, "postMsg")) 
		{
			xmlNode *pUpdateMsgNode;
			orderBookMsgData = &xmlMarketByOrderMsgs.postMsgs[postMsgCount++];

			/* postMsg should contain updateMsg tag. */
			for (pUpdateMsgNode = pXmlMsg->children; pUpdateMsgNode && (strcmp((char*)pUpdateMsgNode->name, "updateMsg") || pUpdateMsgNode->type != XML_ELEMENT_NODE); pUpdateMsgNode = pUpdateMsgNode->next);
			if (!pUpdateMsgNode)
				return(printf(XML_ERR_STR "postMsg should contain exactly one element, 'updateMsg'\n"), RSSL_RET_FAILURE);

			pContainerNode = pUpdateMsgNode;
		}
		else if (!strcmp((char*)pXmlMsg->name, "genMsg")) 
		{
			orderBookMsgData = &xmlMarketByOrderMsgs.genMsgs[genMsgCount++];
			pContainerNode = pXmlMsg;
		}
		else
			return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlMsg->name), RSSL_RET_FAILURE);

		clearMarketByOrderMsg(orderBookMsgData);

		for (pDataBodyNode = pContainerNode->children; pDataBodyNode && (strcmp((char*)pDataBodyNode->name, "dataBody") || pDataBodyNode->type != XML_ELEMENT_NODE); pDataBodyNode = pDataBodyNode->next);
		if (!pDataBodyNode)
			return(printf(XML_ERR_STR "Message should contain exactly one element, 'dataBody'\n"), RSSL_RET_FAILURE);

		/* map */
		for (pXmlMap = pDataBodyNode->children; pXmlMap && (strcmp((char*)pXmlMap->name, "map") || pXmlMap->type != XML_ELEMENT_NODE); pXmlMap = pXmlMap->next);
		if (!pXmlMap)
			return(printf(XML_ERR_STR "dataBody should contain exactly one element, 'map'\n"), RSSL_RET_FAILURE);

		for(iNode = pXmlMap->children; iNode; iNode = iNode->next)
		{
			if (iNode->type != XML_ELEMENT_NODE)
				continue;

			if (!strcmp((char*)iNode->name, "mapEntry"))
				++mapEntryCount;
			else if (!strcmp((char*)iNode->name, "fieldSetDefs"))
				;
			else
				return(printf(XML_ERR_STR "map contained unknown element, '%s'\n", iNode->name), RSSL_RET_FAILURE);

		}

		orderBookMsgData->orders = (MarketOrder*)rsslReserveAlignedBufferMemory(&mboMsgMemory, mapEntryCount, sizeof(MarketOrder));
		if (!orderBookMsgData->orders)
			return RSSL_RET_BUFFER_TOO_SMALL;

		/* mapEntries */
		orderBookMsgData->orderCount = 0;
		for(pXmlMapEntry = pXmlMap->children; pXmlMapEntry; pXmlMapEntry = pXmlMapEntry->next)
		{
			xmlNode *pXmlFieldList;
			char *pAttr;
			RsslInt32 fieldEntryIndex = 0;
			MarketOrder *order = &orderBookMsgData->orders[orderBookMsgData->orderCount];
			RsslInt32 stringToEnumRet;

			if (pXmlMapEntry->type != XML_ELEMENT_NODE)
				continue;

			clearMarketOrder(order);

			if (!strcmp((char*)pXmlMapEntry->name, "mapEntry")) 
			{
				RsslInt32 fieldEntryCount = 0;

				pAttr = (char*)xmlGetProp(pXmlMapEntry, (xmlChar*)"key");
				if (!pAttr) return(printf(XML_ERR_STR "mapEntry has no key\n"), RSSL_RET_FAILURE);

				clearMarketOrder(order);

				order->orderId.data = order->orderIdString;
				order->orderId.length = snprintf(order->orderIdString, MAX_ORDER_ID_STRLEN, "%s", pAttr); 
				assert(order->orderId.length <= MAX_ORDER_ID_STRLEN);
				xmlFree(pAttr);

				pAttr = (char*)xmlGetProp(pXmlMapEntry, (xmlChar*)"action");
				if (!pAttr) return(printf(XML_ERR_STR "mapEntry has no action\n"), RSSL_RET_FAILURE);
				stringToEnumRet = stringToEnum(mapActionTable, mapActionTableSize, pAttr);
				if (stringToEnumRet < 0) return(printf(XML_ERR_STR "failed to parse map action: %s\n", pAttr), RSSL_RET_FAILURE);
				order->action = (RsslMapEntryActions)stringToEnumRet;
				xmlFree(pAttr);

				for (pXmlFieldList = pXmlMapEntry->children; pXmlFieldList && (strcmp((char*)pXmlFieldList->name, "fieldList") || pXmlFieldList->type != XML_ELEMENT_NODE); pXmlFieldList = pXmlFieldList->next);
				if (!pXmlFieldList)
					return(printf(XML_ERR_STR "mapEntry should contain exactly one element, 'fieldList'\n"), RSSL_RET_FAILURE);

				pAttr = (char*)xmlGetProp(pXmlFieldList, (xmlChar*)"setId");
				if (pAttr) 
				{
					scanfRet = sscanf(pAttr, "%d%n", &order->setId, &scanfLen);
					if (scanfLen != strlen(pAttr) || scanfRet != 1) return(printf(XML_ERR_STR "failed to parse setId: %s\n", pAttr), RSSL_RET_FAILURE);
				}
				xmlFree(pAttr);

				for(iNode = pXmlFieldList->children; iNode; iNode = iNode->next)
				{
					if (iNode->type != XML_ELEMENT_NODE)
						continue;

					if (strcmp((char*)iNode->name, "fieldEntry"))
						return(printf(XML_ERR_STR "fieldList contained unknown element, '%s'\n", iNode->name), RSSL_RET_FAILURE);
					++fieldEntryCount;

				}

				order->fieldEntries = (MarketField*)rsslReserveAlignedBufferMemory(&mboMsgMemory, fieldEntryCount, sizeof(MarketField));
				if (!order->fieldEntries)
					return RSSL_RET_BUFFER_TOO_SMALL;

				if ((ret = parseFieldList(pXmlFieldList, order->fieldEntries, &order->fieldEntriesCount, &mboMsgMemory)) != RSSL_RET_SUCCESS)
					return (ret);

				orderBookMsgData->estimatedContentLength += getEstimatedFieldListContentLength(order->fieldEntries, order->fieldEntriesCount);

				++orderBookMsgData->orderCount;
			}
			else if (!strcmp((char*)pXmlMapEntry->name, "fieldSetDefs")) 
			{
				if (orderBookMsgData->setDefDb) return(printf(XML_ERR_STR "duplicate set defs\n"), RSSL_RET_FAILURE);
				orderBookMsgData->setDefDb = (RsslLocalFieldSetDefDb*)rsslReserveAlignedBufferMemory(&mboMsgMemory, 1, sizeof(RsslLocalFieldSetDefDb)); 
				if(!orderBookMsgData->setDefDb)
					return RSSL_RET_BUFFER_TOO_SMALL;

				if ((ret = parseFieldSetDefs(pXmlMapEntry, orderBookMsgData->setDefDb, &mboMsgMemory)) != RSSL_RET_SUCCESS)
					return (ret);
			}
			else
				return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlMapEntry->name), RSSL_RET_FAILURE);


		}


	}
	
	return RSSL_RET_SUCCESS;
}

void xmlMsgDataCleanup()
{
	free(mpMsgMemoryOrig.data);
	free(mboMsgMemoryOrig.data);
	rsslDeleteDataDictionary(&dictionary);
}

RsslRet xmlMsgDataInit(const char *filename)
{
	RsslRet ret;

	xmlDoc *pXmlDoc;
	xmlNode *pXmlRoot, *pXmlMsgList;

	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};

	RsslUInt32 mpMemoryBlockSize = 6000;
	RsslUInt32 mboMemoryBlockSize = 6000;

	rsslClearDataDictionary(&dictionary);

	/* Add field information to dictionary */
	if (rsslLoadFieldDictionary(fieldDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("xmlMsgDataParser Error: Unable to load field dictionary '%s': %.*s\n"
				"xmlMsgDataParser cannot validate fields without the dictionary.\n", fieldDictionaryFileName, errorText.length, errorText.data);
		exit(-1);
	}

	/* Add enumerated types information to dictionary */
	if (rsslLoadEnumTypeDictionary(enumTypeDictionaryFileName, &dictionary, &errorText) < 0)
	{
		printf("xmlMsgDataParser Error: Unable to load enum type dictionary '%s': %.*s\n"
				"xmlMsgDataParser cannot validate fields without the dictionary.\n", enumTypeDictionaryFileName, errorText.length, errorText.data);
		exit(-1);
	}

	pXmlDoc = xmlReadFile(filename, NULL, 0);

	if (!pXmlDoc)
	{
		printf(XML_ERR_STR "xmlReadFile() failed.\n");
		return RSSL_RET_FAILURE;
	}

	xmlMsgDataHasMarketPrice = RSSL_FALSE;
	xmlMsgDataHasMarketByOrder = RSSL_FALSE;

	pXmlRoot = xmlDocGetRootElement(pXmlDoc);
	if (!pXmlRoot) return(printf(XML_ERR_STR "can't find root node '" XML_ROOT_NODE_NAME  "'\n"), xmlFreeDoc(pXmlDoc), RSSL_RET_FAILURE);

	for (pXmlMsgList = pXmlRoot->children; pXmlMsgList; pXmlMsgList = pXmlMsgList->next)
	{
		if (pXmlMsgList->type != XML_ELEMENT_NODE)
			continue;

		if (strcmp((char*)pXmlMsgList->name, XML_MP_ROOT_NODE_NAME) == 0)
		{
			do
			{
				mpMsgMemory.length = mpMemoryBlockSize;
				mpMsgMemory.data = (char*)malloc(mpMemoryBlockSize);
				if (!mpMsgMemory.data)
				{
					printf("malloc of mpMsgMemory failed.\n");
					abort();
				}
				mpMsgMemoryOrig = mpMsgMemory;

				ret = parseXMLMarketPrice(pXmlMsgList);

				switch(ret)
				{
					case RSSL_RET_SUCCESS:
						break;
					case RSSL_RET_BUFFER_TOO_SMALL:
						/* There wasn't enough space in the buffer to load all the data,
						 * try again with a larger buffer. */
						free(mpMsgMemoryOrig.data);
						mpMemoryBlockSize *= 2;
						continue;
					default:
						xmlFreeDoc(pXmlDoc);
						return RSSL_RET_FAILURE;
				}
			} while (ret != RSSL_RET_SUCCESS);

			xmlMsgDataHasMarketPrice = RSSL_TRUE;
		}
		else if (strcmp((char*)pXmlMsgList->name, XML_MBO_ROOT_NODE_NAME) == 0)
		{
			do
			{
				mboMsgMemory.length = mboMemoryBlockSize;
				mboMsgMemory.data = (char*)malloc(mboMemoryBlockSize);
				if (!mboMsgMemory.data)
				{
					printf("malloc of mboMsgMemory failed.\n");
					abort();
				}
				mboMsgMemoryOrig = mboMsgMemory;

				ret = parseXMLMarketByOrder(pXmlMsgList);

				switch(ret)
				{
					case RSSL_RET_SUCCESS:
						break;
					case RSSL_RET_BUFFER_TOO_SMALL:
						/* There wasn't enough space in the buffer to load all the data,
						 * try again with a larger buffer. */
						free(mboMsgMemoryOrig.data);
						mboMemoryBlockSize *= 2;
						continue;
					default:
						xmlFreeDoc(pXmlDoc);
						return RSSL_RET_FAILURE;
				}
			} while (ret != RSSL_RET_SUCCESS);

			xmlMsgDataHasMarketByOrder = RSSL_TRUE;
		}
		else
		{
			printf("Unrecognized element name '%s' while parsing %s.\n", pXmlMsgList->name, filename);
			xmlFreeDoc(pXmlDoc);
			return RSSL_RET_FAILURE;
		}
	}

	xmlFreeDoc(pXmlDoc);

	xmlMsgDataLoaded = RSSL_TRUE;
	printf("Data loaded from XML. Market Price Memory: %u bytes, Market By Order Memory: %u bytes\n", mpMsgMemory.length, mboMsgMemory.length);
	return RSSL_RET_SUCCESS;
}

RsslRet parseXMLMarketPrice(xmlNode *pXmlMsgList)
{
	RsslRet ret;
	xmlNode *pXmlMsg;
	RsslInt32 msgIndex = 0;
	RsslBool refreshMsgFound = RSSL_FALSE;
	xmlNode *iNode;
	RsslInt32 updateMsgCount = 0;
	RsslInt32 postMsgCount = 0;
	RsslInt32 genMsgCount = 0;

	memset(&xmlMarketPriceMsgs, 0, sizeof(xmlMarketPriceMsgs));

	for(iNode = pXmlMsgList->children; iNode; iNode = iNode->next)
	{
		if (!strcmp((char*)iNode->name, "updateMsg")) 
			++xmlMarketPriceMsgs.updateMsgCount;
		else if (!strcmp((char*)iNode->name, "postMsg"))
			++xmlMarketPriceMsgs.postMsgCount;
		else if (!strcmp((char*)iNode->name, "genMsg"))
			++xmlMarketPriceMsgs.genMsgCount;
	}
	xmlMarketPriceMsgs.updateMsgs = (MarketPriceMsg*)rsslReserveAlignedBufferMemory(&mpMsgMemory, xmlMarketPriceMsgs.updateMsgCount, sizeof(MarketPriceMsg));
	if (!xmlMarketPriceMsgs.updateMsgs )
		return RSSL_RET_BUFFER_TOO_SMALL;
	xmlMarketPriceMsgs.postMsgs = (MarketPriceMsg*)rsslReserveAlignedBufferMemory(&mpMsgMemory, xmlMarketPriceMsgs.postMsgCount, sizeof(MarketPriceMsg));
	if (!xmlMarketPriceMsgs.postMsgs )
		return RSSL_RET_BUFFER_TOO_SMALL;
	xmlMarketPriceMsgs.genMsgs = (MarketPriceMsg*)rsslReserveAlignedBufferMemory(&mpMsgMemory, xmlMarketPriceMsgs.genMsgCount, sizeof(MarketPriceMsg));
	if (!xmlMarketPriceMsgs.genMsgs )
		return RSSL_RET_BUFFER_TOO_SMALL;

	for(pXmlMsg = pXmlMsgList->children; pXmlMsg; pXmlMsg = pXmlMsg->next)
	{
		xmlNode *pContainerNode, *pDataBodyNode, *pXmlFieldList;
		RsslInt32 fieldEntryIndex = 0, fieldEntryCount = 0;
		MarketPriceMsg *mpMsgData;

		if (pXmlMsg->type != XML_ELEMENT_NODE)
			continue;


		if (!strcmp((char*)pXmlMsg->name, "refreshMsg")) 
		{
			if (refreshMsgFound)
				return(printf(XML_ERR_STR "duplicate refreshMsg found.\n"), RSSL_RET_FAILURE);
			refreshMsgFound = RSSL_TRUE;
			mpMsgData = &xmlMarketPriceMsgs.refreshMsg;
			pContainerNode = pXmlMsg;
		}
		else if (!strcmp((char*)pXmlMsg->name, "updateMsg")) 
		{
			mpMsgData = &xmlMarketPriceMsgs.updateMsgs[updateMsgCount++];
			pContainerNode = pXmlMsg;
		}
		else if (!strcmp((char*)pXmlMsg->name, "postMsg")) 
		{
			xmlNode *pUpdateMsgNode;
			mpMsgData = &xmlMarketPriceMsgs.postMsgs[postMsgCount++];

			/* postMsg should contain updateMsg tag. */
			for (pUpdateMsgNode = pXmlMsg->children; pUpdateMsgNode && (strcmp((char*)pUpdateMsgNode->name, "updateMsg") || pUpdateMsgNode->type != XML_ELEMENT_NODE); pUpdateMsgNode = pUpdateMsgNode->next);
			if (!pUpdateMsgNode)
				return(printf(XML_ERR_STR "postMsg should contain exactly one element, 'updateMsg'\n"), RSSL_RET_FAILURE);

			pContainerNode = pUpdateMsgNode;
		}
		else if (!strcmp((char*)pXmlMsg->name, "genMsg")) 
		{
			mpMsgData = &xmlMarketPriceMsgs.genMsgs[genMsgCount++];
			pContainerNode = pXmlMsg;
		}
		else
			return(printf(XML_ERR_STR "unknown node name '%s'\n", pXmlMsg->name), RSSL_RET_FAILURE);

		for (pDataBodyNode = pContainerNode->children; pDataBodyNode && (strcmp((char*)pDataBodyNode->name, "dataBody") || pDataBodyNode->type != XML_ELEMENT_NODE); pDataBodyNode = pDataBodyNode->next);
		if (!pDataBodyNode)
			return(printf(XML_ERR_STR "Message should contain exactly one element, 'dataBody'\n"), RSSL_RET_FAILURE);

		for (pXmlFieldList = pDataBodyNode->children; pXmlFieldList && (strcmp((char*)pXmlFieldList->name, "fieldList") || pXmlFieldList->type != XML_ELEMENT_NODE); pXmlFieldList = pXmlFieldList->next);
		if (!pXmlFieldList)
			return(printf(XML_ERR_STR "dataBody should contain exactly one element, 'fieldList'\n"), RSSL_RET_FAILURE);

		for(iNode = pXmlFieldList->children; iNode; iNode = iNode->next)
		{
			if (iNode->type != XML_ELEMENT_NODE)
				continue;

			if (strcmp((char*)iNode->name, "fieldEntry"))
				return(printf(XML_ERR_STR "fieldList contained unknown element, '%s'\n", iNode->name), RSSL_RET_FAILURE);
			++fieldEntryCount;

		}

		mpMsgData->fieldEntries = (MarketField*)rsslReserveAlignedBufferMemory(&mpMsgMemory, fieldEntryCount, sizeof(MarketField));
		if (!mpMsgData->fieldEntries)
			return RSSL_RET_BUFFER_TOO_SMALL;

		if ((ret = parseFieldList(pXmlFieldList, mpMsgData->fieldEntries, &mpMsgData->fieldEntriesCount, &mpMsgMemory)) != RSSL_RET_SUCCESS)
			return ret;

		/* Roughly estimate the amount of data in the message to help us figure out an appropriate buffer for encoding. */
		mpMsgData->estimatedContentLength = getEstimatedFieldListContentLength(mpMsgData->fieldEntries, mpMsgData->fieldEntriesCount);


	}

	if (!refreshMsgFound)
		return(printf(XML_ERR_STR "refreshMsg not found\n"), RSSL_RET_FAILURE);


	return RSSL_RET_SUCCESS;
}
