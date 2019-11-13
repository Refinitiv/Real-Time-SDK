/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslMsgDecoders.h"
#include "xmlDump.h"
#include "xmlMsgDump.h"
#include "xmlDomainDump.h"
#include "decodeRoutines.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/retmacros.h"
#include "rtr/intDataTypes.h"
#include "fieldListDefs.h"
#include "rtr/rsslIteratorUtilsInt.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslSetData.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern int indents;

FieldList * g_fieldLists = 0;
const char* getUserIdTypeAsString( RDMLoginUserIdTypes userIdType )
{
	switch (userIdType)
	{
		case RDM_LOGIN_USER_EMAIL_ADDRESS:
			return "RDM_LOGIN_USER_EMAIL_ADDRESS";
		case RDM_LOGIN_USER_NAME:
			return "RDM_LOGIN_USER_NAME";
		case RDM_LOGIN_USER_TOKEN:
			return "RDM_LOGIN_USER_TOKEN";
		case RDM_LOGIN_USER_COOKIE:
			return "RDM_LOGIN_USER_COOKIE";
		default:
			return "";
	}
}

RSSL_API const char* getInstrumentNameTypeAsString (RDMInstrumentNameTypes symbol)
{
	switch (symbol)
	{
		case RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED: return "RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED";
		case RDM_INSTRUMENT_NAME_TYPE_RIC: return "RDM_INSTRUMENT_NAME_TYPE_RIC";
		case RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR: return "RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR";

		default:
			return "";
	}
}

RSSL_API const char  *getStreamStateAsString(RsslUInt8 code)
{
    switch (code)
    {
    case RSSL_STREAM_UNSPECIFIED:       return "RSSL_STREAM_UNSPECIFIED";
    case RSSL_STREAM_OPEN:              return "RSSL_STREAM_OPEN";
    case RSSL_STREAM_NON_STREAMING:     return "RSSL_STREAM_NON_STREAMING";
    case RSSL_STREAM_CLOSED_RECOVER:    return "RSSL_STREAM_CLOSED_RECOVER";
    case RSSL_STREAM_CLOSED:            return "RSSL_STREAM_CLOSED";
    case RSSL_STREAM_REDIRECTED:        return "RSSL_STREAM_REDIRECTED";
      default:                            return "Unknown stream state";
    }
}

RSSL_API const char * getDataStateAsString(RsslUInt8 code)
{
   switch (code)
    {
	case RSSL_DATA_NO_CHANGE:		return "RSSL_DATA_NO_CHANGE";
    case RSSL_DATA_OK:              return "RSSL_DATA_OK";
    case RSSL_DATA_SUSPECT:         return "RSSL_DATA_SUSPECT";
    default:                        return "Unknown Data State";
    }
}

RSSL_API const char* getStateCodeAsString(RsslUInt8 code)
{
	switch (code)
	{
	case	RSSL_SC_NONE: return "RSSL_SC_NONE";
	case	RSSL_SC_NOT_FOUND: return "RSSL_SC_NOT_FOUND";
	case	RSSL_SC_TIMEOUT: return "RSSL_SC_TIMEOUT";
	case	RSSL_SC_NOT_ENTITLED: return "RSSL_SC_NOT_ENTITLED";
	case	RSSL_SC_INVALID_ARGUMENT: return "RSSL_SC_INVALID_ARGUMENT";
	case	RSSL_SC_USAGE_ERROR: return "RSSL_SC_USAGE_ERROR";
	case	RSSL_SC_PREEMPTED: return "RSSL_SC_PREEMPTED";
	case	RSSL_SC_JIT_CONFLATION_STARTED: return "RSSL_SC_JIT_CONFLATION_STARTED";
	case	RSSL_SC_REALTIME_RESUMED: return "RSSL_SC_REALTIME_RESUMED";
	case	RSSL_SC_FAILOVER_STARTED: return "RSSL_SC_FAILOVER_STARTED";
	case	RSSL_SC_FAILOVER_COMPLETED: return "RSSL_SC_FAILOVER_COMPLETED";
	case	RSSL_SC_GAP_DETECTED: return "RSSL_SC_GAP_DETECTED";
	case	RSSL_SC_NO_RESOURCES: return "RSSL_SC_NO_RESOURCES";
	case	RSSL_SC_TOO_MANY_ITEMS: return "RSSL_SC_TOO_MANY_ITEMS";
	case	RSSL_SC_ALREADY_OPEN:	return "RSSL_SC_ALREADY_OPEN";
	case	RSSL_SC_SOURCE_UNKNOWN: return "RSSL_SC_SOURCE_UNKNOWN";
	case	RSSL_SC_NOT_OPEN: return "RSSL_SC_NOT_OPEN";
	case	RSSL_SC_NON_UPDATING_ITEM: return "RSSL_SC_NON_UPDATING_ITEM";
	case	RSSL_SC_UNSUPPORTED_VIEW_TYPE: return "RSSL_SC_UNSUPPORTED_VIEW_TYPE";
	case	RSSL_SC_INVALID_VIEW: return "RSSL_SC_INVALID_VIEW";
	case	RSSL_SC_FULL_VIEW_PROVIDED: return "RSSL_SC_FULL_VIEW_PROVIDED";
	case	RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH: return "RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH";
	case	RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ: return "RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ";
	case	RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER: return "RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER";
	case	RSSL_SC_ERROR: return "RSSL_SC_ERROR";
	case	RSSL_SC_DACS_DOWN: return "RSSL_SC_DACS_DOWN";
	case	RSSL_SC_USER_UNKNOWN_TO_PERM_SYS: return "RSSL_SC_USER_UNKNOWN_TO_PERM_SYS";
	case	RSSL_SC_DACS_MAX_LOGINS_REACHED: return "RSSL_SC_DACS_MAX_LOGINS_REACHED";
	case	RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED: return "RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED";
	case	RSSL_SC_GAP_FILL: return "RSSL_SC_GAP_FILL";
	case	RSSL_SC_APP_AUTHORIZATION_FAILED: return "RSSL_SC_APP_AUTHORIZATION_FAILED";
    default:   return "";
    }
}

RSSL_API const char* getNakCodeAsString(RsslUInt8 code)
{
	switch (code)
	{
		case RSSL_NAKC_NONE: return "RSSL_NAKC_NONE";
		case RSSL_NAKC_ACCESS_DENIED: return "RSSL_NAKC_ACCESS_DENIED";
		case RSSL_NAKC_DENIED_BY_SRC: return "RSSL_NAKC_DENIED_BY_SRC";
		case RSSL_NAKC_SOURCE_DOWN: return "RSSL_NAKC_SOURCE_DOWN";
		case RSSL_NAKC_SOURCE_UNKNOWN: return "RSSL_NAKC_SOURCE_UNKNOWN";
		case RSSL_NAKC_NO_RESOURCES: return "RSSL_NAKC_NO_RESOURCES";
		case RSSL_NAKC_NO_RESPONSE: return "RSSL_NAKC_NO_RESPONSE";
		case RSSL_NAKC_GATEWAY_DOWN: return "RSSL_NAKC_GATEWAY_DOWN";
		case RSSL_NAKC_SYMBOL_UNKNOWN: return "RSSL_NAKC_SYMBOL_UNKNOWN";
		case RSSL_NAKC_NOT_OPEN: return "RSSL_NAKC_NOT_OPEN";
		case RSSL_NAKC_INVALID_CONTENT: return "RSSL_NAKC_INVALID_CONTENT";
	      
		default:   return "";
	}
}
	            

/* getMsgName
   returns a Message name based on the message class */
const char * getMsgName( const RsslMsg *pMsg )
{
    switch( pMsg->msgBase.msgClass )
    {
		case RSSL_MC_UPDATE			: 
            return "updateMsg";

		case RSSL_MC_GENERIC		:
			return "genericMsg";

	    case RSSL_MC_REFRESH			: 
            return "refreshMsg";			

		case RSSL_MC_REQUEST			: 
            return "requestMsg"; 
 		
		case RSSL_MC_POST			:
			return "postMsg";

	    case RSSL_MC_STATUS			: 
            return "statusMsg";

		case RSSL_MC_CLOSE			:
			return "closeMsg";

		case RSSL_MC_ACK:
			return "ackMsg";
    };
	return "";
}

int decodeDataStringToXML(FILE * file, RsslDataType type, RsslBuffer * buffer, RsslDecodeIterator *pIter)
{
	RsslRet ret;
	RsslBuffer out = RSSL_INIT_BUFFER;

	out.length = (buffer->length > 265) ? buffer->length + 1 : 266;
	out.data = alloca(out.length);
	ret = rsslEncodedPrimitiveToString(pIter, type, &out);
	RSSL_CHECKRET(ret);
	
	fprintf(file, " data=\"");
	xmlDumpString(file, ret, out.data);
	return ret;
}

RSSL_API int decodeDataTypeToXML(FILE * file, RsslDataType dataType, RsslBuffer * buffer, const RsslDataDictionary * dictionary, void * setDb, RsslDecodeIterator *iter)
{
	RsslDouble r64 = 0;
	RsslFloat r32 = 0;
	RsslInt64 i64 = 0;
	RsslUInt64 u64 = 0;
	RsslUInt16 u16 = 0;
	RsslDateTime datetime = RSSL_INIT_DATETIME;
	RsslReal oReal64 = RSSL_INIT_REAL;
	RsslQos qos = RSSL_INIT_QOS;
	RsslState state = RSSL_INIT_STATE;

	RsslRet ret;
	RsslBuffer out = RSSL_INIT_BUFFER;

	switch (dataType)
	{
		case RSSL_DT_INT:
			ret = rsslDecodeInt(iter, &i64);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf(file, " data=\"");
				xmlDumpInt(file, i64);
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_UINT:
			ret = rsslDecodeUInt(iter, &u64);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf(file, " data=\"");
				xmlDumpUInt(file, u64);
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_FLOAT:
			ret = rsslDecodeFloat(iter, &r32);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf(file, " data=\"");
				xmlDumpDouble(file, r32);
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_DOUBLE:
			ret = rsslDecodeDouble(iter, &r64);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf (file, " data=\"");
				xmlDumpDouble(file, r64);
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_ENUM:
			ret = rsslDecodeEnum(iter, &u16);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf (file, " data=\"");
				xmlDumpInt(file, u16);
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_QOS:
            ret = rsslDecodeQos(iter, &qos);
			if (ret == RSSL_RET_SUCCESS)
			{
				xmlDumpQos(file, &qos);
			}
        break;

		case RSSL_DT_STATE:
			ret = rsslDecodeState(iter, &state);
			if (ret == RSSL_RET_SUCCESS)
			{
				xmlDumpState(file, &state);
			}
		break;

		case RSSL_DT_BUFFER	:
		case RSSL_DT_ASCII_STRING:
		case RSSL_DT_UTF8_STRING:
		case RSSL_DT_RMTES_STRING:
			ret = buffer->length;
			out.length = buffer->length + 1;
			out.data = alloca(out.length);
			rsslEncodedPrimitiveToString(iter, dataType, &out);
			fprintf (file, " data=\"");
			xmlDumpString(file, out.length, out.data);
		break;

		case RSSL_DT_DATE:
			ret = rsslDecodeDate(iter, &datetime.date);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf (file, " data=\"");
				xmlDumpDate(file, &datetime.date);				
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;	

		case RSSL_DT_TIME:
			ret = rsslDecodeTime(iter, &datetime.time);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf (file, " data=\"");
				xmlDumpTime(file, &datetime.time);				
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_DATETIME:
			ret = rsslDecodeDateTime(iter, &datetime);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf (file, " data=\"");
				xmlDumpDateTime(file, &datetime);				
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;	

		case RSSL_DT_REAL:
			ret = rsslDecodeReal(iter, &oReal64);
			if (ret == RSSL_RET_SUCCESS)
			{
				fprintf(file, " data=\"");
				xmlDumpReal(file, &oReal64);				
			}
			else if (ret == RSSL_RET_BLANK_DATA)
			{
				fprintf(file, " data=\"\"");
			}
		break;

		case RSSL_DT_ELEMENT_LIST:
			ret = decodeElementListToXML(file, iter, dictionary, (RsslLocalElementSetDefDb*) setDb);
		break;

		case RSSL_DT_ARRAY:
			ret = decodeArrayToXML(file, iter, dictionary);
		break;
	
		case RSSL_DT_FIELD_LIST:
			ret = decodeFieldListToXML(file, iter, dictionary, (RsslLocalFieldSetDefDb*) setDb);
		break;
		
		case RSSL_DT_FILTER_LIST:
			ret = decodeFilterListToXML(file, iter, dictionary);
		break;
	
		case RSSL_DT_VECTOR:
			ret = decodeVectorToXML(file, iter, dictionary);
		break;
		
		case RSSL_DT_MAP:
			ret = decodeMapToXML(file, iter, dictionary);
		break;
	
		case RSSL_DT_SERIES:
			ret = decodeSeriesToXML(file, iter, dictionary);
		break;

		case RSSL_DT_ANSI_PAGE:
			ret = decodeAnsiPageToXML(file, buffer, dictionary);
		break;

		case RSSL_DT_MSG:
  			ret = decodeNestedRwfMsgToXML(file, iter, dictionary);
  		break;

		case RSSL_DT_NO_DATA:
			return 0;
	
		case RSSL_DT_JSON: /* at some point in the future, this could invoke the OMM JSON helpers to dump as JSON if we want */
			ret = decodeJSONToXML(file, buffer, dictionary);
		break;

		case RSSL_DT_OPAQUE:
			ret = dumpOpaqueToXML(file, buffer, dictionary);	
		break;
		
		default:
			ret = dumpOpaqueToXML(file, buffer, dictionary);
	}
	return ret;
}

RSSL_API int decodeElementListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary, RsslLocalElementSetDefDb * setDb)
{
	RsslRet ret = 0;
	RsslElementList eList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry element = RSSL_INIT_ELEMENT_ENTRY;

	ret = rsslDecodeElementList(iter, &eList, setDb);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);
	
	xmlDumpElementListBegin(file, &eList);
	
	while ((ret = rsslDecodeElementEntry(iter, &element)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		xmlDumpElementBegin(file, &element);
		if ((element.dataType >= RSSL_DT_CONTAINER_TYPE_MIN && element.dataType <= RSSL_DT_CONTAINER_TYPE_MAX) || 
			(element.dataType == RSSL_DT_ARRAY) ||
			(element.dataType >= RSSL_DT_MAX_RESERVED))
			fprintf(file, ">\n");
		if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_GROUP))
		{
			fprintf(file, " data=\"");
			xmlDumpGroupId(file, &element.encData);
			fprintf(file, "\"");
			ret = RSSL_RET_SUCCESS;
		}
		else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_MERG_TO_GRP))
		{
			fprintf(file, " data=\"");
			xmlDumpGroupId(file, &element.encData);
			fprintf(file, "\"");
			ret = RSSL_RET_SUCCESS;
		}
		else
		{
   			ret = decodeDataTypeToXML(file, element.dataType, &element.encData, dictionary, setDb, iter);
		}
		RSSL_CHECKRET(ret);
		if ((element.dataType >= RSSL_DT_CONTAINER_TYPE_MIN && element.dataType <= RSSL_DT_CONTAINER_TYPE_MAX) || 
			(element.dataType == RSSL_DT_ARRAY) ||
			(element.dataType >= RSSL_DT_MAX_RESERVED))
			xmlDumpElementEnd(file);
		else
			xmlDumpEndNoTag(file);
	}

	xmlDumpElementListEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeFieldListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary, RsslLocalFieldSetDefDb * fieldListSetDb)
{
	RsslRet ret = 0;
	RsslDataType dataType = RSSL_DT_UNKNOWN;
	RsslFieldList fList = RSSL_INIT_FIELD_LIST;
	RsslFieldEntry field = RSSL_INIT_FIELD_ENTRY;

	ret = rsslDecodeFieldList(iter, &fList, fieldListSetDb);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);
	
	xmlDumpFieldListBegin(file, &fList);

	while ((ret = rsslDecodeFieldEntry(iter, &field)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		if (!dictionary || !dictionary->entriesArray || (dictionary->entriesArray[field.fieldId] == 0))
			dataType = field.dataType;
		else
			dataType = dictionary->entriesArray[field.fieldId]->rwfType;

		xmlDumpFieldBegin(file, &field, dataType );

		if ((dataType >= RSSL_DT_CONTAINER_TYPE_MIN && dataType <= RSSL_DT_CONTAINER_TYPE_MAX) || 
			(dataType == RSSL_DT_ARRAY) ||
			(dataType >= RSSL_DT_MAX_RESERVED))
			fprintf( file, ">\n");

		if (dataType != RSSL_DT_UNKNOWN)
		{
			ret = decodeDataTypeToXML(file, dataType, &field.encData, dictionary, fieldListSetDb, iter);
		}
		else
		{
			fprintf(file, " data=\"");
			xmlDumpHexBuffer(file, &field.encData);
			fprintf(file, "\"");
		}

		if ((dataType >= RSSL_DT_CONTAINER_TYPE_MIN && dataType <= RSSL_DT_CONTAINER_TYPE_MAX) || 
			(dataType == RSSL_DT_ARRAY) ||
			(dataType >= RSSL_DT_MAX_RESERVED))
			xmlDumpFieldEnd(file);
		else
			xmlDumpEndNoTag(file);
		
		RSSL_CHECKRET(ret);
	}

	xmlDumpFieldListEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeSummaryData(FILE *file, RsslDecodeIterator *iter, RsslContainerType containerType, const RsslBuffer * input, const RsslUInt8 majorVer, const RsslUInt8 minorVer, const RsslDataDictionary * dictionary, void * setDb)
{
	int ret = 0;

	xmlDumpSummaryDataBegin(file);
	
	ret = decodeDataTypeToXML(file, containerType, (RsslBuffer*)input, dictionary, setDb, iter);

	xmlDumpSummaryDataEnd(file);

	return ret;
}

RSSL_API int decodeMapToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslMap rsslMap = RSSL_INIT_MAP;
	int ret = 0;
	RsslLocalElementSetDefDb elListSetDb;
	RsslLocalFieldSetDefDb flListSetDb;
	void * setDb = 0;
	RsslMapEntry mapEntry;
	char mapKeyData[32];
	
	ret = rsslDecodeMap(iter, &rsslMap);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);
	
	xmlDumpMapBegin(file, &rsslMap);

	if (rsslMapCheckHasSetDefs(&rsslMap))
	{
		if (rsslMap.containerType == RSSL_DT_FIELD_LIST)
		{	
			rsslClearLocalFieldSetDefDb(&flListSetDb);
			rsslDecodeLocalFieldSetDefDb(iter, &flListSetDb);
			setDb = &flListSetDb;
			xmlDumpLocalFieldSetDefDb(file, &flListSetDb);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&elListSetDb);
			rsslDecodeLocalElementSetDefDb(iter, &elListSetDb);
			setDb = &elListSetDb;
			xmlDumpLocalElementSetDefDb(file, &elListSetDb);
		}
	}

	/* dump summary data */
	if (rsslMapCheckHasSummaryData(&rsslMap))
		decodeSummaryData(file, iter, rsslMap.containerType, &rsslMap.encSummaryData, iter->_majorVersion, iter->_minorVersion, dictionary, setDb);

	rsslClearMapEntry(&mapEntry);
	while ((ret = rsslDecodeMapEntry(iter, &mapEntry, mapKeyData)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		xmlDumpMapEntryBegin(file, rsslMap.keyPrimitiveType, &mapEntry, mapKeyData);
   		ret = decodeDataTypeToXML(file, rsslMap.containerType, &mapEntry.encData, dictionary, setDb, iter);
		xmlDumpMapEntryEnd(file);
	
		rsslClearMapEntry(&mapEntry);
		RSSL_CHECKRET(ret);
	}
	
	xmlDumpMapEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeVectorToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslVector vec = RSSL_INIT_VECTOR;
	int ret = 0;
	RsslLocalElementSetDefDb elListSetDb;
	RsslLocalFieldSetDefDb flListSetDb;
	void * setDb = 0;
	RsslVectorEntry vectorEntry;

	ret = rsslDecodeVector(iter, &vec);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);

	xmlDumpVectorBegin(file, &vec);

	if (rsslVectorCheckHasSetDefs(&vec))
	{
		if (vec.containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&flListSetDb);
			rsslDecodeLocalFieldSetDefDb(iter, &flListSetDb);
			setDb = &flListSetDb;
			xmlDumpLocalFieldSetDefDb(file, &flListSetDb);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&elListSetDb);
			rsslDecodeLocalElementSetDefDb(iter, &elListSetDb);
			setDb = &elListSetDb;
			xmlDumpLocalElementSetDefDb(file, &elListSetDb);
		}
	}

	/* dump summary data */
	if (rsslVectorCheckHasSummaryData(&vec))
		decodeSummaryData(file, iter, vec.containerType, &vec.encSummaryData, iter->_majorVersion, iter->_majorVersion, dictionary, setDb);

	rsslClearVectorEntry(&vectorEntry);
	while ((ret = rsslDecodeVectorEntry(iter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		xmlDumpVectorEntryBegin(file, &vectorEntry);
   		ret = decodeDataTypeToXML(file, vec.containerType, &vectorEntry.encData, dictionary, setDb, iter);
		xmlDumpVectorEntryEnd(file);
		
		rsslClearVectorEntry(&vectorEntry);
		RSSL_CHECKRET(ret);
	}

	xmlDumpVectorEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeArrayToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslArray rsslArray = RSSL_INIT_ARRAY;
	RsslBuffer arrayItem = RSSL_INIT_BUFFER;
	int ret = 0;

	ret = rsslDecodeArray(iter, &rsslArray);
	if (ret == RSSL_RET_BLANK_DATA) return ret;
	RSSL_CHECKRET(ret);
	
	xmlDumpArrayBegin(file, &rsslArray);

	while ((ret = rsslDecodeArrayEntry(iter, &arrayItem)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);
	
		xmlDumpArrayItemBegin(file);

		ret = decodeDataTypeToXML(file, rsslArray.primitiveType, &arrayItem, dictionary, 0, iter);

		xmlDumpArrayItemEnd(file);
		
		RSSL_CHECKRET(ret);
	}

	xmlDumpArrayEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeFilterListToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslFilterList fList = RSSL_INIT_FILTER_LIST;
	RsslFilterEntry filterItem = RSSL_INIT_FILTER_ENTRY;
	int ret = 0;

	ret = rsslDecodeFilterList(iter, &fList);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);

	xmlDumpFilterListBegin(file, &fList);

	while ((ret = rsslDecodeFilterEntry(iter, &filterItem)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		xmlDumpFilterItemBegin(file, &filterItem);
	
		ret = decodeDataTypeToXML(file, filterItem.containerType, &filterItem.encData, dictionary, 0, iter);
		xmlDumpFilterItemEnd(file);
	
		RSSL_CHECKRET(ret);
	}

	xmlDumpFilterListEnd(file);

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeAnsiPageToXML(FILE * file, const RsslBuffer * input, const RsslDataDictionary * dictionary)
{
	encodeindents(file);
	indents++;
	
	fprintf(file, "<ansiPage data=\"");
	xmlDumpHexBuffer(file, input);
	fprintf(file, "\"/>\n");

	indents--;

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeJSONToXML(FILE * file, const RsslBuffer *input, const RsslDataDictionary * dictionary)
{
	encodeindents(file);
	indents++;

	fprintf(file, "<json data=\"");
	/* dump like ASCII/XML */
	xmlDumpString(file, input->length, input->data);
	fprintf(file, ">\n");

	indents--;

	return RSSL_RET_SUCCESS;
}

RSSL_API int dumpOpaqueToXML(FILE * file, const RsslBuffer * input, const RsslDataDictionary * dictionary)
{
	encodeindents(file);
	indents++;
	
	fprintf(file, "<opaque data=\"");
	xmlDumpHexBuffer(file, input);
	fprintf(file, "\" />\n");

	indents--;

	return RSSL_RET_SUCCESS;
}

RSSL_API int decodeSeriesToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslSeries series = RSSL_INIT_SERIES;
	RsslSeriesEntry row = RSSL_INIT_SERIES_ENTRY;

	int ret = 0;

	RsslLocalElementSetDefDb elListSetDb;
	RsslLocalFieldSetDefDb flListSetDb;
	void * setDb = 0;

	ret = rsslDecodeSeries(iter, &series);
	if (ret == RSSL_RET_NO_DATA) return ret;
	RSSL_CHECKRET(ret);
	
	xmlDumpSeriesBegin(file, &series);

	if (rsslSeriesCheckHasSetDefs(&series))
	{
		if (series.containerType == RSSL_DT_FIELD_LIST)
		{
			rsslClearLocalFieldSetDefDb(&flListSetDb);
			rsslDecodeLocalFieldSetDefDb(iter, &flListSetDb);
			setDb = &flListSetDb;
			xmlDumpLocalFieldSetDefDb(file, &flListSetDb);
		}
		else
		{
			rsslClearLocalElementSetDefDb(&elListSetDb);
			rsslDecodeLocalElementSetDefDb(iter, &elListSetDb);
			setDb = &elListSetDb;
			xmlDumpLocalElementSetDefDb(file, &elListSetDb);
		}
	}

	/* dump summary data */
	if (rsslSeriesCheckHasSummaryData(&series))
		decodeSummaryData(file, iter, series.containerType, &series.encSummaryData, iter->_majorVersion, iter->_minorVersion, dictionary, setDb);

	if (rsslDecodeItemCount(iter) == 0)
	{
		xmlDumpSeriesEnd(file);
		return RSSL_RET_SUCCESS;
	}

	while ((ret = rsslDecodeSeriesEntry(iter, &row)) != RSSL_RET_END_OF_CONTAINER)
	{
		RSSL_CHECKRET(ret);

		xmlDumpSeriesRowBegin(file, &row);
		
		ret = decodeDataTypeToXML(file, series.containerType, &row.encData, dictionary, setDb, iter);
		RSSL_CHECKRET(ret);
		xmlDumpSeriesRowEnd(file);
	}

	xmlDumpSeriesEnd(file);

	return RSSL_RET_SUCCESS;
}

RsslRet decodeReqKeysToXML(FILE * file, const RsslMsgKey * key, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslRet ret = 0;

	xmlDumpReqKeyBegin(file, key);
	if (key->flags & RSSL_MKF_HAS_ATTRIB)
	{
		decodeKeyOpaque(file, key, iter , dictionary);
		xmlDumpReqKeyEnd(file);
	}
	return ret;
}

RSSL_API RsslRet decodeKeysToXML(FILE * file, const RsslMsgKey * key, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslRet ret = 0;

	xmlDumpKeyBegin(file, key);
	if (key->flags & RSSL_MKF_HAS_ATTRIB)
	{
		decodeKeyOpaque(file, key, iter , dictionary);
		xmlDumpKeyEnd(file);
	}
	return ret;
}

RSSL_API RsslRet decodeMsgClassToXML(FILE * file, const RsslMsg * msg, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	switch (msg->msgBase.msgClass)
	{
	case RSSL_MC_UPDATE:
		if (msg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
		{

    		decodeKeysToXML(file, &msg->msgBase.msgKey, iter,  dictionary);
		}
		if (msg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->updateMsg.extendedHeader);
		}
		break;

	case RSSL_MC_REFRESH:
		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY)
		{
    		decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);
		}
		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY)
		{
    		decodeReqKeysToXML(file, &msg->refreshMsg.reqMsgKey, iter, dictionary);
		}
		if (msg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->refreshMsg.extendedHeader);
		}
		break;
	
	case RSSL_MC_REQUEST:
		decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);

		if (msg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->requestMsg.extendedHeader);
		}
		break;
	
	case RSSL_MC_GENERIC:
		if(msg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY)
			decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);

		if (msg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
		{
    		decodeReqKeysToXML(file, &msg->genericMsg.reqMsgKey, iter, dictionary);
		}
		if (msg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->genericMsg.extendedHeader);
		}
			
		break;

	case RSSL_MC_POST:
		if (msg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY)
		{
    		decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);
		}

		if (msg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->postMsg.extendedHeader);
		}
		break;

	case RSSL_MC_STATUS:
		if (msg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY)
		{
    		decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);
		}
		if (msg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY)
		{
    		decodeReqKeysToXML(file, &msg->statusMsg.reqMsgKey, iter, dictionary);
		}
		if (msg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->statusMsg.extendedHeader);
		}
		break;

	case RSSL_MC_CLOSE:
		if (msg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->closeMsg.extendedHeader);
		}

		break;

	case RSSL_MC_ACK:
		if (msg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY)
		{
			decodeKeysToXML(file, &msg->msgBase.msgKey, iter, dictionary);
		}

		if (msg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER)
		{
			xmlDumpExtendedHeader(file, &msg->ackMsg.extendedHeader);
		}
		break;
	}

	return RSSL_RET_SUCCESS;
}

static RsslRet decodeMsgToXMLShared(FILE * file, const RsslMsg * msg, const RsslDataDictionary * dictionary, RsslDecodeIterator *iter, RsslBool isNested)
{
	RsslRet ret;
	const char *tagName = getMsgName( (const RsslMsg *)msg);
	
	fprintf(file, "<!-- rwfMajorVer=\"%d\" rwfMinorVer=\"%d\" -->\n", iter->_majorVersion, iter->_minorVersion);
	xmlDumpMsgBegin(file, msg, tagName);

	ret = decodeMsgClassToXML(file, msg, iter, dictionary);
	RSSL_CHECKRET(ret);

	xmlDumpDataBodyBegin ( file);

	ret = decodeDataTypeToXML(file, msg->msgBase.containerType, (RsslBuffer*)&msg->msgBase.encDataBody, dictionary, 0, iter);
	RSSL_CHECKRET(ret);
	
	xmlDumpDataBodyEnd ( file);

	xmlDumpMsgEnd(file, tagName, isNested);

	return ret;
}

/* version should be set on the msg here */
RSSL_API RsslRet decodeMsgToXML(FILE * file, const RsslMsg * msg, const RsslDataDictionary * dictionary, RsslDecodeIterator *iter)
{
	RsslDecodeIterator xmlIter;

	/* set xml iterator to same version and buffer of iterator passed into function */
	/* this is done so passed in iterater is not altered by this function */
	rsslClearDecodeIterator(&xmlIter);
	xmlIter._majorVersion = iter->_majorVersion;
	xmlIter._minorVersion = iter->_minorVersion;
	rsslSetDecodeIteratorBuffer(&xmlIter, &((RsslMsg *)msg)->msgBase.encDataBody);

	return decodeMsgToXMLShared(file, msg, dictionary, &xmlIter, RSSL_FALSE);
}

RSSL_API RsslRet decodeNestedRwfMsgToXML(FILE * file, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslRet ret;
	RsslMsg msg;

	rsslClearMsg(&msg);
	ret = rsslDecodeMsg(iter, &msg); RSSL_CHECKRET(ret);

	return decodeMsgToXMLShared(file, &msg, dictionary, iter, RSSL_TRUE);
}

RSSL_API RsslRet decodeKeyOpaque(FILE * file,
						const RsslMsgKey * key, RsslDecodeIterator *iter, const RsslDataDictionary * dictionary)
{
	RsslRet ret;
	RsslBuffer opaqueBufferValue;
	int xmlBufLength = 0;
	int i = 0;
	fprintf(file, "<attrib>\n");	
	
	indents++;
	  
	switch(key->attribContainerType)
	{
		case RSSL_DT_OPAQUE:
			rsslDecodeBuffer(iter, &opaqueBufferValue);
			ret = dumpOpaqueToXML(file, &opaqueBufferValue, dictionary);
			break;
		case RSSL_DT_FILTER_LIST:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeFilterListToXML(file, iter, dictionary);
			break;
		case RSSL_DT_ELEMENT_LIST:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeElementListToXML(file, iter, dictionary, 0);
			break;
		case RSSL_DT_FIELD_LIST:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeFieldListToXML(file, iter, dictionary, 0);
			break;
		case RSSL_DT_SERIES:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeSeriesToXML(file, iter, dictionary);
			break;
		case RSSL_DT_VECTOR:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeVectorToXML(file, iter, dictionary);
			break;
		case RSSL_DT_MAP:
			ret = rsslDecodeMsgKeyAttrib(iter, key); RSSL_CHECKRET(ret);
			ret = decodeMapToXML(file, iter, dictionary);
			break;
		case RSSL_DT_JSON: /* dump as ASCII, similar to XML */
			rsslDecodeBuffer(iter, &opaqueBufferValue);
			ret = decodeJSONToXML(file, &opaqueBufferValue, dictionary);
		break;
		case RSSL_DT_XML:
			fprintf(file, "%s\n", rsslDataTypeToString(key->attribContainerType));
			xmlBufLength = key->encAttrib.length;
			for( i = 0; i < xmlBufLength; ++i)
				fprintf(file, "%c", key->encAttrib.data[i]);
			fprintf(file, "\n");
			break;
		case RSSL_DT_NO_DATA:
			return 0;
		default:
			encodeindents(file);
			fprintf(file, "Unknown data\n");
			ret = 0;
	}

	indents--;
	encodeindents(file);
	fprintf(file, "</attrib>\n");

	return RSSL_RET_SUCCESS;
}
