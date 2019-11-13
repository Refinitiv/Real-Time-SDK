/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "xmlDump.h"
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include "decodeRoutines.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/rwfNet.h"
#include "rtr/custmem.h"

#ifdef WIN32
#include "Windows.h"
#else
#include "sys/time.h"
#endif


int indents = 0;

RSSL_API void encodeindents(FILE *file)
{
	int i;
	for (i = 0; i < indents; i++)
		fprintf(file, "%4s","");
}


RSSL_API void xmlDumpStreamState(FILE *file, RsslUInt8 code)
{
	fprintf(file, "%s", rsslStreamStateToString(code));
}
	

RSSL_API void xmlDumpDataState(FILE *file, RsslUInt8 code)
{
	fprintf(file, "%s", rsslDataStateToString(code));
}

RSSL_API void xmlDumpStateCode(FILE *file, RsslUInt8 code)
{
	/* have to check if it is unknown - all state codes start with R except Unknown */
	/* if unknown, just dump the code number */
	const char* tempCode = rsslStateCodeToString(code);
	if (tempCode[0] == 'U')
		fprintf(file, "%u", code);
	else
		fprintf(file, "%s", tempCode);
}


RSSL_API void xmlDumpDataType(FILE *file, RsslDataType type)
{
	const char * str = rsslDataTypeToString(type);
	if (!str)
		fprintf(file, "%d", type);
	else
		fprintf(file, "%s", str);
}


RSSL_API void xmlDumpBuffer(FILE *file, const RsslBuffer * buffer)
{
	if (buffer->length && buffer->data)
		fprintf(file, "%.*s", buffer->length, buffer->data);
}

RSSL_API void xmlDumpHexBuffer(FILE *file, const RsslBuffer * buffer)
{
	unsigned int i;
	char * position = buffer->data;
	for (i = 0; i < buffer->length; i++, position++)
	{
		if (i % 32 == 0)
		{
			if (i != 0)
			{
				fprintf(file, "\n");
				encodeindents(file);
			}
		}
		else if ((i != 0) && (i % 2 == 0))
		{
			fprintf(file, " ");
		}
		fprintf(file, "%2.2X", (unsigned char) *position);
	}
}

RSSL_API void xmlDumpGroupId(FILE *file, const RsslBuffer * buffer)
{
	RsslUInt16 tempVal = 0;
	int index = 0;
	unsigned int i = 0;
	int printPeriod = 0;

	for (i = 0; i < buffer->length; i++)
	{
		if (printPeriod)
			fprintf(file, ".");
		index += RWF_MOVE_16(&tempVal, buffer->data + index);
		fprintf(file, "%d", tempVal);
		printPeriod = 1;
		/* need to add 2 to i */
		i++;
	}
}

RSSL_API void xmlDumpString(FILE *file, int length, const char * data)
{
	int i = 0;
	const char * c = data;
	char h;
	if ( length > 0 )  /* skip stuff below for empty string */
	{
		for (; i < length; i++)
		{
			h = *c;
			if ( h < 0x20 || h > 0x7e )
			{
				fprintf(file,"(");
				fprintf(file, "0x%02x", (unsigned char)h);
				fprintf(file,")");
			}
			else if ( h == '<' )
				fprintf( file, "&lt;" );
			else if ( h == '>' )
				fprintf (file, "&gt;");
			else if ( h == 0x26 ) /* ampersand */
				fprintf (file, "&amp;");
			else if ( h == 0x22 ) /* quotation mark */
				fprintf (file, "&quot;");
			else if ( h == 0x27 ) /* apostrophe */
				fprintf (file, "&apos;");
			else /* printable */
				fprintf(file,"%c", h);
			c++;
		}
	}
	fprintf(file, "\"");
}

RSSL_API void xmlDumpLocalElementSetDefDb(FILE *file, RsslLocalElementSetDefDb *elListSetDb)
{
	unsigned int i;
	encodeindents(file);
	fprintf(file, "<elementSetDefs>\n");

	indents++;

	for(i = 0; i <= RSSL_ELEMENT_SET_MAX_LOCAL_ID; ++i)
	{
		if (elListSetDb->definitions[i].setId != RSSL_ELEMENT_SET_BLANK_ID)
		{
			unsigned int j;
			RsslElementSetDef *pSetDef = &elListSetDb->definitions[i];
			encodeindents(file);
			fprintf(file, "<elementSetDef setId=\"%u\">\n", i);
			
			++indents;
			for(j = 0; j < pSetDef->count; ++j)
			{
				RsslElementSetDefEntry *pEntry = &pSetDef->pEntries[j];
				encodeindents(file);
				fprintf(file, "<elementSetDefEntry name=\"%.*s\" dataType=\"", pEntry->name.length, pEntry->name.data);
				xmlDumpDataType(file, pEntry->dataType);
				fprintf(file, "\" />\n");
			}
			--indents;

			encodeindents(file);
			fprintf(file, "</elementSetDef>\n");
		}
	}

	indents--;
	encodeindents(file);
	fprintf(file, "</elementSetDefs>\n");
}

RSSL_API void xmlDumpLocalFieldSetDefDb(FILE *file, RsslLocalFieldSetDefDb *flListSetDb)
{
	unsigned int i;
	encodeindents(file);
	fprintf(file, "<fieldSetDefs>\n");

	indents++;

	for(i = 0; i <= RSSL_FIELD_SET_MAX_LOCAL_ID; ++i)
	{
		if (flListSetDb->definitions[i].setId != RSSL_FIELD_SET_BLANK_ID)
		{
			unsigned int j;
			RsslFieldSetDef *pSetDef = &flListSetDb->definitions[i];
			encodeindents(file);
			fprintf(file, "<fieldSetDef setId=\"%u\">\n", i);
			
			++indents;
			for(j = 0; j < pSetDef->count; ++j)
			{
				RsslFieldSetDefEntry *pEntry = &pSetDef->pEntries[j];
				encodeindents(file);
				fprintf(file, "<fieldSetDefEntry fieldId=\"%d\" dataType=\"", pEntry->fieldId);
				xmlDumpDataType(file, pEntry->dataType);
				fprintf(file, "\" />\n");
			}
			--indents;

			encodeindents(file);
			fprintf(file, "</fieldSetDef>\n");
		}
	}

	indents--;
	encodeindents(file);
	fprintf(file, "</fieldSetDefs>\n");
}

RSSL_API void xmlDumpUInt(FILE *file, RsslUInt64 value)
{
	RsslBuffer tempOutput;
	char tempBuf[21];

	tempOutput.data = tempBuf;
	tempOutput.length = 21;

	if (rsslPrimitiveToString(&value, RSSL_DT_UINT, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpInt(FILE *file, RsslInt64 value)
{
	RsslBuffer tempOutput;
	char tempBuf[21];

	tempOutput.data = tempBuf;
	tempOutput.length = 21;

	if (rsslPrimitiveToString(&value, RSSL_DT_INT, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpDouble(FILE *file, RsslDouble value)
{
	RsslBuffer tempOutput;
	char tempBuf[30];

	tempOutput.data = tempBuf;
	tempOutput.length = 30;

	if (rsslPrimitiveToString(&value, RSSL_DT_DOUBLE, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpDate(FILE *file, RsslDate * value)
{
	RsslBuffer tempOutput;
	char tempBuf[21];

	tempOutput.data = tempBuf;
	tempOutput.length = 21;

	if (rsslPrimitiveToString(value, RSSL_DT_DATE, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpTime(FILE *file, RsslTime * value)
{
	RsslBuffer tempOutput;
	char tempBuf[35];

	tempOutput.data = tempBuf;
	tempOutput.length = 35;

	if (rsslPrimitiveToString(value, RSSL_DT_TIME, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpDateTime(FILE *file, RsslDateTime * value)
{
	RsslBuffer tempOutput;
	char tempBuf[66];

	tempOutput.data = tempBuf;
	tempOutput.length = 66;

	if (rsslPrimitiveToString(value, RSSL_DT_DATETIME, &tempOutput) == RSSL_RET_SUCCESS)
		fprintf(file, "%s", tempOutput.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpQos(FILE * file, const RsslQos * qos)
{
	fprintf(file, " qosDynamic=\"%d\" qosRate=\"%d\" qosTimeliness=\"%d\"", qos->dynamic, qos->rate, qos->timeliness);
	
	if (qos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
		fprintf(file, " qosTimeInfo=\"%d\"", qos->timeInfo);
	if (qos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
		fprintf(file, " qosRateInfo=\"%d\"", qos->rateInfo);
}


RSSL_API void xmlDumpWorstQos(FILE * file, const RsslQos * qos)
{
	fprintf(file, " worstQosDynamic=\"%d\" worstQosRate=\"%d\" worstQosTimeliness=\"%d\"", qos->dynamic, qos->rate, qos->timeliness);
	
	if (qos->timeliness > RSSL_QOS_TIME_DELAYED_UNKNOWN)
		fprintf(file, " worstQosTimeInfo=\"%d\"", qos->timeInfo);
	if (qos->rate > RSSL_QOS_RATE_JIT_CONFLATED)
		fprintf(file, " worstQosRateInfo=\"%d\"", qos->rateInfo);
}

RSSL_API void xmlDumpReal(FILE *file, RsslReal * value)
{
	RsslBuffer buf;
	buf.data = alloca(35);
	buf.length = 35;

	rsslRealToString(&buf, value);
	
	fprintf(file, "%s", buf.data);
	fprintf(file, "\"");
}

RSSL_API void xmlDumpElementListBegin(FILE *file, RsslElementList *eList)
{
	RsslBool firstFlag = RSSL_TRUE;

	encodeindents(file);
	indents++;
	fprintf(file, "<elementList flags=\"0x%X", eList->flags);

	if (eList->flags != 0)
		fputs(" (", file);
	if (eList->flags & RSSL_ELF_HAS_ELEMENT_LIST_INFO)
	{
		fprintf(file, "RSSL_ELF_HAS_ELEMENT_LIST_INFO");
		firstFlag = RSSL_FALSE;
	}

	if (eList->flags & RSSL_ELF_HAS_SET_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_ELF_HAS_SET_DATA");
	}

	if (eList->flags & RSSL_ELF_HAS_SET_ID)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_ELF_HAS_SET_ID");
	}

	if (eList->flags & RSSL_ELF_HAS_STANDARD_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_ELF_HAS_STANDARD_DATA");
	}
	if (eList->flags != 0)
		fputc(')', file);
	fputc('"', file);

	if (rsslElementListHasInfo(eList))
	{
		fprintf(file, " elementListNum=\"%d\"", eList->elementListNum);
	}
	if (rsslElementListCheckHasSetData(eList))
	{
		if (rsslElementListCheckHasSetId(eList))
			fprintf(file, " setId=\"%d\"", eList->setId);
	}
	fprintf(file, ">\n");
}

RSSL_API void xmlDumpElementBegin(FILE *file, RsslElementEntry *element)
{
	encodeindents(file);
	fprintf(file, "<elementEntry name=\"");
	xmlDumpBuffer(file, &element->name);		
	fprintf(file, "\" dataType=\"");
	xmlDumpDataType(file, element->dataType);
	fprintf(file, "\"");

	indents++;
}

RSSL_API void xmlDumpElementEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</elementEntry>\n");
}

RSSL_API void xmlDumpElementListEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</elementList>\n");
}

RSSL_API void xmlDumpFieldListBegin(FILE *file, RsslFieldList *fList)
{
	RsslBool firstFlag = RSSL_TRUE;

	encodeindents(file);
	indents++;
	fprintf(file, "<fieldList flags=\"0x%X", fList->flags);
	
	if (fList->flags != 0)
		fputs(" (", file);
	if (fList->flags & RSSL_FLF_HAS_FIELD_LIST_INFO)
	{
		fprintf(file, "RSSL_FLF_HAS_FIELD_LIST_INFO");
		firstFlag = RSSL_FALSE;
	}

	if (fList->flags & RSSL_FLF_HAS_SET_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_FLF_HAS_SET_DATA");
	}

	if (fList->flags & RSSL_FLF_HAS_SET_ID)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_FLF_HAS_SET_ID");
	}

	if (fList->flags & RSSL_FLF_HAS_STANDARD_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_FLF_HAS_STANDARD_DATA");
	}
	if (fList->flags != 0)
		fputc(')', file);
	fputc('"', file);

	if (rsslFieldListCheckHasInfo(fList))
	{
		fprintf(file, " fieldListNum=\"%d\" dictionaryId=\"%d\"", fList->fieldListNum, fList->dictionaryId);
	}
	if (rsslFieldListCheckHasSetData(fList))
	{
		if (rsslFieldListCheckHasSetId(fList))
			fprintf(file, " setId=\"%d\"", fList->setId);
	}
	fprintf(file, ">\n");
}

RSSL_API void xmlDumpFieldBegin(FILE *file, RsslFieldEntry *field, RsslDataType dataType)
{
	encodeindents(file);
	fprintf(file, "<fieldEntry fieldId=\"%d", field->fieldId);
	if (dataType != RSSL_DT_UNKNOWN)
	{
		fprintf(file, "\" dataType=\"");
		xmlDumpDataType(file, dataType);
	}
	fprintf(file, "\"");
	indents++;
}

RSSL_API void xmlDumpFieldEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</fieldEntry>\n");
}


RSSL_API void xmlDumpFieldListEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</fieldList>\n");
}


RSSL_API void xmlDumpSummaryDataBegin(FILE *file)
{
	encodeindents(file);
	indents++;
	fprintf(file, "<summaryData>\n");

}


RSSL_API void xmlDumpSummaryDataEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</summaryData>\n");
}


RSSL_API void xmlDumpVectorBegin(FILE *file, RsslVector *vec)
{
	RsslBool firstFlag = RSSL_TRUE;
	encodeindents(file);
	indents++;
	fprintf(file, "<vector flags=\"0x%X", vec->flags);

	if (vec->flags != 0)
		fputs(" (", file);
	if (vec->flags & RSSL_VTF_HAS_SET_DEFS)
	{
		fprintf(file, "RSSL_VTF_HAS_SET_DEFS");
		firstFlag = RSSL_FALSE;
	}
	if (vec->flags & RSSL_VTF_HAS_SUMMARY_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_VTF_HAS_SUMMARY_DATA");
	}
	if (vec->flags & RSSL_VTF_HAS_PER_ENTRY_PERM_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_VTF_HAS_PER_ENTRY_PERM_DATA");
	}
	if (vec->flags & RSSL_VTF_HAS_TOTAL_COUNT_HINT)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_VTF_HAS_TOTAL_COUNT_HINT");
	}
	if (vec->flags & RSSL_VTF_SUPPORTS_SORTING)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_VTF_SUPPORTS_SORTING");
	}
	if (vec->flags != 0)
		fputc(')', file);
	fputc('"', file);

	fprintf(file, " countHint=\"%u\" containerType=\"", vec->totalCountHint);
	xmlDumpDataType(file, vec->containerType);
	fprintf(file, "\">\n");
}


RSSL_API void xmlDumpVectorEntryBegin(FILE *file, RsslVectorEntry *vEntry)
{
	const char *actionString; 
	encodeindents(file);
	indents++;
	switch(vEntry->action)
	{
		case RSSL_VTEA_UPDATE_ENTRY:
			actionString = "RSSL_VTEA_UPDATE_ENTRY"; 
			break;
		case RSSL_VTEA_SET_ENTRY:
			actionString = "RSSL_VTEA_SET_ENTRY";
			break;
		case RSSL_VTEA_INSERT_ENTRY:
			actionString = "RSSL_VTEA_INSERT_ENTRY";
			break;
		case RSSL_VTEA_DELETE_ENTRY:
			actionString = "RSSL_VTEA_DELETE_ENTRY";
			break;
		case RSSL_VTEA_CLEAR_ENTRY:
			actionString = "RSSL_VTEA_CLEAR_ENTRY";
			break;
		default:
			actionString = "Unknown";
	}
	/* Don't print the data element for deleted rows,  there should not be any. */
	fprintf(file, "<vectorEntry index=\"%u\" action=\"%s\" flags=\"0x%X", vEntry->index, actionString, vEntry->flags);
	if (vEntry->flags & RSSL_VTEF_HAS_PERM_DATA)
	{
		fprintf(file, " (RSSL_VTEF_HAS_PERM_DATA)\"");
		fprintf(file, "permData=\"");
		xmlDumpHexBuffer(file, &vEntry->permData);
		fprintf(file, "\">\n");
	}
	else
		fprintf(file, "\">\n");
}


RSSL_API void xmlDumpVectorEntryEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</vectorEntry>\n");
}


RSSL_API void xmlDumpVectorEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</vector>\n");
}


RSSL_API void xmlDumpMapBegin(FILE *file, RsslMap *rsslMap)
{
	RsslBool firstFlag = RSSL_TRUE;
	encodeindents(file);
	indents++;
	fprintf(file, "<map flags=\"0x%X", rsslMap->flags);

	if (rsslMap->flags != 0)
		fputs(" (", file);
	if (rsslMap->flags & RSSL_MPF_HAS_SET_DEFS)
	{
		fprintf(file, "RSSL_MPF_HAS_SET_DEFS");
		firstFlag = RSSL_FALSE;
	}
	if (rsslMap->flags & RSSL_MPF_HAS_SUMMARY_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MPF_HAS_SUMMARY_DATA");
	}
	if (rsslMap->flags & RSSL_MPF_HAS_PER_ENTRY_PERM_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MPF_HAS_PER_ENTRY_PERM_DATA");
	}
	if (rsslMap->flags & RSSL_MPF_HAS_TOTAL_COUNT_HINT)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MPF_HAS_TOTAL_COUNT_HINT");
	}
	if (rsslMap->flags & RSSL_MPF_HAS_KEY_FIELD_ID)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_MPF_HAS_KEY_FIELD_ID");
	}
	if (rsslMap->flags != 0)
		fputc(')', file);
	fputc('"', file);

	fprintf(file, " countHint=\"%u\" keyPrimitiveType=\"", rsslMap->totalCountHint);
	xmlDumpDataType(file, rsslMap->keyPrimitiveType);
	fprintf(file, "\" containerType=\"");
	xmlDumpDataType(file, rsslMap->containerType);
	fprintf(file, "\" ");
	if (rsslMapCheckHasKeyFieldId(rsslMap))
	{
		fprintf(file, "keyFieldId=\"%d\" ", rsslMap->keyFieldId);
	}
	fprintf(file, ">\n");
}


RSSL_API void xmlDumpMapEntryBegin(FILE *file, RsslDataType keyPrimitiveType, RsslMapEntry *mEntry, void * pMapKeyData)
{
	const char *actionString; 
	RsslBuffer stringBuf = RSSL_INIT_BUFFER;
	char data[1024];

	encodeindents(file);
	indents++;
	
	stringBuf.data = data;
	stringBuf.length = 1024;
	
	switch(mEntry->action)
	{
		case RSSL_MPEA_UPDATE_ENTRY:
			actionString = "RSSL_MPEA_UPDATE_ENTRY"; 
			break;
		case RSSL_MPEA_ADD_ENTRY:
			actionString = "RSSL_MPEA_ADD_ENTRY";
			break;
		case RSSL_MPEA_DELETE_ENTRY:
			actionString = "RSSL_MPEA_DELETE_ENTRY";
			break;
		default:
			actionString = "Unknown";
	}
	/* Don't print the data element for deleted rows,  there should not be any. */
	fprintf(file, "<mapEntry flags=\"0x%X", mEntry->flags);

	if (mEntry->flags & RSSL_MPEF_HAS_PERM_DATA)
	{
		fprintf(file, " (RSSL_MPEF_HAS_PERM_DATA)");
	}

	fprintf(file, "\" action=\"%s\" key=\"", actionString);
	if (rsslPrimitiveToString(pMapKeyData, keyPrimitiveType, &stringBuf) < 0)
		snprintf(stringBuf.data, stringBuf.length, "<Unknown>");
	fprintf(file, "%s\" ", stringBuf.data); 

	if (mEntry->flags & RSSL_MPEF_HAS_PERM_DATA)
	{
		fprintf(file, "permData=\"");
		xmlDumpHexBuffer(file, &mEntry->permData);
		fprintf(file, "\">\n");
	}
	else
		fprintf(file, ">\n");
}


RSSL_API void xmlDumpMapEntryEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</mapEntry>\n");
}


RSSL_API void xmlDumpMapEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</map>\n");
}


RSSL_API void xmlDumpArrayBegin(FILE *file, RsslArray *rsslArray)
{
	encodeindents(file);
	indents++;
	fprintf(file, "<array itemLength=\"%d\" primitiveType=\"", rsslArray->itemLength);
	xmlDumpDataType(file, rsslArray->primitiveType);
	fprintf(file, "\">\n");
}

RSSL_API void xmlDumpArrayItemBegin(FILE *file)
{
	encodeindents(file);
	fprintf(file, "<arrayEntry");
}

RSSL_API void xmlDumpArrayItemEnd(FILE *file)
{
	fprintf(file, "/>\n");
}

RSSL_API void xmlDumpArrayEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</array>\n");
}

RSSL_API void xmlDumpState(FILE * file, const RsslState * state)
{
	fprintf(file, " dataState=\"");
	xmlDumpDataState(file, state->dataState);
	fprintf(file, "\" streamState=\"");
	xmlDumpStreamState(file, state->streamState);
	fprintf(file, "\" code=\"");
	xmlDumpStateCode(file, state->code);
	fprintf(file, "\" text=\"");
	xmlDumpBuffer(file, &state->text);
	fprintf(file, "\" ");
}

RSSL_API void xmlDumpFilterListBegin(FILE *file, RsslFilterList *fList)
{
	RsslBool firstFlag = RSSL_TRUE;
	encodeindents(file);
	indents++;	
	fprintf(file, "<filterList containerType=\"");
	xmlDumpDataType(file, fList->containerType);
	fprintf(file, "\" countHint=\"%d\" flags=\"0x%X", fList->totalCountHint, fList->flags);

	if (fList->flags != 0)
		fputs(" (", file);
	if (fList->flags & RSSL_FTF_HAS_PER_ENTRY_PERM_DATA)
	{
		fprintf(file, "RSSL_FTF_HAS_PER_ENTRY_PERM_DATA");
		firstFlag = RSSL_FALSE;
	}
	if (fList->flags & RSSL_FTF_HAS_TOTAL_COUNT_HINT)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_FTF_HAS_TOTAL_COUNT_HINT");
	}
	if (fList->flags != 0)
		fputc(')', file);
	fprintf(file, "\">\n");
}

RSSL_API void xmlDumpFilterItemBegin(FILE *file, RsslFilterEntry *fItem)
{
	RsslBool firstFlag = RSSL_TRUE;
	const char *actionString; 
	encodeindents(file);
	indents++;
	switch(fItem->action)
	{
		case RSSL_FTEA_UPDATE_ENTRY:
			actionString = "RSSL_FTEA_UPDATE_ENTRY"; 
			break;
		case RSSL_FTEA_SET_ENTRY:
			actionString = "RSSL_FTEA_SET_ENTRY";
			break;
		case RSSL_FTEA_CLEAR_ENTRY:
			actionString = "RSSL_FTEA_CLEAR_ENTRY";
			break;
		default:
			actionString = "Unknown";
	}
	/* Don't print the data element for deleted rows,  there should not be any. */
	fprintf(file, "<filterEntry id=\"%d\" action=\"%s\" flags=\"0x%X", fItem->id, actionString, fItem->flags);	

	if (fItem->flags != 0)
		fputs(" (", file);
	if (fItem->flags & RSSL_FTEF_HAS_PERM_DATA)
	{
		fprintf(file, "RSSL_FTEF_HAS_PERM_DATA");
		firstFlag = RSSL_FALSE;
	}
	if (fItem->flags & RSSL_FTEF_HAS_CONTAINER_TYPE)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_FTEF_HAS_CONTAINER_TYPE");
	}
	if (fItem->flags != 0)
		fputc(')', file);

	fprintf(file, "\" containerType=\"");	
	xmlDumpDataType(file, fItem->containerType);
	if (fItem->flags & RSSL_FTEF_HAS_PERM_DATA)
	{
		fprintf(file, "\" permData=\"");
		xmlDumpHexBuffer(file, &fItem->permData);
		fprintf(file, "\">\n");
	}
	else
		fprintf(file, "\">\n");
}

RSSL_API void xmlDumpFilterItemEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</filterEntry>\n");
}

RSSL_API void xmlDumpFilterListEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</filterList>\n");
}

RSSL_API void xmlDumpSeriesBegin(FILE *file, RsslSeries *ser)
{
	RsslBool firstFlag = RSSL_TRUE;

	encodeindents(file);
	indents++;

	fprintf(file, "<series  flags=\"0x%X", ser->flags);
	if (ser->flags != 0)
		fputs(" (", file);
	if (ser->flags & RSSL_SRF_HAS_SET_DEFS)
	{
		fprintf(file, "RSSL_SRF_HAS_SET_DEFS");
		firstFlag = RSSL_FALSE;
	}
	if (ser->flags & RSSL_SRF_HAS_SUMMARY_DATA)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_SRF_HAS_SUMMARY_DATA");
	}
	if (ser->flags & RSSL_SRF_HAS_TOTAL_COUNT_HINT)
	{
		if (!firstFlag)
			fputc('|', file);
		else
			firstFlag = RSSL_FALSE;
		fprintf(file, "RSSL_SRF_HAS_TOTAL_COUNT_HINT");
	}
	if (ser->flags != 0)
		fputc(')', file);

	fprintf(file, "\" countHint=\"%u\" containerType=\"", ser->totalCountHint);
	xmlDumpDataType(file, ser->containerType); 
	fprintf(file, "\">\n");
}

RSSL_API void xmlDumpSeriesRowBegin(FILE *file, RsslSeriesEntry *sRow)
{
	encodeindents(file);
	indents++;
	fprintf(file, "<seriesEntry>\n");
}

RSSL_API void xmlDumpSeriesRowEnd(FILE *file)
{
	indents--;
	encodeindents(file);	
	fprintf(file, "</seriesEntry>\n");
}

RSSL_API void xmlDumpSeriesEnd(FILE *file)
{
	indents--;
	encodeindents(file);
	fprintf(file, "</series>\n");
}

RSSL_API void xmlDumpEndNoTag(FILE *file)
{
	indents--;

	fprintf(file, "/>\n");
}

RSSL_API void xmlDumpComment(FILE *file, const char* comment)
{
	encodeindents(file);
	fprintf(file, "<!-- %s -->\n", comment);
}

RSSL_API void xmlDumpTimestamp(FILE *file)
{
	long hour = 0, 
		min = 0, 
		sec = 0, 
		msec = 0;

#if defined(WIN32)
	struct _timeb	_time;
	_ftime(&_time);
	sec = (long)(_time.time - 60 * (_time.timezone - _time.dstflag * 60));
	min = sec / 60 % 60;
	hour = sec / 3600 % 24;
	sec = sec % 60;
	msec = _time.millitm;
#elif defined(LINUX)
	/* localtime must be used to get the correct system time. */
	struct tm stamptime;
	time_t currTime;
	currTime = time(NULL);
	stamptime = *localtime_r(&currTime, &stamptime);
	sec = stamptime.tm_sec;
	min = stamptime.tm_min;
	hour = stamptime.tm_hour;

	/* localtime, however, does not give us msec. */
	struct timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
#endif

	fprintf(file, "<!-- Time: %ld:%02ld:%02ld:%03ld -->\n",
		hour,
		min,
		sec,
		msec);
}

RSSL_API void xmlGetTimeFromEpoch(unsigned long long *hour, unsigned long long *min, unsigned long long *sec, unsigned long long *msec)
{
/* this function gives the time elapsed since January 1st, 1970 */
#if defined(WIN32)
	FILETIME timeVal;
	ULARGE_INTEGER tempTime;

	GetSystemTimeAsFileTime(&timeVal);

	tempTime.LowPart = timeVal.dwLowDateTime;
	tempTime.HighPart = timeVal.dwHighDateTime;

	tempTime.QuadPart = (tempTime.QuadPart/10000000) - 11644473600LL;
	
	*sec = tempTime.QuadPart;
	*min = *sec / 60;
	*hour = *sec / 3600;

	tempTime.LowPart = timeVal.dwLowDateTime;
	tempTime.HighPart = timeVal.dwHighDateTime;

	*msec = (tempTime.QuadPart/10000) - 11644473600LL;

#elif defined(LINUX)
	struct timeval tv;
	gettimeofday(&tv, NULL);

	*sec = tv.tv_sec;
	*min = *sec/60;
	*hour = *sec/3600;
	*msec = ((tv.tv_sec)*1000 )+(tv.tv_usec / 1000);
#endif
}
