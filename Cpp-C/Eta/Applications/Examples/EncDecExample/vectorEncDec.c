

#include "vectorEncDec.h"
#include "elementListEncDec.h"


/* This function encodes a vector which contain a nested element list */
RsslRet exampleEncodeVector(RsslEncodeIterator *encIter)
{
	RsslBool success = RSSL_TRUE;

	/* use this to store and check return codes */
	RsslRet retVal;

	/* create and initialize our vector structure */
	RsslVector rsslVector = RSSL_INIT_VECTOR; 

	/*create a single rsslVectorEntry*/
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;


	/* populate vector structure prior to call to rsslEncodeVectorInit */
	/* NOTE: the key names used for this example may not correspond to actual name values */

	/* indicate that a total count hint and summary data will be encoded  */
	rsslVector.flags = RSSL_VTF_HAS_TOTAL_COUNT_HINT | RSSL_VTF_HAS_SUMMARY_DATA;
	/* populate vector's containerType */
	rsslVector.containerType = RSSL_DT_ELEMENT_LIST;
	/* populate total count hint with approximate expected entry count */
	rsslVector.totalCountHint = 1;


	/* begin encoding of vector - assumes that the RsslEncodeIterator pointed by the encIter pointer 
	   is already populated with buffer and version information, store return value to determine success or failure */
	/* expect summary data of approx. 50 bytes, no set definition data */
	if ((retVal = rsslEncodeVectorInit(encIter, &rsslVector, 50, 0 )) < RSSL_RET_SUCCESS)
	{
		/* error condition - switch our success value to false so we can roll back */
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeVectorInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	/* VectorInit encoding was successful */
	printf("\tVector Header Encoded\n");

	/* begin encoding summary data*/
	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	printf("\tSummary Data Encoding Complete\n");

	/* complete encoding of summary data.  If any nested element list encoding failed, success is false */
	if ((retVal = rsslEncodeVectorSummaryDataComplete(encIter, success)) < RSSL_RET_SUCCESS)	
	{
		/* error condition - switch our success value to false so we can roll back */
		success = RSSL_FALSE;
		printf("Error %s (%d) encountered with rsslEncodeVectorSummaryDataComplete().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
	}

	/*Indicate that our vector entry will be an update*/
	vectorEntry.index = 1;
	vectorEntry.flags = RSSL_VTEF_NONE;
	vectorEntry.action = RSSL_VTEA_UPDATE_ENTRY;

	/*get ready to encode vector entry*/
	if((retVal = rsslEncodeVectorEntryInit(encIter, &vectorEntry, 90))  < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeVectorEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tEncoding Vector Entry (index: " RTR_LLU ") containing an encoded element list\n", (RsslUInt)vectorEntry.index);

	/* now encode nested container using its own specific encode fucntions*/
	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	// the vector entry containing an element list has been encoded. mark it complete
	if ((retVal = rsslEncodeVectorEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeVectorEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tVector Entry (index: " RTR_LLU ") Encoding Complete\n", (RsslUInt)vectorEntry.index);

	/* complete vector encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeVectorInit */
	if ((retVal = rsslEncodeVectorComplete(encIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeVectorComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	printf("\tVector Encoding Complete\n");
	return RSSL_RET_SUCCESS;
	
}		


/* decodes the simple vector that is encoded above */
RsslRet exampleDecodeVector(RsslDecodeIterator *decIter)
{
	/* used to store and check return codes */
	RsslRet retVal;

	/* create our vector to decode into */
	RsslVector rsslVector = RSSL_INIT_VECTOR;

	/*create vector entry to decode into*/
	RsslVectorEntry vectorEntry = RSSL_INIT_VECTOR_ENTRY;

	/* decode contents into the vector structure */
	/* decode our vector header */
	if ((retVal = rsslDecodeVector(decIter, &rsslVector)) < RSSL_RET_SUCCESS)
	{
		/* decoding failures tend to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeVector().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}
    printf("\tVector Header Decoded: TotalCountHint: %d\n", rsslVector.totalCountHint);

	/* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
	indicates to ETA that user wants to decode summary data */
	if(rsslVector.flags & RSSL_VTF_HAS_SUMMARY_DATA) {
		
		/*summary data is present.  Its type should be that of rsslVector.containerType*/

		printf("\tDecoding Summary Data\n");

		if ((retVal = exampleDecodeElementList(decIter)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with exampleDecodeElementList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}

		printf("\tSummary Data Decoding Complete\n");
	}
	/* decode each vector entry  */
	/* since this succeeded, we can decode fields until we reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
	while ((retVal = rsslDecodeVectorEntry(decIter, &vectorEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslDecodeVectorEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}

		/* Continue decoding field entries. */

		printf("\tDecoding Vector Entry (index: " RTR_LLU ")\n", (RsslUInt)vectorEntry.index);

		if ((retVal = exampleDecodeElementList(decIter)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with exampleDecodeElementList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}
		printf("\tVector Entry (index: " RTR_LLU ") Decoding Complete\n", (RsslUInt)vectorEntry.index);
	}
	printf("\tVector Decoding Complete\n");
	return RSSL_RET_SUCCESS;
}
