/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "seriesEncDec.h"
#include "elementListEncDec.h"

/* These functions are used by Encoding and Decoding for RsslSeries Container Type */

/* this function will encode a basic series with several primitives embedded in it */
RsslRet exampleEncodeSeries(RsslEncodeIterator *encIter)
{
	RsslBool success = RSSL_TRUE;

	/* used to store and check return values */
	RsslRet retVal;

	/* create and initialize series structure */
	RsslSeries rsslSeries = RSSL_INIT_SERIES;

	/* create a single RsslSeriesEntry and reuse for each entry */
	RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;

	/* populate series structure prior to call to rsslEncodeSeriesInit */

	/* indicate that a total count hint will be encoded */
	rsslSeries.flags = RSSL_SRF_HAS_TOTAL_COUNT_HINT;
	rsslSeries.containerType = RSSL_DT_ELEMENT_LIST;
	rsslSeries.totalCountHint = 1;

	/* begin encoding of series - assumes that the RsslEncodeIterator pointed by the encIter pointer 
	   is already populated with buffer and version information */
	if ((retVal = rsslEncodeSeriesInit(encIter, &rsslSeries, 0, 0)) < RSSL_RET_SUCCESS)
	{
		/* print out message with return value string, value, and text */
		printf("Error %s (%d) encountered with rsslEncodeSeriesInit().  Error Text: %s\n", 
			rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}
	printf("\tSeries Header Encoded\n");

	/* get ready to encode a series entry */
	if ((retVal = rsslEncodeSeriesEntryInit(encIter, &seriesEntry, 0))  < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeSeriesEntryInit().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* now encode nested container using its own specific encode functions */
	printf("\tElementList Encoding Begin\n");
	if ((retVal = exampleEncodeElementList(encIter)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with exampleEncodeElementList().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	// the series entry containing an element list has been encoded. mark it complete
	if ((retVal = rsslEncodeSeriesEntryComplete(encIter, success)) < RSSL_RET_SUCCESS)
	{
		printf("<%s:%d> Error encoding element list.\n", __FILE__, __LINE__);
		printf("Error %s (%d) encountered with rsslEncodeSeriesEntryComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	/* complete series encoding.  If success parameter is true, this will finalize encoding.  
	   If success parameter is false, this will roll back encoding prior to rsslEncodeSeriesInit */
	if ((retVal = rsslEncodeSeriesComplete(encIter, RSSL_TRUE)) < RSSL_RET_SUCCESS)
	{
		printf("Error %s (%d) encountered with rsslEncodeSeriesComplete().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal)); 
		return retVal;
	}

	printf("\tSeries Encoding Complete\n");
	return RSSL_RET_SUCCESS;
}


/* this function will decode a basic series with several primitives embedded in it */
RsslRet exampleDecodeSeries(RsslDecodeIterator *decIter)
{
	/* used to store and check return codes */
	RsslRet retVal;

	/* create a series to decode into */
	RsslSeries rsslSeries = RSSL_INIT_SERIES;

	/* create series entry to decode into */
	RsslSeriesEntry seriesEntry = RSSL_INIT_SERIES_ENTRY;


	/* decode into the series structure */
	/* decode the series header */
	if ((retVal = rsslDecodeSeries(decIter, &rsslSeries)) < RSSL_RET_SUCCESS)
	{
		/* decoding failures tend to be unrecoverable */
		printf("Error %s (%d) encountered with rsslDecodeSeries().  Error Text: %s\n", 
				rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
		return retVal;
	}
	printf("\tSeries Header Decoded\n");

	/* decode each series entry  */
	/* since this succeeded, we can decode fields until we reach the end of the fields - until RSSL_RET_END_OF_CONTAINER is returned */
	while ((retVal = rsslDecodeSeriesEntry(decIter, &seriesEntry)) != RSSL_RET_END_OF_CONTAINER)
	{
		if (retVal < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with rsslDecodeSeriesEntry().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}
		if ((retVal = exampleDecodeElementList(decIter)) < RSSL_RET_SUCCESS)
		{
			printf("Error %s (%d) encountered with exampleDecodeElementList().  Error Text: %s\n", 
					rsslRetCodeToString(retVal), retVal, rsslRetCodeInfo(retVal));
			return retVal;
		}
	}
	printf("\tSeries Decoding Complete\n");
	return RSSL_RET_SUCCESS;
}
