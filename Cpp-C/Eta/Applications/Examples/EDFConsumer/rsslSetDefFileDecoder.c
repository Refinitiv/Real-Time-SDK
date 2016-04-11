/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

/*
 * This loads the XML file set definitions into a local message set definitions
 * database for the EDF example application.
 */

#include <stdio.h>
#include "rsslSetDefFileDecoder.h"
#include "rtr/rsslDataUtils.h"

static void _saxStartElement(void *pData, const xmlChar *xmlName, const xmlChar **attrList);
static void _saxEndElement(void *pData, const xmlChar *name);

RsslFieldSetDefDb globalFieldSetDefDb;

// memory to back RsslFieldSetDefEntry's in RsslLocalFieldSetDefDb
static RsslBool isParsingComplete;
static int currentSetId, entryCount;

RsslBool decodeSetDefFile(char* setDefFilename)
{
	int ret;
	xmlSAXHandler saxHandler;
	RsslBuffer version;

	rsslClearBuffer(&version);
	rsslClearFieldSetDb(&globalFieldSetDefDb);
	if ((ret = rsslAllocateFieldSetDefDb(&globalFieldSetDefDb, &version)) != RSSL_RET_SUCCESS)
	{
		printf("rsslAllocateFieldSetDefDb() failed: %d (%s)\n", ret, rsslRetCodeToString(ret));
		return RSSL_FALSE;
	}

	// parse file
	memset(&saxHandler, 0, sizeof(saxHandler));
	saxHandler.startElement = _saxStartElement;
	saxHandler.endElement = _saxEndElement;
	if (xmlSAXUserParseFile(&saxHandler, &globalFieldSetDefDb, setDefFilename) < 0)
	{
		printf("xmlSAXUserParseFile() failed.\n");
		free (globalFieldSetDefDb.definitions);
		globalFieldSetDefDb.isInitialized = RSSL_FALSE;
		return RSSL_FALSE;
	}
	else if	(isParsingComplete != RSSL_TRUE)
	{
		printf("xmlSAXUserParseFile() returned without complete parsing.\n");
		free (globalFieldSetDefDb.definitions);
		globalFieldSetDefDb.isInitialized = RSSL_FALSE;
		return RSSL_FALSE;
	}
	return RSSL_TRUE;
}

static void _saxStartElement(void *pData, const xmlChar *xmlName, const xmlChar **attrList)
{
	//RsslLocalFieldSetDefDb *localFieldSetDefDb = (RsslLocalFieldSetDefDb *)pData;
	RsslFieldSetDefDb *fieldSetDefDb = (RsslFieldSetDefDb *)pData;

	char *name = (char*)xmlName;

	if (0 == strcmp(name, "fieldSetDefDb"))
	{
		isParsingComplete = RSSL_FALSE;
	}
	else if (0 == strcmp(name, "fieldSetDef"))
	{
		RsslFieldSetDef *setDef;

		setDef = (RsslFieldSetDef*)malloc(sizeof(RsslFieldSetDef));

		// get currentSetId
		if (0 == strcmp((char*)attrList[0], "setId"))
		{
			currentSetId = atoi((char*)attrList[1]);
			if (currentSetId <= RSSL_FIELD_SET_MAX_LOCAL_ID || currentSetId > 0xFFFF)
			{
				printf("Out or range SetId, range is 16 to 65535\n");
				exit(-1);
			}
		}
		else
		{
			printf("Unable to find setId attribute\n");
			exit(-1);
		}

		// set the setId in the FieldSetDefDb
		setDef->setId = currentSetId;
		fieldSetDefDb->definitions[setDef->setId] = setDef;
		if (setDef->setId > fieldSetDefDb->maxSetId)
			fieldSetDefDb->maxSetId = setDef->setId;

		// reset entryCount
		entryCount = 0;
	}
	else if (0 == strcmp(name, "fieldSetDefEntry"))
	{
		RsslFieldSetDef *setDef = fieldSetDefDb->definitions[currentSetId];

		if (entryCount == 0)
		{
			setDef->pEntries = (RsslFieldSetDefEntry*)malloc(sizeof(RsslFieldSetDefEntry)*255); 
		}

		// get fieldId
		if (0 == strcmp((char*)attrList[0], "fieldId"))
		{
			setDef->pEntries[entryCount].fieldId = atoi((char*)attrList[1]);
		}
		else
		{
			printf("Unable to find fieldId attribute\n");
			exit(-1);
		}

		// get dataTypeNum
		if (0 == strcmp((char*)attrList[2], "dataTypeNum"))
		{
			setDef->pEntries[entryCount].dataType = atoi((char*)attrList[3]);
		}
		else
		{
			printf("Unable to find dataTypeNum attribute\n");
			exit(-1);
		}

		// increment entryCount
		entryCount++;
		fieldSetDefDb->definitions[currentSetId]->count = entryCount;
	}
}

static void _saxEndElement(void *pData, const xmlChar *name)
{
	if (0 == strcmp((char*)name, "fieldSetDefDb"))
	{
		isParsingComplete = RSSL_TRUE;
	}
}
