package com.rtsdk.eta.examples.codec;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Series;
import com.rtsdk.eta.codec.SeriesEntry;
import com.rtsdk.eta.codec.SeriesFlags;

/**
 * This is used for encoding and decoding a Series.
*/
public class SeriesCodec
{
	int exampleEncode(EncodeIterator encIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		
		boolean success = true;

		/* used to store and check return values */
		int retVal;

		/* create and initialize series structure */
		Series series = CodecFactory.createSeries();

		/* create a single SeriesEntry and reuse for each entry */
		SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

		/* populate series structure prior to call to EncodeSeriesInit */

		/* indicate that a total count hint will be encoded */
		series.flags(SeriesFlags.HAS_TOTAL_COUNT_HINT);
		series.containerType(DataTypes.ELEMENT_LIST);
		series.totalCountHint(1);

		/* begin encoding of series - assumes that (*encIter) is already populated with buffer and version information */
		if ((retVal = series.encodeInit(encIter, 0, 0)) < CodecReturnCodes.SUCCESS)
		{
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeSeriesInit.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		System.out.printf("\tSeries Header Encoded");

		/* get ready to encode a series entry */
		if ((retVal = seriesEntry.encodeInit(encIter, 0))  < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeSeriesEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/* now encode nested container using its own specific encode methods */
		System.out.printf("\n\tElementList Encoding Begin\n");
		if ((retVal = elementListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		// the series entry containing an element list has been encoded. mark it complete
		if ((retVal = seriesEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with EncodeSeriesEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/* complete series encoding.  If success parameter is true, this will finalize encoding.  
		   If success parameter is false, this will roll back encoding prior to EncodeSeriesInit */
		if ((retVal = series.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeSeriesComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\n\tSeries Encoding Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
	
	int exampleDecode(DecodeIterator decIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		
		/* used to store and check return codes */
		int retVal;

		/* create a field list to decode into - 
		   since set data is not supported yet, there is nothing to put 
		   here except the count of the number of fields encoded in this message*/
		Series series = CodecFactory.createSeries();

		/* create series entry to decode into */
		SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

		/* decode into the series structure */
		/* decode the series header */
		if ((retVal = series.decode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			/* decoding failures tend to be unrecoverable */
			System.out.printf("Error %s (%d) encountered with DecodeFieldList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
			return retVal;
		}
		System.out.printf("\tSeries Header Decoded\n");

		/* decode each series entry  */
		/* since this succeeded, we can decode fields until we reach the end of the fields - until CodecReturnCodes.END_OF_CONTAINER is returned */
		while ((retVal = seriesEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retVal < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeSeriesEntry.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}
			if ((retVal = elementListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with exampleDecodeElementList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}
		}
		System.out.printf("\tSeries Decoding Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
}
