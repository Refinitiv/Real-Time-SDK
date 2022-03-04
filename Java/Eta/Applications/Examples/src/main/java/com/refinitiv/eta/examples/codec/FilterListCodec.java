/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.codec;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldListFlags;
import com.refinitiv.eta.codec.FilterEntry;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.FilterEntryFlags;
import com.refinitiv.eta.codec.FilterList;


/**
 * This is used for encoding and decoding a FilterList Container Type.
*/
public class FilterListCodec
{
	int exampleEncode(EncodeIterator encIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec
		
		boolean success = true;

		/* use this to store and check return codes */
		int retVal;

		/* create and initialize our filter list structure */
		FilterList filterList = CodecFactory.createFilterList(); 

		/* create a single FilterEntry and use this for all filter list entry encodings*/
		FilterEntry filterEntry = CodecFactory.createFilterEntry();

		/* populate filter list structure prior to call to EncodeFilterInit */
		/* NOTE: the key names used for this example may not correspond to actual name values */

		/* indicate that a total count hint will be encoded  */
		filterList.flags(FieldListFlags.NONE);
		/* populate filter list's containerType.  Since the majority of the items in the filter list are element lists, we'll choose that as the container type*/
		filterList.containerType(DataTypes.ELEMENT_LIST);

		/* begin encoding of filter list - assumes that (*encIter) is already populated with
		   buffer and version information, store return value to determine success or failure */
		if ((retVal = filterList.encodeInit(encIter)) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeFilterListInit.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		/* FilterListInit encoding was successful */
		System.out.printf("\tFilterList Header Encoded\n");


		/*FIRST FILTER ENTRY: encode entry from element list*/
		filterEntry.id(1);
		filterEntry.containerType(DataTypes.ELEMENT_LIST);
		filterEntry.action(FilterEntryActions.UPDATE);

		/*get ready to encode first filter entry*/
		if((retVal = filterEntry.encodeInit(encIter, 100))  < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/* now encode nested container using its own specific encode methods*/

		System.out.printf("\tEncoding Filter Entry (id: %d) containing an encoded element list", filterEntry.id());

		System.out.printf("\n\tElementList Encoding Begin\n");
		if ((retVal = elementListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		// the filter entry containing an element list has been encoded. mark it complete
		if ((retVal = filterEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		System.out.printf("\n\tFilter Entry (id: %d) Encoding Complete\n", filterEntry.id());

		/*SECOND FILTER ENTRY: encode entry from field list*/
		filterEntry.clear();
		
		filterEntry.id(2);
		filterEntry.containerType(DataTypes.FIELD_LIST);
		/*need to set the filter entry flag for container type since this entry is type field list, and the container type of the filter list is element list.*/
		filterEntry.flags(FilterEntryFlags.HAS_CONTAINER_TYPE);
		filterEntry.action(FilterEntryActions.UPDATE);

		/*get ready to encode second filter entry*/
		if((retVal = filterEntry.encodeInit(encIter, 100))  < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/* now encode nested container using its own specific encode fucntions*/

		System.out.printf("\tEncoding Filter Entry (id: %d) containing an encoded field list", filterEntry.id());

		System.out.printf("\n\tFieldList Encoding Begin\n");
		if ((retVal = fieldListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding field list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/*the filter entry containing a field list has been encoded. mark it complete*/
		if ((retVal = filterEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\n\tFilter Entry (id: %d) Encoding Complete\n", filterEntry.id());

		/*THIRD FILTER ENTRY: encode entry from element list*/
		filterEntry.clear();
		
		filterEntry.id(3);
		filterEntry.containerType(DataTypes.ELEMENT_LIST);
		filterEntry.action(FilterEntryActions.UPDATE);

		/*get ready to encode third filter entry*/
		if((retVal = filterEntry.encodeInit(encIter, 100))  < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		/* now encode nested container using its own specific encode methods*/

		System.out.printf("\tEncoding Filter Entry (id: %d) containing an encoded element list", filterEntry.id());

		System.out.printf("\n\tElementList Encoding Begin\n");
		if ((retVal = elementListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error encoding element list.\n");
			System.out.printf("Error %s (%d) encountered with exampleEncodeElementList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		// the filter entry containing an element list has been encoded. mark it complete
		if ((retVal = filterEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\n\tFilter Entry (id: %d) Encoding Complete", filterEntry.id());

		/* complete filter list encoding.  If success parameter is true, this will finalize encoding.  
		   If success parameter is false, this will roll back encoding prior to EncodeFilterListInit */
		if ((retVal = filterList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFilterListComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\n\tFilterList Encoding Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
	
	int exampleDecode(DecodeIterator decIter)
	{
		ElementListCodec elementListCodec = new ElementListCodec(); // the element list codec
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec

		/* used to store and check return codes */
		int retVal;

		/* create our filter list to decode into */
		FilterList filterList = CodecFactory.createFilterList();

		/*create filter list entry to decode into */
		FilterEntry filterEntry = CodecFactory.createFilterEntry();

		/*used to check the data type of the filter list entries*/
		int cType;

		/* decode contents into the filter list structure */
		/* decode our filter list header */
		if ((retVal = filterList.decode(decIter)) < CodecReturnCodes.SUCCESS)
		{
			/* decoding failures tend to be unrecoverable */
			System.out.printf("Error %s (%d) encountered with DecodeFilterList.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
			return retVal;
		}
		System.out.printf("\tFilterList Header Decoded \n");

		/* decode each filter list entry  */
		/* since this succeeded, we can decode fields until we reach the end of the fields - until CodecReturnCodes.END_OF_CONTAINER is returned */
		while ((retVal = filterEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retVal < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeFilterEntry.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
				return retVal;
			}
			
			/* Continue decoding field entries. */

			System.out.printf("\tDecoding Filter Entry (id: %d)\n", filterEntry.id());

			/*check the filter entry's container type, and use the appropriate decoding method. 
			since our filter-list-encode exmample only encodes elements list and field list, we only test for those two cases*/
			cType = filterEntry.containerType();
			switch(cType)
			{
				case DataTypes.ELEMENT_LIST:
					if ((retVal = elementListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with exampleDecodeElementList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
						return retVal;
					}
				break;
				case DataTypes.FIELD_LIST:
					if ((retVal = fieldListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with exampleDecodeFieldList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
						return retVal;
					}
				break;
				default:
					break;
			}
			System.out.printf("\tFilter Entry (id: %d) Decoding Complete\n", filterEntry.id());

			filterEntry.clear();
		}
		System.out.printf("\tFilterList Decoding Complete\n");
		return CodecReturnCodes.SUCCESS;
	}
}
