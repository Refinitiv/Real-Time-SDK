/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stddef.h>

#include "rtr/rsslDataDictionary.h"
#include "rtr/rsslDataUtils.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslSeries.h"
#include "rtr/rsslPrimitiveEncoders.h"
#include "rtr/rsslPrimitiveDecoders.h"
#include "rtr/encoderTools.h"
#include "rtr/textFileReader.h"
#include "rtr/rsslHashTable.h"

#define DICTIONARY_MAX_ENTRIES 65535

typedef struct {
	RsslHashLink	nameTableLink;		/* Link for fields-by-name table. */
	RsslDictionaryEntry *pDictionaryEntry;	/* Entry for this link*/
} FieldsByNameLink;

typedef struct {
	RsslHashTable	fieldsByName;	/* Table of fields by acronym. */

	/* Indicate whether the entries in this dictionary are linked by another one, so we don't delete them on cleanup. */
	RsslBool isLinked;

	/* Hash table links to use in fieldsByName table. 
	 * The links are stored on the dictionary itself, not the entry objects. This allows rsslLinkDataDictionary to add fields to the new
	 * dictionary without editing the links in the old dictionary (so that the table in the old dictionary can still be safely used 
	 * without locking it). */
	FieldsByNameLink fieldsByNameLinks[DICTIONARY_MAX_ENTRIES];
} RsslDictionaryInternal;

typedef struct {
	RsslEnumType	base;	/* Base enum object. */
	RsslUInt16		flags;	/* Flags for this enum. See RsslEnumTypeFlags. */
} RsslEnumTypeImpl;

typedef enum {
	RSSL_ENUMTYPE_FL_DUPLICATE_DISPLAY	= 0x1 /* This value is one of multiple values that correspond to the same display string. */
} RsslEnumTypeFlags;



/* used for decoding local element list set definitions if user does not pass in memory */
/* 15 defs * 255 entries per def = 3825 -- * 13 bytes per element list def (4 byte length, 8 byte pointer, 1 byte type) = ~50000 */
#define LOCAL_EL_DEFS_TLS_SIZE 3825

static const RsslUInt32 ENUM_TABLE_MAX_COUNT = ((RSSL_MAX_FID)-(RSSL_MIN_FID)+1);

/*** Temporary storage objects for use when loading/decoding the enum type dictionary ***/

/* Temporarily stores field references while building an enum type table */
typedef struct _RsslReferenceFidStore {
	RsslInt16		fid;
	RsslBuffer		acronym;
	struct _RsslReferenceFidStore	*next;
} RsslReferenceFidStore;

/* Temporarily stores enumerated types while building an enum type table */
typedef struct _RsslEnumTypeStore {
	RsslEnumType	enumType;
	struct _RsslEnumTypeStore *next;
} RsslEnumTypeStore;


// Dictionary - Element names that should be hidden
static const RsslBuffer RSSL_ENAME_ENUM_FID = {3, (char*)"FID"};
static const RsslBuffer RSSL_ENAME_VALUES = {6, (char*)"VALUES"};
static const RsslBuffer RSSL_ENAME_DISPLAYS = {8, (char*)"DISPLAYS"};


static const char 		*c_defaultVersion = "";

static const int		ONE_BYTE_START = 0;
static const int		ONE_BYTE_START_MASK = 128;

static const int		TWO_BYTE_FIRST_START = 192;
static const int		TWO_BYTE_FIRST_MASK = 224;

static const int		THREE_BYTE_FIRST_START = 224;
static const int		THREE_BYTE_FIRST_MASK = 240;

static const int		FOUR_BYTE_FIRST_START = 240;
static const int		FOUR_BYTE_FIRST_MASK = 248;

static const int		MULTIBYTE_NEXT_START = 128;
static const int		MULTIBYTE_NEXT_MASK = 192;

/* Version should only have numbers and dots. Remove all else (will strip whitespace too). */
static void trimInfoVersion(RsslBuffer *infoVersion)
{
	char *pRead, *pWrite;

	pRead = pWrite = infoVersion->data;
	while ((RsslUInt32)(pRead - infoVersion->data) < infoVersion->length)
	{
		char read = *pRead++;
		/* Shift up all valid characters. */
		if (isdigit(read) || read == '.' || read == '\0')
		{
			*pWrite = read;
			if (read == '\0') break; /* If we just wrote a null-terminator we're done */
			++pWrite;
		}
	}

	if (pRead > pWrite) /* If we had to trim some characters, null-terminate the result */
		*pWrite = '\0'; 

	infoVersion->length = (rtrUInt32)(pWrite - infoVersion->data);
}

/* Creates a copy of the string, adding a null terminator just in case */
RTR_C_INLINE RsslRet _rsslCreateStringCopy(RsslBuffer *targetBuf, RsslBuffer *sourceBuf)
{
	RSSL_ASSERT(targetBuf && !targetBuf->data, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(sourceBuf, Invalid parameters or parameters passed in as NULL);

	if (sourceBuf && sourceBuf->data)
	{
		targetBuf->length = sourceBuf->length;
		targetBuf->data = (char*)malloc(targetBuf->length * sizeof(char) + 1);
		if (!targetBuf->data)
			return RSSL_RET_FAILURE;

		strncpy(targetBuf->data, sourceBuf->data, targetBuf->length);
		targetBuf->data[targetBuf->length] = '\0';
	}
	else
	{
		/* Create an empty string if the buffer was empty. */
		targetBuf->data = (char*)malloc(1);
		if (!targetBuf->data)
			return RSSL_RET_FAILURE;
		targetBuf->data[0] = '\0';
		targetBuf->length = 0;
	}
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE RsslRet _rsslCreateStringCopyFromChar(RsslBuffer *targetBuf, char *sourceStr)
{
	RSSL_ASSERT(targetBuf && !targetBuf->data, Invalid parameters or parameters passed in as NULL);
	RSSL_ASSERT(sourceStr, Invalid parameters or parameters passed in as NULL);

	targetBuf->length = (rtrUInt32)strlen(sourceStr);
	targetBuf->data = (char*)malloc(targetBuf->length * sizeof(char) + 1);
	if (!targetBuf->data)
		return RSSL_RET_FAILURE;
	strcpy(targetBuf->data, sourceStr);
	targetBuf->data[targetBuf->length] = '\0';
	return RSSL_RET_SUCCESS;
}

void _setError(RsslBuffer *errorText, char *format, ...)
{
	if (errorText && errorText->data)
	{
    	va_list fmtArgs; 
    	va_start(fmtArgs, format);
		vsnprintf(errorText->data, errorText->length, format, fmtArgs);
		va_end(fmtArgs);
	}
}

void _freeLists( RsslReferenceFidStore *pFids, RsslEnumTypeStore *pEnumTypes, RsslBool success)
{
	if (pFids)
	{
		RsslReferenceFidStore *pTmpFid;
		while(pFids)
		{
			pTmpFid = pFids;
			pFids = pFids->next;
			if (pTmpFid->acronym.data) free(pTmpFid->acronym.data);
			free(pTmpFid);
		}
	}

	if (pEnumTypes)
	{
		RsslEnumTypeStore *pTmpEnumType;
		while(pEnumTypes)
		{
			pTmpEnumType = pEnumTypes;
			pEnumTypes = pEnumTypes->next;

			if (!success)
			{
				/* If we failed, the enum table didn't make it into the dictionary. So free these. */
				if (pTmpEnumType->enumType.display.data) free(pTmpEnumType->enumType.display.data);
				if (pTmpEnumType->enumType.meaning.data) free(pTmpEnumType->enumType.meaning.data);
			}
			free(pTmpEnumType);
		}
	}
}

/* atoi() with error checking */
RsslInt64 _rsslAtoi64( const char * keyword, RsslBool* success, RsslUInt32 lineNum, RsslBuffer *errorText)
{
    RsslInt64 result;
    int scanRet, scanCount;

	/* sscanf function converts 64b values.
	* The "%n" character returns the number of bytes written, allowing us to validate input.
	* It's only documented in MSDN, but seems to work on all platforms! */
	scanCount = 0;
	scanRet = sscanf(keyword, RTR_LLD "%n", &result, &scanCount);
	if (scanRet != 1 || (size_t)scanCount != strlen(keyword))
	{    
		if (errorText)
    		snprintf(errorText->data, errorText->length, "Expected Integer instead of \"%s\" (Line: %u)", keyword, lineNum);
		*success = RSSL_FALSE;
		return 0;
	}    
	else 
	{    
		*success = RSSL_TRUE;
		return result;
	}    
}

static const int c_rsslMfeedError = -2;
static int getFieldType(char *strval)
{
	if (strcmp(strval,"INTEGER") == 0)
		return RSSL_MFEED_INTEGER;
	else if (strcmp(strval,"ALPHANUMERIC") == 0)
		return RSSL_MFEED_ALPHANUMERIC;
	else if (strcmp(strval,"ENUMERATED") == 0)
		return RSSL_MFEED_ENUMERATED;
	else if (strcmp(strval,"NUMERIC") == 0)
		return RSSL_MFEED_NUMERIC;
	else if (strcmp(strval,"TIME") == 0)
		return RSSL_MFEED_TIME;
	else if (strcmp(strval,"PRICE") == 0)
		return RSSL_MFEED_PRICE;
	else if (strcmp(strval,"DATE") == 0)
		return RSSL_MFEED_DATE;
	else if (strcmp(strval,"BINARY") == 0)
		return RSSL_MFEED_BINARY;
	else if (strcmp(strval,"TIME_SECONDS") == 0)
		return RSSL_MFEED_TIME_SECONDS;
	else if (strcmp(strval,"NONE") == 0)
		return RSSL_MFEED_NONE;
	else if (strcmp(strval,"LONG_ALPHANUMERIC") == 0)
		return RSSL_MFEED_LONG_ALPHANUMERIC;
	else if (strcmp(strval,"ALPHANUM_XTND") == 0)
		return RSSL_MFEED_LONG_ALPHANUMERIC;
	else if (strcmp(strval,"OPAQUE") == 0)
		return RSSL_MFEED_OPAQUE;
	return c_rsslMfeedError;
}

static int getRwfFieldType(char *strval)
{
	/* INT32/64/REAL32/64/UINT32/64 will eventually go away in lieu of INT/REAL/UINT */
	if (strcmp(strval,"INT32") == 0)
		return RSSL_DT_INT;
	else if (strcmp(strval,"UINT32") == 0)
		return RSSL_DT_UINT;
	else if (strcmp(strval,"INT64") == 0)
		return RSSL_DT_INT;
	else if (strcmp(strval,"UINT64") == 0)
		return RSSL_DT_UINT;
    else if (strcmp(strval,"UINT") == 0)
        return RSSL_DT_UINT;
    else if (strcmp(strval,"INT") == 0)
        return RSSL_DT_INT;
    else if (strcmp(strval,"REAL") == 0)
        return RSSL_DT_REAL;
	else if (strcmp(strval,"FLOAT") == 0)
		return RSSL_DT_FLOAT;
	else if (strcmp(strval,"DOUBLE") == 0)
		return RSSL_DT_DOUBLE;
	else if (strcmp(strval,"REAL32") == 0)
		return RSSL_DT_REAL;
	else if (strcmp(strval,"REAL64") == 0)
		return RSSL_DT_REAL;
	else if (strcmp(strval,"DATE") == 0)
		return RSSL_DT_DATE;
	else if (strcmp(strval,"TIME") == 0)
		return RSSL_DT_TIME;
	else if (strcmp(strval,"DATETIME") == 0)
		return RSSL_DT_DATETIME;
	else if (strcmp(strval,"DATE_TIME") == 0)
		return RSSL_DT_DATETIME;
	else if (strcmp(strval,"QOS") == 0)
		return RSSL_DT_QOS;
	else if (strcmp(strval,"STATE") == 0)
		return RSSL_DT_STATE;
	else if (strcmp(strval,"STATUS") == 0)
		return RSSL_DT_STATE;
	else if (strcmp(strval,"ENUM") == 0)
		return RSSL_DT_ENUM;
	else if (strcmp(strval,"ARRAY") == 0)
		return RSSL_DT_ARRAY;
	else if (strcmp(strval,"BUFFER") == 0)
		return RSSL_DT_BUFFER;
	else if (strcmp(strval,"ASCII_STRING") == 0)
		return RSSL_DT_ASCII_STRING;
	else if (strcmp(strval,"UTF8_STRING") == 0)
		return RSSL_DT_UTF8_STRING;
	else if (strcmp(strval,"RMTES_STRING") == 0)
		return RSSL_DT_RMTES_STRING;
	else if (strcmp(strval,"VECTOR") == 0)
		return RSSL_DT_VECTOR;
	else if (strcmp(strval,"MAP") == 0)
		return RSSL_DT_MAP;
	else if (strcmp(strval,"SERIES") == 0)
		return RSSL_DT_SERIES;
	else if (strcmp(strval,"FIELD_LIST") == 0)
		return RSSL_DT_FIELD_LIST;
	else if (strcmp(strval,"FILTER_LIST") == 0)
		return RSSL_DT_FILTER_LIST;
	else if (strcmp(strval,"ELEMENT_LIST") == 0)
		return RSSL_DT_ELEMENT_LIST;
	else if (strcmp(strval,"ELEM_LIST") == 0)
		return RSSL_DT_ELEMENT_LIST;
	else if (strcmp(strval,"XML") == 0)
		return RSSL_DT_XML;
	else if (strcmp(strval,"ANSI_PAGE") == 0)
		return RSSL_DT_ANSI_PAGE;
	else if (strcmp(strval,"OPAQUE") == 0)
		return RSSL_DT_OPAQUE;
	else if (strcmp(strval,"MSG") == 0)
		return RSSL_DT_MSG;
	else if (strcmp(strval,"JSON") == 0)
		return RSSL_DT_JSON;

	return -1;
}

/* Returns whether the character is whitespace. Works around an assertion in Windows MDd
 * thrown when a character is negative (which performs the same range check as below). */
static int isWhitespace(int myChar)
{
	return (myChar >= -1 && myChar <= 255 && isspace(myChar));
}

static int getCopyUntilSpace( char * line, int curpos, char *copyToStr )
{
	int copyIdx=0;

	/* Remove whitespace */
	while ((line[curpos] != '\0') && isWhitespace(line[curpos]))
		curpos++;

	/* Copy information until whitespace is found */
	while ((line[curpos] != '\0') && (line[curpos] != '\"') && !isWhitespace(line[curpos]))
		copyToStr[copyIdx++] = line[curpos++];

	/* Null Terminate */
	/* Return -2 if we encountered a quotation mark, so we know we actually hit
	 * something that wasn't the end of the line.  Could make enums if this gets more
	 * complicated. */
	copyToStr[copyIdx] = '\0';
	return (line[curpos] == '\"')  ? -2 : ((copyIdx == 0) ? -1 : curpos);
}

static int getCopyQuotedStr( char * line, int curpos, char *copyToStr )
{
	int copyIdx=0;

	/* Remove whitespace */
	while ((line[curpos] != '\0') && isWhitespace(line[curpos]))
		curpos++;

	/* Check for quote */
	if (line[curpos++] != '"')
		return -1;

	/* Copy information until another quote is found */
	while ((line[curpos] != '\0') && (line[curpos] != '"'))
		copyToStr[copyIdx++] = line[curpos++];

	/* Null Terminate */
	copyToStr[copyIdx] = '\0';

	return( line[curpos] != '"' ? -1 : curpos + 1 );
}

static int getCopyQuotedOrSharpedStr( char * line, int curpos, char *copyToStr, RsslBool *isHex )
{
	int copyIdx=0;
	char delim;

	/* Remove whitespace */
	while ((line[curpos] != '\0') && isWhitespace(line[curpos]))
		curpos++;

	/* Check for quote */
	if (line[curpos] != '"' && line[curpos] != '#')
		return -1;

	delim = line[curpos++];

	*isHex = (delim == '#');

	/* Copy information until another quote is found */
	while ((line[curpos] != '\0') && (line[curpos] != delim))
		copyToStr[copyIdx++] = line[curpos++];

	/* Null Terminate */
	copyToStr[copyIdx] = '\0';

	return( line[curpos] != delim ? -1 : curpos + 1 );
}

static int getRestOfLine( char * line, int curpos, char *copyToStr )
{
    int copyIdx;

    RSSL_ASSERT(line && copyToStr, Invalid parameters or parameters passed in as NULL);

    copyIdx = 0;

    /* Remove whitespace */
    while ((line[curpos] != '\0') && isWhitespace(line[curpos]))
        curpos++;

    /* Copy information until whitespace is found */
    while (line[curpos] != '\0' && line[curpos] != '\n' && line[curpos] != '\r')
        copyToStr[copyIdx++] = line[curpos++];

    /* Null Terminate */
    copyToStr[copyIdx] = '\0';
    return( copyIdx == 0 ? -1 : curpos);
}

static RsslRet copyTagDataFromElement(RsslBuffer *targetBuf, const RsslBuffer *strBuf)
{
	/* Look for null-terminator. For the safety of people who might try to print it(without using %.*s),
	* go ahead and add one if it's not there(not adding to the advertised length, of course). */
	rtrUInt32 addLen = memchr(strBuf->data, '\0', strBuf->length ) ? 0 : 1;

	if (targetBuf->data) return RSSL_RET_SUCCESS; /* Don't overwrite */;
	targetBuf->data = (char*)malloc((strBuf->length + addLen) * sizeof(char)) ;
	if (!targetBuf->data)
		return RSSL_RET_FAILURE;
	targetBuf->length = strBuf->length;

	memcpy(targetBuf->data, strBuf->data, strBuf->length);
	if (addLen) targetBuf->data[strBuf->length] = '\0';
	return RSSL_RET_SUCCESS;
}

static RsslRet copyTagData(RsslBuffer *targetBuf, const char * strval)
{
    if (targetBuf->data)
		return RSSL_RET_SUCCESS; /* Don't overwrite */
    targetBuf->length = (rtrUInt32)strlen(strval);
    targetBuf->data = (char*)malloc((targetBuf->length) * sizeof(char) + 1);
	if (!targetBuf->data)
		return RSSL_RET_FAILURE;
    strncpy(targetBuf->data, strval, (targetBuf->length * sizeof(char) + 1));
	return RSSL_RET_SUCCESS;
}

/* Handle dictionary tags.
 * The logic is put here so the file-loading and wire-decoding versions can be kept close to each other.
 * It would be nice if they could be merged further somehow. */
RsslRet _decodeDictionaryTag(RsslDecodeIterator *dIter, RsslElementEntry *element, RDMDictionaryTypes type, RsslDataDictionary *pDictionary, RsslBuffer *errorText)
{
	RsslUInt tempUInt;
	RsslInt	tempInt;
	RsslRet ret;

	switch(type)
	{
	case RDM_DICTIONARY_FIELD_DEFINITIONS:
		if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_TYPE))
		{
			if ((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
				return (_setError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		
			if (tempUInt != RDM_DICTIONARY_FIELD_DEFINITIONS)
				return (_setError(errorText, "Type '" RTR_LLU "' indicates this is not a field definitions dictionary.", tempUInt), RSSL_RET_FAILURE);
		}
		else if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICTIONARY_ID))
		{
			/* second element is dictionary id */
			if ((ret = rsslDecodeInt(dIter, &tempInt)) < 0)
				return (_setError(errorText, "rsslDecodeInt failed - %d", ret), RSSL_RET_FAILURE);
			if (tempInt != 0 && pDictionary->info_DictionaryId != 0 && tempInt != pDictionary->info_DictionaryId)
				return (_setError(errorText, "DictionaryId mismatch('" RTR_LLD "' vs. previously found '%d').", tempInt, pDictionary->info_DictionaryId), RSSL_RET_FAILURE);
			pDictionary->info_DictionaryId = (RsslInt32)tempInt;
		}
		else if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_VERSION))
		{
			if (copyTagDataFromElement(&pDictionary->infoField_Version, &element->encData) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for version information"), RSSL_RET_FAILURE);
			trimInfoVersion(&pDictionary->infoField_Version);
		}
		break;

	case RDM_DICTIONARY_ENUM_TABLES:
		if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICT_TYPE))
		{
			if ((ret = rsslDecodeUInt(dIter, &tempUInt)) < 0)
				return (_setError(errorText, "rsslDecodeUInt failed - %d", ret), RSSL_RET_FAILURE);
		
			if (tempUInt != RDM_DICTIONARY_ENUM_TABLES)
				return (_setError(errorText, "Type '" RTR_LLU "' indicates this is not a set of enum tables .", tempUInt), RSSL_RET_FAILURE);
		}
		else if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_DICTIONARY_ID))
		{
			/* second element is dictionary id */
			if ((ret = rsslDecodeInt(dIter, &tempInt)) < 0)
				return (_setError(errorText, "rsslDecodeInt failed - %d", ret), RSSL_RET_FAILURE);
			if (tempInt != 0 && pDictionary->info_DictionaryId != 0 && tempInt != pDictionary->info_DictionaryId)
				return (_setError(errorText, "DictionaryId mismatch('" RTR_LLD "' vs. previously found '%d').", tempInt, pDictionary->info_DictionaryId), RSSL_RET_FAILURE);
			pDictionary->info_DictionaryId = (RsslInt32)tempInt;
		}
		else if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_ENUM_RT_VERSION))
		{
			if (copyTagDataFromElement(&pDictionary->infoEnum_RT_Version, &element->encData) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for RT Version information"), RSSL_RET_FAILURE);
		}
		else if (rsslBufferIsEqual(&element->name, &RSSL_ENAME_ENUM_DT_VERSION))
		{
			if (copyTagDataFromElement(&pDictionary->infoEnum_DT_Version, &element->encData) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for DT Version information"), RSSL_RET_FAILURE);
		}
		break;

	default:
		RSSL_ASSERT(0, Error); break;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet _copyDictionaryTag(char *tag, char *value, RDMDictionaryTypes type, RsslDataDictionary *pDictionary, RsslBuffer *errorText)
{
	switch(type)
	{
	case RDM_DICTIONARY_FIELD_DEFINITIONS:
		if (0 == strcmp(tag, RSSL_ENAME_DICT_TYPE.data))
		{
			/* No need to store, just make sure it's correct so we might avoid blowing up later. */
			if (atoi(value) != RDM_DICTIONARY_FIELD_DEFINITIONS)
				return (_setError(errorText, "Type '%s' indicates this is not a field definitions dictionary.", value), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, RSSL_ENAME_DICT_VERSION.data))
		{
			if (copyTagData(&pDictionary->infoField_Version, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for version information"), RSSL_RET_FAILURE);
			trimInfoVersion(&pDictionary->infoField_Version);
		}
		else if (0 == strcmp(tag, RSSL_ENAME_DICTIONARY_ID.data))
		{
			int id = atoi(value);
			if (id != 0 && pDictionary->info_DictionaryId != 0 && id != pDictionary->info_DictionaryId)
				return (_setError(errorText, "DictionaryId mismatch('%d' vs. previously found '%d').", id, pDictionary->info_DictionaryId), RSSL_RET_FAILURE);
			pDictionary->info_DictionaryId = id;
		}

		/* Other tags (not encoded or decoded by this package)*/
		else if ( 0 == strcmp(tag, "Filename"))
		{
			if (copyTagData(&pDictionary->infoField_Filename, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for filename information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "Desc"))
		{
			if (copyTagData(&pDictionary->infoField_Desc, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for description information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "Build"))
		{
			if (copyTagData(&pDictionary->infoField_Build, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for dictionary build information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "Date"))
		{
			if (copyTagData(&pDictionary->infoField_Date, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for dictionary date information"), RSSL_RET_FAILURE);
		}
		/* Ignore other tags */
    	break;

	case RDM_DICTIONARY_ENUM_TABLES:
		if (0 == strcmp(tag, RSSL_ENAME_DICT_TYPE.data))
		{
			if (atoi(value) != RDM_DICTIONARY_ENUM_TABLES)
				return (_setError(errorText, "Type '%s' indicates this is not a set of enum tables.", value), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, RSSL_ENAME_DICTIONARY_ID.data))
		{
			int id = atoi(value);
			if (id != 0 && pDictionary->info_DictionaryId != 0 && id != pDictionary->info_DictionaryId)
				return (_setError(errorText, "DictionaryId mismatch('%d' vs. previously found '%d').", id, pDictionary->info_DictionaryId), RSSL_RET_FAILURE);
		}

		/* Other tags (not encoded or decoded by this package)*/
		else if (0 == strcmp(tag, "Filename"))
		{
			if (copyTagData(&pDictionary->infoEnum_Filename, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for filename information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "Desc"))
		{
			if (copyTagData(&pDictionary->infoEnum_Desc, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for description information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "Date"))
		{
			if (copyTagData(&pDictionary->infoEnum_Date, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for dictionary date information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "RT_Version"))
		{
			if (copyTagData(&pDictionary->infoEnum_RT_Version, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for RT Version information"), RSSL_RET_FAILURE);
		}
		else if (0 == strcmp(tag, "DT_Version"))
		{
			if (copyTagData(&pDictionary->infoEnum_DT_Version, value) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for DT Version information"), RSSL_RET_FAILURE);
		}
		/* Ignore other tags */
		break;

	default:
		RSSL_ASSERT(0, Error); break;
	}
	return RSSL_RET_SUCCESS;
}


typedef struct _RippleDefinition
{
	RsslBuffer	rippleAcronym;
	short	rippleFid;
	struct	_RippleDefinition *next;
} RippleDefintion;

/* _finishFailure: Delete dictionary and cleanup unresolved ripples */
RsslRet _finishFailure(FILE *fp, RsslDataDictionary *pDict, RippleDefintion *pRipples, RsslDictionaryEntry *entry, TextFileReader *pReader)
{
	RippleDefintion *tmpRipple;

	while(pRipples)
	{
		tmpRipple = pRipples;
		pRipples = pRipples->next;
		free(tmpRipple);
	}

	if (pDict)
		rsslDeleteDataDictionary(pDict);
	if (fp)
		fclose(fp);
	if (entry) 
	{
		if (entry->acronym.data) free(entry->acronym.data);
		if (entry->ddeAcronym.data) free(entry->ddeAcronym.data);
    	free(entry);
	}

	if (pReader)
		textFileReaderCleanup(pReader);

	return RSSL_RET_FAILURE; /* Return failure code for caller to use */
}

RsslRet _finishEnumLoadFailure(FILE *fp, RsslDataDictionary *pDict, RsslReferenceFidStore *pFids, RsslEnumTypeStore *pEnumTypes, TextFileReader *pReader)
{
	_freeLists(pFids, pEnumTypes, RSSL_FALSE);

	if (pDict)
		rsslDeleteDataDictionary(pDict);
	if (fp)
		fclose(fp);

	if (pReader)
		textFileReaderCleanup(pReader);

	return RSSL_RET_FAILURE; /* Return failure code for caller to use */
}

RsslRet _initDictionary(RsslDataDictionary *dictionary, RsslBuffer *errorText)
{
	RsslDictionaryEntry ** newDict;
	RsslDictionaryInternal *pDictionaryInternal;
	RsslErrorInfo	rsslErrorInfo;
	RSSL_ASSERT(dictionary, Invalid parameters or parameters passed in as NULL); 
	RSSL_ASSERT(!dictionary->isInitialized, Dictionary already initialized);

	rsslClearBuffer(&dictionary->infoField_Filename);
	rsslClearBuffer(&dictionary->infoField_Desc);
	rsslClearBuffer(&dictionary->infoField_Version);
	rsslClearBuffer(&dictionary->infoField_Build);
	rsslClearBuffer(&dictionary->infoField_Date);

	rsslClearBuffer(&dictionary->infoEnum_Filename);
	rsslClearBuffer(&dictionary->infoEnum_Date);
	rsslClearBuffer(&dictionary->infoEnum_Desc);
	rsslClearBuffer(&dictionary->infoEnum_RT_Version);
	rsslClearBuffer(&dictionary->infoEnum_DT_Version);

	dictionary->enumTableCount = 0;
	dictionary->numberOfEntries = 0;
	dictionary->info_DictionaryId = 1;

	pDictionaryInternal = (RsslDictionaryInternal*)calloc(sizeof(RsslDictionaryInternal), 1);
	if (pDictionaryInternal == 0)
	{
		_setError(errorText, "malloc() of internal dictionary failed.");
		return RSSL_RET_FAILURE;
	}

	dictionary->_internal = pDictionaryInternal;

	if ( rsslHashTableInit(&pDictionaryInternal->fieldsByName, DICTIONARY_MAX_ENTRIES, rsslHashBufferSum, rsslHashBufferCompare,
				RSSL_TRUE, &rsslErrorInfo) != RSSL_RET_SUCCESS )
	{
		_setError(errorText, "Failed to initailize fields-by-name table.");
		return RSSL_RET_FAILURE;
	}


	newDict = (RsslDictionaryEntry**)calloc((RSSL_MAX_FID-RSSL_MIN_FID+1), sizeof(RsslDictionaryEntry*));
	if (newDict == 0)
	{
		_setError(errorText, "malloc() of entry array failed.");
		return RSSL_RET_FAILURE;
	}

	dictionary->entriesArray = &newDict[RSSL_MIN_FID < 0 ? -(RSSL_MIN_FID) : 0];
	dictionary->minFid = RSSL_MAX_FID+1;
	dictionary->maxFid = RSSL_MIN_FID-1;
	
	/* The range of fids is a practical limit for the table, since no field can use more than one table. */
	dictionary->enumTables = (RsslEnumTypeTable**)calloc(ENUM_TABLE_MAX_COUNT, sizeof(RsslEnumTypeTable*));
	if (dictionary->enumTables == 0)
	{
		_setError(errorText, "malloc() of enumTables failed.");
		return RSSL_RET_FAILURE;
	}

	dictionary->isInitialized = RSSL_TRUE;

	return RSSL_RET_SUCCESS;
}

/* Copies FieldDictionary-related information between entries.
 * Used for entries that were already initialized by the enumType dictionary. */
RsslRet _copyEntryFieldDictInfo(RsslDictionaryEntry *oEntry, RsslDictionaryEntry *iEntry, RsslBuffer *errorText)
{
	/* oEntry has the enumType info. iEntry has the field dictionary info.  */
	RSSL_ASSERT(oEntry->rwfType == RSSL_DT_UNKNOWN, Invalid type);

	/* Match the acronym if present(for enum type dictionies the files contain them, but domain-modeled messages do not). */
	if (oEntry->acronym.data)
	{
		if(!rsslBufferIsEqual(&oEntry->acronym, &iEntry->acronym))
		{
    		return (_setError(errorText, "Acronym mismatch \"%s\" and \"%s\" between Field Dictionary and Enum Type Dictionary", oEntry->acronym.data, iEntry->acronym.data), RSSL_RET_FAILURE);
		}

		if (iEntry->acronym.data)
			free(iEntry->acronym.data);
	}
	else
		oEntry->acronym = iEntry->acronym; /* Soft copy (input entry should be discarded) */

	oEntry->ddeAcronym = iEntry->ddeAcronym; /* Soft copy (input entry should be discarded) */

	oEntry->enumLength = iEntry->enumLength;
	oEntry->fid = iEntry->fid;
	oEntry->fieldType = iEntry->fieldType;
	oEntry->length = iEntry->length;
	oEntry->rippleToField = iEntry->rippleToField;
	oEntry->rwfLength = iEntry->rwfLength;
	oEntry->rwfType = iEntry->rwfType;

	return RSSL_RET_SUCCESS;
}

/* Adds field information to a dictionary entry.
 * Maintains a enumeration table reference if one is found. 
 * Callers should not use the entry pointer afterwards -- if the entry is copied rather than used the pointer will be freed. */
RsslRet _addFieldToDictionary(RsslDataDictionary *dictionary, RsslDictionaryEntry *pEntry, RsslBuffer *errorText, int lineNum)
{
	RsslDictionaryInternal *pDictionaryInternal = (RsslDictionaryInternal*)dictionary->_internal;
	FieldsByNameLink *pFieldsByNameLink;
	RsslHashLink *rsslHashLink;
	int fidNum = pEntry->fid;
	RsslUInt32 hashSum;

	/* fid 0 is reserved, & type cannot be UNKNOWN */
	if (pEntry->fid == 0)
	{
		if (lineNum > 0)
			return (_setError(errorText, "fid 0 is reserved(Line=%d).", lineNum), RSSL_RET_FAILURE);
		else
			return (_setError(errorText, "fid 0 is reserved."), RSSL_RET_FAILURE);
	}
	else if (pEntry->rwfType == RSSL_DT_UNKNOWN)
	{
		if (lineNum > 0)
			return (_setError(errorText, "Invalid rwfType for fid %d (Line=%d).", pEntry->fid, lineNum), RSSL_RET_FAILURE);
		else
			return (_setError(errorText, "Invalid rwfType for fid %d.", pEntry->fid), RSSL_RET_FAILURE);
	}

	hashSum = rsslHashBufferSum(&pEntry->acronym);

	/* Check if this acronym is already present. */
	if ((rsslHashLink = rsslHashTableFind(&pDictionaryInternal->fieldsByName, &pEntry->acronym, &hashSum)) != NULL)
	{
		pFieldsByNameLink = RSSL_HASH_LINK_TO_OBJECT(FieldsByNameLink, nameTableLink, rsslHashLink);
		RSSL_ASSERT(pFieldsByNameLink->pDictionaryEntry != NULL, Link in fieldsByName table does not have an associated entry);
		_setError(errorText, "Acronym '%.*s' already exists in dictionary with fid %d (Line=%d).",
				pEntry->acronym.length, pEntry->acronym.data, pFieldsByNameLink->pDictionaryEntry->fid, lineNum);
	}

	if (dictionary->entriesArray[fidNum] != 0)
	{
		if (dictionary->entriesArray[fidNum]->rwfType != RSSL_DT_UNKNOWN)
		{
			if (lineNum > 0)
    			_setError(errorText, "Duplicate definition for fid %d (Line=%d).",fidNum,lineNum);
			else
    			_setError(errorText, "Duplicate definition for fid %d.", fidNum);
			return RSSL_RET_FAILURE;
		}
		else
		{
			/* Entry exists because it was loaded from an enumType def. Copy the fieldDict-related info. */
			if(_copyEntryFieldDictInfo(dictionary->entriesArray[fidNum], pEntry, errorText) != RSSL_RET_SUCCESS)
				return RSSL_RET_FAILURE;

			free(pEntry);
			pEntry = dictionary->entriesArray[fidNum];
		}
	}
	else
	{
		pEntry->pEnumTypeTable = 0;
		dictionary->entriesArray[fidNum] = pEntry;
	}

	pDictionaryInternal->fieldsByNameLinks[pEntry->fid - (RSSL_MIN_FID)].pDictionaryEntry = pEntry;
	rsslHashTableInsertLink(&pDictionaryInternal->fieldsByName, &pDictionaryInternal->fieldsByNameLinks[pEntry->fid - (RSSL_MIN_FID)].nameTableLink, 
		&pEntry->acronym, &hashSum);

	dictionary->numberOfEntries++;
	if (pEntry->fid > dictionary->maxFid) dictionary->maxFid = pEntry->fid;
	if (pEntry->fid < dictionary->minFid) dictionary->minFid = pEntry->fid;

	return RSSL_RET_SUCCESS;
}

/* Adds an enum table refrence to a dictionary entry
 * If the entry does not exist, a placeholder will be created */
RsslRet _addFieldTableReferenceToDictionary(RsslDataDictionary *dictionary, RsslReferenceFidStore *pFid, RsslEnumTypeTable *pTable, RsslBuffer *errorText)
{
	RsslDictionaryEntry *pEntry = dictionary->entriesArray[pFid->fid];

	if (!pEntry)
	{
		/* No field exists for this yet, so create one for purposes of referencing the table.
		 * It's marked with type UNKNOWN and does not officially exist until the corresponding field is loaded(and finds this reference). */
		pEntry = (RsslDictionaryEntry*)calloc(1, sizeof(RsslDictionaryEntry));
		if (!pEntry)
		{
			_setError(errorText, "<%s:%d> Error allocating space for entry", __FILE__, __LINE__);
			return RSSL_RET_FAILURE;
		}
		if (pFid->acronym.data /* Won't be present in decoded payload */) 
		{
			if (_rsslCreateStringCopy(&pEntry->acronym, &pFid->acronym) < RSSL_RET_SUCCESS)
			{
				_setError(errorText, "<%s:%d> Error allocating space to store acronym", __FILE__, __LINE__);
				return RSSL_RET_FAILURE;
			}
		}
		pEntry->fid = pFid->fid;
		pEntry->rwfType = RSSL_DT_UNKNOWN;
		pEntry->pEnumTypeTable = pTable;

		dictionary->entriesArray[pFid->fid] = pEntry;
	}
	else
	{
		if (pFid->acronym.data && !rsslBufferIsEqual(&pEntry->acronym, &pFid->acronym))
		{
			_setError(errorText, "Acronym mismatch \"%s\" and \"%s\" between Field Dictionary and Enum Type Dictionary", pEntry->acronym.data, pFid->acronym.data);
			return RSSL_RET_FAILURE;
		}

		/* Already exists, just point the field to the table. */
		if (pEntry->pEnumTypeTable)
		{
			_setError(errorText, "FieldId %d has duplicate Enum Table reference", pFid->fid);
			return RSSL_RET_FAILURE;
		}

		pEntry->pEnumTypeTable = pTable;
	}

	return RSSL_RET_SUCCESS;
}

RsslRet _rollbackEnumDictionaryElementList(
	RsslEncodeIterator	*eIter)
{
	RsslRet ret;
	if ((ret = rsslEncodeElementListComplete(eIter, RSSL_FALSE)) < 0)
		return ret;
	if ((ret = rsslEncodeSeriesEntryComplete(eIter, RSSL_FALSE)) < 0)
		return ret;
	return rsslEncodeSeriesComplete(eIter, RSSL_TRUE);
}

RsslRet _rollbackEnumDictionaryElementEntry(
	RsslEncodeIterator	*eIter)
{
	RsslRet ret;
	if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_FALSE)) < 0)
		return ret;
	return _rollbackEnumDictionaryElementList(eIter);
}

RsslRet _rollbackEnumDictionaryArray(
	RsslEncodeIterator	*eIter)
{
	RsslRet ret;
	if ((ret = rsslEncodeArrayComplete(eIter, RSSL_FALSE)) < 0)
		return ret;
	return _rollbackEnumDictionaryElementEntry(eIter);
}

RsslRet _rollbackEnumDictionarySeriesEntry(
	RsslEncodeIterator	*eIter)
{
	RsslRet ret;
	if ((ret = rsslEncodeSeriesEntryComplete(eIter, RSSL_FALSE)) < 0)
		return ret;
	return rsslEncodeSeriesComplete(eIter, RSSL_TRUE);
}

RSSL_API RsslRet rsslLoadFieldDictionary(	const char				*filename,
							RsslDataDictionary	*dictionary,
							RsslBuffer			*errorText )
{
	FILE				*fp;
	TextFileReader		textFileReader;
	RsslDictionaryEntry	*newDictEntry = NULL;
	int					fidNum;
	int					curPos;
	int					lineNum = 0;
	RsslBuffer			rippleAcronym;
	short				rippleFid = 0;
	RippleDefintion		*undefinedRipples=0;
	int					tmpRwfType;
	int					ret;

	rsslClearBuffer(&rippleAcronym);

	if (filename == 0)
	{
		_setError(errorText, "NULL Filename pointer.");
		return RSSL_RET_FAILURE;
	}

	if (dictionary == 0)
	{
		_setError(errorText, "NULL Dictionary pointer.");
		return RSSL_RET_FAILURE;
	}

	if ((fp = fopen(filename, "r")) == NULL)
	{
		_setError(errorText, "Can't open file: '%s'.", filename);
		return RSSL_RET_FAILURE;
	}

	if (!dictionary->isInitialized && _initDictionary(dictionary, errorText) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if ((ret = textFileReaderInit(&textFileReader, fp, errorText)) != RSSL_RET_SUCCESS)
		return ret;

	while ((ret = textFileReaderReadLine(&textFileReader, errorText)) > 0)
	{
		lineNum++;

		if (textFileReader.currentLine[0] == '!')
		{
             curPos = getCopyUntilSpace( textFileReader.currentLine, 0, textFileReader.usrString);
			 if (0 == strcmp(textFileReader.usrString, "!tag"))
			 {
				 /* Tags */
				 if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
					 continue;
				 getRestOfLine( textFileReader.currentLine, curPos, textFileReader.usrString2);
				 if (_copyDictionaryTag(textFileReader.usrString, textFileReader.usrString2, RDM_DICTIONARY_FIELD_DEFINITIONS, dictionary, errorText) != RSSL_RET_SUCCESS)
					return _finishFailure(fp, dictionary, undefinedRipples, 0, &textFileReader);
			 }
			 continue;
		}

		curPos = 0;
		/* Look for acronym */
		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0) {
		  if (curPos != -1) { /* We hit something, but it wasn't what we wanted(probably a quotation mark). Return error. */
    			return (_setError(errorText, "Cannot find Acronym (Line=%d).", lineNum), _finishFailure(fp, dictionary, undefinedRipples, 0, &textFileReader));
		  }
		  else {
				continue; /* Didn't hit anything. Must be a blank line. Move on. */
		  }
		}
		if ((newDictEntry = (RsslDictionaryEntry*)malloc(sizeof(RsslDictionaryEntry))) == 0)
		{
            _setError(errorText, "Cannot malloc RsslDictionaryEntry.");
			return _finishFailure(fp, dictionary, undefinedRipples, 0, &textFileReader);
		}
		rsslClearBuffer(&newDictEntry->acronym);
		rsslClearBuffer(&newDictEntry->ddeAcronym);

		if (_rsslCreateStringCopyFromChar(&newDictEntry->acronym, textFileReader.usrString) < RSSL_RET_SUCCESS)
			return (_setError(errorText,"Cannot create storage for acronym", lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));


		if ((curPos = getCopyQuotedStr( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			return (_setError(errorText,"Cannot find DDE Acronym (Line=%d).", lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));
		if (_rsslCreateStringCopyFromChar(&newDictEntry->ddeAcronym, textFileReader.usrString) < RSSL_RET_SUCCESS)
			return (_setError(errorText,"Cannot create storage for DDE Acronym", lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));

		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			return (_setError(errorText, "Cannot find Fid Number (Line=%d).",lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));
		fidNum = atoi(textFileReader.usrString);

		if ((fidNum < RSSL_MIN_FID) || (fidNum > RSSL_MAX_FID))
			return (_setError(errorText, "Illegal fid number %d (Line=%d).", fidNum, lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));
		newDictEntry->fid = fidNum;

		if (fidNum < dictionary->minFid)
			dictionary->minFid = fidNum;
		else if (fidNum > dictionary->maxFid)
			dictionary->maxFid = fidNum;

		if (rippleAcronym.data)
		{
			if (rsslBufferIsEqual(&rippleAcronym, &newDictEntry->acronym))
			{
				dictionary->entriesArray[rippleFid]->rippleToField = (RsslInt16)fidNum;
				free(rippleAcronym.data);
				rsslClearBuffer(&rippleAcronym);
				rippleFid = 0;
			}
		}

		if ((curPos = getCopyUntilSpace(textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			return (_setError(errorText, "Cannot find Ripples To (Line=%d).",lineNum), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));

		/* Initialize to zero since will be filled in later, if exists */
		newDictEntry->rippleToField = 0;

		if (strcmp(textFileReader.usrString,"NULL") != 0)
		{
			if (rippleAcronym.data)
			{
				RippleDefintion *newDef = (RippleDefintion*)calloc(1,sizeof(RippleDefintion));
				if (newDef == 0)
					return (_setError(errorText, "malloc() failed for RippleDefintion temporary memory."), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));

				newDef->rippleAcronym = rippleAcronym;
				rsslClearBuffer(&rippleAcronym);

				newDef->rippleFid = rippleFid;
				newDef->next = undefinedRipples;
				undefinedRipples = newDef;

			}
			if (_rsslCreateStringCopyFromChar(&rippleAcronym, textFileReader.usrString) < RSSL_RET_SUCCESS)
				return (_setError(errorText, "Unable to create storage for ripple acronym."), _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader));
			rippleFid = fidNum;
		}

		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
		{
			_setError(errorText, "Cannot find Field Type (Line=%d).", lineNum);
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		}
		newDictEntry->fieldType = (RsslInt8)getFieldType(textFileReader.usrString);
		if (newDictEntry->fieldType == c_rsslMfeedError)
		{
			_setError(errorText, "Unknown Field Type '%s' (Line=%d).", textFileReader.usrString, lineNum);
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		}

		/* this is a UInt8 so this should never happen */
		/*
		if (newDictEntry->fieldType < 0)
		{
			if ((errorText != 0) && (errorText->data != 0))
				snprintf(errorText->data,errorText->length,"Illegal Field Type '%s' (Line=%d).",strval,lineNum);
			rsslDeleteDataDictionary(dictionary);
			free(newDictEntry);
			return RSSL_RET_FAILURE;
		}
		*/

		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
		{
			_setError(errorText, "Cannot find Length (Line=%d).",lineNum);
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		}
		newDictEntry->length = (RsslUInt16)atoi(textFileReader.usrString);

		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
		{
			_setError(errorText, "Cannot find EnumLen or RwfType (Line=%d).",lineNum);
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		}

		if (textFileReader.usrString[0] == '(')
		{
			if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			{
				_setError(errorText, "Cannot find EnumLen (Line=%d).",lineNum);
				return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
			}
			newDictEntry->enumLength = (RsslUInt8)atoi(textFileReader.usrString);
			if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			{
				_setError(errorText, "Cannot find end ')' in EnumLen (Line=%d).",lineNum);
				return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
			}
			if (textFileReader.usrString[0] != ')')
			{
				_setError(errorText, "No ')' at end of EnumLen defition (Line=%d).",lineNum);
				return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
			}
			if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
			{
				_setError(errorText, "Cannot find Rwf Type (Line=%d).",lineNum);
				return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
			}
		}
		else
		{
			/* No enum length */
			newDictEntry->enumLength = 0;
		}
			
		tmpRwfType = getRwfFieldType(textFileReader.usrString);
		if (tmpRwfType < 0)
		{
			if ((errorText != 0) && (errorText->data != 0))
				snprintf(errorText->data,errorText->length,"Invalid Rwf Type '%s' (Line=%d)",textFileReader.usrString,lineNum);
			rsslDeleteDataDictionary(dictionary);
			free(newDictEntry);
			return RSSL_RET_FAILURE;
		}
		newDictEntry->rwfType = (RsslUInt8)tmpRwfType;

		if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
		{
			_setError(errorText, "Cannot find Rwf Length (Line=%d).",lineNum);
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		}
		newDictEntry->rwfLength = (RsslUInt16)atoi(textFileReader.usrString);

		if (_addFieldToDictionary(dictionary, newDictEntry, errorText, lineNum) != RSSL_RET_SUCCESS)
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);
		newDictEntry = 0;
	}

	if (ret < 0)
		return RSSL_RET_FAILURE;


	if ((dictionary->minFid <= RSSL_MAX_FID) && (dictionary->maxFid >= RSSL_MIN_FID))
	{
		/* Go through the undefined ripplesTo fields and find */
		while (undefinedRipples != 0)
		{
			RippleDefintion *tdef = undefinedRipples;
			int j;
			for (j = dictionary->minFid ; j <= dictionary->maxFid; j++)
			{
				if ((dictionary->entriesArray[j] != 0) &&
					(rsslBufferIsEqual(&tdef->rippleAcronym,&dictionary->entriesArray[j]->acronym)))
				{
					dictionary->entriesArray[tdef->rippleFid]->rippleToField = j;
					break;
				}
			}
			undefinedRipples = tdef->next;
			free(tdef->rippleAcronym.data);
			free(tdef);
		}
	}

	if (!dictionary->infoField_Version.data) /* Set default if tag not found */
		if (copyTagData(&dictionary->infoField_Version, c_defaultVersion) < RSSL_RET_SUCCESS)
			return _finishFailure(fp, dictionary, undefinedRipples, newDictEntry, &textFileReader);

	fclose(fp);
	textFileReaderCleanup(&textFileReader);
	return RSSL_RET_SUCCESS;
}

static RsslRet rsslEncodeDataDictSummaryData(
				RsslEncodeIterator		*seriesIter,
				RsslDataDictionary		*dictionary,
				RsslInt					type,
				RsslBuffer				*errorText )
{
	RsslRet				ret;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslElementList		elementList = RSSL_INIT_ELEMENT_LIST;
	RsslInt64			tmpI64;

	rsslElementListApplyHasStandardData(&elementList);

	if ((ret = rsslEncodeElementListInit(seriesIter, &elementList,0, 0)) < 0)
	{
		_setError(errorText, "rsslEncodeElementListInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.dataType = RSSL_DT_INT;
	element.name = RSSL_ENAME_DICT_TYPE;
	if ((ret = rsslEncodeElementEntry( seriesIter, &element, &type)) < 0)
	{
		_setError(errorText, "rsslEncodeElementEntry failed %d - Type",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.dataType = RSSL_DT_INT;
	element.name = RSSL_ENAME_DICTIONARY_ID;
	tmpI64 = dictionary->info_DictionaryId;
	if ((ret = rsslEncodeElementEntry( seriesIter, &element, &tmpI64)) < 0)
	{
		_setError(errorText, "rsslEncodeElementEntry failed %d - DictionaryId",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	switch(type)
	{
	case RDM_DICTIONARY_FIELD_DEFINITIONS:
		/* Version */
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_DICT_VERSION;
		if ((ret = rsslEncodeElementEntry( seriesIter, &element, &dictionary->infoField_Version )) < 0)
		{
			_setError(errorText, "rsslEncodeElementEntry failed %d - Version",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
		break;

	case RDM_DICTIONARY_ENUM_TABLES:
		/* RT_Version */
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_ENUM_RT_VERSION;
		if ((ret = rsslEncodeElementEntry( seriesIter, &element, &dictionary->infoEnum_RT_Version )) < 0)
		{
			_setError(errorText, "rsslEncodeElementEntry failed %d - RT_Version",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		/* DT_Version */
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_ENUM_DT_VERSION;
		if ((ret = rsslEncodeElementEntry( seriesIter, &element, &dictionary->infoEnum_DT_Version )) < 0)
		{
			_setError(errorText, "rsslEncodeElementEntry failed %d - DT_Version",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
		/* Version */
		element.dataType = RSSL_DT_ASCII_STRING;
		element.name = RSSL_ENAME_DICT_VERSION;
		if ((ret = rsslEncodeElementEntry( seriesIter, &element, &dictionary->infoEnum_DT_Version )) < 0)
		{
			_setError(errorText, "rsslEncodeElementEntry failed %d - Version",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
		break;
	}

	if ((ret = rsslEncodeElementListComplete(seriesIter,1)) < 0)
	{
		_setError(errorText, "rsslEncodeElementListComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeSeriesSummaryDataComplete(seriesIter, 1)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesSummaryDataComplete failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	return 1;
}

static RsslElementSetDefEntry rsslElementSetDefEntries[9] =
{
	{ { 4, "NAME"}, RSSL_DT_ASCII_STRING },
	{ { 3, "FID"}, RSSL_DT_INT_2 },
	{ { 8, "RIPPLETO"}, RSSL_DT_INT_2 },
	{ { 4, "TYPE"}, RSSL_DT_INT_1 },
	{ { 6, "LENGTH"}, RSSL_DT_UINT_2 },
	{ { 7, "RWFTYPE"}, RSSL_DT_UINT_1 },
	{ { 6, "RWFLEN"}, RSSL_DT_UINT_2 },
	{ { 10, "ENUMLENGTH"}, RSSL_DT_UINT_1 },
	{ { 8, "LONGNAME"}, RSSL_DT_ASCII_STRING }
};

static const RsslElementSetDef setDef0_Minimal =
{
	0, /* SetID */
	7, /* count */
	rsslElementSetDefEntries
};

static const RsslElementSetDef setDef0_Normal =
{
	0, /* SetID */
	9, /* count */
	rsslElementSetDefEntries
};

static RsslElementSetDefEntry enumSetDefEntries[4] =
{
	{ { 4, "FIDS" }, RSSL_DT_ARRAY },
	{ { 5, "VALUE" }, RSSL_DT_ARRAY },
	{ { 7, "DISPLAY" }, RSSL_DT_ARRAY },
	{ { 7, "MEANING" }, RSSL_DT_ARRAY }
};

static const RsslElementSetDef enumSetDef0_Normal =
{
	0,
	3,
	enumSetDefEntries
};

static const RsslElementSetDef enumSetDef0_Verbose =
{
	0,
	4,
	enumSetDefEntries
};

static RsslRet rsslEncodeDataDictEntry(
				RsslEncodeIterator	*seriesIter,
				RsslDictionaryEntry *entry,
				RDMDictionaryVerbosityValues	verbosity,
				RsslBuffer			*errorText,
				RsslLocalElementSetDefDb *pSetDb )
{
	RsslRet				ret;
	RsslSeriesEntry		SeriesEntry = RSSL_INIT_SERIES_ENTRY;
	RsslElementList		elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	int					maxEncSizeNeeded = entry->acronym.length + entry->ddeAcronym.length + 14;
	RsslInt64			tmp64;
	RsslUInt64			tmpU64;

	if (_rsslIteratorOverrun( seriesIter, 3 + maxEncSizeNeeded))
		return RSSL_RET_DICT_PART_ENCODED;

	if ((ret = rsslEncodeSeriesEntryInit(seriesIter, &SeriesEntry, maxEncSizeNeeded)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesEntryInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	rsslElementListApplyHasSetData(&elementList);

	if ((ret = rsslEncodeElementListInit(seriesIter, &elementList, pSetDb, 0)) < 0)
	{
		_setError(errorText, "rsslEncodeElementListInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_NAME;
	element.dataType = RSSL_DT_ASCII_STRING;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, (void*)&entry->acronym)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry NAME '%s' failed %d",entry->acronym.data,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_ID;
	element.dataType = RSSL_DT_INT;
	tmp64 = entry->fid;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmp64)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry FID %d failed %d",entry->fid,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_RIPPLETO;
	element.dataType = RSSL_DT_INT;
	tmp64 = entry->rippleToField;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmp64)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry RIPPLETO %d failed %d",entry->rippleToField,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_TYPE;
	element.dataType = RSSL_DT_INT;
	tmp64 = entry->fieldType;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmp64)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry TYPE %d failed %d",entry->fieldType,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_LENGTH;
	element.dataType = RSSL_DT_UINT;
	tmpU64 = entry->length;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmpU64)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry LENGTH %d failed %d",entry->length,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_RWFTYPE;
	element.dataType = RSSL_DT_UINT;
	tmpU64 = entry->rwfType;
	if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmpU64)) != RSSL_RET_SUCCESS)
	{
		_setError(errorText, "rsslEncodeElementEntry RWFTYPE %d failed %d",entry->rwfType,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	element.name = RSSL_ENAME_FIELD_RWFLEN;
	element.dataType = RSSL_DT_UINT;
	tmpU64 = entry->rwfLength;
	ret = rsslEncodeElementEntry(seriesIter, &element, &tmpU64);
	if (((verbosity >= RDM_DICTIONARY_NORMAL && ret != RSSL_RET_SUCCESS)
		|| (verbosity < RDM_DICTIONARY_NORMAL && ret != RSSL_RET_SET_COMPLETE)))
	{
		_setError(errorText, "rsslEncodeElementEntry RWFLEN %d failed %d",entry->rwfLength,ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	if (verbosity >= RDM_DICTIONARY_NORMAL)
	{
		element.name = RSSL_ENAME_FIELD_ENUMLENGTH;
    	element.dataType = RSSL_DT_UINT;
		tmpU64 = entry->enumLength;
		if ((ret = rsslEncodeElementEntry(seriesIter, &element, &tmpU64)) != RSSL_RET_SUCCESS)
		{
			_setError(errorText, "rsslEncodeElementEntry ENUMLENGTH %d failed %d",entry->enumLength,ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		element.name = RSSL_ENAME_FIELD_LONGNAME;
		element.dataType = RSSL_DT_ASCII_STRING;
		if ((ret = rsslEncodeElementEntry(seriesIter, &element, (void*)&entry->ddeAcronym)) != RSSL_RET_SET_COMPLETE)
		{
			_setError(errorText, "rsslEncodeElementEntry LONGNAME Acronym '%s' failed %d",entry->ddeAcronym.data,ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}

	if ((ret = rsslEncodeElementListComplete(seriesIter,1)) < 0)
	{
		_setError(errorText, "rsslEncodeElementListComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}

	if ((ret = rsslEncodeSeriesEntryComplete(seriesIter, 1)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesEntryComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeFieldDictionary(
	RsslEncodeIterator	*eIter,
	RsslDataDictionary	*dictionary,
	int					*currentFid,
	RDMDictionaryVerbosityValues verbosity,
	RsslBuffer			*errorText )
{
	RsslRet				ret;
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslLocalElementSetDefDb	setDb;
	int curFid = *currentFid;

	if (!dictionary->isInitialized)
		return (_setError(errorText, "Dictionary not initialized"), RSSL_RET_FAILURE);

	rsslClearLocalElementSetDefDb(&setDb);
	setDb.definitions[0] = (verbosity >= RDM_DICTIONARY_NORMAL) ? setDef0_Normal : setDef0_Minimal;

	/* Set the data format */
	series.containerType = RSSL_DT_ELEMENT_LIST;

	/* Don't encode set definitions for info */
	if (verbosity > RDM_DICTIONARY_INFO)
		rsslSeriesApplyHasSetDefs(&series);

	/* If first packet, then send hint and summary data */
	if (curFid <= dictionary->minFid)
	{
		/* Set the total count hint if exists */
		if (dictionary->numberOfEntries > 0)
		{
			rsslSeriesApplyHasTotalCountHint(&series);
			series.totalCountHint = dictionary->numberOfEntries;
		}
		rsslSeriesApplyHasSummaryData(&series);
	}

	if ((ret = rsslEncodeSeriesInit(eIter, &series, 0, 0)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesInit failed %d",ret);
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	/* Don't encode set definitions for info */
	if (verbosity > RDM_DICTIONARY_INFO)
	{
		if ((ret = rsslEncodeLocalElementSetDefDb(eIter, &setDb)) < 0)
		{
			_setError(errorText, "rsslEncodeLocalElementSetDefDb failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}

		if ((ret = rsslEncodeSeriesSetDefsComplete( eIter, 1 )) < 0)
		{
			_setError(errorText, "rsslEncodeSeriesSetDefsComplete failed %d",ret);
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
		}
	}

	/* If first packet, encode the summary data */
	if (curFid <= dictionary->minFid)
	{
		if ((ret = rsslEncodeDataDictSummaryData(eIter,dictionary, RDM_DICTIONARY_FIELD_DEFINITIONS, errorText)) < 0)
			return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	while (curFid <= dictionary->maxFid)
	{
		/* Don't encode actual entries for info */
		if (verbosity > RDM_DICTIONARY_INFO)
		{
			/* Entries with type UNKNOWN were loaded from an enumtype. Don't send them since they aren't officially defined yet. */
			if (dictionary->entriesArray[curFid] != 0 && dictionary->entriesArray[curFid]->rwfType != RSSL_DT_UNKNOWN)
			{
				if ((ret = rsslEncodeDataDictEntry(
								eIter,
								dictionary->entriesArray[curFid],
								verbosity,
								errorText,
								&setDb)) < 0)
					return RSSL_RET_FAILURE;

				/* If we have filled the buffer, then complete */
				if (ret == RSSL_RET_DICT_PART_ENCODED)
					break;
			}
		}
		(curFid)++;
	}

	if ((ret = rsslEncodeSeriesComplete(eIter, 1)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}

	*currentFid = curFid;
	return (curFid > dictionary->maxFid ? RSSL_RET_SUCCESS : RSSL_RET_DICT_PART_ENCODED);
}

RSSL_API RsslRet rsslPrintDataDictionary( FILE * fileptr, RsslDataDictionary *dictionary )
{
	RsslInt32 i;

	if (fileptr == 0)
		return RSSL_RET_FAILURE;
	if (dictionary == 0)
		return RSSL_RET_FAILURE;
	if (dictionary->entriesArray == 0)
		return RSSL_RET_FAILURE;
	if (!dictionary->isInitialized)
		return RSSL_RET_FAILURE;

	fprintf(fileptr,"Data Dictionary Dump: MinFid=%d MaxFid=%d NumEntries %d\n\n",dictionary->minFid,dictionary->maxFid,dictionary->numberOfEntries);

	fprintf(	fileptr, "Tags:\n"
				"  DictionaryId=\"%d\"\n\n",
				dictionary->info_DictionaryId);

	fprintf(	fileptr,
				"  [Field Dictionary Tags]\n"
				"      Filename=\"%s\"\n"
				"          Desc=\"%s\"\n"
				"       Version=\"%s\"\n"
				"         Build=\"%s\"\n"
				"          Date=\"%s\"\n\n"
				,
			dictionary->infoField_Filename.data ? dictionary->infoField_Filename.data : "",
			dictionary->infoField_Desc.data ? dictionary->infoField_Desc.data : "",
			dictionary->infoField_Version.data ? dictionary->infoField_Version.data : "",
			dictionary->infoField_Build.data ? dictionary->infoField_Build.data : "",
			dictionary->infoField_Date.data ? dictionary->infoField_Date.data : "");

	fprintf(	fileptr,
				"  [Enum Type Dictionary Tags]\n"
				"      Filename=\"%s\"\n"
				"          Desc=\"%s\"\n"
				"    RT_Version=\"%s\"\n"
				"    DT_Version=\"%s\"\n"
				"          Date=\"%s\"\n\n"
				,
			dictionary->infoEnum_Filename.data ? dictionary->infoEnum_Filename.data : "",
			dictionary->infoEnum_Desc.data ? dictionary->infoEnum_Desc.data : "",
			dictionary->infoEnum_RT_Version.data ? dictionary->infoEnum_RT_Version.data : "",
			dictionary->infoEnum_DT_Version.data ? dictionary->infoEnum_DT_Version.data : "",
			dictionary->infoEnum_Date.data ? dictionary->infoEnum_Date.data : "");

	fprintf(fileptr, "Field Dictionary:\n");

	for (i=RSSL_MIN_FID; i<=RSSL_MAX_FID; i++)
	{
		if (dictionary->entriesArray[i] != 0 && dictionary->entriesArray[i]->rwfType != RSSL_DT_UNKNOWN)
		{
			fprintf(fileptr,"  Fid=%d '%s' '%s' Type=%d RippleTo=%d Len=%d EnumLen=%d RwfType=%d RwfLen=%d\n",  dictionary->entriesArray[i]->fid,
					dictionary->entriesArray[i]->acronym.data,
					dictionary->entriesArray[i]->ddeAcronym.data,
					dictionary->entriesArray[i]->fieldType,
					dictionary->entriesArray[i]->rippleToField,
					dictionary->entriesArray[i]->length,
					dictionary->entriesArray[i]->enumLength,
					dictionary->entriesArray[i]->rwfType,
					dictionary->entriesArray[i]->rwfLength );
		}
	}

	/* Enum Tables Dump */

	fprintf(fileptr, "\nEnum Type Tables:\n");

	for (i = 0; i < dictionary->enumTableCount; ++i)
	{
		RsslUInt32 j;
		RsslEnumTypeTable *pTable = dictionary->enumTables[i];
			
		for (j = 0; j < pTable->fidReferenceCount; ++j)
			fprintf(fileptr, "(Referenced by Fid %d)\n", pTable->fidReferences[j]);

		for( j = 0; j <= pTable->maxValue; ++j)
		{
			RsslEnumType *pEnumType = pTable->enumTypes[j];

			if (pEnumType)
				fprintf(fileptr, "value=%u display=\"%.*s\" meaning=\"%.*s\"\n", 
					pEnumType->value, 
					pEnumType->display.length, pEnumType->display.data,
					pEnumType->meaning.length, pEnumType->meaning.data);
		}

		fprintf(fileptr, "\n");
	}

	fflush(fileptr);
	return RSSL_RET_SUCCESS;
}

/* Frees an RsslDictionaryEntry -- cleans up acronym and ddeAcronym text (but not the enum table) */
static void _deleteDictionaryEntry(RsslDataDictionary *pDictionary, RsslFieldId fid)
{
	RsslDictionaryEntry *pEntry = pDictionary->entriesArray[fid];
	pDictionary->entriesArray[fid] = 0;
	if (pEntry->acronym.data) free(pEntry->acronym.data);
	if (pEntry->ddeAcronym.data) free(pEntry->ddeAcronym.data);
	free(pEntry);
}

static void _deleteDictionaryEnumTable(RsslEnumTypeTable *pTable)
{
	int i;

	for(i = 0; i <= pTable->maxValue; ++i)
	{
		RsslEnumType *pDef = pTable->enumTypes[i];

		if (pDef != NULL)
		{
			if (pDef->display.data) free(pDef->display.data);
			if (pDef->meaning.data) free(pDef->meaning.data);
			free(pDef);
		}
	}

	free(pTable->enumTypes);
	free(pTable->fidReferences);
	free(pTable);
}

RSSL_API RsslRet rsslDeleteDataDictionary( RsslDataDictionary *dictionary )
{
	RsslInt32 i;
	RsslDictionaryEntry **origDictMem;
	RsslDictionaryInternal *pDictionaryInternal;

	if (dictionary == 0 || !dictionary->isInitialized)
		return RSSL_RET_FAILURE;

	pDictionaryInternal = (RsslDictionaryInternal*)dictionary->_internal;
	RSSL_ASSERT(pDictionaryInternal != NULL, Dictionary internal pointers is null); /* Dictionary was initialized, so this should exist. */

	/* Delete tag info */
	if (dictionary->infoField_Version.data)
		free(dictionary->infoField_Version.data);
	if (dictionary->infoField_Filename.data)
		free(dictionary->infoField_Filename.data);
	if (dictionary->infoField_Desc.data)
		free(dictionary->infoField_Desc.data);
	if (dictionary->infoField_Build.data)
		free(dictionary->infoField_Build.data);
	if (dictionary->infoField_Date.data)
		free(dictionary->infoField_Date.data);

	if (dictionary->infoEnum_DT_Version.data)
		free(dictionary->infoEnum_DT_Version.data);
	if (dictionary->infoEnum_RT_Version.data)
		free(dictionary->infoEnum_RT_Version.data);
	if (dictionary->infoEnum_Filename.data)
		free(dictionary->infoEnum_Filename.data);
	if (dictionary->infoEnum_Desc.data)
		free(dictionary->infoEnum_Desc.data);
	if (dictionary->infoEnum_Date.data)
		free(dictionary->infoEnum_Date.data);
  
	if (dictionary->entriesArray != 0)
	{
		if (!pDictionaryInternal->isLinked /* Don't cleanup entries if another dictionary is pointing to them. */)
		{
			for (i=RSSL_MIN_FID; i<=RSSL_MAX_FID; i++)
			{
				if (dictionary->entriesArray[i] != 0)
					_deleteDictionaryEntry(dictionary, (RsslFieldId)i);
			}
		}

		if (RSSL_MIN_FID < 0)
			origDictMem = &dictionary->entriesArray[RSSL_MIN_FID];
		else
			origDictMem = &dictionary->entriesArray[0];

		free(origDictMem);
	}

	if (dictionary->enumTables)
	{
		if  (!pDictionaryInternal->isLinked /* Don't cleanup entries if another dictionary is pointing to them. */)
		{
			for (i = 0; i < dictionary->enumTableCount; ++i)
				_deleteDictionaryEnumTable(dictionary->enumTables[i]);
		}

		free(dictionary->enumTables);
	}

	rsslHashTableCleanup(&pDictionaryInternal->fieldsByName);
	free(pDictionaryInternal);

	dictionary->isInitialized = RSSL_FALSE;
	return RSSL_RET_SUCCESS;
}

/* if failure is returned, dictionary needs to be deleted */
RSSL_API RsslRet rsslDecodeFieldDictionary(
	RsslDecodeIterator	*dIter,
	RsslDataDictionary	*dictionary,
	RDMDictionaryVerbosityValues verbosity,
	RsslBuffer			*errorText )
{
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslElementList		elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb elListSetDb;
	RsslRet ret;
	RsslSeriesEntry row = RSSL_INIT_SERIES_ENTRY;
	RsslDictionaryEntry	*newDictEntry;
	RsslInt64 tempInt = 0;
	RsslUInt64 tempUInt = 0;
	char setDefMemory[LOCAL_EL_DEFS_TLS_SIZE];
	
	if (dictionary == 0)
	{
		_setError(errorText, "NULL dictionary pointer.");
		return RSSL_RET_FAILURE;
	}

	if (!dictionary->isInitialized && _initDictionary(dictionary, errorText) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if ((ret = rsslDecodeSeries(dIter, &series)) < 0)
		return (_setError(errorText, "rsslDecodeSeries failed %d",ret), _finishFailure(0, dictionary, 0, 0, 0));

	/* if this is not an element list, we should fail for now */
	if (series.containerType != RSSL_DT_ELEMENT_LIST)
		return (_setError(errorText, "Invalid container type of %d; expecting %d (RSSL_DT_ELEMENT_LIST)", series.containerType, RSSL_DT_ELEMENT_LIST), _finishFailure(0, dictionary, 0, 0, 0));

	/* decode summary data */
	if (rsslSeriesCheckHasSummaryData(&series))
	{
		/* decode summary data here */

		/* since we own dictionary, lets assume that we create memory here - they should only delete this with our
			delete dictionary function */

		if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < 0)
			return (_setError(errorText, "rsslDecodeElementList failed - %d", ret), _finishFailure(0, dictionary, 0, 0, 0));

		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if ( ret < 0)
				return (_setError(errorText, "rsslDecodeElementEntry failed - %d", ret), _finishFailure(0, dictionary, 0, 0, 0));

			if (_decodeDictionaryTag(dIter, &element, RDM_DICTIONARY_FIELD_DEFINITIONS, dictionary, errorText) != RSSL_RET_SUCCESS)
		 		return _finishFailure(0, dictionary, 0, 0, 0);
		}
	}

	if (rsslSeriesCheckHasSetDefs(&series))
	{
		rsslClearLocalElementSetDefDb(&elListSetDb);
		elListSetDb.entries.data = setDefMemory;
		elListSetDb.entries.length = sizeof(setDefMemory);
		if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elListSetDb)) < 0)
    		return (_setError(errorText, "rsslDecodeLocalElementSetDefDb failed - %d", ret), RSSL_RET_FAILURE);
	}

	while( (ret = rsslDecodeSeriesEntry(dIter, &row)) != RSSL_RET_END_OF_CONTAINER)
	{
		if ( ret < 0)
			return (_setError(errorText, "rsslDecodeSeriesEntry failed - %d", ret), _finishFailure(0, dictionary, 0, 0, 0));

		/* decode element list here */
		if ((ret = rsslDecodeElementList(dIter, &elementList, &elListSetDb)) < 0)
			return (_setError(errorText, "rsslDecodeElementList failed - %d", ret), _finishFailure(0, dictionary, 0, 0, 0));


		if ((newDictEntry = (RsslDictionaryEntry*)malloc(sizeof(RsslDictionaryEntry))) == 0)
		{
			_setError(errorText, "Cannot malloc RsslDictionaryEntry.");
			return _finishFailure(0, dictionary, 0, 0, 0);
		}
		rsslClearBuffer(&newDictEntry->acronym);
		rsslClearBuffer(&newDictEntry->ddeAcronym);

		while ( (ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret < 0)
				return (_setError(errorText, "rsslDecodeElementEntry failed - %d", ret), _finishFailure(0, dictionary, 0, newDictEntry, 0));

			/* Can't make any assumptions about the order of elements
			 * (even though at this time Infra send them in this order) */

			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_NAME))
			{
				if (element.dataType != RSSL_DT_ASCII_STRING)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_NAME.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}

				if (_rsslCreateStringCopy(&newDictEntry->acronym, &element.encData) < RSSL_RET_FAILURE)
				{
					_setError(errorText, "<%s:%d> Error allocating space to store acronym", __FILE__, __LINE__);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_ID))
			{
				if (element.dataType != RSSL_DT_INT || rsslDecodeInt(dIter, &tempInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_ID.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}

				/* now populate fid */
				newDictEntry->fid = (RsslInt16)tempInt;

				/* do max and min fid stuff */
				if (newDictEntry->fid > dictionary->maxFid)
					dictionary->maxFid = newDictEntry->fid;
				if (newDictEntry->fid < dictionary->minFid)
					dictionary->minFid = newDictEntry->fid;
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_RIPPLETO))
			{
				if (element.dataType != RSSL_DT_INT || rsslDecodeInt(dIter, &tempInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_RIPPLETO.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}
				newDictEntry->rippleToField = (RsslInt16)tempInt;
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_TYPE))
			{
				if (element.dataType != RSSL_DT_INT || rsslDecodeInt(dIter, &tempInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_TYPE.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}
				newDictEntry->fieldType = (RsslInt8)tempInt;
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_LENGTH))
			{
				if (element.dataType != RSSL_DT_UINT || rsslDecodeUInt(dIter, &tempUInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_LENGTH.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}
				newDictEntry->length = (RsslUInt16)tempUInt;
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_RWFTYPE))
			{
				if (element.dataType != RSSL_DT_UINT || rsslDecodeUInt(dIter, &tempUInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_RWFTYPE.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}

				/* Need to do table lookup so legacy types (e.g. INT32/REAL32) are converted. */
				newDictEntry->rwfType = _rsslPrimitiveType(tempUInt);
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_RWFLEN))
			{
				if (element.dataType != RSSL_DT_UINT || rsslDecodeUInt(dIter, &tempUInt) < 0)
				{
					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_RWFLEN.data);
					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
				}
				newDictEntry->rwfLength = (RsslUInt16)tempUInt;
			}
			else if (verbosity >= RDM_DICTIONARY_NORMAL)/* optional elements depending on verbosity */
			{
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_ENUMLENGTH))
				{
					if (element.dataType != RSSL_DT_UINT || rsslDecodeUInt(dIter, &tempUInt) < 0)
    				{
    					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_ENUMLENGTH.data);
    					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
    				}
					newDictEntry->enumLength = (RsslUInt8)tempUInt;
				}
				else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_FIELD_LONGNAME))
				{
    				if (element.dataType != RSSL_DT_ASCII_STRING)
    				{
    					_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_FIELD_LONGNAME.data);
    					return _finishFailure(0, dictionary, 0, newDictEntry, 0);
    				}
					if (_rsslCreateStringCopy(&newDictEntry->ddeAcronym, &element.encData) < RSSL_RET_FAILURE)
					{
						_setError(errorText, "<%s:%d> Error allocating space to store acronym", __FILE__, __LINE__);
						return _finishFailure(0, dictionary, 0, newDictEntry, 0);
					}
				}
			}
		}

		if (_addFieldToDictionary(dictionary, newDictEntry, errorText, -1) != RSSL_RET_SUCCESS)
			return _finishFailure(0, dictionary, 0, newDictEntry, 0);
		newDictEntry = 0;
	}

	return RSSL_RET_SUCCESS;
}


RsslRet _addTableToDictionary( RsslDataDictionary *dictionary, RsslUInt32 fidsCount, RsslReferenceFidStore *pFids, RsslEnum maxValue, RsslEnumTypeStore *pEnumTypes, RsslBuffer *errorText, int lineNum)
{
	RsslEnumTypeTable *pTable;
	RsslFieldId *fidRefs;
	int i, j;

	if (dictionary->enumTableCount == ENUM_TABLE_MAX_COUNT) /* Unlikely. */
		return (_setError(errorText, "Cannot add more tables to this dictionary.", lineNum), RSSL_RET_FAILURE);

	if (!pFids) {
		if (lineNum > 0)
    		return (_setError(errorText, "No referencing FIDs found before enum table(Line=%d).", lineNum), RSSL_RET_FAILURE);
		else
    		return (_setError(errorText, "No referencing FIDs found before enum table."), RSSL_RET_FAILURE);
	}
	
	pTable = (RsslEnumTypeTable*)malloc(sizeof(RsslEnumTypeTable));
	if (!pTable)
		return (_setError(errorText, "Unable to create memory for enumeration table."), RSSL_RET_FAILURE);
	pTable->maxValue = maxValue;
	pTable->enumTypes = (RsslEnumType**)calloc(maxValue+1, sizeof(RsslEnumType*));

	/* Create table and add it to dictionary */
	while(pEnumTypes)
	{
		RsslEnum value = pEnumTypes->enumType.value;

		RSSL_ASSERT(pEnumTypes->enumType.value <= maxValue, Invalid content);

		if (pTable->enumTypes[value])
		{
			_setError(errorText, "Enum type table has Duplicate value: \"%u\"", value);
			for(i = 0; i <= pTable->maxValue; ++i) if (pTable->enumTypes[i]) free(pTable->enumTypes[i]);
			free(pTable->enumTypes);
			free(pTable);
			return RSSL_RET_FAILURE;
		}


		pTable->enumTypes[value] = (RsslEnumType*)malloc(sizeof(RsslEnumTypeImpl));
		if (!pTable->enumTypes[value])
			return (_setError(errorText, "Unable to create storage for enum type value."), RSSL_RET_FAILURE);
		*pTable->enumTypes[value] = pEnumTypes->enumType;
		((RsslEnumTypeImpl*)pTable->enumTypes[value])->flags = 0;
		pEnumTypes = pEnumTypes->next;
	}

	pTable->fidReferences = fidRefs = (RsslFieldId*)malloc(fidsCount*sizeof(RsslFieldId));
	if (!pTable->fidReferences)
		return (_setError(errorText, "Unable to create storage for fid cross references."), RSSL_RET_FAILURE);
	pTable->fidReferenceCount = fidsCount;

	/* Point all referencing fields at it */
	while(pFids)
	{
		RSSL_ASSERT(fidsCount > 0, Invalid content);

		if (_addFieldTableReferenceToDictionary(dictionary, pFids, pTable, errorText) != RSSL_RET_SUCCESS)
		{
			for(i = 0; i <= pTable->maxValue; ++i) if (pTable->enumTypes[i]) free(pTable->enumTypes[i]);
			free(pTable->enumTypes);
			free(pTable);
			return RSSL_RET_FAILURE;
		}

		fidRefs[--fidsCount] = pFids->fid;
		pFids = pFids->next;
	}

	/* Look for enum values with the same display strings as other values. Mark them as duplicate. */
	for (i = 1; i < pTable->maxValue; ++i)
	{
		RsslEnumTypeImpl *pEnum1 = (RsslEnumTypeImpl*)pTable->enumTypes[i];

		/* If enum not in table, or we already know it's a duplicate, skip. */
		if (pEnum1 == NULL || pEnum1->flags & RSSL_ENUMTYPE_FL_DUPLICATE_DISPLAY) 
			continue; 

		for (j = 0; j < i; ++j)
		{
			RsslEnumTypeImpl *pEnum2 = (RsslEnumTypeImpl*)pTable->enumTypes[j];

			/* If enum not in table, skip. */
			if (pEnum2 == NULL)
				continue;

			if (rsslBufferIsEqual(&pEnum1->base.display, &pEnum2->base.display))
			{
				/* Values have the same display string; mark them as duplicates. */
				pEnum1->flags |= RSSL_ENUMTYPE_FL_DUPLICATE_DISPLAY;
				pEnum2->flags |= RSSL_ENUMTYPE_FL_DUPLICATE_DISPLAY;
				break;
			}
		}
	}

	RSSL_ASSERT(fidsCount == 0, Invalid content);
	dictionary->enumTables[dictionary->enumTableCount++] = pTable;
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslLoadEnumTypeDictionary(	const char				*filename,
							RsslDataDictionary	*dictionary,
							RsslBuffer			*errorText )
{
	FILE				*fp;
	TextFileReader		textFileReader;
	int					curPos;
	int					lineNum = 0;
	int					ret;
	RsslBool success;
	RsslBool textIsHex;
	RsslReferenceFidStore			*pFids = 0;
	RsslUInt32			fidsCount = 0;
	RsslEnumTypeStore		*pEnumTypes = 0; RsslEnum maxValue = 0;

	if (filename == 0)
	{
		_setError(errorText, "NULL Filename pointer.");
		return RSSL_RET_FAILURE;
	}

	if (dictionary == 0)
	{
		_setError(errorText, "NULL Dictionary pointer.");
		return RSSL_RET_FAILURE;
	}

	if ((fp = fopen(filename, "r")) == NULL)
	{
		_setError(errorText, "Can't open file: '%s'.", filename);
		return RSSL_RET_FAILURE;
	}

	if (!dictionary->isInitialized && _initDictionary(dictionary, errorText) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if ((ret = textFileReaderInit(&textFileReader, fp, errorText)) != RSSL_RET_SUCCESS)
		return ret;

	while ((ret = textFileReaderReadLine(&textFileReader, errorText)) > 0)
	{
		lineNum++;

		if (textFileReader.currentLine[0] == '!')
		{
             curPos = getCopyUntilSpace( textFileReader.currentLine, 0, textFileReader.usrString);
			 if (0 == strcmp(textFileReader.usrString, "!tag"))
			 {
				 /* Tags */
				 if ((curPos = getCopyUntilSpace( textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
					 continue;
				 getRestOfLine( textFileReader.currentLine, curPos, textFileReader.usrString2);
				 if (_copyDictionaryTag(textFileReader.usrString, textFileReader.usrString2, RDM_DICTIONARY_ENUM_TABLES, dictionary, errorText))
					return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader);
			 }
			 continue;
		}

		/* Build a list of Fids. Once finished, make sure the fields point to the parsed enum table
		* If the field does not exist, create it with UNKNOWN type. */

		if ((curPos = getCopyUntilSpace(textFileReader.currentLine, 0, textFileReader.usrString)) == -2) /* Keyword is definitely missing. */
			return (_setError(errorText, "Missing keyword(Line=%d).", lineNum), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
		else if (curPos < 0) /* Blank line. */
			continue;

		_rsslAtoi64(textFileReader.usrString, &success, lineNum, 0);

		if (!success)
		{
			/* Must be an acronym, so still working on fids. */
			RsslReferenceFidStore *pTmpFid;

			/* If we were working on a value table it's finished */
			if (pEnumTypes)
			{
				if (_addTableToDictionary(dictionary, fidsCount, pFids, maxValue, pEnumTypes, errorText, lineNum) != RSSL_RET_SUCCESS)
					return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader);

				maxValue = 0; fidsCount = 0;
				_freeLists(pFids, pEnumTypes, RSSL_TRUE); pFids = 0; pEnumTypes = 0;
			}
			pTmpFid = pFids;
			pFids = (RsslReferenceFidStore*)calloc(1, sizeof(RsslReferenceFidStore));
			if (!pFids)
				return (_setError(errorText, "Unable to create storage for field table."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
			pFids->next = pTmpFid;
			++fidsCount;

			if (_rsslCreateStringCopyFromChar(&pFids->acronym, textFileReader.usrString) < RSSL_RET_SUCCESS)
				return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader);

			if ((curPos = getCopyUntilSpace(textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
				return (_setError(errorText, "Missing FID(Line=%d).", lineNum), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));

			pFids->fid = atoi(textFileReader.usrString);
			continue;
		}
		else
		{
			/* Working on values */
			RsslEnumTypeStore *pTmpEnumType = pEnumTypes;
			pEnumTypes = (RsslEnumTypeStore*)calloc(1, sizeof(RsslEnumTypeStore));
			if (!pEnumTypes)
				return (_setError(errorText, "Unable to create storage for enumeration table."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
			pEnumTypes->next = pTmpEnumType;
		}

		/* Parsing Enum Values */

		/* Since most value lists are likely to be 1) short, 2) fairly contiguous, and 3) on the low end
		* Figure out the max value and then create an appproprately-sized table. */

		pEnumTypes->enumType.value = (RsslEnum)atoi(textFileReader.usrString);
		if (pEnumTypes->enumType.value > maxValue) maxValue = pEnumTypes->enumType.value;

		if ((curPos = getCopyQuotedOrSharpedStr(textFileReader.currentLine, curPos, textFileReader.usrString, &textIsHex)) < 0)
		{
			_setError(errorText, "Missing DISPLAY(Line=%d).", lineNum);
			return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader);
		}

		if (textIsHex) /* Special character -- store as binary */
		{
			RsslUInt32 pos;
			RsslUInt32 length = (rtrUInt32)strlen(textFileReader.usrString);

			if (length & 0x1) /* Make sure it's even */
				return (_setError(errorText, "Odd-length hexadecimal input(Line=%d).", lineNum), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));

			pEnumTypes->enumType.display.length = (rtrUInt32)(strlen(textFileReader.usrString) / 2);
			pEnumTypes->enumType.display.data = (char*)malloc(pEnumTypes->enumType.display.length + 1);
			if (!pEnumTypes->enumType.display.data)
				return (_setError(errorText, "Unable to create storage for enumeration display table."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
			pEnumTypes->enumType.display.data[pEnumTypes->enumType.display.length] = '\0'; /* May as well null-terminate it. */ 

			for (pos = 0; pos < pEnumTypes->enumType.display.length; ++pos)
			{
				/* Translate two digits into a byte. */
				char hex[] = { textFileReader.usrString[pos*2], textFileReader.usrString[pos*2+1], '\0' }; 
				int byte;

				if (!isxdigit(hex[0]) || !isxdigit(hex[1]))
					return (_setError(errorText, "Invalid hexadecimal input(Line=%d).", lineNum), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));

				sscanf(hex, "%x", &byte);
				((unsigned char*)pEnumTypes->enumType.display.data)[pos] = (unsigned char)byte;
			}
		}
		else
		{
			pEnumTypes->enumType.display.length = (rtrUInt32)strlen(textFileReader.usrString);
			pEnumTypes->enumType.display.data = (char*)malloc(pEnumTypes->enumType.display.length + 1);
			if (!pEnumTypes->enumType.display.data)
				return (_setError(errorText, "Unable to create storage for enumeration display table."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
			strncpy(pEnumTypes->enumType.display.data, textFileReader.usrString, (pEnumTypes->enumType.display.length + 1));
		}

		if ((curPos = getRestOfLine(textFileReader.currentLine, curPos, textFileReader.usrString)) < 0)
		{
			/* No meaning present. Believe it's optional. */
			pEnumTypes->enumType.meaning.length = 0;
			pEnumTypes->enumType.meaning.data = 0;
		}
		else
		{
			pEnumTypes->enumType.meaning.length = (rtrUInt32)strlen(textFileReader.usrString);
			pEnumTypes->enumType.meaning.data = (char*)malloc(pEnumTypes->enumType.meaning.length + 1);
			if (!pEnumTypes->enumType.meaning.data)
				return (_setError(errorText, "Unable to create storage for enumeration meaning table."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader));
			strncpy(pEnumTypes->enumType.meaning.data, textFileReader.usrString, (pEnumTypes->enumType.meaning.length + 1));
		}
	}

	if (ret < 0)
		return ret;

	/* Finish last table */
	if (!pEnumTypes)
		return (_setError(errorText, "No EnumTable found(Line=%d)", lineNum), _finishEnumLoadFailure(0, dictionary, pFids, 0, &textFileReader));

	if (_addTableToDictionary(dictionary, fidsCount, pFids, maxValue, pEnumTypes, errorText, -1) != RSSL_RET_SUCCESS)
		return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypes, &textFileReader);

	_freeLists(pFids, pEnumTypes, RSSL_TRUE); pFids = 0; pEnumTypes = 0;
	maxValue = 0; fidsCount = 0;

	fclose(fp);
	textFileReaderCleanup(&textFileReader);
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslDecodeEnumTypeDictionary(
	RsslDecodeIterator	*dIter,
	RsslDataDictionary	*dictionary,
	RDMDictionaryVerbosityValues verbosity,
	RsslBuffer			*errorText )
{
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslElementList		elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslLocalElementSetDefDb elListSetDb;
	RsslRet ret;
	RsslSeriesEntry row = RSSL_INIT_SERIES_ENTRY;
	RsslArray arr; RsslBuffer arrEntry;
	
	RsslReferenceFidStore *pFids = 0, *pTmpFid;
	RsslEnumTypeStore *pEnumTypesHead = 0, *pCurEnumType;
	RsslEnum maxValue = 0;
	RsslUInt32 fidRefs = 0;
	char setDefMemory[LOCAL_EL_DEFS_TLS_SIZE];

	if (dictionary == 0)
	{
		_setError(errorText, "NULL dictionary pointer.");
		return RSSL_RET_FAILURE;
	}

	if (!dictionary->isInitialized && _initDictionary(dictionary, errorText) != RSSL_RET_SUCCESS)
		return RSSL_RET_FAILURE;

	if ((ret = rsslDecodeSeries(dIter, &series)) < 0)
		return (_setError(errorText, "rsslDecodeSeries failed %d",ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

	/* if this is not an element list, we should fail for now */
	if (series.containerType != RSSL_DT_ELEMENT_LIST)
		return (_setError(errorText, "Invalid container type of %d; expecting %d (RSSL_DT_ELEMENT_LIST)", series.containerType, RSSL_DT_ELEMENT_LIST), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

	/* decode summary data */
	if (rsslSeriesCheckHasSummaryData(&series))
	{
		if ((ret = rsslDecodeElementList(dIter, &elementList, NULL)) < 0)
			return (_setError(errorText, "rsslDecodeElementList failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

		while ((ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if ( ret < 0)
				return (_setError(errorText, "rsslDecodeElementEntry failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

			if (_decodeDictionaryTag(dIter, &element, RDM_DICTIONARY_ENUM_TABLES, dictionary, errorText) != RSSL_RET_SUCCESS)
				return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL);
		}
	}

	if (rsslSeriesCheckHasSetDefs(&series))
	{
		rsslClearLocalElementSetDefDb(&elListSetDb);
		elListSetDb.entries.data = setDefMemory;
		elListSetDb.entries.length = sizeof(setDefMemory);
		if ((ret = rsslDecodeLocalElementSetDefDb(dIter, &elListSetDb)) < 0)
    		return (_setError(errorText, "rsslDecodeLocalElementSetDefDb failed - %d", ret), RSSL_RET_FAILURE);
	}

	while( (ret = rsslDecodeSeriesEntry(dIter, &row)) != RSSL_RET_END_OF_CONTAINER)
	{
		RsslBool haveEnumValues = RSSL_FALSE, haveEnumDisplays = RSSL_FALSE;

		if ( ret < 0)
			 return (_setError(errorText, "rsslDecodeSeriesEntry failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

		/* decode element list here */
		if ((ret = rsslDecodeElementList(dIter, &elementList, &elListSetDb)) < 0)
			return (_setError(errorText, "rsslDecodeElementList failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

		while ( (ret = rsslDecodeElementEntry(dIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret < 0)
				return (_setError(errorText, "rsslDecodeElementEntry failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

			/* Can't make any assumptions about the order of elements
			 * (even though at this time Infra send them in this order) */
			if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ENUM_FIDS) || rsslBufferIsEqual(&element.name, &RSSL_ENAME_ENUM_FID))
			{
				if (element.dataType != RSSL_DT_ARRAY)
					return (_setError(errorText, "'%s' element has wrong data type.", RSSL_ENAME_ENUM_FIDS.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if ((ret = rsslDecodeArray(dIter, &arr)) < 0)
					return (_setError(errorText, "rsslDecodeArray failed - %d", ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if (arr.primitiveType != RSSL_DT_INT)
					return (_setError(errorText, "'%s' array has wrong primitive type.", RSSL_ENAME_ENUM_FIDS.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					RsslInt fidInt;
					if (ret < 0 || (ret = rsslDecodeInt(dIter, &fidInt)) < 0)
						return (_setError(errorText, "Error while decoding '%s' array - %d", RSSL_ENAME_ENUM_FIDS.data, ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
					
					pTmpFid = pFids;
					pFids = (RsslReferenceFidStore*)malloc(sizeof(RsslReferenceFidStore));
					if (!pFids)
						return (_setError(errorText, "Unable to create storage for field information"), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
					pFids->next = pTmpFid;
					rsslClearBuffer(&pFids->acronym);
					pFids->fid = (RsslInt16)fidInt;
					++fidRefs;
				}
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ENUM_VALUE) ||
                rsslBufferIsEqual(&element.name, &RSSL_ENAME_VALUES) )
			{
				if (haveEnumValues)
					return (_setError(errorText, "Duplicate '%s' element.", RSSL_ENAME_ENUM_VALUE.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if (element.dataType != RSSL_DT_ARRAY)
					return (_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_ENUM_VALUE.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if (rsslDecodeArray(dIter, &arr) < 0)
					return (_setError(errorText, "Cannot decode '%s' array.", RSSL_ENAME_ENUM_VALUE.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if (arr.primitiveType != RSSL_DT_ENUM)
					return (_setError(errorText, "'%s' array has wrong primtive type.", RSSL_ENAME_ENUM_VALUE.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				pCurEnumType = pEnumTypesHead; /* Move to start of list, if it exists */
				while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					RsslEnum valueEnum;
					if (ret < 0 || (ret = rsslDecodeEnum(dIter, &valueEnum)) < 0)
    					return (_setError(errorText, "Error while decoding '%s' array - %d", RSSL_ENAME_ENUM_VALUE.data, ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

					if (haveEnumDisplays)
					{
						/* Found the display values first, so go down the list filling up the entries */

						if (!pCurEnumType)
							return (_setError(errorText, "Different number of display and value elements."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

						pCurEnumType->enumType.value = valueEnum;
						pCurEnumType = pCurEnumType->next;
					}
					else
					{
						RsslEnumTypeStore *pTmpEnumType = pCurEnumType;

						pCurEnumType = (RsslEnumTypeStore*)calloc(1, sizeof(RsslEnumTypeStore));
						if (!pCurEnumType)
							return (_setError(errorText, "Unable to create storage for current enumeration."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
						pCurEnumType->enumType.value = valueEnum;

						/* Set head if first element; otherwise add new element to end of list */
						if (pEnumTypesHead)
							pTmpEnumType->next = pCurEnumType;
						else
							pEnumTypesHead = pCurEnumType;
					}

					if (valueEnum > maxValue)
						maxValue = valueEnum;
				}

				/* Make sure we didn't have more display elements than values */
				if (haveEnumDisplays && pCurEnumType)
					return (_setError(errorText, "Different number of display and value elements."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				haveEnumValues = RSSL_TRUE;
			}
			else if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_ENUM_DISPLAY) ||
				rsslBufferIsEqual(&element.name, &RSSL_ENAME_DISPLAYS))
			{
				if (element.dataType != RSSL_DT_ARRAY)
					return (_setError(errorText, "Cannot decode '%s' element.", RSSL_ENAME_ENUM_DISPLAY.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if ((ret = rsslDecodeArray(dIter, &arr)) < 0)
					return (_setError(errorText, "Cannot decode '%s' array - %d", RSSL_ENAME_ENUM_DISPLAY.data, ret),  _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				if (arr.primitiveType != RSSL_DT_ASCII_STRING && arr.primitiveType != RSSL_DT_RMTES_STRING && arr.primitiveType != RSSL_DT_UTF8_STRING)
					return (_setError(errorText, "'%s' array has wrong primtive type.", RSSL_ENAME_ENUM_DISPLAY.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				pCurEnumType = pEnumTypesHead; /* Move to start of list, if it exists */
				while((ret = rsslDecodeArrayEntry(dIter, &arrEntry)) != RSSL_RET_END_OF_CONTAINER)
				{
					if (ret < 0)
    					return (_setError(errorText, "Error while decoding '%s' array - %d", RSSL_ENAME_ENUM_DISPLAY.data, ret), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

					if (haveEnumValues)
					{
						/* Found the enum values first, so go down the list filling up the entries */
						if (!pCurEnumType)
							return (_setError(errorText, "Different number of display and value elements."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

						pCurEnumType->enumType.display.length = arrEntry.length;
						pCurEnumType->enumType.display.data = (char*)malloc(arrEntry.length);
						if (!pCurEnumType->enumType.display.data)
							return (_setError(errorText, "Unable to create memory for enumeration display information."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
						strncpy(pCurEnumType->enumType.display.data, arrEntry.data, arrEntry.length);

						pCurEnumType = pCurEnumType->next;
					}
					else
					{
						RsslEnumTypeStore *pTmpEnumType = pCurEnumType;
						pCurEnumType = (RsslEnumTypeStore*)calloc(1, sizeof(RsslEnumTypeStore));
						if (!pCurEnumType)
							return (_setError(errorText, "Unable to create memory for enumeration display information."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
						pCurEnumType->enumType.display.length = arrEntry.length;
						pCurEnumType->enumType.display.data = (char*)malloc(arrEntry.length);
						if (!pCurEnumType->enumType.display.data)
							return (_setError(errorText, "Unable to create memory for enumeration display information."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));
						strncpy(pCurEnumType->enumType.display.data, arrEntry.data, arrEntry.length);

						/* Set head if first element; otherwise add new element to end of list */
						if (pEnumTypesHead) pTmpEnumType->next = pCurEnumType;
						else pEnumTypesHead = pCurEnumType;
					}
				}

				/* Make sure we didn't have more value elements than displays */
				if (haveEnumValues && pCurEnumType)
					return (_setError(errorText, "Different number of display and value elements."), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL));

				haveEnumDisplays = RSSL_TRUE;
			}
		}

		if (!haveEnumValues)
			return(_setError(errorText, "\"%s\" element not found", RSSL_ENAME_ENUM_VALUE.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL)); 

		if (!haveEnumDisplays)
			return(_setError(errorText, "\"%s\" element not found", RSSL_ENAME_ENUM_DISPLAY.data), _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL)); 

		if (_addTableToDictionary(dictionary, fidRefs, pFids, maxValue, pEnumTypesHead, errorText, -1) != RSSL_RET_SUCCESS)
			return _finishEnumLoadFailure(0, dictionary, pFids, pEnumTypesHead, NULL);

		_freeLists(pFids, pEnumTypesHead, RSSL_TRUE); pFids = 0; pEnumTypesHead = 0; maxValue = 0; fidRefs = 0;
	}

	return RSSL_RET_SUCCESS;
}

static RsslDataTypes getStringType(RsslBuffer *buffer) {

	rtrUInt32 i = 0;
	RsslRet type = RSSL_DT_ASCII_STRING;

	while (i < buffer->length) {
		if (((buffer->data[i] & 0xFF) & ONE_BYTE_START_MASK) == ONE_BYTE_START) {
			i++;
		}
		else if (((buffer->data[i] & 0xFF) & TWO_BYTE_FIRST_MASK) == TWO_BYTE_FIRST_START) {
			if ((i + 1 < buffer->length) && ((buffer->data[i + 1] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START) {
				type = RSSL_DT_UTF8_STRING;
			}
			else {
				return RSSL_DT_RMTES_STRING;
			}
			i += 2;
		}
		else if (((buffer->data[i] & 0xFF) & THREE_BYTE_FIRST_MASK) == THREE_BYTE_FIRST_START) {
			if ((i + 2 < buffer->length) && ((buffer->data[i + 1] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START
				&& ((buffer->data[i + 2] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START) {
				type = RSSL_DT_UTF8_STRING;
			}
			else {
				return RSSL_DT_RMTES_STRING;
			}
			i += 3;
		}
		else if (((buffer->data[i] & 0xFF) & FOUR_BYTE_FIRST_MASK) == FOUR_BYTE_FIRST_START) {
			if ((i + 3 < buffer->length) && ((buffer->data[i + 1] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START
				&& ((buffer->data[i + 2] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START
				&& ((buffer->data[i + 3] & 0xFF) & MULTIBYTE_NEXT_MASK) == MULTIBYTE_NEXT_START) {
				type = RSSL_DT_UTF8_STRING;
			}
			else {
				return RSSL_DT_RMTES_STRING;
			}
			i += 4;
		}
		else {
			return RSSL_DT_RMTES_STRING;
		}
	}

	return type;
}

static RsslDataTypes getDisplayPrimitiveType(RsslEnumTypeTable *pTable) {

	RsslDataTypes dataType = RSSL_DT_ASCII_STRING;
	RsslDataTypes currType;
	int i;
	for (i = 0; i <= pTable->maxValue; i++) {
		if (pTable->enumTypes[i] != NULL) {
			currType = getStringType(&pTable->enumTypes[i]->display);
			if (currType == RSSL_DT_RMTES_STRING) {
				return RSSL_DT_RMTES_STRING;
			}
			else if (currType == RSSL_DT_UTF8_STRING) {
				dataType = currType;
			}
		}
	}

	return dataType;
}

RSSL_API RsslRet rsslEncodeEnumTypeDictionaryAsMultiPart(
	RsslEncodeIterator	*eIter,
	RsslDataDictionary	*dictionary,
	int								*currentFid,
	RDMDictionaryVerbosityValues verbosity,
	RsslBuffer			*errorText )
{
	RsslRet				ret;
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslLocalElementSetDefDb	setDb;

	unsigned int curFid = 0;

	if (*currentFid < 0)
		return (_setError(errorText, "currentFid cann't be negative"), RSSL_RET_FAILURE);

	curFid = *currentFid;

	if (!dictionary->isInitialized)
		return (_setError(errorText, "Dictionary not initialized"), RSSL_RET_FAILURE);

	rsslClearLocalElementSetDefDb(&setDb);
	setDb.definitions[0] = enumSetDef0_Normal;

		/* Set the data format */
	series.containerType = RSSL_DT_ELEMENT_LIST;
	if (!curFid)
	{
		rsslSeriesApplyHasSummaryData(&series);
	}

	/* Don't encode set definitions for info */
	if (verbosity > RDM_DICTIONARY_INFO)
		rsslSeriesApplyHasSetDefs(&series);

	/* If first packet, then send hint and summary data */

	if ((ret = rsslEncodeSeriesInit(eIter, &series, 0, 0)) < 0)
		return (_setError(errorText, "rsslEncodeSeriesInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

	if (verbosity > RDM_DICTIONARY_INFO)
	{
		/* Encode set definition */
		if ((ret = rsslEncodeLocalElementSetDefDb(eIter, &setDb)) < 0)
			return (_setError(errorText, "rsslEncodeLocalElementSetDefDb failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

		if ((ret = rsslEncodeSeriesSetDefsComplete(eIter, RSSL_TRUE) ) < 0)
			return (_setError(errorText, "rsslEncodeSeriesSetDefsComplete failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
	}

	/* Summary data */
	if (!curFid)
	if ((ret = rsslEncodeDataDictSummaryData(eIter,dictionary, RDM_DICTIONARY_ENUM_TABLES, errorText)) < 0)
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

	/* Don't encode actual entries for info */
	if (verbosity > RDM_DICTIONARY_INFO)
	{
		/* Need to keep track of the number of the series entry we are encoding, if it is the first series entry
			in the message and we get the RSSL_RET_BUFFER_TOO_SMALL we can not encode partial series entry and 
			we need to fail */
		unsigned int startCount = curFid;

		for(; curFid < dictionary->enumTableCount; ++curFid)
		{
			/* Encode each table */
			RsslSeriesEntry seriesEntry;
			RsslElementList elemList; RsslElementEntry elemEntry;
			RsslEnumTypeTable *pTable;
			RsslArray arr;
			RsslUInt32 j;

			*currentFid = curFid;

			rsslClearSeriesEntry(&seriesEntry);

			if ((ret = rsslEncodeSeriesEntryInit(eIter, &seriesEntry, 0)) == RSSL_RET_BUFFER_TOO_SMALL && 
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionarySeriesEntry(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionarySeriesEntry failed %d",ret), RSSL_RET_FAILURE);
				else 
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeSeriesEntryInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			rsslClearElementList(&elemList);
			elemList.flags = RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID;
			elemList.setId = 0;

			if ((ret = rsslEncodeElementListInit(eIter, &elemList, &setDb, 0)) == RSSL_RET_BUFFER_TOO_SMALL &&
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryElementList(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryElementList failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeElementListInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			pTable = dictionary->enumTables[curFid];

			/* Fids */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_FIDS;

			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) == RSSL_RET_BUFFER_TOO_SMALL &&
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryElementEntry(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryElementEntry failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			rsslClearArray(&arr);
			arr.itemLength = 2;
			arr.primitiveType = RSSL_DT_INT;

			if ((ret = rsslEncodeArrayInit(eIter, &arr)) == RSSL_RET_BUFFER_TOO_SMALL && 
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			for (j = 0; j < pTable->fidReferenceCount; ++j)
			{
				RsslInt64 myInt = pTable->fidReferences[j];
				if ((ret = rsslEncodeArrayEntry(eIter, 0, &myInt)) == RSSL_RET_BUFFER_TOO_SMALL &&
					curFid > startCount)
				{
					if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
						return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
					else
						return RSSL_RET_DICT_PART_ENCODED;
				}
				else if (ret < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			/* Values */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_VALUE;

			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) == RSSL_RET_BUFFER_TOO_SMALL && 
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryElementEntry(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryElementEntry failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			
			rsslClearArray(&arr);
			arr.itemLength = 0;
			arr.primitiveType = RSSL_DT_ENUM;

			if ((ret = rsslEncodeArrayInit(eIter, &arr)) == RSSL_RET_BUFFER_TOO_SMALL && 
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			for (j = 0; j <= pTable->maxValue; ++j)
			{
				if ( pTable->enumTypes[j] && 
					(ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->value)) == RSSL_RET_BUFFER_TOO_SMALL && 
					curFid > startCount)
				{
					if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
						return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
					else
						return RSSL_RET_DICT_PART_ENCODED;
				}
				else if (ret < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			/* Display */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_DISPLAY;

			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) == RSSL_RET_BUFFER_TOO_SMALL && 
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryElementEntry(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryElementEntry failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			rsslClearArray(&arr);
			arr.itemLength = 0;
			arr.primitiveType = getDisplayPrimitiveType(pTable);

			if ((ret = rsslEncodeArrayInit(eIter, &arr)) == RSSL_RET_BUFFER_TOO_SMALL &&
				curFid > startCount)
			{
				if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
					return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
				else
					return RSSL_RET_DICT_PART_ENCODED;
			}
			else if (ret < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

			for (j = 0; j <= pTable->maxValue; ++j)
			{
				if ( pTable->enumTypes[j] && 
					(ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->display)) == RSSL_RET_BUFFER_TOO_SMALL &&
					curFid > startCount)
				{
					if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
						return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
					else
						return RSSL_RET_DICT_PART_ENCODED;
				}
				else if (ret < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			/* RDM Usage Guide does not define or typically recommend the meaning column, so leave it out for now. */
#ifdef MEANING
			/* Meaning */
			if (verbosity >= RSSL_DICTIONARY_VERBOSE)
			{
				rsslClearElementEntry(&elemEntry);
				elemEntry.dataType = RSSL_DT_ARRAY;
				elemEntry.name = RSSL_ENAME_MEANING;
				if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) == RSSL_RET_BUFFER_TOO_SMALL && 
					curFid > startCount)
				{
					if ((ret = _rollbackEnumDictionaryElementEntry(eIter)) < 0)
						return (_setError(errorText, "rollbackEnumDictionaryElementEntry failed %d",ret), RSSL_RET_FAILURE);
					else
						return RSSL_RET_DICT_PART_ENCODED;
				}
				else if (ret < 0)
					return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

				rsslClearArray(&arr);
				arr.itemLength = 0;
				arr.primitiveType = RSSL_DT_ASCII_STRING;
				if ((ret = rsslEncodeArrayInit(eIter, &arr)) == RSSL_RET_BUFFER_TOO_SMALL &&
					curFid > startCount)
				{
					if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
						return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
					else
						return RSSL_RET_DICT_PART_ENCODED;
				}
				else if (ret < 0)
					return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

				for (j = 0; j <= pTable->maxValue; ++j)
				{
					/* This 'could' result in putting many empty strings on the wire, but it's got to be all or nothing
					 * or they won't match up with the values. */
					if ( pTable->enumTypes[j] && 
						(ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->meaning)) == RSSL_RET_BUFFER_TOO_SMALL &&
						curFid > startCount)
					{
						if ((ret = _rollbackEnumDictionaryArray(eIter)) < 0)
							return (_setError(errorText, "rollbackEnumDictionaryArray failed %d",ret), RSSL_RET_FAILURE);
						else
							return RSSL_RET_DICT_PART_ENCODED;
					}
					else if (ret < 0)
						return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);
				}

				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
					return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
					return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			} 
#endif
			if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementListComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeSeriesEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeSeriesEntryComplete failed %d",ret), RSSL_RET_FAILURE);
		}
	}

	if ((ret = rsslEncodeSeriesComplete(eIter, RSSL_TRUE)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslEncodeEnumTypeDictionary(
	RsslEncodeIterator	*eIter,
	RsslDataDictionary	*dictionary,
	RDMDictionaryVerbosityValues verbosity,
	RsslBuffer			*errorText )
{
	RsslRet				ret;
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslLocalElementSetDefDb	setDb;

	if (!dictionary->isInitialized)
		return (_setError(errorText, "Dictionary not initialized"), RSSL_RET_FAILURE);

	rsslClearLocalElementSetDefDb(&setDb);
	setDb.definitions[0] = enumSetDef0_Normal;

	/* Set the data format */
	series.containerType = RSSL_DT_ELEMENT_LIST;
	series.flags = RSSL_SRF_HAS_SUMMARY_DATA;

	/* Don't encode set definitions for info */
	if (verbosity > RDM_DICTIONARY_INFO)
		rsslSeriesApplyHasSetDefs(&series);

	/* If first packet, then send hint and summary data */

	if ((ret = rsslEncodeSeriesInit(eIter, &series, 0, 0)) < 0)
		return (_setError(errorText, "rsslEncodeSeriesInit failed %d",ret), RSSL_RET_FAILURE);

	if (verbosity > RDM_DICTIONARY_INFO)
	{
		/* Encode set definition */
		if ((ret = rsslEncodeLocalElementSetDefDb(eIter, &setDb)) < 0)
			return (_setError(errorText, "rsslEncodeLocalElementSetDefDb failed %d",ret), RSSL_RET_FAILURE);

		if ((ret = rsslEncodeSeriesSetDefsComplete(eIter, RSSL_TRUE) ) < 0)
			return (_setError(errorText, "rsslEncodeSeriesSetDefsComplete failed %d",ret), RSSL_RET_FAILURE);
	}

	/* Summary data */
	if ((ret = rsslEncodeDataDictSummaryData(eIter,dictionary, RDM_DICTIONARY_ENUM_TABLES, errorText)) < 0)
		return (ret == RSSL_RET_BUFFER_TOO_SMALL ? ret : RSSL_RET_FAILURE);

	/* Don't encode actual entries for info */
	if (verbosity > RDM_DICTIONARY_INFO)
	{
		RsslUInt32 i;

		for(i = 0; i < dictionary->enumTableCount; ++i)
		{
			/* Encode each table */

			RsslSeriesEntry seriesEntry;
			RsslElementList elemList; RsslElementEntry elemEntry;
			RsslEnumTypeTable *pTable;
			RsslArray arr;
			RsslUInt32 j;

			rsslClearSeriesEntry(&seriesEntry);

			if ((ret = rsslEncodeSeriesEntryInit(eIter, &seriesEntry, 0)) < 0)
				return (_setError(errorText, "rsslEncodeSeriesEntryInit failed %d",ret), RSSL_RET_FAILURE);

			rsslClearElementList(&elemList);
			elemList.flags = RSSL_ELF_HAS_SET_DATA | RSSL_ELF_HAS_SET_ID;
			elemList.setId = 0;

			if ((ret = rsslEncodeElementListInit(eIter, &elemList, &setDb, 0)) < 0)
				return (_setError(errorText, "rsslEncodeElementListInit failed %d",ret), RSSL_RET_FAILURE);

			pTable = dictionary->enumTables[i];

			/* Fids */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_FIDS;
			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), RSSL_RET_FAILURE);

			rsslClearArray(&arr);
			arr.itemLength = 2;
			arr.primitiveType = RSSL_DT_INT;
			if ((ret = rsslEncodeArrayInit(eIter, &arr)) < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), RSSL_RET_FAILURE);

			for (j = 0; j < pTable->fidReferenceCount; ++j)
			{
				RsslInt64 myInt = pTable->fidReferences[j];
				if ((ret = rsslEncodeArrayEntry(eIter, 0, &myInt)) < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);


			/* Values */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_VALUE;
			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), RSSL_RET_FAILURE);

			rsslClearArray(&arr);
			arr.itemLength = 0;
			arr.primitiveType = RSSL_DT_ENUM;
			if ((ret = rsslEncodeArrayInit(eIter, &arr)) < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), RSSL_RET_FAILURE);

			for (j = 0; j <= pTable->maxValue; ++j)
			{
				if ( pTable->enumTypes[j] && (ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->value)) < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			/* Display */
			rsslClearElementEntry(&elemEntry);
			elemEntry.dataType = RSSL_DT_ARRAY;
			elemEntry.name = RSSL_ENAME_ENUM_DISPLAY;
			if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), RSSL_RET_FAILURE);

			rsslClearArray(&arr);
			arr.itemLength = 0;
			arr.primitiveType = getDisplayPrimitiveType(pTable);
			if ((ret = rsslEncodeArrayInit(eIter, &arr)) < 0)
				return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), RSSL_RET_FAILURE);

			for (j = 0; j <= pTable->maxValue; ++j)
			{
				if ( pTable->enumTypes[j] && (ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->display)) < 0)
					return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), RSSL_RET_FAILURE);
			}

			if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);

			/* RDM Usage Guide does not define or typically recommend the meaning column, so leave it out for now. */
#ifdef MEANING
			/* Meaning */
			if (verbosity >= RSSL_DICTIONARY_VERBOSE)
			{
				rsslClearElementEntry(&elemEntry);
				elemEntry.dataType = RSSL_DT_ARRAY;
				elemEntry.name = RSSL_ENAME_MEANING;
				if ((ret = rsslEncodeElementEntryInit(eIter, &elemEntry, 0)) < 0)
					return (_setError(errorText, "rsslEncodeElementEntryInit failed %d",ret), RSSL_RET_FAILURE);

				rsslClearArray(&arr);
				arr.itemLength = 0;
				arr.primitiveType = RSSL_DT_ASCII_STRING;
				if ((ret = rsslEncodeArrayInit(eIter, &arr)) < 0)
					return (_setError(errorText, "rsslEncodeArrayInit failed %d",ret), RSSL_RET_FAILURE);

				for (j = 0; j <= pTable->maxValue; ++j)
				{
					/* This 'could' result in putting many empty strings on the wire, but it's got to be all or nothing
					 * or they won't match up with the values. */
					if ( pTable->enumTypes[j] && (ret = rsslEncodeArrayEntry(eIter, 0, &pTable->enumTypes[j]->meaning)) < 0)
						return (_setError(errorText, "rsslEncodeArrayEntry failed %d",ret), RSSL_RET_FAILURE);
				}

				if ((ret = rsslEncodeArrayComplete(eIter, RSSL_TRUE)) < 0)
					return (_setError(errorText, "rsslEncodeArrayComplete failed %d",ret), RSSL_RET_FAILURE);

				if ((ret = rsslEncodeElementEntryComplete(eIter, RSSL_TRUE)) < 0)
					return (_setError(errorText, "rsslEncodeElementEntryComplete failed %d",ret), RSSL_RET_FAILURE);
			} 
#endif

			if ((ret = rsslEncodeElementListComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeElementListComplete failed %d",ret), RSSL_RET_FAILURE);

			if ((ret = rsslEncodeSeriesEntryComplete(eIter, RSSL_TRUE)) < 0)
				return (_setError(errorText, "rsslEncodeSeriesEntryComplete failed %d",ret), RSSL_RET_FAILURE);
		}

	}

	if ((ret = rsslEncodeSeriesComplete(eIter, 1)) < 0)
	{
		_setError(errorText, "rsslEncodeSeriesComplete failed %d",ret);
		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet rsslExtractDictionaryType(RsslDecodeIterator *dIter,
							  RDMDictionaryTypes *dictionaryType,
							  RsslBuffer *errorText )
{
	RsslDecodeIterator  tempDecIter;
	RsslSeries			series = RSSL_INIT_SERIES;
	RsslElementList		elementList = RSSL_INIT_ELEMENT_LIST;
	RsslElementEntry	element = RSSL_INIT_ELEMENT_ENTRY;
	RsslRet ret = 0;
	RsslUInt tempUInt = 0;
	RsslBuffer tempBuffer = RSSL_INIT_BUFFER;

	rsslClearDecodeIterator(&tempDecIter);

	rsslSetDecodeIteratorRWFVersion(&tempDecIter, dIter->_majorVersion, dIter->_minorVersion);
	tempBuffer.data = dIter->_curBufPtr;
	tempBuffer.length = (rtrUInt32)((dIter->_pBuffer->data + dIter->_pBuffer->length) - dIter->_curBufPtr);

	rsslSetDecodeIteratorBuffer(&tempDecIter, &tempBuffer);

	if ((ret = rsslDecodeSeries(&tempDecIter, &series)) < 0)
		return (_setError(errorText, "rsslDecodeSeries failed %d",ret), RSSL_RET_FAILURE);
		
	/* if this is not an element list, we should fail */
	if (series.containerType != RSSL_DT_ELEMENT_LIST)
		return (_setError(errorText, "Invalid container type of %d; expecting %d (RSSL_DT_ELEMENT_LIST)", series.containerType, RSSL_DT_ELEMENT_LIST), RSSL_RET_FAILURE);

	/* decode summary data */
	if (rsslSeriesCheckHasSummaryData(&series))
	{
		if ((ret = rsslDecodeElementList(&tempDecIter, &elementList, NULL)) < 0)
			return (_setError(errorText, "rsslDecodeElementList failed %d",ret), RSSL_RET_FAILURE);
			
		while ((ret = rsslDecodeElementEntry(&tempDecIter, &element)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (ret >= RSSL_RET_SUCCESS)
			{
				if (rsslBufferIsEqual(&element.name, &RSSL_ENAME_TYPE))
				{
					ret = rsslDecodeUInt(&tempDecIter, &tempUInt);
					if (ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA)
					{
						return (_setError(errorText, "rsslDecodeUInt failed %d",ret), RSSL_RET_FAILURE);
					}
					*dictionaryType = (RDMDictionaryTypes)tempUInt;
					break;
				}
			}
			else
			{
				return (_setError(errorText, "rsslDecodeElementEntry failed %d",ret), RSSL_RET_FAILURE);
			}
		}
	}
	else
	{
		return (_setError(errorText, "No summary data present on message!"), RSSL_RET_FAILURE);
	}

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslDictionaryEntry *rsslDictionaryGetEntryByFieldName(RsslDataDictionary *pDictionary, const RsslBuffer *pFieldName)
{
	FieldsByNameLink *pFieldsByNameLink;
	RsslHashLink *rsslHashLink = rsslHashTableFind(&((RsslDictionaryInternal*)pDictionary->_internal)->fieldsByName, (RsslBuffer*)pFieldName, NULL);
	
	if (rsslHashLink == NULL)
		return NULL;

	pFieldsByNameLink = RSSL_HASH_LINK_TO_OBJECT(FieldsByNameLink, nameTableLink, rsslHashLink);

	RSSL_ASSERT(pFieldsByNameLink->pDictionaryEntry != NULL, Link in fieldsByName table does not have an associated entry);
	return pFieldsByNameLink->pDictionaryEntry;
}
 
RSSL_API RsslRet rsslDictionaryEntryGetEnumValueByDisplayString(const RsslDictionaryEntry *pEntry, const RsslBuffer *pEnumDisplay, RsslEnum *pEnumValue, RsslBuffer *errorText)
{
	RsslUInt32 i;
	RsslEnumTypeTable *pEnumTypeTable = pEntry->pEnumTypeTable;

	if (pEnumTypeTable == NULL)
		return RSSL_RET_FAILURE;

	/* Find a value matching the given display string. */
	for (i = 0; i < pEnumTypeTable->maxValue; ++i)
	{
		if (pEnumTypeTable->enumTypes[i] == NULL)
			continue;

		if (rsslBufferIsEqual(pEnumDisplay, &pEnumTypeTable->enumTypes[i]->display))
		{
			/* If there are multiple values corresponding to the given display string, return an error.
			 * We cannot provide a correct value. */
			if (((RsslEnumTypeImpl*)pEnumTypeTable->enumTypes[i])->flags & RSSL_ENUMTYPE_FL_DUPLICATE_DISPLAY)
				return RSSL_RET_DICT_DUPLICATE_ENUM_VALUE;

			*pEnumValue = pEnumTypeTable->enumTypes[i]->value;
			return RSSL_RET_SUCCESS;
		}
	}

	return RSSL_RET_FAILURE;
}

RSSL_API RsslRet rsslLinkDataDictionary(RsslDataDictionary *pNewDictionary, RsslDataDictionary *pOldDictionary, RsslBuffer *errorText)
{
	RsslDictionaryInternal *pOldDictionaryInternal = (RsslDictionaryInternal*)pOldDictionary->_internal;
	RsslDictionaryInternal *pNewDictionaryInternal = (RsslDictionaryInternal*)pNewDictionary->_internal;
	int i;

	/* Check that the new dictionary is a superset of the old dictionary before linking them. */

	/* Check that major versions match. */
	if (pOldDictionary->infoField_Version.data != NULL && pNewDictionary->infoField_Version.data != NULL)
	{
		RsslUInt32 i;
		RsslBuffer oldVersion = pOldDictionary->infoField_Version;
		RsslBuffer newVersion = pNewDictionary->infoField_Version;

		/* Major version should be the text up until the first dot. Find it for both dictionaries. */
		for (i = 0; i < oldVersion.length; ++i)
		{
			if (oldVersion.data[i] == '.')
			{
				oldVersion.length = i + 1;
				break;
			}
		}

		for (i = 0; i < newVersion.length; ++i)
		{
			if (newVersion.data[i] == '.')
			{
				newVersion.length = i + 1;
				break;
			}
		}

		if (!rsslBufferIsEqual(&oldVersion, &newVersion))
			return (_setError(errorText, "Dictionary major versions do not match."), RSSL_RET_FAILURE);
	}

	/* Check that all fields match. */
	for (i = RSSL_MIN_FID; i <= RSSL_MAX_FID; ++i)
	{
		RsslDictionaryEntry *pOldEntry = pOldDictionary->entriesArray[i];
		RsslDictionaryEntry *pNewEntry = pNewDictionary->entriesArray[i];

		if (!pOldEntry) /* New dictionary can have new entries. */
			continue;
		else if (!pNewEntry && pOldEntry) /* New dictionary must have at least the same entries as the old dictionary. */
			return (_setError(errorText, "New dictionary has no entry for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);
			
		/* Check acronym. */
		if (!rsslBufferIsEqual(&pOldEntry->acronym, &pNewEntry->acronym))
			return (_setError(errorText, "Acronym mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check ddeAcronym. */
		if (!rsslBufferIsEqual(&pOldEntry->ddeAcronym, &pNewEntry->ddeAcronym))
			return (_setError(errorText, "DDEAcronym mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check rippleToField. */
		if (pOldEntry->rippleToField != pNewEntry->rippleToField)
			return (_setError(errorText, "RippleToFid mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check fieldType. */
		if (pOldEntry->fieldType != pNewEntry->fieldType)
			return (_setError(errorText, "Marketfeed FieldType mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check length. */
		if (pOldEntry->length != pNewEntry->length)
			return (_setError(errorText, "Marketfeed length mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check enumLength. */
		if (pOldEntry->enumLength != pNewEntry->enumLength)
			return (_setError(errorText, "Enumerated length mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check rwfType. */
		if (pOldEntry->rwfType != pNewEntry->rwfType)
			return (_setError(errorText, "RWFType mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);

		/* Check rwfLength. */
		if (pOldEntry->rwfLength != pNewEntry->rwfLength)
			return (_setError(errorText, "RWFLength mismatch for field with FID %d.", pOldEntry->fid), RSSL_RET_FAILURE);
	}

	/* Go over enum tables that were in the old dictionary. Make sure the all fields that had a reference to it still have them in the
	 * new dictionary. */
	for(i = 0; i < pOldDictionary->enumTableCount; ++i)
	{
		RsslEnumTypeTable *pOldTable = pOldDictionary->enumTables[i];
		rtrUInt32 j;

		/* Take one of the FIDs that was using this table in the old dictionary, and use it to find the corresponding table in the new one. */
		RsslEnumTypeTable *pNewTable = pNewDictionary->entriesArray[pOldTable->fidReferences[0]]->pEnumTypeTable;

		if (pNewTable == NULL)
			return (_setError(errorText, "New dictionary missing EnumType table for field with FID %d", pOldTable->fidReferences[0]), RSSL_RET_FAILURE);

		/* For each FID reference in the old table, find the same FID reference in the new table. */
		for (j = 0; j < pOldTable->fidReferenceCount; ++j)
		{
			rtrUInt32 k;

			for (k = 0; k < pNewTable->fidReferenceCount; ++k)
				if (pOldTable->fidReferences[j] == pNewTable->fidReferences[k])
					break;

			if (k == pNewTable->fidReferenceCount)
				return (_setError(errorText, "New dictionary enum table removed reference from FID %d.", pOldTable->fidReferences[j]), RSSL_RET_FAILURE);
		}

		if (pNewTable->maxValue < pOldTable->maxValue)
				return (_setError(errorText, "New enum table (referenced by FID %d) has fewer values than existing table.", pNewTable->fidReferences[0]), RSSL_RET_FAILURE);

		/* Check that new enum table is a superset of the existing one. */
		for (j = 0; j <= pOldTable->maxValue; ++j)
		{
			if (!pOldTable->enumTypes[j])
				continue;

			/* Value should appear in new table. */
			if (!pNewTable->enumTypes[j])
				return (_setError(errorText, "New enum table (referenced by FID %d) removed value %u.", 
							pNewTable->fidReferences[0], pNewTable->enumTypes[j]->value), RSSL_RET_FAILURE);

			/* Display string in new table should match the old one. */
			if (!rsslBufferIsEqual(&pNewTable->enumTypes[j]->display, &pOldTable->enumTypes[j]->display))
				return (_setError(errorText, "New enum table (referenced by FID %d) has different display string for value %u.", 
							pNewTable->fidReferences[0], pNewTable->enumTypes[j]->value), RSSL_RET_FAILURE);
		}

	}

	/* At this point, we can safely link the entries and enum tables in the new and old dictionaries. */

	/* Take the enum tables that exist in the old dictionary, and use them in the new dictionary.
	 * The old one is used to make sure we don't delete it prematurely while in use.
	 * Update all fields in the new table so they point to the table in the old dictionary.
	 * Swap the old table into this dictionary and free the new one. */
	for(i = 0; i < pOldDictionary->enumTableCount; ++i)
	{
		RsslEnumTypeTable *pOldTable = pOldDictionary->enumTables[i];
		RsslEnumTypeTable *pNewTable = pNewDictionary->entriesArray[pOldTable->fidReferences[0]]->pEnumTypeTable;
		rtrUInt32 j;

		for (j = 0; j < pNewTable->fidReferenceCount; ++j)
		{
			RsslDictionaryEntry *pNewEntry = pNewDictionary->entriesArray[pNewTable->fidReferences[j]];
			pNewEntry->pEnumTypeTable = pOldTable;
		}

		/* Find where the new table exists in the new dictionary. Swap the old one in, then free the new one. */
		for (j = 0; j < pNewDictionary->enumTableCount; ++j)
		{
			if (pNewDictionary->enumTables[j] == pNewTable)
			{
				pNewDictionary->enumTables[j] = pOldTable;
				_deleteDictionaryEnumTable(pNewTable);
				break;
			}
		}

		if (j == pNewDictionary->enumTableCount)
			return (_setError(errorText, "Internal error: New dictionary missing expected enum table (first FID reference in old dictionary is %d).",
					   pOldTable->fidReferences[0]), RSSL_RET_FAILURE);
	}

	for (i = RSSL_MIN_FID; i <= RSSL_MAX_FID; ++i)
	{
		RsslDictionaryEntry *pOldEntry = pOldDictionary->entriesArray[i];
		RsslHashLink *rsslHashLink;

		/* For entries that exist in the old dictionary, use them in the new dictionary. */
		if (pOldEntry)
		{
			/* Update the entry key of FieldsByNameLink.nameTableLink as the existing key 
			   in pNewDictionary will be replaced with the entry from pOldDictionary */
			if ((rsslHashLink = rsslHashTableFind(&pNewDictionaryInternal->fieldsByName, &pOldEntry->acronym, NULL)) != NULL)
			{
				rsslHashLink->pKey = &pOldEntry->acronym;
			}

			_deleteDictionaryEntry(pNewDictionary, (RsslFieldId)i); /* Cleanup the new entry. */
			pNewDictionary->entriesArray[i] = pOldEntry;
			pNewDictionaryInternal->fieldsByNameLinks[i - (RSSL_MIN_FID)].pDictionaryEntry = pOldEntry;
		}
	}

	/* Mark the old dictionary as linked, so that the entries aren't cleaned up. */
	pOldDictionaryInternal->isLinked = RSSL_TRUE;

	return RSSL_RET_SUCCESS;
}

#ifdef __cplusplus
}
#endif
