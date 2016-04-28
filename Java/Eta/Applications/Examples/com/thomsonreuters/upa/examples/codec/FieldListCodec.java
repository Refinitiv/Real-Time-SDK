package com.thomsonreuters.upa.examples.codec;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Enum;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.FieldListFlags;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.Float;
import com.thomsonreuters.upa.codec.Double;

/**
 * This is used for encoding and decoding a FieldList Container Type.
*/
public class FieldListCodec
{
	/* These are user defined FIDs to be used in the example so that we can show types */
	/* that are not in the standard dictionary. User defined FIDs are always negative  */
	private final int FID_INT		= -1;
	private final int FID_FLOAT		= -2;
	private final int FID_DOUBLE	= -3;
	private final int FID_DATETIME	= -4;
	private final int FID_QOS		= -5;
	private final int FID_STATE		= -6;
	private final int FID_BUFFER	= -7;
	private final int FID_ENUM		= -8;

	/* this method will encode a basic field list with several primitives
	   embedded in it */
	int exampleEncode(EncodeIterator encIter)
	{
		/* used to store and check return values */
		int retVal;

		/* create and initialize field list structure */
		FieldList fieldList = CodecFactory.createFieldList();

		/* populate field list structure prior to call to EncodeFieldListInit */
		/* NOTE: the fieldId, dictionaryId and fieldListNum values used for this example 
		* do not correspond to actual id values */

		/* indicate that standard data will be encoded and that dictionaryId and fieldListNum are included */
		fieldList.flags(FieldListFlags.HAS_STANDARD_DATA | FieldListFlags.HAS_FIELD_LIST_INFO);
		/* populate dictionaryId and fieldListNum with info needed to cross-reference fieldIds and cache */
		fieldList.dictionaryId(2); 
		fieldList.fieldListNum(5);

		/* begin encoding of field list - assumes that (*encIter) is already populated with
		   buffer and version information */

		/* Please note: here for simplicity, we did not use success parameter for EncodeFieldListInit/EncodeFieldListComplete calls. 
		   We are just simply displaying an error if it occurs and exit, thus true is used in replacement for success parameter */

		if ((retVal = fieldList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
		{
			/* print out message with return value string, value, and text */
			System.out.printf("Error %s (%d) encountered with EncodeFieldListInit.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		else
		{
			/* fieldListInit encoding was successful */
			/* create a single FieldEntry and reuse for each entry */
			FieldEntry fieldEntry = CodecFactory.createFieldEntry();

			boolean success = true;

			/* stack allocate a date and populate {day, month, year} */
			Date date = CodecFactory.createDate();
			date.month(11);
			date.day(30);
			date.year(2010);
			Real real = CodecFactory.createReal();
			Time time = CodecFactory.createTime();
			DateTime dateTime = CodecFactory.createDateTime();
			Array array = CodecFactory.createArray();
			Qos qos = CodecFactory.createQos();
			State state = CodecFactory.createState();
			Enum Enum = CodecFactory.createEnum();
			Buffer buffer = CodecFactory.createBuffer();
		
			UInt uInt = CodecFactory.createUInt();
			uInt.value(23456);
			Int Int = CodecFactory.createInt();
			Int.value(65432);
			Float Float = CodecFactory.createFloat();
			Float.value(3.14f);
			Double Double = CodecFactory.createDouble();
			Double.value(3.1416);

			/* create a Buffer for uInt to encode into */
			/* Typically, for performance, the transport layer can provide a pool of buffers
			 * for use and reuse that avoids the constant allocation/garbage collection penalty. */
			/* For this example I am heap allocating the buffer (10 bytes is large enough for
			 * UInt encoding). */
			Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			
			System.out.printf("\tFieldList Header Encoded\n");
		
			/* FIRST Field Entry: encode entry from the Date primitive type */
			/* populate and encode field entry with fieldId and dataType information for this field */
			fieldEntry.fieldId(16); 
			fieldEntry.dataType(DataTypes.DATE);
			if ((retVal = fieldEntry.encode(encIter, date)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Date: %d-%d-%d\n", fieldEntry.fieldId(), date.month(), date.day(), date.year());

			/* SECOND Field Entry: encode entry from the UInt primitive type */
			fieldEntry.fieldId(1080); 
			fieldEntry.dataType(DataTypes.UINT);
			if ((retVal = fieldEntry.encode(encIter, uInt)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Unsigned Integer: %d\n", fieldEntry.fieldId(), uInt.toLong());

			/* THIRD Field Entry: encode entry from preencoded buffer containing an encoded UInt type */
			/* populate and encode field entry with fieldId and dataType information for this field */
			/* because we are re-populating all values on FieldEntry, there is no need to clear it */
			fieldEntry.fieldId(1081); 
			fieldEntry.dataType(DataTypes.UINT);

			/* assuming encUInt is a Buffer with length and data properly populated */
			if ((retVal = getPreEncodedUIntBuffer(encUInt, uInt)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with getPreEncodedUIntBuffer.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			// pre-encoded data
			fieldEntry.encodedData(encUInt);
			if ((retVal = fieldEntry.encode(encIter)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Unsigned Integer: from preencoded buffer\n", fieldEntry.fieldId());

			/* FOURTH Field Entry: encode entry as a blank Real primitive type */
			/* populate and encode field entry with fieldId and dataType information for this field */
			/* need to ensure that FieldEntry is appropriatley cleared
			 * - clearing will ensure that encodedData is properly emptied */          
			fieldEntry.clear();

			fieldEntry.fieldId(22); 
			fieldEntry.dataType(DataTypes.REAL);
			/* void* parameter is passed in as null and encodedData is empty due to clearing */
			if ((retVal = fieldEntry.encodeBlank(encIter)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Real as blank.\n", fieldEntry.fieldId());

			/* FIFTH Field Entry: encode entry for a Real primitive type */
			fieldEntry.fieldId(24); 
			fieldEntry.dataType(DataTypes.REAL); 
			real.value(227, RealHints.EXPONENT_2);
			if ((retVal = fieldEntry.encode(encIter, real)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Real: hint: %d  value: %d\n", fieldEntry.fieldId(), real.hint(), real.toLong());

			/* SIXTH Field Entry: encode entry for another Real primitive type */
			fieldEntry.fieldId(25); 
			fieldEntry.dataType(DataTypes.REAL);  
			real.value(22801, RealHints.EXPONENT_4);
			if ((retVal = fieldEntry.encode(encIter, real)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Real: hint: %d  value: %d\n", fieldEntry.fieldId(), real.hint(), real.toLong());

			/* SEVENTH Field Entry: encode entry for another Time primitive type */
			fieldEntry.fieldId(18); 
			fieldEntry.dataType(DataTypes.TIME);  
			time.hour(8);
			time.minute(39);
			time.second(24);
			if ((retVal = fieldEntry.encode(encIter, time)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			System.out.printf("\t\tFID %d  Encoded Time: %d:%d:%d\n", fieldEntry.fieldId(), time.hour(), time.minute(), time.second());

			/* EIGHTH Field Entry: encode entry from the Int primitive type */
			fieldEntry.fieldId(FID_INT); 
			fieldEntry.dataType(DataTypes.INT);
			if ((retVal = fieldEntry.encode(encIter, Int)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded signed Integer: %d\n", fieldEntry.fieldId(), Int.toLong());

			/* NINETH Field Entry: encode entry from the Float primitive type */
			fieldEntry.fieldId(FID_FLOAT); 
			fieldEntry.dataType(DataTypes.FLOAT);
			if ((retVal = fieldEntry.encode(encIter, Float)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded float: %f\n", fieldEntry.fieldId(), Float.toFloat());

			/* TENTH Field Entry: encode entry from the Double primitive type */
			fieldEntry.fieldId(FID_DOUBLE); 
			fieldEntry.dataType(DataTypes.DOUBLE);
			if ((retVal = fieldEntry.encode(encIter, Double)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Double: %f\n", fieldEntry.fieldId(), Double.toDouble());

			/* ELEVENTH Field Entry: encode entry from the DateTime primitive type */
			fieldEntry.fieldId(FID_DATETIME); 
			fieldEntry.dataType(DataTypes.DATETIME);
			dateTime.month(11);
			dateTime.day(15);
			dateTime.year(2011);
			dateTime.hour(8);
			dateTime.minute(39);
			dateTime.second(24);
			if ((retVal = fieldEntry.encode(encIter, dateTime)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded DateTime: %d-%d-%d %d:%d:%d\n", fieldEntry.fieldId(), 
				dateTime.month(), dateTime.day(), dateTime.year(), dateTime.hour(), dateTime.minute(), dateTime.second());

			/* TWELVETH Field Entry: encode entry from the Qos primitive type */
			fieldEntry.fieldId(FID_QOS); 
			fieldEntry.dataType(DataTypes.QOS);
			qos.timeliness(QosTimeliness.REALTIME);
			qos.rate(QosRates.TICK_BY_TICK);
			qos.dynamic(true);
			qos.rateInfo(0);
			qos.timeInfo(0);
			if ((retVal = fieldEntry.encode(encIter, qos)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded QOS: %d %d %d %d %d\n", fieldEntry.fieldId(), qos.timeliness(), qos.rate(), (qos.isDynamic() ? 1 : 0), qos.rateInfo(), qos.timeInfo());

			/* THIRTEENTH Field Entry: encode entry from the State primitive type */
			fieldEntry.fieldId(FID_STATE); 
			fieldEntry.dataType(DataTypes.STATE);
			state.streamState(StreamStates.OPEN);
			state.dataState(DataStates.OK);
			state.code(StateCodes.NONE);
			state.text().data("Succeeded");
			if ((retVal = fieldEntry.encode(encIter, state)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded State: %d %d %d %s\n", fieldEntry.fieldId(), state.streamState(), state.dataState(), state.code(), state.text().toString());

			/* FOURTEENTH Field Entry: encode entry from the Buffer primitive type */
			fieldEntry.fieldId(FID_BUFFER); 
			fieldEntry.dataType(DataTypes.BUFFER);
			buffer.data("BUFFEREXAMPLE");
			if ((retVal = fieldEntry.encode(encIter, buffer)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Buffer: %s\n", fieldEntry.fieldId(), buffer.toString());

			/* FIFTEENTH Field Entry: encode entry from the Enum primitive type */
			fieldEntry.fieldId(FID_ENUM); 
			fieldEntry.dataType(DataTypes.ENUM);
			Enum.value(999);
			if ((retVal = fieldEntry.encode(encIter, Enum)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntry.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			System.out.printf("\t\tFID %d  Encoded Enum: %d\n", fieldEntry.fieldId(), Enum.toInt());

			/* SIXTEENTH Field Entry: encode entry as a complex type, Array primitive */
			/* populate and encode field entry with fieldId and dataType information for this field */
			/* need to ensure that FieldEntry is appropriately cleared
			 * - clearing will ensure that encodedData is properly emptied */          
			fieldEntry.clear();
			fieldEntry.fieldId(1021); 
			fieldEntry.dataType(DataTypes.ARRAY);
			/* begin complex field entry encoding, we are not sure of the approximate max encoding length */
			if ((retVal = fieldEntry.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
			{
				/* error condition - switch our success value to false so we can roll back */
				success = false;
				/* print out message with return value string, value, and text */
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntryInit.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			}
			else
			{
				/* now encode nested container using its own specific encode methods */
				/* encode Real values into the array */
				array.primitiveType(DataTypes.UINT);
				/* values are variable length */
				array.itemLength(2);
				/* begin encoding of array - using same encIterator as field list */
				if ((retVal = array.encodeInit(encIter)) < CodecReturnCodes.SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = false;
					/* print out message with return value string, value, and text */
					System.out.printf("Error %s (%d) encountered with EncodeArrayInit.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				}
				else
				{
					/*----- Continue encoding array entries. ---- */
					UInt uInt1 = CodecFactory.createUInt();
					uInt1.value(10);
					UInt uInt2 = CodecFactory.createUInt();
					uInt2.value(20);
					UInt uInt3 = CodecFactory.createUInt();
					uInt3.value(30);
					UInt uInt4 = CodecFactory.createUInt();
					uInt4.value(40);

					/* array encoding was successful */

					System.out.printf("\t\tFID %d Encoded Array: [", fieldEntry.fieldId());
					ArrayEntry ae = CodecFactory.createArrayEntry();

					/* encode first entry from a UInt from a primitive type */
					if ((retVal = ae.encode(encIter, uInt1)) < CodecReturnCodes.SUCCESS)
					{
						/* error condition - switch our success value to false so we can roll back */
						success = false;
						System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
					}
					System.out.printf(" %d ", uInt1.toLong());

					/* encode second entry from a UInt from a primitive type */
					if ((retVal = ae.encode(encIter, uInt2)) < CodecReturnCodes.SUCCESS)
					{
						/* error condition - switch our success value to false so we can roll back */
						success = false;
						System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
					}
					System.out.printf(" %d ", uInt2.toLong());

					/* encode third entry from a UInt from a primitive type */
					if ((retVal = ae.encode(encIter, uInt3)) < CodecReturnCodes.SUCCESS)
					{
						/* error condition - switch our success value to false so we can roll back */
						success = false;
						System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
					}
					System.out.printf(" %d ", uInt3.toLong());

					/* encode forth entry from a UInt from a primitive type */
					if ((retVal = ae.encode(encIter, uInt4)) < CodecReturnCodes.SUCCESS)
					{
						/* error condition - switch our success value to false so we can roll back */
						success = false;
						System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); ;
					}
					System.out.printf(" %d ", uInt4.toLong());

					/* encode fifth entry from a UInt from pre-encoded integer contained in a buffer */
					/* this buffer.data should point to encoded int and the length should be number of bytes encoded */
					ae.encodedData(encUInt);
					if ((retVal = ae.encode(encIter)) < CodecReturnCodes.SUCCESS)
					{
						/* error condition - switch our success value to false so we can roll back */
						success = false;
						System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
					}
					System.out.printf(" <Preencoded> ]\n");
						
				}

				/* complete array encoding.  If success parameter is true, this will finalize encoding.  
				   If success parameter is false, this will roll back encoding prior to EncodeArrayInit */
				if ((retVal = array.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
				{
					/* error condition - switch our success value to false so we can roll back */
					success = false;
					System.out.printf("Error %s (%d) encountered with EncodeArrayComplete.  Error Text: %s\n", 
						CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				}

			}
			/* complete encoding of complex field entry.  If any array encoding failed, success is false */
			if ((retVal = fieldEntry.encodeComplete(encIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with EncodeFieldEntryComplete.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
				return retVal;
			}

		}

		/* complete fieldList encoding.  If success parameter is true, this will finalize encoding.  
		   If success parameter is false, this will roll back encoding prior to EncodeFieldListInit */
		
		/* Please note: here for simplicity, we did not use success parameter for EncodeFieldListInit/EncodeFieldListComplete calls. 
		   We are just simply displaying an error if it occurs and exit, thus true is used in replacement for success parameter */

		if ((retVal = fieldList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeFieldListComplete.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.printf("\tFieldList Encoding Complete");

		return CodecReturnCodes.SUCCESS;
	}
	
	/* this method will decode a basic field list with several primitives embedded in it */
	int exampleDecode(DecodeIterator decIter)
	{
		/* used to store and check return codes */
		int retVal;

		/* create a field list to decode into - 
		   since set data is not supported yet, there is nothing to put 
		   here except the count of the number of fields encoded in this 
		   message*/
		FieldList fieldList = CodecFactory.createFieldList();;

		/* create field entry to decode into */
		FieldEntry fieldEntry = CodecFactory.createFieldEntry();

		/* create our primitives we will decode into */
		Date date = CodecFactory.createDate();
		Real real = CodecFactory.createReal();
		Time time = CodecFactory.createTime();
		Array array = CodecFactory.createArray();
		DateTime DateTime = CodecFactory.createDateTime();
		Qos qos = CodecFactory.createQos();
		State state = CodecFactory.createState();
		Enum Enum = CodecFactory.createEnum();
		Buffer buffer = CodecFactory.createBuffer();

		UInt uInt = CodecFactory.createUInt();
		Int Int = CodecFactory.createInt();
		Float Float = CodecFactory.createFloat();
		Double Double = CodecFactory.createDouble();

		/* decode into the field list structure */
		/* decode the field list header */
		if ((retVal = fieldList.decode(decIter, null)) >= CodecReturnCodes.SUCCESS)
		{
			System.out.printf("\tField List Header Decoded\n");

			/* decode each field entry  */
			/* since this succeeded, we can decode fields until we
			reach the end of the fields - until CodecReturnCodes.END_OF_CONTAINER is returned */
			while ((retVal = fieldEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (retVal < CodecReturnCodes.SUCCESS)
				{
					/* decoding failure tends to be unrecoverable */
					System.out.printf("Error %s (%d) encountered with DecodeFieldEntry.  Error Text: %s\n", 
							CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
					return retVal;
				}
				else
				{
					/* A UPA application will typically use a field dictionary to decode field list content.  
					When using, code similar to the following can be used to look up type in field dictionary 
					and call correct primitive decode method */ 

					/*switch (fieldDict.entry(fEntry.fieldId()).rwfType())
					{	
							case DataTypes.REAL:
								Real.decode(decIter);
							break;
							case DataTypes.DATE:
								Date.decode(decIter);
							break;
							/* full switch statement omitted to shorten sample code * /

					}*/

					/* decode our entry - this is typically based on the FID and what type 
					   it is as designated by the field dictionary.  Because we know
					   the FIDs we used to encode, we can handle them here without
		               looking up in the dictionary */

					/* FID encodings: 
					   FID 16 is Date
					   FID 1080 is UInt
					   FID 1081 is Unit
					   FID 22 is Real
			           FID 24 is Real
					   FID 25 is Real
					   FID 18 is Time
					   FID 1021 is Array
					   negative FIDs are user defined FIDs
		            */

					switch (fieldEntry.fieldId())
					{
						case 16: //Date
						{
							if ((retVal = date.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding Date.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Date Decoded: %d-%d-%d\n", fieldEntry.fieldId(), date.month(), date.day(), date.year());
						}
						break;

						case 22:
						case 24:
						case 25: //Real
						{
							if ((retVal = real.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding Real.\n");
								return retVal;
							}

							/* Handle blank case.  This should be done across all types, however in this example
							 * we know the only field sent as blank is FID 22 */
							if (retVal == CodecReturnCodes.BLANK_DATA)
								System.out.printf("\t\tFID %d  Real Decoded as blank.\n", fieldEntry.fieldId());
							else
								System.out.printf("\t\tFID %d  Real Decoded: hint: %d  value: %d\n", fieldEntry.fieldId(), real.hint(), real.toLong());
						}
						break;

						case 1080:
						case 1081: //UInt
						{
							if ((retVal = uInt.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding  unsigned integer.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d Unsigned Integer Decoded: %d\n", fieldEntry.fieldId(), uInt.toLong());
						}
						break;

						case 18: //Time
						{
							if ((retVal = time.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding Time.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Time Decoded: %d:%d:%d\n", fieldEntry.fieldId(), time.hour(), time.minute(), time.second());
						}
						break;

						/* full switch statement omitted to shorten sample code */

						case 1021: //Array
						{
							/* decode into the array structure header */
							if ((retVal = array.decode(decIter)) >= CodecReturnCodes.SUCCESS)
							{
							/* decode each array entry  */
								ArrayEntry ae = CodecFactory.createArrayEntry();
								System.out.printf("\t\tFID %d Array Decoded: [", fieldEntry.fieldId());
								while ((retVal = ae.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
								{
									if (retVal < CodecReturnCodes.SUCCESS)
									{
										/* decoding failure tends to be unrecoverable */
										System.out.printf("Error %s (%d) encountered with DecodeArrayEntry.  Error Text: %s\n", 
												CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
										return retVal;
									}
									else
									{
										/* decode array entry into primitive type */
										/* we can use the same decode iterator, or set the encoded
										   entry buffer onto a new iterator */
										if ((retVal = uInt.decode(decIter)) < CodecReturnCodes.SUCCESS)
										{
											System.out.printf("Error decoding  unsigned integer.\n");
											return retVal;
										}

										System.out.printf(" %d ", uInt.toLong());
									}
								}
								System.out.printf("]\n");
							}
							else
							{
								/* decoding failure tends to be unrecoverable */
								System.out.printf("Error %s (%d) encountered with DecodeArray.  Error Text: %s\n", 
									CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
								return retVal;
							}

						}
						break;

						case FID_INT:
						{
							if ((retVal = Int.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding  signed integer.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d signed Integer Decoded: %d\n", fieldEntry.fieldId(), Int.toLong());
						}
						break;

						case FID_FLOAT:
						{
							if ((retVal = Float.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding  float.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d float Decoded: %f\n", fieldEntry.fieldId(), Float.toFloat());
						}
						break;

						case FID_DOUBLE:
						{
							if ((retVal = Double.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding  double.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d double Decoded: %f\n", fieldEntry.fieldId(), Double.toDouble());
						}
						break;

						case FID_DATETIME:
						{
							if ((retVal = DateTime.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding DateTime.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  DateTime Decoded: %d-%d-%d %d:%d:%d\n", fieldEntry.fieldId(), 
										DateTime.month(), DateTime.day(), DateTime.year(), 
										DateTime.hour(), DateTime.minute(), DateTime.second());
						}
						break;
						

						case FID_QOS:
						{
							if ((retVal = qos.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding QOS.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Decoded QOS: %d %d %d %d %d\n", fieldEntry.fieldId(), qos.timeliness(), qos.rate(), (qos.isDynamic() ? 1 : 0), qos.rateInfo(), qos.timeInfo());
						}
						break;

						case FID_STATE:
						{
							if ((retVal = state.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding State.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Decoded State: %d %d %d %s\n", fieldEntry.fieldId(), state.streamState(), state.dataState(), state.code(), state.text().toString());
						}
						break;

						case FID_BUFFER:
						{
							if ((retVal = buffer.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding Buffer.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Buffer Decoded: %s\n", fieldEntry.fieldId(), buffer.toString());	// printf assumes its a null terminated character string 
						}
						break;

						case FID_ENUM:
						{
							if ((retVal = Enum.decode(decIter)) < CodecReturnCodes.SUCCESS)
							{
								System.out.printf("Error decoding Enum.\n");
								return retVal;
							}

							System.out.printf("\t\tFID %d  Enum Decoded: %d\n", fieldEntry.fieldId(), Enum.toInt());
						}
						break;

						default:
							System.out.printf("\t\tUnexpected FID %d encountered!\n", fieldEntry.fieldId());
					}
				}
			}
		}
		else
		{
			/* decoding failure tends to be unrecoverable */
			System.out.printf("Error %s (%d) encountered with DecodeFieldList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal));
			return retVal;
		}

		System.out.printf("\tFieldList Decoding Complete\n");

		return CodecReturnCodes.SUCCESS;
	}

	/* this method returns a preencoded buffer containing an encoded UInt type */
	/* assuming encUInt is a Buffer with length and data properly populated */
	int getPreEncodedUIntBuffer(Buffer encUInt, UInt uInt)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encUInt.data() points to sufficient memory and encUInt.length()
		 * indicates number of bytes available in encUInt.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encUInt, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		if ((retVal = uInt.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with EncodeUInt.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	/* this method returns a preencoded buffer containing an encoded FieldList type */
	/* assuming encFieldList Buffer contains the pre-encoded payload with data and length populated */
	int getPreEncodedFieldListBuffer(Buffer encFieldList)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded

		/* create and clear iterator to prepare for encoding */
		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		/* associate buffer and iterator and set proper protocol version information on iterator */
		/* code assumes that encFieldList.data() points to sufficient memory and encFieldList.length()
		 * indicates number of bytes available in encFieldList.data() */
		/* protocol version information can typically be obtained from the Channel
		 * associated with the connection once it becomes active */
		if ((retVal = encodeIter.setBufferAndRWFVersion(encFieldList, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with setBufferAndRWFVersion.  Error Text: %s\n", 
					CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		if ((retVal = exampleEncode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.printf("Error %s (%d) encountered with exampleEncodeFieldList.  Error Text: %s\n", 
				CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
}
