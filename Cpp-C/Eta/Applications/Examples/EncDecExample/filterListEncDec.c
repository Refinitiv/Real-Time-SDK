/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "filterListEncDec.h"
#include "elementListEncDec.h"
#include "fieldListEncDec.h"


/* This function encodes a filter list which contains 2 nested element list and a field list */
RsslRet exampleEncodeFilterList(RsslEncodeIterator *encIter)
{
	RsslBool success = RSSL_TRUE;

	/* use this to store and check return codes */
	RsslRet retVal;

	/* create and initialize our filter list structure */
	RsslFilterList rsslFilterList = RSSL_INIT_FILTER_LIST; 

	/*create a single rsslFilterEntry and use this for all filter list entry encodings*/
	RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;


	/* populate filter list structure prior to call to rsslEncodeFilterInit */
	/* NOTE: the key names used for this example may not correspond to actual name values */

	/* indicate that a total count hint will be encoded  */
	rsslFilterList.flags = RSSL_FTF_NONE;
	/* populate filter list's containerType.  Since the majority of the items in the filter list are element lists, we'll choose that as the container type*/
	rsslFilterList.containerType = RSSL_DT_ELEMENT_LIST;

	rsslFilterList.totalCountHint = 3;

	/* begin encoding of filter list - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with
	   buffer and version information, store return value to determine success or failure */
	if ((retVal = rsslEncodeFilterListInit(encIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeFilterListInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* FilterListInit encoding was successful */
	printf("\tFilterList Header Encoded\n");


	/*FIRST FILTER ENTRY: encode entry from element list*/
	filterEntry.id = 1;
	filterEntry.containerType = RSSL_DT_ELEMENT_LIST;
	filterEntry.action = RSSL_VTEA_UPDATE_ENTRY;

	/*get ready to encode first filter entry*/
	if((retVal = rsslEncodeFilterEntryInit(encIter, &filterEntry, 100))  < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* now encode nested container using its own specific encode functions*/

	printf("\tEncoding Filter Entry (id: " RTR_LLU ") containing an encoded element list\n", (RsslUInt)filterEntry.id);

	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	// the filter entry containing an element list has been encoded. mark it complete
	if ((retVal = rsslEncodeFilterEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tFilter Entry (id: " RTR_LLU ") Encoding Complete\n", (RsslUInt)filterEntry.id);

	/*SECOND FILTER ENTRY: encode entry from field list*/
	rsslClearFilterEntry(&filterEntry);
	
	filterEntry.id = 2;
	filterEntry.containerType = RSSL_DT_FIELD_LIST;
	/*need to set the filter entry flag for container type since this entry is type field list, and the container type of the filter list is element list.*/
	filterEntry.flags = RSSL_FTEF_HAS_CONTAINER_TYPE;
	filterEntry.action = RSSL_VTEA_UPDATE_ENTRY;

	/*get ready to encode second filter entry*/
	if((retVal = rsslEncodeFilterEntryInit(encIter, &filterEntry, 100))  < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* now encode nested container using its own specific encode functions*/

	printf("\tEncoding Filter Entry (id: " RTR_LLU ") containing an encoded field list\n", (RsslUInt)filterEntry.id);

	printf("\tFieldList Encoding Begin\n");
	if ((retVal = exampleEncodeFieldList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding field list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeFieldList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/*the filter entry containing an field list has been encoded. mark it complete*/
	if ((retVal = rsslEncodeFilterEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tFilter Entry (id: " RTR_LLU ") Encoding Complete\n", (RsslUInt)filterEntry.id);

	/*THIRD FILTER ENTRY: encode entry from element list*/
	rsslClearFilterEntry(&filterEntry);
	
	filterEntry.id = 3;
	filterEntry.containerType = RSSL_DT_ELEMENT_LIST;
	filterEntry.action = RSSL_VTEA_UPDATE_ENTRY;

	/*get ready to encode third filter entry*/
	if((retVal = rsslEncodeFilterEntryInit(encIter, &filterEntry, 100))  < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* now encode nested container using its own specific encode functions*/

	printf("\tEncoding Filter Entry (id: " RTR_LLU ") containing an encoded element list\n", (RsslUInt)filterEntry.id);

	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	// the filter entry containing an element list has been encoded. mark it complete
	if ((retVal = rsslEncodeFilterEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tFilter Entry (id: " RTR_LLU ") Encoding Complete\n", (RsslUInt)filterEntry.id);


	/* complete filter list encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeFilterListInit */
	if ((retVal = rsslEncodeFilterListComplete(encIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeFilterListComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tFilterList Encoding Complete\n");
	return RSSL_RET_SUCCESS;
	
}		


/* decodes the simple filter list that is encoded above */
RsslRet exampleDecodeFilterList(RsslDecodeIterator *decIter)
{
	/* used to store and check return codes */
	RsslRet retVal;

	/* create our filter list to decode into */
	RsslFilterList rsslFilterList = RSSL_INIT_FILTER_LIST;

	/*create filter list entry to decode into*/
	RsslFilterEntry filterEntry = RSSL_INIT_FILTER_ENTRY;

	/*used to check the data type of the filter list entries*/
	RsslDataType cType;

	/* decode contents into the filter list structure */
	/* decode our filter list header */
	if ((retVal = rsslDecodeFilterList(decIter, &rsslFilterList)) < RSSL_RET_SUCCESS)
	{
		/* decoding failures tend to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeFilterList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}
	printf("\tFilterList Header Decoded \n");

	/* decode each filter list entry  */
	/* since this succeeded, we can decode fields until we reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
	while ((retVal = rsslDecodeFilterEntry(decIter, &filterEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslDecodeFilterEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}
		
		/* Continue decoding field entries. */

		printf("\tDecoding Filter Entry (id: " RTR_LLU ")\n", (RsslUInt)filterEntry.id);

		/*check the filter entry's container type, and use the appropriate decoding function. 
		since our filter-list-encode exmample only encodes elements list and field list, we only test for those two cases*/
		cType = filterEntry.containerType;
		switch(cType)
		{
			case RSSL_DT_ELEMENT_LIST:
				if ((retVal = exampleDecodeElementList(decIter)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with exampleDecodeElementList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return retVal;
				}
			break;
			case RSSL_DT_FIELD_LIST:
				if ((retVal = exampleDecodeFieldList(decIter)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with exampleDecodeFieldList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return retVal;
				}
			break;
		}
		printf("\tFilter Entry (id: " RTR_LLU ") Decoding Complete\n", (RsslUInt)filterEntry.id);

		rsslClearFilterEntry(&filterEntry);
	}
	printf("\tFilterList Decoding Complete\n");
	return RSSL_RET_SUCCESS;
}
