package com.refinitiv.eta.examples.codec;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.MapFlags;
import com.refinitiv.eta.codec.UInt;

/**
 * This is used for encoding and decoding a Map Container Type.
*/
public class MapCodec
{
	int exampleEncode(EncodeIterator encIter)
	{
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec
		
		/* use this to store and check return codes */
		int retVal;
		boolean success = true;

		/* create and initialize our map structure */
		Map map = CodecFactory.createMap();

		/* populate map structure prior to call to EncodeMapInit */
		/* NOTE: the key names used for this example may not correspond to actual name values */

		/* indicate that summary data and a total count hint will be encoded  */
		map.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_TOTAL_COUNT_HINT);
		/* populate maps keyPrimitiveType and containerType */
		map.containerType(DataTypes.FIELD_LIST);
		map.keyPrimitiveType(DataTypes.UINT);
		/* populate total count hint with approximate expected entry count */
		map.totalCountHint(3);

		/* begin encoding of map - assumes that (*encIter) is already populated with
		   buffer and version information, store return value to determine success or failure */
		/* expect summary data of approx. 256 bytes, no set definition data */
		if ((retVal = map.encodeInit(encIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			/* error condition - switch our success value to false so we can roll back */
			success = false;
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeMapInit.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
		}
		else
		{
			/* mapInit encoding was successful */
			/* create a single MapEntry and FieldList and reuse for each entry */
			MapEntry mapEntry = CodecFactory.createMapEntry();
			UInt entryKeyUInt = CodecFactory.createUInt();

			/* create a Buffer for uInt and field list to encode into */
			/* Typically, for performance, the transport layer can provide a pool of buffers
			 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
			/* For this example I am heap allocating the buffers (10 bytes is large enough for
			 * UInt encoding and 500 bytes is large enough for FieldList encoding). */
			Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.printf("\tMap Header Encoded\n");

			/* encode expected summary data, init for this was done by EncodeMapInit
		    - this type should match Map.containerType */
			{
				System.out.printf("\tEncoding Summary Data\n");

				/* now encode nested container using its own specific encode methods */
				/* begin encoding of field list - using same encIterator as map list */
				if ((retVal = fieldListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = false;
					System.out.printf("Error encoding field list.\n");
					System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				}

				System.out.printf("\n\tSummary Data Encoding Complete\n");
			
			}
			/* complete encoding of summary data.  If any field list encoding failed, success is false */
			if ((retVal = map.encodeSummaryDataComplete(encIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with EncodeMapSummaryDataComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			/* FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown */
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyUInt.value(1);
			if ((retVal = mapEntry.encodeInit(encIter, entryKeyUInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with EncodeMapEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}
			else
			/* encode contained field list - this type should match Map.containerType */
			{
				System.out.printf("\tEncoding Map Entry (key: %d) from non pre-encoded data and key\n", entryKeyUInt.toLong());

				/* now encode nested container using its own specific encode methods */
				/* clear, then begin encoding of field list - using same encIterator as map */
				
				if ((retVal = fieldListCodec.exampleEncode(encIter)) < CodecReturnCodes.SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = false;
					System.out.printf("Error encoding field list.\n");
					System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with EncodeMapEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			System.out.printf("\n\tMap Entry (key: %d) Encoding Complete\n", entryKeyUInt.toLong());

			
			/* SECOND Map Entry: encode entry from pre-encoded buffer containing an encoded FieldList */
			/* because we are re-populating all values on MapEntry, there is no need to clear it */

			mapEntry.action(MapEntryActions.ADD);
			entryKeyUInt.value(2);

			System.out.printf("\tEncoding Map Entry (key: 2) from pre-encoded buffer\n");

			/* assuming encUInt Buffer contains the pre-encoded key with length and data properly populated */
			if ((retVal = fieldListCodec.getPreEncodedUIntBuffer(encUInt, entryKeyUInt)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with getPreEncodedUIntBuffer.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			/* assuming encFieldList Buffer contains the pre-encoded payload with data and length populated */
			if ((retVal = fieldListCodec.getPreEncodedFieldListBuffer(encFieldList)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with getPreEncodedFieldListBuffer.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			// set pre-encoded data on map entry
			mapEntry.encodedData().data(encFieldList.data(), encFieldList.position(), encFieldList.length());

			/* pre-encoded key */
			mapEntry.encodedKey(encUInt);
			if ((retVal = mapEntry.encode(encIter)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with EncodeMapEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			System.out.printf("\n\tMap Entry (key: %d) Encoding Complete\n", entryKeyUInt.toLong());


			/* THIRD Map Entry: encode entry with delete action.  Delete actions have no payload */
			/* need to ensure that MapEntry is appropriatley cleared
			 * - clearing will ensure that encodedData and encodedKey are properly emptied */          
			mapEntry.clear();

			mapEntry.action(MapEntryActions.DELETE);
			entryKeyUInt.value(3);

			System.out.printf("\tEncoding Map Entry (key: %d) with delete action with no payload\n", entryKeyUInt.toLong());

			/* entryKeyUInt parameter is passed in as pointer to key primitive value.  encodedData is empty for delete */
			if ((retVal = mapEntry.encode(encIter, entryKeyUInt)) < CodecReturnCodes.SUCCESS) 
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				System.out.printf("Error %s (%d) encountered with EncodeMapEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}

			System.out.printf("\tMap Entry (key: %d) Encoding Complete\n", entryKeyUInt.toLong());

		}

		/* complete map encoding.  If success parameter is true, this will finalize encoding.  
		   If success parameter is false, this will roll back encoding prior to EncodeMapInit */
		if ((retVal = map.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.printf("Error %s (%d) encountered with EncodeMapEntry.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		System.out.printf("\tMap Encoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
	
	int exampleDecode(DecodeIterator decIter)
	{
		FieldListCodec fieldListCodec = new FieldListCodec(); // the field list codec
		
		/* used to store and check return codes */
		int retVal;

		/* create our map to decode into */
		Map map = CodecFactory.createMap();

		/* decode contents into the map structure */
		/* decode our map header */
		if ((retVal = map.decode(decIter)) >= CodecReturnCodes.SUCCESS)
		{
			/* create primitive value to have key decoded into and a single map entry to reuse */
			UInt uInt = CodecFactory.createUInt();
			MapEntry mapEntry = CodecFactory.createMapEntry();

			System.out.printf("\tMap Header Decoded: TotalCountHint: %d\n", map.totalCountHint());

			/* if summary data is present, invoking decoder for that type (instead of DecodeEntry)
			indicates to ETA that user wants to decode summary data */
			if (map.checkHasSummaryData())
			{
				/* summary data is present.  Its type should be that of Map.containerType */

				System.out.printf("\tDecoding Summary Data\n");

				/* Continue decoding field entries.  See example in Section 11.3.1 */
				/* decode the field list */
				if ((retVal = fieldListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.printf("Error decoding nested field list.\n");
					System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
					return retVal;
				}

				System.out.printf("\tSummary Data Decoding Complete\n");
			}

			/* decode each map entry, passing in pointer to keyPrimitiveType decodes mapEntry key as well  */
			while ((retVal = mapEntry.decode(decIter, uInt)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (retVal < CodecReturnCodes.SUCCESS)
				{
					/* decoding failure tends to be unrecoverable */
					System.out.printf("Error %s (%d) encountered with DecodeMapEntry.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
					return retVal;
				}
				else
				{
					/* Continue decoding field entries. */

					System.out.printf("\tDecoding Map Entry (key: %d)\n", uInt.toLong());

					/* Delete actions have no payload and we need to avoid decoding FieldList for this situation */
					if (mapEntry.action() != MapEntryActions.DELETE)
					{
						if ((retVal = fieldListCodec.exampleDecode(decIter)) < CodecReturnCodes.SUCCESS)
						{
							System.out.printf("Error decoding nested field list.\n");
							System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
								CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
							return retVal;
						}
					}
					else
					{
						System.out.printf("\tMap Entry (key: %d) delete actions have no payload to decode\n", uInt.toLong());
					}

					System.out.printf("\tMap Entry (key: %d) Decoding Complete\n", uInt.toLong());
				}
			}
		}
		else
		{
			/* decoding failure tends to be unrecoverable */
			System.out.printf("Error %s (%d) encountered with DecodeMap.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
			return retVal;
		}

		System.out.printf("\tMap Decoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}
}
