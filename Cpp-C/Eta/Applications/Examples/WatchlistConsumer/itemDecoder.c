/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/* 
 * This file demonstrates the decoding of data from the various domains such as
 * MarketPrice and MarketByOrder.
 */

#include "itemDecoder.h"
#include "watchlistConsumerConfig.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslDataPackage.h"

static RsslDataDictionary dictionary;
RsslBool fieldDictionaryLoaded = RSSL_FALSE;
RsslBool enumDictionaryLoaded = RSSL_FALSE;

/* Prints spaces for indentation. */
static void printIndent(RsslUInt32 indentCount)
{
	RsslUInt32 indentIndex;
	for (indentIndex = 0; indentIndex < indentCount; ++indentIndex)
		printf("  ");
}

/* Converts map entry actions to strings. */
const char *mapActionToString(RsslMapEntryActions action)
{
	switch(action)
	{
		case RSSL_MPEA_ADD_ENTRY: return "RSSL_MPEA_ADD_ENTRY";
		case RSSL_MPEA_UPDATE_ENTRY: return "RSSL_MPEA_UPDATE_ENTRY";
		case RSSL_MPEA_DELETE_ENTRY: return "RSSL_MPEA_DELETE_ENTRY";
		default: return "Unknown";
	}
}

/* Converts vector entry actions to strings. */
const char *vectorActionToString(RsslVectorEntryActions action)
{
	switch(action)
	{
		case RSSL_VTEA_UPDATE_ENTRY: return "RSSL_VTEA_UPDATE_ENTRY";
		case RSSL_VTEA_SET_ENTRY: return "RSSL_VTEA_SET_ENTRY";
		case RSSL_VTEA_CLEAR_ENTRY: return "RSSL_VTEA_CLEAR_ENTRY";
		case RSSL_VTEA_INSERT_ENTRY: return "RSSL_VTEA_INSERT_ENTRY";
		case RSSL_VTEA_DELETE_ENTRY: return "RSSL_VTEA_DELETE_ENTRY";
		default: return "Unknown";
	}
}

/* Initializes the item decoder. Loads dictionaries from their files if the files
 * are found in the application's folder. */
RsslRet itemDecoderInit()
{
	char errorString[128];
	RsslBuffer errorBuffer = { 128, errorString };
	RsslRet ret;

	rsslClearDataDictionary(&dictionary);

	if ((ret = rsslLoadFieldDictionary("RDMFieldDictionary", &dictionary, &errorBuffer))
			!= RSSL_RET_SUCCESS)
	{
		printf("rsslLoadFieldDictionary failed: %d(%.*s)\n  Will attempt to request dictionaries from provider.\n\n", ret, errorBuffer.length, 
				errorBuffer.data);
		rsslDeleteDataDictionary(&dictionary);
		return ret;
	}

	if ((ret = rsslLoadEnumTypeDictionary("enumtype.def", &dictionary, &errorBuffer))
			!= RSSL_RET_SUCCESS)
	{
		printf("rsslLoadEnumTypeDictionary failed: %d(%.*s)\n  Will attempt to request dictionaries from provider.\n\n", ret, errorBuffer.length, 
				errorBuffer.data);
		rsslDeleteDataDictionary(&dictionary);
		return ret;
	}

	fieldDictionaryLoaded = RSSL_TRUE;
	enumDictionaryLoaded = RSSL_TRUE;

	return RSSL_RET_SUCCESS;
}

/* Cleans up the item decoder. */
void itemDecoderCleanup()
{
	rsslDeleteDataDictionary(&dictionary);
}

/* Decodes the payload of a message according to its domain. */
RsslRet decodeDataBody(RsslReactorChannel *pChannel, RsslMsg *pRsslMsg)
{
	switch (pRsslMsg->msgBase.domainType)
	{
		case RSSL_DMT_MARKET_PRICE:
			return decodeMarketPriceDataBody(pChannel, pRsslMsg);

		case RSSL_DMT_MARKET_BY_ORDER:
			return decodeMarketByOrderDataBody(pChannel, pRsslMsg);

		case RSSL_DMT_MARKET_BY_PRICE:
			return decodeMarketByPriceDataBody(pChannel, pRsslMsg);

		case RSSL_DMT_YIELD_CURVE:
			return decodeYieldCurveDataBody(pChannel, pRsslMsg);
			

		case RSSL_DMT_SYMBOL_LIST:
			return decodeSymbolListDataBody(pChannel, pRsslMsg);

		default:
			printf("Received message with unhandled domain %u\n\n",
					pRsslMsg->msgBase.domainType);
			return RSSL_RET_FAILURE;
	}
}

/* Decodes and prints the entries of an RsslArray. */
static RsslRet decodeArray(RsslDecodeIterator *pIter, RsslUInt32 indentCount)
{
	RsslRet ret;
	RsslArray rsslArray;
	RsslBuffer arrayEntry;
	RsslBool firstArrayEntry = RSSL_TRUE;

	printf("{ ");

	if ((ret = rsslDecodeArray(pIter, &rsslArray) == RSSL_RET_SUCCESS))
	{

		while ((ret = rsslDecodeArrayEntry(pIter, &arrayEntry)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret != RSSL_RET_SUCCESS)
			{
				printf("Error in rsslDecodeArrayEntry(): %d(%s)", ret, rsslRetCodeToString(ret));
				return ret;
			}

			if (firstArrayEntry)
				firstArrayEntry = RSSL_FALSE;
			else
				printf(", ");

			if ((ret = decodeDataType(pIter, rsslArray.primitiveType, NULL, indentCount + 1))
					!= RSSL_RET_SUCCESS)
				return ret;

		}
	}
	else
	{
		printf("Error in rsslDecodeArray(): %d(%s)\n", ret, rsslRetCodeToString(ret));
		return ret;
	}

	printf(" }");

	return RSSL_RET_SUCCESS;
}

/* Decodes and prints the entries of an RsslVector. */
static RsslRet decodeVector(RsslDecodeIterator *dIter, RsslUInt32 indentCount)
{
	RsslRet retVal = 0;
	RsslRet retValVector = 0;
	RsslLocalFieldSetDefDb fieldSetDefDb;	
	RsslVector rsslVector;
	RsslVectorEntry vectorEntry;
	
	/* decode contents into the vector structure */
	/* decode our vector header */
	if ((retVal = rsslDecodeVector(dIter, &rsslVector)) < RSSL_RET_SUCCESS)
	{
		/* decoding failures tend to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}

	if (retVal == RSSL_RET_NO_DATA)
	{
		printIndent(indentCount); printf("<no data>\n");
		return RSSL_RET_SUCCESS;
	}

	if (rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA)
	{
		if (decodeFieldList(dIter, NULL, indentCount) != RSSL_RET_SUCCESS)
			return RSSL_RET_FAILURE;
	}

	/* If the vector flags indicate that set definition content is present, decode the set def db */
	if (rsslVector.flags & RSSL_VTF_HAS_SET_DEFS)
	{
		/* must ensure it is the correct type - if map contents are field list, this is a field set definition db */
		if (rsslVector.containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&fieldSetDefDb);
			if ((retVal = rsslDecodeLocalFieldSetDefDb(dIter, &fieldSetDefDb)) < RSSL_RET_SUCCESS)
			{
				/* decoding failures tend to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeLocalFieldSetDefDb().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
				return retVal;
			}		
		}
	}

	printf("\n");

	/* decode each vector entry */
	/* since this succeeded, we can decode fields until we reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
	while ((retValVector = rsslDecodeVectorEntry(dIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retValVector < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retValVector), retValVector, rsslRetCodeInfo(retValVector));
			return retValVector;
		}


		printIndent(indentCount); printf("INDEX: %d\n", vectorEntry.index);
		printIndent(indentCount); printf("ACTION: %s\n", 
				vectorActionToString((RsslVectorEntryActions)vectorEntry.action));

		switch (rsslVector.containerType)
		{
			case RSSL_DT_FIELD_LIST:
				if (decodeFieldList(dIter, &fieldSetDefDb, indentCount + 1) != RSSL_RET_SUCCESS)
					return RSSL_RET_FAILURE;
				break;
			default:
				printf("Error: Vector contained unhandled containerType %d.\n", 
						rsslVector.containerType);
				return RSSL_RET_FAILURE;
		}
	}

	return retVal;
}

/* Decodes and prints data according to its type. This is used by the decodeFieldList
 * and decodeArray functions to print their individual entries. */
static RsslRet decodeDataType(RsslDecodeIterator *pIter, RsslUInt8 dataType, 
		RsslDictionaryEntry *dictionaryEntry, RsslUInt32 indentCount)
{
	RsslUInt64 fidUIntValue;
	RsslInt64 fidIntValue;
	RsslReal fidRealValue;
	RsslEnum fidEnumValue;
	RsslFloat fidFloatValue;
	RsslDouble fidDoubleValue;
	RsslQos fidQosValue;
	RsslDateTime fidDateTimeValue;
	RsslState fidStateValue;
	RsslBuffer fidBufferValue;
	RsslBuffer fidDateTimeBuf;
	RsslBuffer fidRealBuf;
	RsslBuffer fidStateBuf;
	RsslBuffer fidQosBuf;
	RsslRet ret;

	switch (dataType)
	{
		case RSSL_DT_UINT:
			if ((ret = rsslDecodeUInt(pIter, &fidUIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLU, fidUIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeUInt() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_INT:
			if ((ret = rsslDecodeInt(pIter, &fidIntValue)) == RSSL_RET_SUCCESS)
			{
				printf(RTR_LLD, fidIntValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeInt() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_FLOAT:
			if ((ret = rsslDecodeFloat(pIter, &fidFloatValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f", fidFloatValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeFloat() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_DOUBLE:
			if ((ret = rsslDecodeDouble(pIter, &fidDoubleValue)) == RSSL_RET_SUCCESS) 
			{
				printf("%f", fidDoubleValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDouble() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_REAL:
			if ((ret = rsslDecodeReal(pIter, &fidRealValue)) == RSSL_RET_SUCCESS)
			{
				fidRealBuf.data = (char*)alloca(35);
				fidRealBuf.length = 35;
				rsslRealToString(&fidRealBuf, &fidRealValue);
				printf("%s", fidRealBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeReal() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_ENUM:
			if ((ret = rsslDecodeEnum(pIter, &fidEnumValue)) == RSSL_RET_SUCCESS)
			{
				RsslEnumType *pEnumType;
				if (dictionaryEntry && (pEnumType = getFieldEntryEnumType(dictionaryEntry, fidEnumValue)))
					printf("%.*s(%d)", pEnumType->display.length, pEnumType->display.data, fidEnumValue);
				else
					printf("%d", fidEnumValue);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeEnum() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATE:
			if ((ret = rsslDecodeDate(pIter, &fidDateTimeValue.date)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATE, &fidDateTimeValue);
				printf("%s", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDate() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_TIME:
			if ((ret = rsslDecodeTime(pIter, &fidDateTimeValue.time)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(30);
				fidDateTimeBuf.length = 30;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_TIME, &fidDateTimeValue);
				printf("%s", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeTime() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_DATETIME:
			if ((ret = rsslDecodeDateTime(pIter, &fidDateTimeValue)) == RSSL_RET_SUCCESS)
			{
				fidDateTimeBuf.data = (char*)alloca(50);
				fidDateTimeBuf.length = 50;
				rsslDateTimeToString(&fidDateTimeBuf, RSSL_DT_DATETIME, &fidDateTimeValue);
				printf("%s", fidDateTimeBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeDateTime() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_QOS:
			if((ret = rsslDecodeQos(pIter, &fidQosValue)) == RSSL_RET_SUCCESS) {
				fidQosBuf.data = (char*)alloca(100);
				fidQosBuf.length = 100;
				rsslQosToString(&fidQosBuf, &fidQosValue);
				printf("%s", fidQosBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeQos() failed with return code: %d", ret);
				return ret;
			}
			break;
		case RSSL_DT_STATE:
			if((ret = rsslDecodeState(pIter, &fidStateValue)) == RSSL_RET_SUCCESS) {
				int stateBufLen = 80;
				if (fidStateValue.text.data)
					stateBufLen += fidStateValue.text.length;
				fidStateBuf.data = (char*)alloca(stateBufLen);
				fidStateBuf.length = stateBufLen;
				rsslStateToString(&fidStateBuf, &fidStateValue);
				printf("%.*s", fidStateBuf.length, fidStateBuf.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA)
			{
				printf("rsslDecodeState() failed with return code: %d", ret);
				return ret;
			}
			break;

		case RSSL_DT_BUFFER:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			if((ret = rsslDecodeBuffer(pIter, &fidBufferValue)) == RSSL_RET_SUCCESS)
			{
				printf("%.*s", fidBufferValue.length, fidBufferValue.data);
			}
			else if (ret != RSSL_RET_BLANK_DATA) 
			{
				printf("rsslDecodeBuffer() failed with return code: %d", ret);
				return ret;
			}
			break;

		case RSSL_DT_ARRAY:
			/* A field containing an array is typically encountered with Yield Curve data. */
			 return decodeArray(pIter, indentCount);

		case RSSL_DT_VECTOR:
			 /* A field containing a vector is typically encountered with Yield Curve data. */
			 return decodeVector(pIter, indentCount + 1);

		default:
			printf("Unsupported data type (%d) for fid value", dictionaryEntry->rwfType);
			break;
	}

	if (ret == RSSL_RET_BLANK_DATA)
	{
		printf("<blank data>");
	}

	return RSSL_RET_SUCCESS;
}

/* Decodes and prints the entries of an RsslFieldList. */
static RsslRet decodeFieldList(RsslDecodeIterator *pIter, RsslLocalFieldSetDefDb *pSetDefDb,
		RsslUInt32 indentCount)
{
	RsslRet ret;
	RsslFieldList fieldList;
	RsslFieldEntry fieldEntry;

	if ((ret = rsslDecodeFieldList(pIter, &fieldList, pSetDefDb)) != RSSL_RET_SUCCESS)
	{
		printf("  Error in rsslDecodeFieldList: %d(%s)\n", ret,
				rsslRetCodeToString(ret));
		return ret;
	}

	while((ret = rsslDecodeFieldEntry(pIter, &fieldEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		RsslDictionaryEntry *dictionaryEntry;

		if (ret != RSSL_RET_SUCCESS)
		{
			printf("  Error in rsslDecodeFieldEntry: %d(%s)\n", ret,
					rsslRetCodeToString(ret));
			return ret;
		}

		dictionaryEntry = dictionary.entriesArray[fieldEntry.fieldId];
		if (dictionaryEntry)
		{
			printIndent(indentCount); 
			printf("%-*.*s ", 20 - indentCount * 2, dictionaryEntry->acronym.length, 
					dictionaryEntry->acronym.data); 

			if ((ret = decodeDataType(pIter, dictionaryEntry->rwfType, dictionaryEntry,
					indentCount)) != RSSL_RET_SUCCESS)
				return ret;

			printf("\n");
		}
		else
			printf("  Field %d (no dictionary entry)\n", fieldEntry.fieldId);
	}

	return RSSL_RET_SUCCESS;
}

/* Decodes dictionary information.  This is used when the application requests dictionaries
 * from the provider. This function uses the helper functions for decoding field dictionary data
 * and enumerated types dictionary data.  This information is stored in a dictionary structure
 * that will be used when decoding field list data from item streams. */
RsslRet decodeDictionaryDataBody(RsslReactorChannel *pReactorChannel, 
		RsslRDMDictionaryRefresh *pDictionaryRefresh)
{
	RsslDecodeIterator dIter;
	RsslRet ret;
	char errorTextString[256];
	RsslBuffer errorText = { 256, errorTextString };

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pDictionaryRefresh->dataBody);

	switch(pDictionaryRefresh->rdmMsgBase.streamId)
	{
		case FIELD_DICTIONARY_STREAM_ID:

			if ((ret = rsslDecodeFieldDictionary(&dIter, &dictionary, 
							RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
			{
				printf("  Error in rsslDecodeFieldDictionary: %d(%s -- %.*s)\n", ret,
						rsslRetCodeToString(ret), errorText.length, errorText.data);
				return ret;
			}

			if (pDictionaryRefresh->flags & RDM_DC_RFF_IS_COMPLETE)
				fieldDictionaryLoaded = RSSL_TRUE;
			break;

		case ENUM_DICTIONARY_STREAM_ID:

			if ((ret = rsslDecodeEnumTypeDictionary(&dIter, &dictionary, 
							RDM_DICTIONARY_NORMAL, &errorText)) != RSSL_RET_SUCCESS)
			{
				printf("  Error in rsslDecodeFieldDictionary: %d(%s -- %.*s)\n", ret,
						rsslRetCodeToString(ret), errorText.length, errorText.data);
				return ret;
			}

			if (pDictionaryRefresh->flags & RDM_DC_RFF_IS_COMPLETE)
				enumDictionaryLoaded = RSSL_TRUE;
			break;
		default:
			printf("  Dictionary message received on unknown stream %d\n",
					pDictionaryRefresh->rdmMsgBase.streamId);
			return RSSL_RET_FAILURE;
	}

	return RSSL_RET_SUCCESS;
}

/* Decodes a MarketPrice payload.
 * MarketPrice is represented as an RsslFieldList, where each field contains data about the item. */
RsslRet decodeMarketPriceDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg)
{
	RsslDecodeIterator dIter;
	RsslRet ret;

	if (!fieldDictionaryLoaded || !enumDictionaryLoaded)
	{
		printf("  (Dictionary not loaded).\n");
		return RSSL_RET_FAILURE;
	}

	if (pRsslMsg->msgBase.containerType != RSSL_DT_FIELD_LIST)
	{
		printf("  Incorrect container type: %u\n", pRsslMsg->msgBase.containerType);
		return RSSL_RET_FAILURE;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);
	if ((ret = decodeFieldList(&dIter, NULL, 1)) != RSSL_RET_SUCCESS)
		return ret;

	printf("\n");

	return RSSL_RET_SUCCESS;
}

/* Decodes a MarketByOrder payload.
 * MarketByOrder is represented as an RsslMap, where each entry represents an order.
 * Each entry contains an RsslFieldList which contains data about that order. */
RsslRet decodeMarketByOrderDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg)
{
	RsslDecodeIterator dIter;
	RsslRet ret;
	RsslMap	map;
	RsslMapEntry mapEntry;
	RsslBuffer itemName;
	RsslLocalFieldSetDefDb fieldSetDefDb, *pFieldSetDefDb = NULL;

	if (pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
	{
		printf("  Incorrect container type: %u\n", pRsslMsg->msgBase.containerType);
		return RSSL_RET_FAILURE;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

	if ((ret = rsslDecodeMap(&dIter, &map)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (map.containerType != RSSL_DT_FIELD_LIST)
	{
		printf("Incorrect Market By Order map container type %u.", map.keyPrimitiveType);
		return RSSL_RET_FAILURE;
	}

	if (map.keyPrimitiveType != RSSL_DT_BUFFER
			&& map.keyPrimitiveType != RSSL_DT_ASCII_STRING
			&& map.keyPrimitiveType != RSSL_DT_RMTES_STRING)
	{
		printf("Incorrect Market By Order map key type %u.", map.keyPrimitiveType);
		return RSSL_RET_FAILURE;
	}

	if (map.flags & RSSL_MPF_HAS_SET_DEFS)
	{
		rsslClearLocalFieldSetDefDb(&fieldSetDefDb);
		if ((ret = rsslDecodeLocalFieldSetDefDb(&dIter, &fieldSetDefDb)) != RSSL_RET_SUCCESS)
		{
			printf("rsslDecodeLocalFieldSetDefDb() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
			return ret;
		}
		pFieldSetDefDb = &fieldSetDefDb;
	}

	if (map.flags & RSSL_MPF_HAS_SUMMARY_DATA)
	{
		printf("  Summary Data:\n");

		if ((ret = decodeFieldList(&dIter, pFieldSetDefDb, 2)) != RSSL_RET_SUCCESS)
			return ret;

		printf("\n");
	}

	while ((ret = rsslDecodeMapEntry(&dIter, &mapEntry, &itemName)) !=
			RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			printf("rsslDecodeMapEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
			return ret;
		}

		printf("  %-6s : %.*s\n", mapActionToString((RsslMapEntryActions)mapEntry.action), itemName.length, 
				itemName.data);

		if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
			if ((ret = decodeFieldList(&dIter, pFieldSetDefDb, 2)) != RSSL_RET_SUCCESS)
				return ret;

		printf("\n");
	}

	printf("\n");

	return RSSL_RET_SUCCESS;
}

/* Decodes a MarketByPrice payload.
 * MarketByPrice is represented as an RsslMap, where each entry represents a price point.
 * Each entry contains an RsslFieldList which contains data about that price point. */
RsslRet decodeMarketByPriceDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg)
{
	RsslDecodeIterator dIter;
	RsslRet ret;
	RsslMap	map;
	RsslMapEntry mapEntry;
	RsslBuffer itemName;
	RsslLocalFieldSetDefDb fieldSetDefDb, *pFieldSetDefDb = NULL;

	if (pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
	{
		printf("  Incorrect container type: %u\n", pRsslMsg->msgBase.containerType);
		return RSSL_RET_FAILURE;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

	if ((ret = rsslDecodeMap(&dIter, &map)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
		return ret;
	}

	if (map.containerType != RSSL_DT_FIELD_LIST)
	{
		printf("Incorrect Market By Price map container type %u.", map.keyPrimitiveType);
		return RSSL_RET_FAILURE;
	}

	if (map.keyPrimitiveType != RSSL_DT_BUFFER)
	{
		printf("Incorrect Market By Price map key type %u.", map.keyPrimitiveType);
		return RSSL_RET_FAILURE;
	}

	if (map.flags & RSSL_MPF_HAS_SET_DEFS)
	{
		rsslClearLocalFieldSetDefDb(&fieldSetDefDb);
		if ((ret = rsslDecodeLocalFieldSetDefDb(&dIter, &fieldSetDefDb)) != RSSL_RET_SUCCESS)
		{
			printf("rsslDecodeLocalFieldSetDefDb() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
			return ret;
		}
		pFieldSetDefDb = &fieldSetDefDb;
	}

	if (map.flags & RSSL_MPF_HAS_SUMMARY_DATA)
	{
		printf("  Summary Data:\n");

		if ((ret = decodeFieldList(&dIter, pFieldSetDefDb, 2)) != RSSL_RET_SUCCESS)
			return ret;

		printf("\n");
	}

	while ((ret = rsslDecodeMapEntry(&dIter, &mapEntry, &itemName)) !=
			RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			printf("rsslDecodeMapEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
			return ret;
		}

		printf("  %-6s : %.*s\n", mapActionToString((RsslMapEntryActions)mapEntry.action), itemName.length, 
				itemName.data);

		if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
			if ((ret = decodeFieldList(&dIter, pFieldSetDefDb, 2)) != RSSL_RET_SUCCESS)
				return ret;

		printf("\n");
	}

	printf("\n");

	return RSSL_RET_SUCCESS;
}

/* Decodes a YieldCurve payload.
 * YieldCurve is represented as an RsslFieldList, where each field contains data about the item. */
RsslRet decodeYieldCurveDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg)
{
	RsslDecodeIterator dIter;
	RsslRet ret;

	if (!fieldDictionaryLoaded || !enumDictionaryLoaded)
	{
		printf("  (Dictionary not loaded).\n");
		return RSSL_RET_FAILURE;
	}

	if (pRsslMsg->msgBase.containerType != RSSL_DT_FIELD_LIST)
	{
		printf("  Incorrect container type: %u\n", pRsslMsg->msgBase.containerType);
		return RSSL_RET_FAILURE;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);
	if ((ret = decodeFieldList(&dIter, NULL, 1)) != RSSL_RET_SUCCESS)
		return ret;

	printf("\n");

	return RSSL_RET_SUCCESS;
}


/* Decodes a SymbolList payload.
 * SymbolList is represented as an RsslMap, where each entry represents an item.
 * The entries are indexed by the item's name. */
RsslRet decodeSymbolListDataBody(RsslReactorChannel *pReactorChannel, RsslMsg *pRsslMsg)
{
	RsslDecodeIterator dIter;
	RsslRet ret;
	RsslMap	map;
	RsslMapEntry mapEntry;
	RsslBuffer itemName;

	if (pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
	{
		printf("  Incorrect container type: %u\n", pRsslMsg->msgBase.containerType);
		return RSSL_RET_FAILURE;
	}

	rsslClearDecodeIterator(&dIter);
	rsslSetDecodeIteratorRWFVersion(&dIter, pReactorChannel->pRsslChannel->majorVersion,
			pReactorChannel->pRsslChannel->minorVersion);
	rsslSetDecodeIteratorBuffer(&dIter, &pRsslMsg->msgBase.encDataBody);

	if ((ret = rsslDecodeMap(&dIter, &map)) != RSSL_RET_SUCCESS)
	{
		printf("rsslDecodeMap() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
		return RSSL_RET_FAILURE;
	}

	if (map.keyPrimitiveType != RSSL_DT_BUFFER
			&& map.keyPrimitiveType != RSSL_DT_ASCII_STRING
			&& map.keyPrimitiveType != RSSL_DT_RMTES_STRING)
	{
		printf("Incorrect Symbol List map key type %u.", map.keyPrimitiveType);
		return RSSL_RET_FAILURE;
	}

	while ((ret = rsslDecodeMapEntry(&dIter, &mapEntry, &itemName)) !=
			RSSL_RET_END_OF_CONTAINER)
	{
		if (ret != RSSL_RET_SUCCESS)
		{
			printf("rsslDecodeMapEntry() failed: %d(%s)\n", ret, rsslRetCodeToString(ret));
			return ret;
		}

		printf("  Name: %-20.*s Action: %s\n", itemName.length, itemName.data,
				mapActionToString((RsslMapEntryActions)mapEntry.action));
	}

	printf("\n");

	return RSSL_RET_SUCCESS;
}
