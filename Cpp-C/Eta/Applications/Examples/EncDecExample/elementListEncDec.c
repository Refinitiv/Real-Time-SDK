

#include "elementListEncDec.h"

/* These functions are used by Encoding and Decoding for RsslElementList Container Type */

/* this function will encode a basic Element list with several primitives embedded in it */
RsslRet exampleEncodeElementList(RsslEncodeIterator *encIter)
{
	/* create a single RsslElementEntry and reuse for each entry */
	RsslElementEntry elemEntry = RSSL_INIT_ELEMENT_ENTRY;

	/* used to store and check return values */
	RsslRet retVal;

	/* create and initialize element list structure */
	RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;

	/* various data types used during encoding */
	RsslTime rsslTime = {10, 21, 16, 777};
	RsslUInt rsslUInt = 17;
	RsslInt rsslInt = 13;

	/* populate element list structure prior to call to rsslEncodeElementListInit */

	/* indicate that standard data will be encoded and that elementListNum is included */
	elementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;

	/* populate elementListNum with info needed to cache */
	elementList.elementListNum = 5;

	/* begin encoding of element list - assumes that the RsslEncodeIterator pointed by the encIter pointer is already populated with buffer and version information */

	/* Please note: here for simplicity, we did not use success parameter for rsslEncodeElementListInit/rsslEncodeElementListComplete calls. 
	   We are just simply displaying an error if it occurs and exit, thus RSSL_TRUE is used in replacement for success parameter */

	if ((retVal = rsslEncodeElementListInit(encIter, &elementList, 0, 0)) < RSSL_RET_SUCCESS)
	{
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeElementListInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* FIRST Element Entry: encode entry from the RsslTime primitive type */
	/* populate and encode element entry with name and dataType information for this element */
	elemEntry.name.data = (char *)"Element - RsslTime";
	elemEntry.name.length = 20; 
	elemEntry.dataType = RSSL_DT_TIME;
	printf("\tEncoding Element Entry (name: %.*s) \n", elemEntry.name.length, elemEntry.name.data);
	retVal = rsslEncodeElementEntry(encIter, &elemEntry, &rsslTime); 
	printf("\t\tEncoded RsslTime: %d:%d:%d\n", rsslTime.hour, rsslTime.minute, rsslTime.second);

	/* SECOND Element Entry: encode entry from the RsslInt primitive type */
	/* populate and encode element entry with name and dataType information for this element */
	elemEntry.name.data = (char *)"Element - RsslInt";
	elemEntry.name.length = 17; 
	elemEntry.dataType = RSSL_DT_INT;
	printf("\tEncoding Element Entry (name: %.*s) \n", elemEntry.name.length, elemEntry.name.data);
	retVal = rsslEncodeElementEntry(encIter, &elemEntry, &rsslInt); 		
	printf("\t\tEncoded signed Integer: " RTR_LLD "\n", rsslInt);

	/* THIRD Element Entry: encode entry from the RsslUInt primitive type */
	/* populate and encode element entry with name and dataType information for this element */
	elemEntry.name.data = (char *)"Element - RsslUInt";
	elemEntry.name.length = 18; 
	elemEntry.dataType = RSSL_DT_UINT;
	printf("\tEncoding Element Entry (name: %.*s) \n", elemEntry.name.length, elemEntry.name.data);
	retVal = rsslEncodeElementEntry(encIter, &elemEntry, &rsslUInt); 
	printf("\t\tEncoded Unsigned Integer: " RTR_LLU "\n", rsslUInt);

	/* FOURTH Element Entry: encode entry from the RsslReal primitive type */
	/* populate and encode element entry with name and dataType information for this element */
	rsslClearElementEntry(&elemEntry);	// clear this to ensure a blank field
	elemEntry.name.data = (char *)"Element - RsslReal - Blank";
	elemEntry.name.length = 26; 
	elemEntry.dataType = RSSL_DT_REAL;
	printf("\tEncoding Element Entry (name: %.*s) \n", elemEntry.name.length, elemEntry.name.data);
	retVal = rsslEncodeElementEntry(encIter, &elemEntry, NULL);		/* this encodes a blank */
	printf("\t\tEncoded RsslReal: Blank\n");

	/* complete elementList encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeElementListInit */
	
	if ((retVal = rsslEncodeElementListComplete(encIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeElementListComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tElementList Encoding Complete\n");
	return RSSL_RET_SUCCESS;
}


/* this function will encode a basic element list with several primitives embedded in it */
RsslRet exampleDecodeElementList(RsslDecodeIterator *decIter)
{
	/* create and initialize element list structure */
	RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;

	/* create a single RsslElementEntry and reuse for each entry */
	RsslElementEntry elemEntry = RSSL_INIT_ELEMENT_ENTRY;

	/* used to store and check return codes */
	RsslRet retVal;

	/* structures for decoding various data types */
	RsslTime rsslTime;
	RsslReal rsslReal;
	RsslInt rsslInt;
	RsslUInt rsslUInt;
	
	/* decode into the element list structure */
	if ((retVal = rsslDecodeElementList(decIter, &elementList, NULL)) < RSSL_RET_SUCCESS)
	{
		/* decoding failure tends to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return RSSL_RET_FAILURE;
	}
	printf("\tElementList Header Decoded\n");
	
	/* decode each element entry */
	while ((retVal = rsslDecodeElementEntry(decIter, &elemEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			/* decoding failure tends to be unrecoverable */
			printf("Error %s (%d) encountered with rsslDecodeElementEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return RSSL_RET_FAILURE;
		}

		/* Continue decoding field entries. */

		printf("\tDecoding Element Entry (name: %.*s) \n", elemEntry.name.length, elemEntry.name.data);

		/* use elemEntry.dataType to call correct primitive decode function */
		switch (elemEntry.dataType)
		{	
			case RSSL_DT_REAL:
				if ((retVal = rsslDecodeReal(decIter, &rsslReal)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with rsslDecodeReal().  Error Text: %s\n", 
							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return RSSL_RET_FAILURE;
				}
				if (rsslReal.isBlank)
					printf("\t\tDecoded RsslReal: Blank\n");
				else
					printf("\t\tRsslReal Decoded: hint: %d  value: " RTR_LLD "\n", rsslReal.hint, rsslReal.value);
				break;
			
			case RSSL_DT_TIME:
				if ((retVal = rsslDecodeTime(decIter, &rsslTime)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with rsslDecodeTime().  Error Text: %s\n", 
							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return RSSL_RET_FAILURE;
				}
				printf("\t\tRsslTime Decoded: %d:%d:%d\n", rsslTime.hour, rsslTime.minute, rsslTime.second);
				break;

			case RSSL_DT_INT:
				if ((retVal = rsslDecodeInt(decIter, &rsslInt)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with rsslDecodeInt().  Error Text: %s\n", 
							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return RSSL_RET_FAILURE;
				}
				printf("\t\tRsslInt Decoded: " RTR_LLD "\n", rsslInt);
				break;
			
			case RSSL_DT_UINT:
				if ((retVal = rsslDecodeUInt(decIter, &rsslUInt)) < RSSL_RET_SUCCESS)
				{
					printf("Error %s (%d) encountered with rsslDecodeUInt().  Error Text: %s\n", 
							rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
					return RSSL_RET_FAILURE;
				}
				printf("\t\tRsslUInt Decoded: " RTR_LLU "\n", rsslUInt);
				break;
			
			default:
				printf("Error: unexpected datatype(%d) found in elementList\n", elemEntry.dataType);
				break;
		}
	}

	printf("\tElementList Decoding Complete\n");
	return RSSL_RET_SUCCESS;
}


