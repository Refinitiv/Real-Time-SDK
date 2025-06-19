/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "mapEncDec.h"
#include "fieldListEncDec.h"


/* This function encodes a map which contains nested field lists */
RsslRet exampleEncodeMap(RsslEncodeIterator *encIter)
{
	/* use this to store and check return codes */
	RsslRet retVal;
	RsslBool success = RSSL_TRUE;

	/* create and initialize our map structure */
	RsslMap rsslMap = RSSL_INIT_MAP;

	/* populate map structure prior to call to rsslEncodeMapInit */
	/* NOTE: the key names used for this example may not correspond to actual name values */

	/* indicate that summary data and a total count hint will be encoded  */
	rsslMap.flags = RSSL_MPF_HAS_SUMMARY_DATA | RSSL_MPF_HAS_TOTAL_COUNT_HINT;
	/* populate maps keyPrimitiveType and containerType */
	rsslMap.containerType = RSSL_DT_FIELD_LIST;
	rsslMap.keyPrimitiveType = RSSL_DT_UINT;
	/* populate total count hint with approximate expected entry count */
	rsslMap.totalCountHint = 3;


	/* begin encoding of map - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with
	   buffer and version information, store return value to determine success or failure */
	/* expect summary data of approx. 256 bytes, no set definition data */
	if ((retVal = rsslEncodeMapInit(encIter, &rsslMap, 256, 0 )) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeMapInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}
	else
	{
		/* mapInit encoding was successful */
		/* create a single RsslMapEntry and RsslFieldList and reuse for each entry */
		RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;
		RsslUInt entryKeyUInt = 0;

		/* create a buffer for uInt and field list to encode into -
		   This buffer can come from anywhere (stack allocated, malloc/new heap allocated).  Typically, for performance, the transport layer can provide
	       a pool of buffers for use and reuse that avoids the constant allocation/deallocation penalty.  
	       For this example I am stack allocating the buffer */
		char buf[10] = "";  //10 bytes is large enough for UInt encoding
		char buf2[500] = "";	//500 bytes is large enough for RsslFieldList encoding

		/* create a RsslBuffer to set the buffer into */
		RsslBuffer encDecBuffer, encDecBuffer2;
		RsslBuffer *pEncUInt = 0, *pEncFieldList = 0;

		/* set the data members to encDecBuffer buf and the length I created */
		encDecBuffer.data = buf;
		encDecBuffer.length = 10;

		encDecBuffer2.data = buf2;
		encDecBuffer2.length = 500;

		printf("\tRsslMap Header Encoded\n");

		/* encode expected summary data, init for this was done by rsslEncodeMapInit
	    - this type should match rsslMap.containerType */
		{
			printf("\tEncoding Summary Data\n");

			/* now encode nested container using its own specific encode functions */
			/* begin encoding of field list - using same encIterator as map list */

			if ((retVal = exampleEncodeFieldList(encIter)) < RSSL_RET_SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = RSSL_FALSE;
				printf("<%s:%d> Error encoding field list.\n", __FILE__, __LINE__);
				printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			}

			printf("\tSummary Data Encoding Complete\n");
		
		}
		/* complete encoding of summary data.  If any field list encoding failed, success is false */
		if ((retVal = rsslEncodeMapSummaryDataComplete(encIter, success)) < RSSL_RET_SUCCESS)	
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with rsslEncodeMapSummaryDataComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		/* FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown */
		mapEntry.action = RSSL_MPEA_UPDATE_ENTRY;
		entryKeyUInt = 1;
		if ((retVal = rsslEncodeMapEntryInit(encIter, &mapEntry, &entryKeyUInt, 0)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with rsslEncodeMapEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}
		else
		/* encode contained field list - this type should match rsslMap.containerType */
		{
			printf("\tEncoding Map Entry (key: " RTR_LLU ") from non pre-encoded data and key\n", entryKeyUInt);

			/* now encode nested container using its own specific encode functions */
			/* clear, then begin encoding of field list - using same encIterator as map */

			if ((retVal = exampleEncodeFieldList(encIter)) < RSSL_RET_SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = RSSL_FALSE;
				printf("<%s:%d> Error encoding field list.\n", __FILE__, __LINE__);
				printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
			}
		
		}
		if ((retVal = rsslEncodeMapEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with rsslEncodeMapEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		printf("\tMap Entry (key: " RTR_LLU ") Encoding Complete\n", entryKeyUInt);

		
		/* SECOND Map Entry: encode entry from pre-encoded buffer containing an encoded RsslFieldList */
		/* because we are re-populating all values on RsslMapEntry, there is no need to clear it */

		mapEntry.action = RSSL_MPEA_ADD_ENTRY;
		entryKeyUInt = 2;

		printf("\tEncoding Map Entry (key: 2) from pre-encoded buffer\n");

		/* assuming pEncUInt RsslBuffer contains the pre-encoded key with length and data properly populated */
		if ((retVal = getPreEncodedRsslUIntBuffer(&encDecBuffer, entryKeyUInt)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with getPreEncodedRsslUIntBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		pEncUInt = &encDecBuffer;

		mapEntry.encKey.length = pEncUInt->length;
		mapEntry.encKey.data = pEncUInt->data;

		/* assuming pEncFieldList RsslBuffer contains the pre-encoded payload with data and length populated */
		if ((retVal = getPreEncodedRsslFieldListBuffer(&encDecBuffer2)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with getPreEncodedRsslFieldListBuffer().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		pEncFieldList = &encDecBuffer2;

		mapEntry.encData.length = pEncFieldList->length;
		mapEntry.encData.data = pEncFieldList->data;

		/* void* parameter is passed in as NULL because pre-encoded key is set on RsslMapEntry itself */
		if ((retVal = rsslEncodeMapEntry(encIter, &mapEntry, NULL)) < RSSL_RET_SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with rsslEncodeMapEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		printf("\tMap Entry (key: " RTR_LLU ") Encoding Complete\n", entryKeyUInt);


		/* THIRD Map Entry: encode entry with delete action.  Delete actions have no payload */
		/* need to ensure that RsslMapEntry is appropriatley cleared
		 * - clearing will ensure that encData and encKey are properly emptied */          
		rsslClearMapEntry(&mapEntry);

		mapEntry.action = RSSL_MPEA_DELETE_ENTRY;
		entryKeyUInt = 3;

		printf("\tEncoding Map Entry (key: " RTR_LLU ") with delete action with no payload\n", entryKeyUInt);

		/* void* parameter is passed in as pointer to key primitive value.  encData is empty for delete */
		if ((retVal = rsslEncodeMapEntry(encIter, &mapEntry, &entryKeyUInt)) < RSSL_RET_SUCCESS) 
		{
			/* error condition - switch our success value to false so we can roll back */
			success = RSSL_FALSE;
			printf("Error %s (%d) encountered with rsslEncodeMapEntry().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		}

		printf("\tMap Entry (key: " RTR_LLU ") Encoding Complete\n", entryKeyUInt);

	}

	/* complete map encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeMapInit */
	if ((retVal = rsslEncodeMapComplete(encIter, success)) < RSSL_RET_SUCCESS) 
	{
		printf("Error %s (%d) encountered with rsslEncodeMapEntry().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	
	printf("\tRsslMap Encoding Complete\n");

	return RSSL_RET_SUCCESS;
}


/* decodes the simple map that is encoded above */
RsslRet exampleDecodeMap(RsslDecodeIterator *decIter)
{
	/* used to store and check return codes */
	RsslRet retVal;

	/* create our map to decode into */
	RsslMap rsslMap = RSSL_INIT_MAP;

	/* decode contents into the map structure */
	/* decode our map header */
	if ((retVal = rsslDecodeMap(decIter, &rsslMap)) >= RSSL_RET_SUCCESS)
	{
		/* create primitive value to have key decoded into and a single map entry to reuse */
		RsslUInt rsslUInt = 0;
		RsslMapEntry mapEntry = RSSL_INIT_MAP_ENTRY;

		printf("\tRsslMap Header Decoded: TotalCountHint: %d\n", rsslMap.totalCountHint);

		/* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
		indicates to ETA that user wants to decode summary data */
		if (rsslMap.flags & RSSL_MPF_HAS_SUMMARY_DATA)
		{
			/* summary data is present.  Its type should be that of rsslMap.containerType */

			printf("\tDecoding Summary Data\n");

			/* Continue decoding field entries.  See example in Section 11.3.1 */
			/* decode the field list */
			if ((retVal = exampleDecodeFieldList(decIter)) < RSSL_RET_SUCCESS)
			{
				printf("<%s:%d> Error decoding nested field list.\n", __FILE__, __LINE__);
				printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
				return retVal;
			}

			printf("\tSummary Data Decoding Complete\n");
		}

		/* decode each map entry, passing in pointer to keyPrimitiveType decodes mapEntry key as well  */
		while ((retVal = rsslDecodeMapEntry(decIter, &mapEntry, &rsslUInt)) != RSSL_RET_END_OF_CONTAINER)
		{
			if (retVal < RSSL_RET_SUCCESS)
			{
				/* decoding failure tends to be unrecoverable */
				printf("Error %s (%d) encountered with rsslDecodeMapEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
				return retVal;
			}
			else
			{
				/* Continue decoding field entries. */

				printf("\tDecoding Map Entry (key: " RTR_LLU ")\n", rsslUInt);

				/* Delete actions have no payload and we need to avoid decoding FieldList for this situation */
				if (mapEntry.action != RSSL_MPEA_DELETE_ENTRY)
				{
					if ((retVal = exampleDecodeFieldList(decIter)) < RSSL_RET_SUCCESS)
					{
						printf("<%s:%d> Error decoding nested field list.\n", __FILE__, __LINE__);
						printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
						return retVal;
					}
				}
				else
				{
					printf("\tMap Entry (key: " RTR_LLU ") delete actions have no payload to decode\n", rsslUInt);
				}

				printf("\tMap Entry (key: " RTR_LLU ") Decoding Complete\n", rsslUInt);
			}
		}
	}
	else
	{
		/* decoding failure tends to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeMap().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}

	printf("\tRsslMap Decoding Complete\n");

	return RSSL_RET_SUCCESS;
}
