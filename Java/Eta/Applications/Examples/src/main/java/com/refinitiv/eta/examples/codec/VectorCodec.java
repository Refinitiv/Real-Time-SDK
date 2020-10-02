package com.refinitiv.eta.examples.codec;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.VectorEntry;
import com.refinitiv.eta.codec.VectorEntryActions;
import com.refinitiv.eta.codec.VectorEntryFlags;
import com.refinitiv.eta.codec.VectorFlags;

/**
 * This is used for encoding and decoding a Vector.
*/
public class VectorCodec
{
	int exampleEncode(EncodeIterator encIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec

		boolean success = true;

		/* use this to store and check return codes */
		int retVal;

		/* create and initialize our vector structure */
		Vector vector = CodecFactory.createVector(); 

		/*create a single VectorEntry*/
		VectorEntry vectorEntry = CodecFactory.createVectorEntry();


		/* populate vector structure prior to call to EncodeVectorInit */
		/* NOTE: the key names used for this example may not correspond to actual name values */

		/* indicate that a total count hint and summary data will be encoded  */
		vector.flags(VectorFlags.HAS_TOTAL_COUNT_HINT | VectorFlags.HAS_SUMMARY_DATA);
		/* populate vector's containerType */
		vector.containerType(DataTypes.ELEMENT_LIST);
		/* populate total count hint with approximate expected entry count */
		vector.totalCountHint(1);

		/* begin encoding of vector - assumes that (*encIter) is already populated with
		   buffer and version information, store return value to determine success or failure */
		/* expect summary data of approx. 50 bytes, no set definition data */
		if ((retVal = vector.encodeInit(encIter, 128, 0)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeVectorInit.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		/* VectorInit encoding was successful */
		System.out.printf("\tVector Header Encoded");

		/* begin encoding summary data*/
		System.out.printf("\n\tElementList Encoding Begin\n");
		if ((retVal = elementListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		System.out.printf("\n\tSummary Data Encoding Complete");

		/* complete encoding of summary data.  If any field list encoding failed, success is false */
		if ((retVal = vector.encodeSummaryDataComplete(encIter, success)) < CodecReturnCodes.SUCCESS)	
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			System.out.printf("Error %s (%d) encountered with EncodeVectorSummaryDataComplete.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}

		/*Indicate that we vector entry will be an update*/
		vectorEntry.index(1);
		vectorEntry.flags(VectorEntryFlags.NONE);
		vectorEntry.action(VectorEntryActions.UPDATE);

		/*get ready to encode vector entry*/
		if((retVal = vectorEntry.encodeInit(encIter, 90))  < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeVectorEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		System.out.printf("\n\tEncoding Vector Entry (index: %d) containing an encoded element list", vectorEntry.index());

		/* now encode nested container using its own specific encode fucntions*/
		System.out.printf("\n\tElementList Encoding Begin\n");
		if ((retVal = elementListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		// the vector entry containing an element list has been encoded. mark it complete
		if ((retVal = vectorEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeVectorEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		System.out.printf("\n\tVector Entry (index: %d) Encoding Complete", vectorEntry.index());
		
		/* complete vector encoding.  If success parameter is true, this will finalize encoding.  
		   If success parameter is false, this will roll back encoding prior to EncodeVectorInit */
		if ((retVal = vector.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeVectorComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		System.out.printf("\n\tVector Encoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
	
	int exampleDecode(DecodeIterator decIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec

		/* used to store and check return codes */
		int retVal;

		/* create our vector to decode into */
		Vector vector = CodecFactory.createVector();

		/*create vector entry to decode into*/
		VectorEntry vectorEntry = CodecFactory.createVectorEntry();

		/* decode contents into the vector structure */
		/* decode our vector header */
		if ((retVal = vector.decode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			/* decoding failures tend to be unrecoverable */
			System.out.printf("Error %s (%d) encountered with DecodeVector.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
			return retVal;
		}
		System.out.printf("\tVector Header Decoded: TotalCountHint: %d\n", vector.totalCountHint());

		/* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
		indicates to ETA that user wants to decode summary data */
		if(vector.checkHasSummaryData()) {
			
			/*summary data is present.  Its type should be that of Vector.containerType*/

			System.out.printf("\tDecoding Summary Data\n");

			if ((retVal = elementListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with exampleDecodeElementList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}

			System.out.printf("\tSummary Data Decoding Complete\n");
		}
		/* decode each vector entry  */
		/* since this succeeded, we can decode fields until we reach the end of the fields - until CodecReturnCodes.END_OF_CONTAINER is returned */
		while ((retVal = vectorEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retVal < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeVectorEntry.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}
			
			/* Continue decoding field entries. */

			System.out.printf("\tDecoding Vector Entry (index: %d)\n", vectorEntry.index());

			if ((retVal = elementListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with exampleDecodeElementList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}
			System.out.printf("\tVector Entry (index: %d) Decoding Complete\n", vectorEntry.index());
		}
		System.out.printf("\tVector Decoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
}
