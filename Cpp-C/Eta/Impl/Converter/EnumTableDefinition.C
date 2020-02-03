/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/EnumTableDefinition.h"
#include "rtr/rsslDataDictionary.h"
#include "rtr/jsonToRwfSimple.h"

EnumTableDefinition::EnumTableDefinition(jsonToRwfSimple* jonToRwfSimple, RsslUInt16 maxCount) :
	_referenceCount(0),
	_pJsonToRwfSimple(jonToRwfSimple),
	_initializedHashTable(false),
	_maxCount(maxCount)
{
}

EnumTableDefinition::~EnumTableDefinition()
{
	if (_initializedHashTable = true)
	{
		_initializedHashTable = false;

		if (_enumByDispalyValue.elementCount > 0)
		{
			EnumDefinition* pEnumDefinition;
			RsslUInt32 index;
			RsslQueueLink *pLink;
			for (index = 0; index < _enumByDispalyValue.queueCount; index++)
			{
				RSSL_QUEUE_FOR_EACH_LINK(&_enumByDispalyValue.queueList[index], pLink)
				{
					pEnumDefinition = RSSL_QUEUE_LINK_TO_OBJECT(EnumDefinition, displayValueLink, pLink);

					if (pEnumDefinition)
					{
						if (pEnumDefinition->enumDisplay.data)
						{
							free(pEnumDefinition->enumDisplay.data);
							pEnumDefinition->enumDisplay.data = 0;
						}

						rsslHashTableRemoveLink(&_enumByDispalyValue, &pEnumDefinition->displayValueLink);

						free(pEnumDefinition);
						pEnumDefinition = 0;
					}
				}
			}
		}

		rsslHashTableCleanup(&_enumByDispalyValue);
	}
}

void EnumTableDefinition::decreaseRefCount()
{
	if ( (_referenceCount > 0) && (--_referenceCount == 0) )
	{
		delete this;
	}
}

RsslRet EnumTableDefinition::addEnumDefinition(RsslEnumTypeTable* pEnumTypeTable, RsslBuffer* displayValue, RsslUInt32 hashSum, int* foundEnumValue)
{
	RsslUInt16 enumValue;
	RsslEnumType* pEnumType;
	RsslBuffer outBuffer = RSSL_INIT_BUFFER;
	RsslErrorInfo rsslErrorInfo;
	EnumDefinition* pEnumDefinition;

	*foundEnumValue = -1; /* Not found */

	if (_initializedHashTable == false)
	{
		if (rsslHashTableInit(&_enumByDispalyValue, _maxCount, rsslHashBufferSum, rsslHashBufferCompare,
			RSSL_TRUE, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}

		_initializedHashTable = true;
	}

	for (enumValue = 0; enumValue < pEnumTypeTable->maxValue; enumValue++)
	{
		pEnumType = pEnumTypeTable->enumTypes[enumValue];

		if (pEnumType)
		{
			if (_pJsonToRwfSimple->rmtesToUtf8(pEnumType->display, outBuffer) == true)
			{
				if (rsslBufferIsEqual(displayValue, &outBuffer))
				{
					*foundEnumValue = pEnumType->value; /* Set the enum value */

					pEnumDefinition = (EnumDefinition*)malloc(sizeof(EnumDefinition));

					if (pEnumDefinition == NULL)
					{
						return RSSL_RET_FAILURE;
					}

					pEnumDefinition->enumDisplay.length = outBuffer.length; 
					pEnumDefinition->enumDisplay.data = (char*)malloc(pEnumDefinition->enumDisplay.length * sizeof(char));

					if (pEnumDefinition->enumDisplay.data == NULL)
					{
						return RSSL_RET_FAILURE;
					}

					memcpy(pEnumDefinition->enumDisplay.data, outBuffer.data, pEnumDefinition->enumDisplay.length);
					pEnumDefinition->enumValue = pEnumType->value;

					rsslHashTableInsertLink(&_enumByDispalyValue, &pEnumDefinition->displayValueLink, &pEnumDefinition->enumDisplay, &hashSum);
					++_referenceCount;

					return RSSL_RET_SUCCESS;
				}

			}
			else
			{
				continue; /* Failed to convert to utf8. Keep searching */
			}
		}
	}

	return RSSL_RET_SUCCESS;
}

RsslBool EnumTableDefinition::findEnumDefinition(RsslBuffer* displayValue, RsslUInt32 hashSum, int* foundEnumValue)
{
	RsslHashLink *rsslHashLink;
	EnumDefinition *pEnumDefinition = NULL;
	*foundEnumValue = -1;  /* Not found */

	if ((rsslHashLink = rsslHashTableFind(&_enumByDispalyValue, displayValue, &hashSum)) != NULL)
	{
		pEnumDefinition = RSSL_HASH_LINK_TO_OBJECT(EnumDefinition, displayValueLink, rsslHashLink);

		if (pEnumDefinition)
		{
			*foundEnumValue = pEnumDefinition->enumValue;
			return RSSL_TRUE;
		}
	}
	
	return RSSL_FALSE;
}

