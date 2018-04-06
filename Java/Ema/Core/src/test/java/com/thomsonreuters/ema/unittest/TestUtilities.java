///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.Data.DataCode;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.MapEntry.MapAction;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.ema.access.OmmError;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.SeriesEntry;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.ema.access.VectorEntry;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FieldListFlags;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MapFlags;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.UInt;

import junit.framework.TestCase;


final class TestUtilities extends TestCase
{
	
	private static String fieldDictionaryFileName = "./src/test/resources/com/thomsonreuters/ema/unittest/DataDictionaryTest/RDMTestDictionary";
	private static String enumTableFileName = "./src/test/resources/com/thomsonreuters/ema/unittest/DataDictionaryTest/testenumtype.def";
	
	static class EncodingTypeFlags
	{
		static final int PRIMITIVE_TYPES = 1;

		static final int CONTAINER_TYPES = 2;

		static final int MESSAGE_TYPES = 4;
	}
	
	// These are user defined FIDs to be used so that we can show types that are not in the standard dictionary.
	// User defined FIDs are always negative.
	final static int XML			= -1;
	final static int INTEGER		= -2;
	final static int TRADE_DATETIME	= -3;
	final static int MY_QOS		= -5;
	final static int MY_STATE	= -6;
	final static int MY_FLOAT	= -9;
	final static int MY_DOUBLE	= -10;
	final static int MY_BUFFER	= -11;
	final static int MY_UTF8		= -12;
	final static int MY_ARRAY	= -16;
	final static int MY_FIELDLIST = -13;
	final static int MY_MAP = -14;
	final static int MY_ELEMENTLIST = -15;
	final static int MY_MSG = -19;
	final static int MY_FILTERLIST = -20;
	final static int MY_SERIES = -21;
	final static int MY_VECTOR = -22;

	final static int KEY_INT = 11;
	final static String KEY_STRING = "KEY_STRING"; 
	final static String KEY_ASCII = "KEY_ASCII";
	
	private static StringBuilder _strBuilder = new StringBuilder();
	@SuppressWarnings("unused")
    private static int _passNum = 0;
	@SuppressWarnings("unused")
    private static int _failNum = 0;
	private static com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
	
	static
	{
		TestUtilities.upa_encodeDictionaryMsg(dictionary);
	}
	
	private TestUtilities()
	{
		throw new AssertionError();
	}
	
	static com.thomsonreuters.upa.codec.DataDictionary getDataDictionary()
	{
		return dictionary;
	}
	
	static void printTestHead(String title, String detail)
	{
		_strBuilder.setLength(0);
		_strBuilder.append("\n\n" + "******************************************************************************" + "\n" + "Title: " + title + "\n" + "Detail: " + detail + "\n");
		System.out.println(_strBuilder.toString());
	}

	static void upa_EncodeErrorFieldList( Buffer rsslBuf )
	{
		com.thomsonreuters.upa.codec.FieldList rsslFL= CodecFactory.createFieldList();
		com.thomsonreuters.upa.codec.EncodeIterator iter = CodecFactory.createEncodeIterator();
		com.thomsonreuters.upa.codec.FieldEntry rsslFEntry = CodecFactory.createFieldEntry();
		
		iter.setBufferAndRWFVersion(rsslBuf, Codec.majorVersion(), Codec.minorVersion());
		rsslFL.flags(FieldListFlags.HAS_FIELD_LIST_INFO | FieldListFlags.HAS_STANDARD_DATA );
		rsslFL.dictionaryId(dictionary.infoDictionaryId());
		rsslFL.fieldListNum( 65);

		rsslFL.encodeInit(iter, null, 0);

		// fid not found case (first)
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.fieldId( -100);
		UInt uint64 = CodecFactory.createUInt();
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );
		
		// corect fid found case (second)
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.fieldId( 1);
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// incorrect data type fid longer than expected (third)
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 1);
		Real real = CodecFactory.createReal();
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );
		
		// correct data type fid (fourth)
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 6);
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real);

		// incorrect data type fid shorter than expected (fifth)
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.fieldId( 6);
		uint64.value( 67);
		rsslFEntry.encode( iter, uint64 );

		// correct data type fid
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 6);
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );

		rsslFL.encodeComplete(iter, true);
	}

	static void upa_EncodeErrorElementList( Buffer rsslBuf )
	{
		com.thomsonreuters.upa.codec.ElementList rsslFL= CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.EncodeIterator iter = CodecFactory.createEncodeIterator();
		com.thomsonreuters.upa.codec.ElementEntry rsslFEntry = CodecFactory.createElementEntry();
        Buffer badDataBuffer = CodecFactory.createBuffer();
		
		badDataBuffer.data(ByteBuffer.allocate(1));
        badDataBuffer.data().put(0, (byte)0xff);

		iter.setBufferAndRWFVersion(rsslBuf, Codec.majorVersion(), Codec.minorVersion());
		rsslFL.flags(ElementListFlags.HAS_ELEMENT_LIST_INFO | ElementListFlags.HAS_STANDARD_DATA );
		rsslFL.elementListNum( 65);

		rsslFL.encodeInit(iter, null, 0);

		// first entry correct
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT");
		UInt uint64 = CodecFactory.createUInt();
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );
		
		// second entry correct
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT2");
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// bad data
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL");
		rsslFEntry.encodedData(badDataBuffer);
		rsslFEntry.encode( iter);
		
		// fourth entry correct
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT3");
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// bad data
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL2");
		rsslFEntry.encodedData(badDataBuffer);
		rsslFEntry.encode( iter);

		// sixth entry correct
		rsslFEntry.dataType( com.thomsonreuters.upa.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL3");
		Real real = CodecFactory.createReal();
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );

		rsslFL.encodeComplete(iter, true);
	}
	
	// Encode (with UPA) a basic field list with several primitives embedded in it
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeFieldListAll(com.thomsonreuters.upa.codec.Buffer upaBuf, int encodeOption)
	{
		// used to store and check return values
		int retVal;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		if ((retVal = upa_EncodeFieldListAll(encodeIter, encodeOption)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with upa_EncodeFieldListAll(encodeIter, dictionary). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	private static int upa_EncodeFieldListAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int encodeFlag)
	{
		int retVal;
		
		// create and initialize field list structure
		com.thomsonreuters.upa.codec.FieldList upaFieldList = CodecFactory.createFieldList();

		// populate field list structure prior to call to EncodeFieldListInit
		// NOTE: some of the fieldId, dictionaryId and fieldListNum values used here do not correspond to actual id values

		// indicate that standard data will be encoded and that dictionaryId and fieldListNum are included
		upaFieldList.flags(FieldListFlags.HAS_STANDARD_DATA | FieldListFlags.HAS_FIELD_LIST_INFO);
		// populate dictionaryId and fieldListNum with info needed to cross-reference fieldIds and cache
		upaFieldList.dictionaryId(dictionary.infoDictionaryId()); 
		upaFieldList.fieldListNum(65);

		// begin encoding of field list - assumes that encodeIter is already populated with buffer and version information
		if ((retVal = upaFieldList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
		{
			// print out message with return value string, value, and text
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldListInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		else
		{
			// fieldListInit encoding was successful
			// create a single FieldEntry and reuse for each entry
			com.thomsonreuters.upa.codec.FieldEntry fieldEntry = CodecFactory.createFieldEntry();

			boolean success = true;
			
			System.out.println("\tUPA FieldList Header Encoded");
			
		    if ( (encodeFlag & EncodingTypeFlags.PRIMITIVE_TYPES) != 0 )
		    {
				// create a date and populate {day, month, year}
				com.thomsonreuters.upa.codec.Date date = CodecFactory.createDate();
				date.month(11);
				date.day(30);
				date.year(2010);
				
				com.thomsonreuters.upa.codec.Real real = CodecFactory.createReal();
				com.thomsonreuters.upa.codec.Time time = CodecFactory.createTime();
				com.thomsonreuters.upa.codec.DateTime DATETIME = CodecFactory.createDateTime();
				com.thomsonreuters.upa.codec.Array array = CodecFactory.createArray();
				com.thomsonreuters.upa.codec.Qos qos = CodecFactory.createQos();
				com.thomsonreuters.upa.codec.State state = CodecFactory.createState();
				com.thomsonreuters.upa.codec.Enum enumValue  = CodecFactory.createEnum();
				com.thomsonreuters.upa.codec.Buffer buffer = CodecFactory.createBuffer();
			
				com.thomsonreuters.upa.codec.UInt uInt = CodecFactory.createUInt();
				uInt.value(23456);
				com.thomsonreuters.upa.codec.Int Int = CodecFactory.createInt();
				Int.value(65432);
				com.thomsonreuters.upa.codec.Float Float = CodecFactory.createFloat();
				Float.value(3.14f);
				com.thomsonreuters.upa.codec.Double Double = CodecFactory.createDouble();
				Double.value(3.1416);
	
				// Create a Buffer for UInt to encode into
				com.thomsonreuters.upa.codec.Buffer encUInt = CodecFactory.createBuffer();
				encUInt.data(ByteBuffer.allocate(10));
	
				// FIRST Field Entry
				// fid not found case (first)
				fieldEntry.fieldId(-100);
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.UINT);
				if ((retVal = fieldEntry.encode(encodeIter, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal));
					return retVal;
				}
	
				// SECOND Field Entry: encode entry from the Date primitive type
				// Populate and encode field entry with fieldId and dataType information for this field.
				fieldEntry.fieldId(16); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.DATE);
				if ((retVal = fieldEntry.encode(encodeIter, date)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Date: " + date.month() + "-" + date.day() + "-" + date.year());
	
				// THIRD Field Entry: encode entry from the UInt primitive type
				fieldEntry.fieldId(147); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.UINT);
				if ((retVal = fieldEntry.encode(encodeIter, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Unsigned Integer: " + uInt.toLong());
	
				// FOURTH Field Entry: encode entry from preencoded buffer containing an encoded UInt type
				// Populate and encode field entry with fieldId and dataType information for this field.
				// Because we are re-populating all values on FieldEntry, there is no need to clear it.
				fieldEntry.fieldId(1080); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.UINT);
	
				// assuming encUInt is a Buffer with length and data properly populated
				if ((retVal = upa_getPreEncodedUIntBuffer(encUInt, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_getPreEncodedUIntBuffer.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				// pre-encoded data
				fieldEntry.encodedData(encUInt);
				if ((retVal = fieldEntry.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
	
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Unsigned Integer: from preencoded buffer");
	
				// FIFTH Field Entry: encode entry as a blank Real primitive type
				// Populate and encode field entry with fieldId and dataType information for this field.
				// Need to ensure that FieldEntry is appropriately cleared.
				// - clearing will ensure that encodedData is properly emptied       
				fieldEntry.clear();
	
				fieldEntry.fieldId(22); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.REAL);
				if ((retVal = fieldEntry.encodeBlank(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real as blank.");
	
				// SIXTH Field Entry: encode entry for a Real primitive type
				fieldEntry.fieldId(24); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.REAL); 
				real.value(227, RealHints.EXPONENT_2);
				if ((retVal = fieldEntry.encode(encodeIter, real)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real: hint: " + real.hint() + " value: " + real.toLong());
	
				// SEVENTH Field Entry: encode entry for another Real primitive type
				fieldEntry.fieldId(25); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.REAL);  
				real.value(22801, RealHints.EXPONENT_4);
				if ((retVal = fieldEntry.encode(encodeIter, real)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real: hint: " + real.hint() + " value: " + real.toLong());
	
				// EIGHTH Field Entry: encode entry for another Time primitive type
				fieldEntry.fieldId(18); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.TIME);  
				time.hour(8);
				time.minute(39);
				time.second(24);
				if ((retVal = fieldEntry.encode(encodeIter, time)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Time: " + time.hour() + ":" + time.minute() + ":" + time.second());
	
				// NINETH Field Entry: encode entry from the Int primitive type
				fieldEntry.fieldId(INTEGER); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.INT);
				if ((retVal = fieldEntry.encode(encodeIter, Int)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded signed Integer: " + Int.toLong());
	
				// TENTH Field Entry: encode entry from the Float primitive type
				fieldEntry.fieldId(MY_FLOAT); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FLOAT);
				if ((retVal = fieldEntry.encode(encodeIter, Float)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID "+ fieldEntry.fieldId() + " Encoded float: " + Float.toFloat());
	
				// ELEVENTH Field Entry: encode entry from the Double primitive type
				fieldEntry.fieldId(MY_DOUBLE); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.DOUBLE);
				if ((retVal = fieldEntry.encode(encodeIter, Double)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Double: " + Double.toDouble());
	
				// TWELVETH Field Entry: encode entry from the DATETIME primitive type
				fieldEntry.fieldId(TRADE_DATETIME); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.DATETIME);
				DATETIME.month(11);
				DATETIME.day(15);
				DATETIME.year(2011);
				DATETIME.hour(8);
				DATETIME.minute(39);
				DATETIME.second(24);
				if ((retVal = fieldEntry.encode(encodeIter, DATETIME)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId()
						+ " Encoded DATETIME: " + DATETIME.month() + "-" + DATETIME.day() + "-" + DATETIME.year()
											+ " " + DATETIME.hour() + ":" + DATETIME.minute() + ":" + DATETIME.second());
	
				// THIRTEENTH Field Entry: encode entry from the Qos primitive type
				fieldEntry.fieldId(MY_QOS); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.QOS);
				qos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.REALTIME);
				qos.rate(com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK);
				qos.dynamic(true);
				qos.rateInfo(0);
				qos.timeInfo(0);
				if ((retVal = fieldEntry.encode(encodeIter, qos)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId()
							+ " Encoded QOS: " + qos.timeliness() + " " + qos.rate()
										+ " " + (qos.isDynamic() ? 1 : 0)
										+ " " + qos.rateInfo() + " " + qos.timeInfo());
	
				// FOURTEENTH Field Entry: encode entry from the State primitive type
				fieldEntry.fieldId(MY_STATE); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.STATE);
				state.streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
				state.dataState(com.thomsonreuters.upa.codec.DataStates.OK);
				state.code(com.thomsonreuters.upa.codec.StateCodes.NONE);
				state.text().data("Succeeded");
				if ((retVal = fieldEntry.encode(encodeIter, state)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded State: " + state.streamState() + " " + state.dataState() + " " + state.code() + " " + state.text().toString());
	
				// FIFTEENTH Field Entry: encode entry from the Buffer primitive type
				fieldEntry.fieldId(MY_BUFFER); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.BUFFER);
				buffer.data("ABCDEFGH");
				if ((retVal = fieldEntry.encode(encodeIter, buffer)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Buffer: " + buffer.toString());
	
				// SIXTEENTH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ENUM);
				enumValue.value(29);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
	
				// SEVENTEENTH Field Entry: encode entry as a complex type, Array primitive
				// Populate and encode field entry with fieldId and dataType information for this field.
				// Need to ensure that FieldEntry is appropriately cleared.
				// - clearing will ensure that encodedData is properly emptied          
				fieldEntry.clear();
				fieldEntry.fieldId(MY_ARRAY); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ARRAY);
				// begin complex field entry encoding, we are not sure of the approximate max encoding length
				if ((retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntryInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					// now encode nested container using its own specific encode methods
					// encode Real values into the array
					array.primitiveType(com.thomsonreuters.upa.codec.DataTypes.UINT);
					// values are variable length
					array.itemLength(2);
					// begin encoding of array - using same encIterator as field list
					if ((retVal = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						// print out message with return value string, value, and text
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						//----- Continue encoding array entries. ----
						com.thomsonreuters.upa.codec.UInt uInt1 = CodecFactory.createUInt();
						uInt1.value(10);
						com.thomsonreuters.upa.codec.UInt uInt2 = CodecFactory.createUInt();
						uInt2.value(20);
						com.thomsonreuters.upa.codec.UInt uInt3 = CodecFactory.createUInt();
						uInt3.value(30);
						com.thomsonreuters.upa.codec.UInt uInt4 = CodecFactory.createUInt();
						uInt4.value(40);
	
						// Array encoding was successful.
	
						System.out.print("\t\tFID " + fieldEntry.fieldId() + " Encoded Array: [");
						com.thomsonreuters.upa.codec.ArrayEntry ae = CodecFactory.createArrayEntry();
	
						// Encode first entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt1)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(uInt1.toLong());
	
						// Encode second entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt2)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(" " + uInt2.toLong());
	
						// Encode third entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt3)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(" " + uInt3.toLong());
	
						// Encode forth entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt4)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(" " + uInt4.toLong());
	
						// Encode fifth entry from a UInt from pre-encoded integer contained in a buffer.
						// This buffer.data should point to encoded int and the length should be number of bytes encoded.
						ae.encodedData(encUInt);
						if ((retVal = ae.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.println(" <Preencoded> ]");
							
					}
					
				}

				// Complete array encoding.
				// If success parameter is true, this will finalize encoding.  
				// If success parameter is false, this will roll back encoding prior to EncodeArrayInit
				if ((retVal = array.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntryComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				// EIGHTEENTH Field Entry: encode entry for the Enum type with blank value
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ENUM);
				if ((retVal = fieldEntry.encodeBlank(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + "Blank");
				
				// NINETEENTH Field Entry: encode entry for the Enum type with undefined display value
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ENUM);
				enumValue.value(2999);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
				
				// TWENTIETH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(115); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ENUM);
				enumValue.value(0);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
				
				// TWENTY FIFTH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(8960); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ENUM);
				enumValue.value(2);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
	
			}
			
			if ( (encodeFlag & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
			{
				fieldEntry.clear();
				fieldEntry.fieldId(MY_FIELDLIST); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_ELEMENTLIST); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_FILTERLIST); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_SERIES); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.SERIES);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_VECTOR); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.VECTOR);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MAP); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MAP);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
			}
			
			if ( (encodeFlag & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
			{
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeUpdateMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeStatusMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeStatusMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeGenericMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodePostMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeAckMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRequestMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
			}
		}

		// Complete fieldList encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeFieldListInit.
		if ((retVal = upaFieldList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldList.encodeCompelte().  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.println("\tUPA FieldList Encoding Complete");
		
		return CodecReturnCodes.SUCCESS;
	}
	
	// Encode (with UPA) a basic array with several primitives embedded in it
    // We pass in the buffer to this method with the total length available.
    static int upa_EncodeArrayAll(com.thomsonreuters.upa.codec.Buffer upaBuf)
    {
        // used to store and check return values
        int retVal;

        int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
        int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
    
        // Create and clear iterator to prepare for encoding
        com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
    
        // Associate buffer and iterator and set proper protocol version information on iterator.
        if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
                            + " Error Text: " + CodecReturnCodes.info(retVal)); 
            return retVal;
        }

        if ((retVal = upa_EncodeArrayAll(encodeIter)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with upa_EncodeFieldListAll(encodeIter, dictionary). "
                            + " Error Text: " + CodecReturnCodes.info(retVal)); 
            return retVal;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
	private static int upa_EncodeArrayAll(EncodeIterator encodeIter)
    {
        int retVal;
        boolean success = true;
        
        // create and initialize Array structure
        com.thomsonreuters.upa.codec.Array array = CodecFactory.createArray();
        
        /* encode Buffer values into the array */
        array.primitiveType(com.thomsonreuters.upa.codec.DataTypes.BUFFER);
        /* values are variable length */
        array.itemLength(8);
        /* begin encoding of array */
        if ((retVal = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
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
            Buffer buf1 = CodecFactory.createBuffer();
            buf1.data("BUFFER 1");
            Buffer buf2 = CodecFactory.createBuffer();
            buf2.data("BUFFER 2");
            Buffer buf3 = CodecFactory.createBuffer();
            buf3.data("BUFFER 3");
            Buffer buf4 = CodecFactory.createBuffer();
            buf4.data("BUFFER 4");

            /* array encoding was successful */
            ArrayEntry ae = CodecFactory.createArrayEntry();

            /* encode first entry from a Buffer from a primitive type */
            if ((retVal = ae.encode(encodeIter, buf1)) < CodecReturnCodes.SUCCESS)
            {
                /* error condition - switch our success value to false so we can roll back */
                success = false;
                System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
                    CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
            }

            /* encode second entry from a Buffer from a primitive type */
            if ((retVal = ae.encode(encodeIter, buf2)) < CodecReturnCodes.SUCCESS)
            {
                /* error condition - switch our success value to false so we can roll back */
                success = false;
                System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
                    CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
            }

            /* encode third entry from a Buffer from a primitive type */
            if ((retVal = ae.encode(encodeIter, buf3)) < CodecReturnCodes.SUCCESS)
            {
                /* error condition - switch our success value to false so we can roll back */
                success = false;
                System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
                    CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
            }

            /* encode forth entry from a Buffer from a primitive type */
            if ((retVal = ae.encode(encodeIter, buf4)) < CodecReturnCodes.SUCCESS)
            {
                /* error condition - switch our success value to false so we can roll back */
                success = false;
                System.out.printf("Error %s (%d) encountered with EncodeArrayEntry.  Error Text: %s\n", 
                    CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); ;
            }
        }

        /* complete array encoding.  If success parameter is true, this will finalize encoding.  
           If success parameter is false, this will roll back encoding prior to EncodeArrayInit */
        if ((retVal = array.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
        {
            /* error condition - switch our success value to false so we can roll back */
            success = false;
            System.out.printf("Error %s (%d) encountered with EncodeArrayComplete.  Error Text: %s\n", 
                CodecReturnCodes.toString(retVal), retVal, CodecReturnCodes.info(retVal)); 
        }

        return 0;
    }

    // Encode (with UPA) a basic element list with several primitives embedded in it
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeElementListAll(com.thomsonreuters.upa.codec.Buffer upaBuf, int encodingFlag)
	{
        // used to store and check return values
        int retVal;
        
		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ((retVal = upa_EncodeElementListAll(encodeIter, encodingFlag)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with upa_EncodeElementListAll(encodeIter). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

        return CodecReturnCodes.SUCCESS;
	}

	static int upa_EncodeElementListAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int encodeFlag)
	{
		int retVal;
		boolean success = true;
		
		// create a single ElementEntry and reuse for each entry
		com.thomsonreuters.upa.codec.ElementEntry elemEntry = CodecFactory.createElementEntry();
		
		// create and initialize element list structure
		com.thomsonreuters.upa.codec.ElementList upaElementList = CodecFactory.createElementList();
		
		// populate element list structure prior to call to EncodeElementListInit

		// indicate that standard data will be encoded and that elementListNum is included
		upaElementList.flags(ElementListFlags.HAS_STANDARD_DATA | ElementListFlags.HAS_ELEMENT_LIST_INFO);
		
		// populate elementListNum with info needed to cache
		upaElementList.elementListNum(7);
		
		// begin encoding of element list - assumes that encodeIter is already populated with buffer and version information
		if ((retVal = upaElementList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
		{
			// print out message with return value string, value, and text
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeElementListInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ( (encodeFlag & EncodingTypeFlags.PRIMITIVE_TYPES) != 0 )
	    {
			// various data types used during encoding
			com.thomsonreuters.upa.codec.Time time = CodecFactory.createTime();
			time.hour(10);
			time.minute(21);
			time.second(16);
			time.millisecond(777);
			com.thomsonreuters.upa.codec.UInt uInt = CodecFactory.createUInt();
			uInt.value(17);
			com.thomsonreuters.upa.codec.Int Int = CodecFactory.createInt();
			Int.value(13);
			
			com.thomsonreuters.upa.codec.Float floatVal = CodecFactory.createFloat();
			floatVal.value(1.34f);
			
			com.thomsonreuters.upa.codec.Double doubleVal = CodecFactory.createDouble();
			doubleVal.value(1.345);

			// FIRST Element Entry: encode entry from the Time primitive type
			// Populate and encode element entry with name and dataType information.
			elemEntry.name().data("Element - Time");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.TIME);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, time);
			System.out.println("\t\tEncoded Time: " + time.hour() + ":" + time.minute() + ":" + time.second());
			
			// SECOND Element Entry: encode entry from the Int primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - Int");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.INT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, Int);
			System.out.println("\t\tEncoded signed Integer: " + Int.toLong());
	
			// THIRD Element Entry: encode entry from the UInt primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - UInt");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.UINT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, uInt);
			System.out.println("\t\tEncoded Unsigned Integer: " + uInt.toLong());
			
			// FOURTH Element Entry: encode entry from the Real primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.clear(); // clear this to ensure a blank element
			elemEntry.name().data("Element - Real - Blank");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.REAL);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			elemEntry.encodeBlank(encodeIter); // this encodes a blank
			System.out.println("\t\tEncoded Real: Blank");
	
			// FIFTH Element Entry: encode entry from the Float primitive type
			// Populate and encode element entry with name and dataType information for this element
			elemEntry.name().data("Element - Float");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FLOAT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, floatVal);
			System.out.println("\t\tEncoded Float: " + floatVal.toFloat());
			
			// SIXTH Element Entry: encode entry from the Double primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - Double");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.DOUBLE);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, doubleVal);
			System.out.println("\t\tEncoded Float: " + doubleVal.toDouble());
	    }
		
		if ( (encodeFlag & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
	    {
			success = true;
			
			elemEntry.clear();
			elemEntry.name().data("Element - FieldList");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - ElementList");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - FilterList");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Series");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.SERIES);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Vector"); 
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.VECTOR);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Map");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MAP);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
	    }
		
		if ( (encodeFlag & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
	    {
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeUpdateMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeStatusMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeStatusMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeGenericMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodePostMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeAckMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRequestMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
	    }
		
		// Complete elementList encoding.
		// If success parameter is true, this will finalize encoding.
		// If success parameter is false, this will roll back encoding prior to EncodeElementListInit.
		if ((retVal = upaElementList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeElementListComplete.  "
				+ "Error Text: " + CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("\tUPA ElementList Encoding Complete");
		
		return CodecReturnCodes.SUCCESS;
	}


	// Encode (with UPA) a map (key is UInt) with field lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyUIntAll(com.thomsonreuters.upa.codec.Buffer upaBuf, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that upaBuf.data() points to sufficient memory and upaBuf.length()
		// indicates number of bytes available in upaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, containerType)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with upa_EncodeFieldListAll(encodeIter, dictionary). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
		
	static int upa_EncodeMapKeyUIntAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(containerType);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.UINT);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.UInt entryKey = CodecFactory.createUInt();

			// Create a Buffer for uInt and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for FieldList encoding).
			com.thomsonreuters.upa.codec.Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();

			switch ( containerType )
			{
				case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");

						if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding field list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						// Now encode nested container using its own specific encode methods.
						// Clear, then begin encoding of field list - using same encIterator as map.
						if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding field list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
	
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.clear();
					entryKey.value(2);
	
					System.out.println("\tEncoding Map Entry (key: 2) from pre-encoded buffer");
	
					// assuming encUInt Buffer contains the pre-encoded key with length and data properly populated
					if ((retVal = upa_getPreEncodedUIntBuffer(encUInt, entryKey)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
	
					// assuming encFieldList Buffer contains the pre-encoded payload with data and length populated
					if ((retVal = upa_getPreEncodedFieldListBuffer(encFieldList)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedFieldListBuffer.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
	
					// set pre-encoded data on map entry
					mapEntry.encodedData().data(encFieldList.data(), encFieldList.position(), encFieldList.length());
	
					// pre-encoded key
					mapEntry.encodedKey(encUInt);
					if ((retVal = mapEntry.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.SERIES:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.MAP:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.thomsonreuters.upa.codec.DataTypes.MSG:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding msg.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.thomsonreuters.upa.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
					mapEntry.clear();
					mapEntry.action(MapEntryActions.UPDATE);
					entryKey.value(2);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				break;
				}
			
				// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
				// Need to ensure that MapEntry is appropriately cleared.
				// Clearing will ensure that encodedData and encodedKey are properly emptied.
				mapEntry.clear();
				mapEntry.action(MapEntryActions.DELETE);
				entryKey.value(3);
	
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ") with delete action with no payload");
	
				// entryKeyUInt parameter is passed in as reference to key primitive value.  encodedData is empty for delete
				if ((retVal = mapEntry.encode(encodeIter, entryKey)) < CodecReturnCodes.SUCCESS) 
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
	
				System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
				System.out.println();
			}
	
			// Complete map encoding.
			// If success parameter is true, this will finalize encoding.  
			// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
			if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
	        System.out.println("\tUPA Map Encoding Complete");
	        
	        return CodecReturnCodes.SUCCESS;
		}
	
		// Encode (with UPA) a map (key is Int) with field lists
		// We pass in the buffer to this method with the total length available.
		static int upa_EncodeMapKeyIntWithEmptyFLs(com.thomsonreuters.upa.codec.Buffer upaBuf)
		{
			// use this to store and check return codes
			int retVal;
			boolean success = true;

			int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
			int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
			// Create and clear iterator to prepare for encoding
			com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
			encodeIter.clear();
		
			// Associate buffer and iterator and set proper protocol version information on iterator.
			// It is assumed that upaBuf.data() points to sufficient memory and upaBuf.length()
			// indicates number of bytes available in upaBuf.data().
			if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
								+ " Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			//  create and initialize map structure
			com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

			// populate map structure prior to call to EncodeMapInit
			// NOTE: the key names used may not correspond to actual name values

			// indicate that summary data, a key field id, and a total count hint will be encoded
			upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
			// populate maps keyPrimitiveType and containerType
			upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.INT);
			// populate total count hint with approximate expected entry count
			upaMap.totalCountHint(1);
			upaMap.keyFieldId(3426);

			// Begin encoding of map - assumes that encodeIter is already populated with
			// buffer and version information, store return value to determine success or failure.
			// Expect summary data of approx. 256 bytes, no set definition data.
			if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				// mapInit encoding was successful
				// create a single MapEntry and FieldList and reuse for each entry
				com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
				com.thomsonreuters.upa.codec.Int entryKeyInt = CodecFactory.createInt();

				// Create a Buffer for Int and field list to encode into
				// Here we are heap allocating the buffers (10 bytes is large enough for
				// Int encoding and 500 bytes is large enough for FieldList encoding).
				com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
				encBuffer.data(ByteBuffer.allocate(10));
				com.thomsonreuters.upa.codec.Buffer encFieldList = CodecFactory.createBuffer();
				encFieldList.data(ByteBuffer.allocate(500));

				System.out.println("\tMap Header Encoded");
				System.out.println();
				
				
				// Encode expected summary data, init for this was done by EncodeMapInit
			    // This type should match Map.containerType
				{
					System.out.println("\tEncoding Summary Data");

					// Now encode nested container using its own specific encode methods.
					// Begin encoding of field list - using same encIterator as map list.
					if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
				}
				// complete encoding of summary data.  If any field list encoding failed, success is false.
				if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				
				// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
				mapEntry.action(MapEntryActions.ADD);
				entryKeyInt.value(1);
				if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back.
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
				if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
				System.out.println();

			// Complete map encoding.
			// If success parameter is true, this will finalize encoding.  
			// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
			if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
			{
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
	        System.out.println("\tUPA Map Encoding Complete");
			}
	        
	        return CodecReturnCodes.SUCCESS;
		}


	// Encode (with UPA) a map (key is Int) with field lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyIntWithFLs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that upaBuf.data() points to sufficient memory and upaBuf.length()
		// indicates number of bytes available in upaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.INT);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Int entryKeyInt = CodecFactory.createInt();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry with delete action..
			mapEntry.action(MapEntryActions.ADD);
			entryKeyInt.value(2);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();

			
			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKeyInt.value(3);

			System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") with delete action with no payload");

			// entryKeyUInt parameter is passed in as refreence to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKeyInt)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is AsciiString) with field lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyAsciiStringWithFLs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that upaBuf.data() points to sufficient memory and upaBuf.length()
		// indicates number of bytes available in upaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry with delete action..
			mapEntry.action(MapEntryActions.ADD);
			entryKey.data("keyData2");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKey.data("keyData3");

			System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") with delete action with no payload");

			// entryKeyUInt parameter is passed in as refreence to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKey)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is Buffer) with field lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyBufferWithFLs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that upaBuf.data() points to sufficient memory and upaBuf.length()
		// indicates number of bytes available in upaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.BUFFER);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry with delete action..
			mapEntry.action(MapEntryActions.ADD);
			entryKey.data("keyData2");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKey.data("keyData3");

			System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") with delete action with no payload");

			// entryKeyUInt parameter is passed in as refreence to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKey)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is UInt) with element lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyUIntWithELs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.UINT);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.UInt entryKeyUInt = CodecFactory.createUInt();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.thomsonreuters.upa.codec.Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyUInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyUInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyUInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyUInt.toLong() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry from pre-encoded buffer containing an encoded ElementList
			// because we are re-populating all values on MapEntry, there is no need to clear it
			mapEntry.action(MapEntryActions.ADD);
			entryKeyUInt.value(2);

			System.out.println("\tEncoding Map Entry (key: 2) from pre-encoded buffer");

			// assuming encUInt Buffer contains the pre-encoded key with length and data properly populated
			if ((retVal = upa_getPreEncodedUIntBuffer(encUInt, entryKeyUInt)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// assuming encElementList Buffer contains the pre-encoded payload with data and length populated
			if ((retVal = upa_getPreEncodedElementListBuffer(encElementList)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_getPreEncodedElementListBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// set pre-encoded data on map entry
			mapEntry.encodedData().data(encElementList.data(), encElementList.position(), encElementList.length());

			// pre-encoded key
			mapEntry.encodedKey(encUInt);
			if ((retVal = mapEntry.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyUInt.toLong() + ") Encoding Complete");
			System.out.println();


			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKeyUInt.value(3);

			System.out.println("\tEncoding Map Entry (key: " + entryKeyUInt.toLong() + ") with delete action with no payload");

			// entryKeyUInt parameter is passed in as refreence to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKeyUInt)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyUInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is Int) with element lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyIntWithELs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.INT);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Int entryKeyInt = CodecFactory.createInt();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.thomsonreuters.upa.codec.Buffer encInt = CodecFactory.createBuffer();
			encInt.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = upa_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry from pre-encoded buffer containing an encoded ElementList
			// because we are re-populating all values on MapEntry, there is no need to clear it
			mapEntry.action(MapEntryActions.ADD);
			entryKeyInt.value(2);

			System.out.println("\tEncoding Map Entry (key: 2) from pre-encoded buffer");

			// assuming encInt Buffer contains the pre-encoded key with length and data properly populated
			if ((retVal = upa_getPreEncodedIntBuffer(encInt, entryKeyInt)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// assuming encElementList Buffer contains the pre-encoded payload with data and length populated
			if ((retVal = upa_getPreEncodedElementListBuffer(encElementList)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_getPreEncodedElementListBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// set pre-encoded data on map entry
			mapEntry.encodedData().data(encElementList.data(), encElementList.position(), encElementList.length());

			// pre-encoded key
			mapEntry.encodedKey(encInt);
			if ((retVal = mapEntry.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();


			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKeyInt.value(3);

			System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") with delete action with no payload");

			// entryKeyUInt parameter is passed in as refreence to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKeyInt)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is Buffer) with element lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyBufferWithELs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.BUFFER);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry with delete action..
			mapEntry.action(MapEntryActions.ADD);
			entryKey.data("keyData2");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter)).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();


			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKey.data("keyData3");

			System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") with delete action with no payload");

			// entryKey parameter is passed in as reference to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKey)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with UPA) a map (key is AsciiString) with element lists
	// We pass in the buffer to this method with the total length available.
	static int upa_EncodeMapKeyAsciiStringWithELs(com.thomsonreuters.upa.codec.Buffer upaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(upaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.thomsonreuters.upa.codec.Map upaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		upaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		upaMap.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		upaMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);
		// populate total count hint with approximate expected entry count
		upaMap.totalCountHint(3);
		upaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = upaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.thomsonreuters.upa.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.thomsonreuters.upa.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.thomsonreuters.upa.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = upaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();

			
			// SECOND Map Entry: encode entry with delete action..
			mapEntry.action(MapEntryActions.ADD);
			entryKey.data("keyData2");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with upa_EncodeElementListAll(encodeIter)).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();


			// THIRD Map Entry: encode entry with delete action.  Delete actions have no payload.
			// Need to ensure that MapEntry is appropriately cleared.
			// Clearing will ensure that encodedData and encodedKey are properly emptied.
			mapEntry.clear();
			mapEntry.action(MapEntryActions.DELETE);
			entryKey.data("keyData3");

			System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") with delete action with no payload");

			// entryKey parameter is passed in as reference to key primitive value.  encodedData is empty for delete
			if ((retVal = mapEntry.encode(encodeIter, entryKey)) < CodecReturnCodes.SUCCESS) 
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = upaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tUPA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}
	
	static int upa_EncodeRequestMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal = 0;
		boolean success;
		
		System.out.println();
		
		System.out.println("Begin UPA RequestMsg Set");
		com.thomsonreuters.upa.codec.RequestMsg requestMsg = (com.thomsonreuters.upa.codec.RequestMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		requestMsg.streamId(555);
		
		requestMsg.applyHasPriority();
		requestMsg.priority().count(5);
		requestMsg.priority().priorityClass(7);
		
		requestMsg.applyHasQos();
		requestMsg.qos().dynamic(false);
		requestMsg.qos().rate(com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK);
		requestMsg.qos().timeliness(com.thomsonreuters.upa.codec.QosTimeliness.REALTIME);
		
		requestMsg.applyPrivateStream();
		
		requestMsg.applyStreaming();
		
		requestMsg.applyConfInfoInUpdates();
		
		requestMsg.applyHasExtendedHdr();
		
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data( "ABCDEF" );
		
		requestMsg.msgKey().applyHasNameType();
		requestMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		requestMsg.msgKey().applyHasServiceId();
		requestMsg.msgKey().serviceId(5);
		
		requestMsg.msgKey().applyHasFilter();
		requestMsg.msgKey().filter( 12 );
	
		requestMsg.msgKey().applyHasIdentifier();
		requestMsg.msgKey().identifier(21);
		
		requestMsg.msgKey().applyHasAttrib();
		requestMsg.msgKey().attribContainerType( containerType );
		
		requestMsg.containerType(containerType);
		
		if ((retVal = requestMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding RequestMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with RequestMsg.encode().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = requestMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding RequestMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with RequestMsg.encode().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA RequestMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeRefreshMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println();
		
		 System.out.println("Begin UPA RefreshMsg Set");
		com.thomsonreuters.upa.codec.RefreshMsg refreshMsg = (com.thomsonreuters.upa.codec.RefreshMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.thomsonreuters.upa.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		permissionData.data("RefreshMsg.permissionData");
		refreshMsg.applyHasPermData();
		refreshMsg.permData(permissionData);
	
		refreshMsg.state().code( com.thomsonreuters.upa.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( containerType );
		
		refreshMsg.containerType(containerType);
		
		if ((retVal = refreshMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding RefreshMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with RefreshMsg.encode().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = refreshMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding RefreshMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with RefreshMsg.encode().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA RefreshMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeUpdateMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		// Create a UPA Buffer to encode into
		System.out.println();
		
		System.out.println("Begin UPA UpdateMsg Set");
		com.thomsonreuters.upa.codec.UpdateMsg updateMsg = (com.thomsonreuters.upa.codec.UpdateMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		updateMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.UPDATE);
		
		updateMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		updateMsg.streamId( 15 );
		
		updateMsg.applyHasSeqNum();
		updateMsg.seqNum( 22 );

		updateMsg.applyHasMsgKey();

		updateMsg.msgKey().applyHasName();
		updateMsg.msgKey().name().data( "ABCDEF" );
		
		updateMsg.msgKey().applyHasNameType();
		updateMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		updateMsg.msgKey().applyHasServiceId();
		updateMsg.msgKey().serviceId(5);
		
		updateMsg.msgKey().applyHasFilter();
		updateMsg.msgKey().filter( 12 );
	
		updateMsg.msgKey().applyHasIdentifier();
		updateMsg.msgKey().identifier(21);
		
		updateMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		permissionData.data("UpdateMsg.permissionData");
		updateMsg.applyHasPermData();
		updateMsg.permData(permissionData);
	
		updateMsg.applyDoNotCache();
		updateMsg.applyDoNotConflate();
		
		updateMsg.applyHasConfInfo();
		updateMsg.conflationCount(10);
		updateMsg.conflationTime(20);
		
		updateMsg.applyHasPostUserInfo();
		updateMsg.postUserInfo().userAddr(15);
		updateMsg.postUserInfo().userId(30);
		
		updateMsg.msgKey().applyHasAttrib();
		updateMsg.msgKey().attribContainerType( containerType );
		updateMsg.containerType(containerType);
		
		if ((retVal = updateMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding UpdateMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with UpdateMsg.encodeInit().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = updateMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding UpdateMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with UpdateMsg.encodeComplete().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA UpdateMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeGenericMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin UPA GenericMsg Set");
		com.thomsonreuters.upa.codec.GenericMsg genericMsg = (com.thomsonreuters.upa.codec.GenericMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.applyHasPartNum();
		genericMsg.partNum( 10 );
		
		genericMsg.applyHasSeqNum();
		genericMsg.seqNum( 22 );

		genericMsg.applyHasSecondarySeqNum();
		genericMsg.secondarySeqNum(123);

		genericMsg.applyMessageComplete();
		
		genericMsg.applyHasMsgKey();

		genericMsg.msgKey().applyHasName();
		genericMsg.msgKey().name().data( "ABCDEF" );
		
		genericMsg.msgKey().applyHasNameType();
		genericMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		permissionData.data("GenericMsg.permissionData");
		genericMsg.applyHasPermData();
		genericMsg.permData(permissionData);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( containerType );
		
		genericMsg.containerType(containerType);
		
		if ((retVal = genericMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding GenericMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with GenericMsg.encodeInit().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = genericMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding GenericMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with GenericMsg.encodeComplete().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA GenericMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodePostMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		// Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        fieldListBuf.data(ByteBuffer.allocate(4048));

		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println();
		
		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
		System.out.println("Begin UPA PostMsg Set");
		com.thomsonreuters.upa.codec.PostMsg postMsg = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.POST);
		
		postMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.thomsonreuters.upa.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.applyHasExtendedHdr();
		Buffer permissionData = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		permissionData.data("PostMsg.permissionData");
		postMsg.applyHasPermData();
		postMsg.permData(permissionData);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType(containerType);
		postMsg.containerType(containerType);
		
		if ((retVal = postMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding PostMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with PostMsg.encodeInit().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = postMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding PostMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with PostMsg.encodeComplete().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA PostMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeAckMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin UPA AckMsg Set");
		com.thomsonreuters.upa.codec.AckMsg ackMsg = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.applyHasExtendedHdr();
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( containerType );
	
		ackMsg.containerType(containerType);
		
		if ((retVal = ackMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding AckMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with AckMsg.encodeInit().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = ackMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding AckMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with AckMsg.encodeComplete().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA AckMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeStatusMsgAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin UPA StatusMsg Set");
		com.thomsonreuters.upa.codec.StatusMsg statusMsg = (com.thomsonreuters.upa.codec.StatusMsg) com.thomsonreuters.upa.codec.CodecFactory
				.createMsg();
		statusMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.STATUS);

		statusMsg.domainType(com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE);

		statusMsg.streamId(15);

		statusMsg.applyHasMsgKey();

		statusMsg.msgKey().applyHasName();
		statusMsg.msgKey().name().data("ABCDEF");

		statusMsg.msgKey().applyHasNameType();
		statusMsg.msgKey().nameType(com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC);

		statusMsg.msgKey().applyHasServiceId();
		statusMsg.msgKey().serviceId(5);

		statusMsg.msgKey().applyHasFilter();
		statusMsg.msgKey().filter(12);

		statusMsg.msgKey().applyHasIdentifier();
		statusMsg.msgKey().identifier(21);
		
		statusMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		permissionData.data("StatusMsg.permissionData");
		statusMsg.applyHasPermData();
		statusMsg.permData(permissionData);

		statusMsg.applyHasState();
		statusMsg.state().code(com.thomsonreuters.upa.codec.StateCodes.NONE);
		statusMsg.state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		statusMsg.state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		statusMsg.state().text().data("status complete");

		statusMsg.applyClearCache();

		statusMsg.applyHasPostUserInfo();
		statusMsg.postUserInfo().userAddr(15);
		statusMsg.postUserInfo().userId(30);

		statusMsg.msgKey().applyHasAttrib();
		statusMsg.msgKey().attribContainerType(containerType);
		statusMsg.containerType(containerType);
		
		if ((retVal = statusMsg.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding StatusMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with StatusMsg.encodeInit().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		success = true;
		
		switch (containerType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		}
		
		if ((retVal = statusMsg.encodeComplete(encodeIter,success)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding StatusMsg.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with StatusMsg.encodeComplete().  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("End UPA StatusMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	static int upa_EncodeFilterListAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int encodeOption)
	{
		int retVal;
		boolean success;
		
		if ( ( encodeOption & EncodingTypeFlags.PRIMITIVE_TYPES ) != 0 )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize filter list structure
		com.thomsonreuters.upa.codec.FilterList upaFilterList = CodecFactory.createFilterList();
		
		upaFilterList.applyHasTotalCountHint();
		upaFilterList.applyHasPerEntryPermData();
		
		com.thomsonreuters.upa.codec.FilterEntry filterEntry = CodecFactory.createFilterEntry();
		
		int totalCountHint = 0;
		
		if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
			totalCountHint = 6;
		
		if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
			totalCountHint += 6;
		
		
		if ( totalCountHint != 0 )
		{
			upaFilterList.totalCountHint(totalCountHint);
			
			// begin encoding of filter list - assumes that encodeIter is already populated with buffer and version information
	        if ((retVal = upaFilterList.encodeInit(encodeIter) ) < CodecReturnCodes.SUCCESS)
	        {
	            // print out message with return value string, value, and text
	            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterList.encodeInit().  "
	                    + "Error Text: " + CodecReturnCodes.info(retVal));
	            return retVal;
	        }
		}
		
		if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
		{
	        // Encode FieldList to filter entry
			filterEntry.id(1);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        success = true;
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode ElementList to filter entry
            filterEntry.id(2);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode FilterList to filter entry
            filterEntry.id(3);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Series to filter entry
            filterEntry.id(4);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.SERIES);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Vector to filter entry
            filterEntry.id(5);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }
            
            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Map to filter entry
            filterEntry.id(6);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MAP);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
		{
			success = true;
			
			filterEntry.clear();
			filterEntry.id(7);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
            }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(8);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeUpdateMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(9);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeStatusMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeStatusMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(10);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeGenericMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(11);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodePostMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(12);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeAckMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	     // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
			filterEntry.id(13);
	        filterEntry.action(com.thomsonreuters.upa.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRequestMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	     // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding of FilterList.
		if ((retVal = upaFilterList.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterList.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}
	
	static int upa_EncodeSeriesAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType )
	{
		int retVal;
		boolean success;
		
		if ( containerType < com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize series structure
		com.thomsonreuters.upa.codec.Series upaSeries = CodecFactory.createSeries();
		
		int totalCountHint = 2; // Add two series entry
		
		upaSeries.containerType(containerType);
		
		upaSeries.applyHasTotalCountHint();
		upaSeries.totalCountHint(totalCountHint);
		
		upaSeries.applyHasSummaryData();
		
		// begin encoding of series - assumes that encodeIter is already populated with buffer and version information
        if ((retVal = upaSeries.encodeInit(encodeIter, 0, 0) ) < CodecReturnCodes.SUCCESS)
        {
            // print out message with return value string, value, and text
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeInit().  "
                    + "Error Text: " + CodecReturnCodes.info(retVal));
            return retVal;
        }

		com.thomsonreuters.upa.codec.SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
		
		success = true;
		int argument = 0;
		
		for ( int i = 0; i < totalCountHint; i++ )
		{
			 seriesEntry.clear();
			
			switch (containerType)
			{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeFieldListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding element list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeElementListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding filter list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeFilterListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding series list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeSeriesAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeVectorAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding map.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding msg.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = upaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			}
			
			 // Complete encoding of complex series entry.
            if ((retVal = seriesEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding of series.
		if ((retVal = upaSeries.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}

	static int upa_EncodeVectorAll(com.thomsonreuters.upa.codec.EncodeIterator encodeIter, int containerType )
	{
		int retVal;
		boolean success;
		
		if ( containerType < com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize vector structure
		com.thomsonreuters.upa.codec.Vector upaVector = CodecFactory.createVector();
		
		int totalCountHint = 2; // Add three filter entry
	
		upaVector.containerType(containerType);
		
		upaVector.applyHasTotalCountHint();
		upaVector.totalCountHint(totalCountHint);
		
		upaVector.applyHasSummaryData();
		
		// begin encoding of vector - assumes that encodeIter is already populated with buffer and version information
        if ((retVal = upaVector.encodeInit(encodeIter, 0, 0) ) < CodecReturnCodes.SUCCESS)
        {
            // print out message with return value string, value, and text
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeInit().  "
                    + "Error Text: " + CodecReturnCodes.info(retVal));
            return retVal;
        }
		
		com.thomsonreuters.upa.codec.VectorEntry vectorEntry = CodecFactory.createVectorEntry();
		
		success = true;
		int argument = 0;
		
		for ( int i = 0; i < totalCountHint; i++ )
		{
			vectorEntry.clear();
			
			switch (containerType)
			{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeFieldListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding element list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeElementListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeElementListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding filter list.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeFilterListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeSeriesAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding series.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeSeriesAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeVectorAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeVectorAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeMapKeyUIntAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeMapKeyUIntAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding msg.");
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = upaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.thomsonreuters.upa.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = upa_EncodeRefreshMsgAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with upa_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			}
			
			 // Complete encoding of complex vector entry.
            if ((retVal = vectorEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding vector.
		if ((retVal = upaVector.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}

    static void upa_encodeDictionaryMsg(com.thomsonreuters.upa.codec.DataDictionary dictionary)
    {
    	com.thomsonreuters.upa.codec.Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(12300));
        com.thomsonreuters.upa.codec.RefreshMsg msg = (com.thomsonreuters.upa.codec.RefreshMsg)CodecFactory.createMsg();
        com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.thomsonreuters.upa.transport.Error error = com.thomsonreuters.upa.transport.TransportFactory.createError();
        com.thomsonreuters.upa.codec.Buffer nameBuf = CodecFactory.createBuffer();
        com.thomsonreuters.upa.codec.Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field and Type dictionaries
        dictionary.clear();

        checkResult(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary(fieldDictionaryFileName, error));
        checkResult(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary(enumTableFileName, error));
        
        // set-up message
        msg.msgClass(MsgClasses.REFRESH);
        msg.domainType(com.thomsonreuters.upa.rdm.DomainTypes.DICTIONARY);
        msg.containerType(com.thomsonreuters.upa.codec.DataTypes.SERIES);
		msg.state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		msg.state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		msg.state().code(com.thomsonreuters.upa.codec.StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
		msg.applySolicited();
		msg.applyHasMsgKey();
    	msg.msgKey().filter(com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(1);
    	
    	// DictionaryName
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	// StreamId
    	msg.streamId(3);

        // clear encode iterator
        encodeIter.clear();
        
        // set iterator buffer
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        checkResult(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        // encode part of field dictionary (limited by buffer size)
        checkResult(CodecReturnCodes.DICT_PART_ENCODED, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid,
        										com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.NORMAL, error));        

        checkResult(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
    }

    
	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a upa Buffer with length and data properly populated.
	private static int upa_getPreEncodedUIntBuffer(com.thomsonreuters.upa.codec.Buffer encUInt, com.thomsonreuters.upa.codec.UInt uInt)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encUInt.data() points to sufficient memory and encUInt.length()
		// indicates number of bytes available in encUInt.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encUInt, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = uInt.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeUInt.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	// This method returns a preencoded buffer containing an encoded Int type.
	// Assuming encInt is a upa Buffer with length and data properly populated.
	private static int upa_getPreEncodedIntBuffer(com.thomsonreuters.upa.codec.Buffer encInt, com.thomsonreuters.upa.codec.Int Int)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encInt.data() points to sufficient memory and encInt.length()
		// indicates number of bytes available in encInt.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encInt, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = Int.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeInt.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

//	// This method returns a preencoded buffer containing an encoded Buffer type.
//	// Assuming encBuffer is a upa Buffer with length and data properly populated.
//	private static int upa_getPreEncodedBufferBuffer(com.thomsonreuters.upa.codec.Buffer encBuffer, com.thomsonreuters.upa.codec.Buffer buffer)
//	{
//		int retVal;  //used to store and check the return value from the method calls
//
//		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
//		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
//		
//		// create and clear iterator to prepare for encoding
//		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
//		encodeIter.clear();
//
//		// associate buffer and iterator and set proper protocol version information on iterator
//		// assuming that encBuffer.data() points to sufficient memory and encBuffer.length()
//		// indicates number of bytes available in encBuffer.data()
//		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
//					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
//
//			return retVal;
//		}
//
//		if ((retVal = buffer.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeBuffer.  "
//								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
//			return retVal;
//		}
//
//		return CodecReturnCodes.SUCCESS;
//	}

	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a upa Buffer with length and data properly populated.
	private static int upa_getPreEncodedFieldListBuffer(com.thomsonreuters.upa.codec.Buffer encFieldList)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encFieldList.data() points to sufficient memory and encFieldList.length()
		// indicates number of bytes available in encFieldList.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encFieldList, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = upa_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with upa_EncodeFieldListAll.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a upa Buffer with length and data properly populated.
	private static int upa_getPreEncodedElementListBuffer(com.thomsonreuters.upa.codec.Buffer encElementList)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encFieldList.data() points to sufficient memory and encFieldList.length()
		// indicates number of bytes available in encFieldList.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encElementList, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = upa_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("UPA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with upa_EncodeElementListAll.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	
	
	static void EmaDecode_UPAFieldListAll(com.thomsonreuters.ema.access.FieldList fl, int encodeOption)
	{
        checkResult(fl.hasInfo());
        checkResult(fl.infoDictionaryId(), dictionary.infoDictionaryId());
        checkResult(fl.infoFieldListNum(), 65);
        
        Iterator<FieldEntry> iter = fl.iterator();
       	checkResult(iter != null);
        
       	if ( (encodeOption & EncodingTypeFlags.PRIMITIVE_TYPES ) != 0 )
       	{
	        // check first field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe1 = iter.next();
	        checkResult(fe1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.error().errorCode(), OmmError.ErrorCode.FIELD_ID_NOT_FOUND);
	
	        try
	        {
	        	fe1.uintValue();
	        	checkResult(false);
	        }
	        catch(OmmException excp)
	        {
	        	System.out.println("Exception expected: " + excp.getMessage());
	        }
	        
	        //check reseting iteration to start of fieldlist
	    	iter = fl.iterator();
	        // check first field entry again
	        checkResult(iter.hasNext());
	        fe1 = iter.next();
	        checkResult(fe1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.error().errorCode(), OmmError.ErrorCode.FIELD_ID_NOT_FOUND);
	
	        // check second field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe2 = iter.next();
	        checkResult(fe2.fieldId(), 16);
	        checkResult(fe2.name(), "TRADE_DATE");
	        checkResult(fe2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.DATE);
	        checkResult(fe2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.DATE);
	        checkResult(fe2.code(), Data.DataCode.NO_CODE);
	        checkResult(fe2.date().day(), 30);
	        checkResult(fe2.date().month(), 11);
	        checkResult(fe2.date().year(), 2010);
	       
	        // check third field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe3 = iter.next();
	        checkResult(fe3.fieldId(), 147);
	        checkResult(fe3.name(), "OFF_BND_NO");
	        checkResult(fe3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe3.code(), Data.DataCode.NO_CODE);
	        checkResult(fe3.uintValue(), 23456);
	
	        // check fourth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe4 = iter.next();
	        checkResult(fe4.fieldId(), 1080);
	        checkResult(fe4.name(), "PREF_DISP");
	        checkResult(fe4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe4.code(), Data.DataCode.NO_CODE);
	        checkResult(fe4.uintValue(), 23456);
	        
	        // check fifth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe5 = iter.next();
	        checkResult(fe5.fieldId(), 22);
	        checkResult(fe5.name(), "BID");
	        checkResult(fe5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe5.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe5.code(), Data.DataCode.BLANK);
	        
	        // check sixth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe6 = iter.next();
	        checkResult(fe6.fieldId(), 24);
	        checkResult(fe6.name(), "BID_2");
	        checkResult(fe6.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe6.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe6.code(), Data.DataCode.NO_CODE);
	        checkResult(fe6.real().mantissa(), 227);
	        checkResult(fe6.real().magnitudeType(), 12);
	        checkResult(fe6.hasEnumDisplay() == false, "hasEnumDisplay() == false");
	        
	        try
	        {
	        	fe6.enumDisplay();
	        	checkResult(false, "Exception expected from calling enumDisplay() with the Real type.");
	        }
	        catch (OmmException excp)
	        {
	        	checkResult(excp.getMessage().equals("Attempt to enumDisplay() while actual entry data type is Real"), "Exception expected. " + excp.getMessage());
	        }
	
	        // check seventh field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe7 = iter.next();
	        checkResult(fe7.fieldId(), 25);
	        checkResult(fe7.name(), "ASK");
	        checkResult(fe7.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe7.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe7.code(), Data.DataCode.NO_CODE);
	        checkResult(fe7.real().mantissa(), 22801);
	        checkResult(fe7.real().magnitudeType(), 10);
	        
	        // check eighth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe8 = iter.next();
	        checkResult(fe8.fieldId(), 18);
	        checkResult(fe8.name(), "TRDTIM_1");
	        checkResult(fe8.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(fe8.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(fe8.code(), Data.DataCode.NO_CODE);
	        checkResult(fe8.time().hour(), 8);
	        checkResult(fe8.time().minute(), 39);
	        checkResult(fe8.time().second(), 24);
	        
	        // check nineth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe9 = iter.next();
	        checkResult(fe9.fieldId(), TestUtilities.INTEGER);
	        checkResult(fe9.name(), "INTEGER");
	        checkResult(fe9.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.INT);
	        checkResult(fe9.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.INT);
	        checkResult(fe9.code(), Data.DataCode.NO_CODE);
	        checkResult(fe9.intValue(), 65432);
	        
	        // check tenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe10 = iter.next();
	        checkResult(fe10.fieldId(), TestUtilities.MY_FLOAT);
	        checkResult(fe10.name(), "MY_FLOAT");
	        checkResult(fe10.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(fe10.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(fe10.code(), Data.DataCode.NO_CODE);
	        checkResult(fe10.floatValue(), 3.14f);
	
	        // check eleventh field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe11 = iter.next();
	        checkResult(fe11.fieldId(), TestUtilities.MY_DOUBLE);
	        checkResult(fe11.name(), "MY_DOUBLE");
	        checkResult(fe11.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(fe11.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(fe11.code(), Data.DataCode.NO_CODE);
	        checkResult(fe11.doubleValue(), 3.1416);
	
	        // check twelveth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe12 = iter.next();
	        checkResult(fe12.fieldId(), TestUtilities.TRADE_DATETIME);
	        checkResult(fe12.name(), "TRADE_DATE");
	        checkResult(fe12.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.DATETIME);
	        checkResult(fe12.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.DATETIME);
	        checkResult(fe12.code(), Data.DataCode.NO_CODE);
	        checkResult(fe12.dateTime().month(), 11);
	        checkResult(fe12.dateTime().day(), 15);
	        checkResult(fe12.dateTime().year(), 2011);
	        checkResult(fe12.dateTime().hour(), 8);
	        checkResult(fe12.dateTime().minute(), 39);
	        checkResult(fe12.dateTime().second(), 24);
	        
	        // check thirteenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe13 = iter.next();
	        checkResult(fe13.fieldId(), TestUtilities.MY_QOS);
	        checkResult(fe13.name(), "MY_QOS");
	        checkResult(fe13.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.QOS);
	        checkResult(fe13.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.QOS);
	        checkResult(fe13.code(), Data.DataCode.NO_CODE);
	        checkResult(fe13.qos().timeliness(), OmmQos.Timeliness.REALTIME);
	        checkResult(fe13.qos().rate(), OmmQos.Rate.TICK_BY_TICK);
	
	        // check fourteenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe14 = iter.next();
	        checkResult(fe14.fieldId(), TestUtilities.MY_STATE);
	        checkResult(fe14.name(), "MY_STATE");
	        checkResult(fe14.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATE);
	        checkResult(fe14.code(), Data.DataCode.NO_CODE);
	        checkResult(fe14.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATE);
	        checkResult(fe14.state().streamState(), OmmState.StreamState.OPEN);
	        checkResult(fe14.state().dataState(), OmmState.DataState.OK);
	        checkResult(fe14.state().statusCode(), OmmState.StatusCode.NONE);
	        checkResult(fe14.state().statusText(), "Succeeded");
	        checkResult(fe14.state().toString(), "Open / Ok / None / 'Succeeded'");
	
	        // check fifteenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe15 = iter.next();
	        checkResult(fe15.fieldId(), TestUtilities.MY_BUFFER);
	        checkResult(fe15.name(), "MY_BUFFER");
	        checkResult(fe15.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
	        checkResult(fe15.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
	        checkResult(fe15.code(), Data.DataCode.NO_CODE);
	        com.thomsonreuters.upa.codec.Buffer Buf = CodecFactory.createBuffer();
	        Buf.data("ABCDEFGH");
	        checkResult( Arrays.equals(fe15.buffer().buffer().array(), Buf.data().array()) );
	
	        // check sixteenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe16 = iter.next();
	        checkResult(fe16.fieldId(), 4);
	        checkResult(fe16.name(), "RDN_EXCHID");
	        checkResult(fe16.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe16.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe16.code(), Data.DataCode.NO_CODE);
	        checkResult(fe16.enumValue() == 29, "enumValue() == 29");
	        checkResult(fe16.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe16.enumDisplay().equals("CSC") , "enumDisplay() == \"CSC\"");
	        
	        // check seventeenth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe17 = iter.next();
	        checkResult(fe17.fieldId(), TestUtilities.MY_ARRAY);
	        checkResult(fe17.name(), "MY_ARRAY");
	        checkResult(fe17.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ARRAY);
	        checkResult(fe17.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ARRAY);
	        checkResult(fe17.code(), Data.DataCode.NO_CODE);
	        OmmArray ar2 = fe17.array();
	        Iterator<OmmArrayEntry> ar2Iter = ar2.iterator();
	        
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae1 = ar2Iter.next();
	                     checkResult(ae1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae1.uintValue(), 10);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae2 = ar2Iter.next();
	                     checkResult(ae2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae2.uintValue(), 20);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae3 = ar2Iter.next();
	                     checkResult(ae3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae3.uintValue(), 30);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae4 = ar2Iter.next();
	                     checkResult(ae4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae4.uintValue(), 40);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae5 = ar2Iter.next();
	                     checkResult(ae5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae5.uintValue(), 23456);
	                checkResult(!ar2Iter.hasNext());
	                
	                
	        // Check eighteenth field entry
            checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe18 = iter.next();
	        checkResult(fe18.fieldId(), 4);
	        checkResult(fe18.name(), "RDN_EXCHID");
	        checkResult(fe18.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe18.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe18.code() == Data.DataCode.BLANK, "code == BLANK" );
	        
	        try
	        {
	        	fe18.enumValue();
	        	checkResult(false, "Exception expected from calling enumValue() with blank entry");
	        }
	        catch (OmmException excp)
	        {
	        	checkResult(excp.getMessage().equals("Attempt to enumValue() while entry data is blank."), "Exception expected. " + excp.getMessage());
	        }
	        
	        checkResult(fe18.hasEnumDisplay() == false, "hasEnumDisplay() == false");
	        
	        try
	        {
	        	fe18.enumDisplay();
	        	checkResult(false, "Exception expected from calling enumDisplay() with blank entry");
	        }
	        catch (OmmException excp)
	        {
	        	checkResult(excp.getMessage().equals("Attempt to enumDisplay() while entry data is blank."), "Exception expected. " + excp.getMessage());
	        }
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe19 = iter.next();
	        checkResult(fe19.fieldId(), 4);
	        checkResult(fe19.name(), "RDN_EXCHID");
	        checkResult(fe19.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe19.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe19.code(), Data.DataCode.NO_CODE);
	        checkResult(fe19.enumValue() == 2999, "enumValue() == 2999");
	        checkResult(fe19.hasEnumDisplay() == false, "hasEnumDisplay() == false");
	        
	        try
	        {
	        	fe19.enumDisplay();
	        	checkResult(false, "Exception expected from calling enumDisplay() with undefined display value");
	        }
	        catch (OmmException excp)
	        {
	        	checkResult(excp.getMessage().equals("The enum value 2999 for the field Id 4 does not exist in the enumerated type dictionary"),
	        			"Exception expected. " + excp.getMessage());
	        }
	        
	        // check twentieth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe20 = iter.next();
	        checkResult(fe20.fieldId(), 115);
	        checkResult(fe20.name(), "BID_TICK_1");
	        checkResult(fe20.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe20.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe20.code(), Data.DataCode.NO_CODE);
	        checkResult(fe20.enumValue() == 0, "enumValue() == 0");
	        checkResult(fe20.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe20.enumDisplay().equals(" ") , "enumDisplay() == \" \"");
	        


	        
	        // check twenty fifth field entry
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fe25 = iter.next();
	        checkResult(fe25.fieldId(), 8960);
	        checkResult(fe25.name(), "GBLISS_IND");
	        checkResult(fe25.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe25.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe25.code(), Data.DataCode.NO_CODE);
	        checkResult(fe25.enumValue() == 2, "enumValue() == 2");
	        checkResult(fe25.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe25.enumDisplay().equals("  EURO  ") , "enumDisplay() == \"  EURO  \"");
	        
	        if ( encodeOption == EncodingTypeFlags.PRIMITIVE_TYPES )
	        	checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
       	
       	if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
       	{
       		// check field entry which contains FieldList
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry1 = iter.next();
	        checkResult(fieldEntry1.fieldId(), TestUtilities.MY_FIELDLIST);
	        checkResult(fieldEntry1.name(), "MY_FIELDLIST");
	        checkResult(fieldEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(fieldEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(fieldEntry1.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAFieldListAll(fieldEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES );
	        
	        // check field entry which contains ElementList
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry2 = iter.next();
	        checkResult(fieldEntry2.fieldId(), TestUtilities.MY_ELEMENTLIST);
	        checkResult(fieldEntry2.name(), "MY_ELEMENTLIST");
	        checkResult(fieldEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(fieldEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(fieldEntry2.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAElementListAll(fieldEntry2.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES );
	        
	        // check field entry which contains FilterList
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry3 = iter.next();
	        checkResult(fieldEntry3.fieldId(), TestUtilities.MY_FILTERLIST);
	        checkResult(fieldEntry3.name(), "MY_FILTERLIST");
	        checkResult(fieldEntry3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(fieldEntry3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(fieldEntry3.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAFilterListAll(fieldEntry3.filterList(), EncodingTypeFlags.MESSAGE_TYPES );
	        
	        // check field entry which contains Series
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry4 = iter.next();
	        checkResult(fieldEntry4.fieldId(), TestUtilities.MY_SERIES);
	        checkResult(fieldEntry4.name(), "MY_SERIES");
	        checkResult(fieldEntry4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
	        checkResult(fieldEntry4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
	        checkResult(fieldEntry4.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPASeriesAll(fieldEntry4.series(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST );
	        
	        // check field entry which contains Vector
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry5 = iter.next();
	        checkResult(fieldEntry5.fieldId(), TestUtilities.MY_VECTOR);
	        checkResult(fieldEntry5.name(), "MY_VECTOR");
	        checkResult(fieldEntry5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(fieldEntry5.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(fieldEntry5.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAVectorAll(fieldEntry5.vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
	        
	        // check field entry which contains Map
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry6 = iter.next();
	        checkResult(fieldEntry6.fieldId(), TestUtilities.MY_MAP);
	        checkResult(fieldEntry6.name(), "MY_MAP");
	        checkResult(fieldEntry6.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
	        checkResult(fieldEntry6.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
	        checkResult(fieldEntry6.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAMapKeyUIntAll(fieldEntry6.map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
	        
	        if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
	        	checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
       	
    	if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
       	{
    		// check first field entry which contains FieldList
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry1 = iter.next();
	        checkResult(fieldEntry1.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry1.name(), "MY_MSG");
	        checkResult(fieldEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(fieldEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(fieldEntry1.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPARefreshMsgAll(fieldEntry1.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry2 = iter.next();
	        checkResult(fieldEntry2.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry2.name(), "MY_MSG");
	        checkResult(fieldEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(fieldEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(fieldEntry2.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAUpdateMsgAll(fieldEntry2.updateMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry3 = iter.next();
	        checkResult(fieldEntry3.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry3.name(), "MY_MSG");
	        checkResult(fieldEntry3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(fieldEntry3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(fieldEntry3.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAStatusMsgAll(fieldEntry3.statusMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry4 = iter.next();
	        checkResult(fieldEntry4.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry4.name(), "MY_MSG");
	        checkResult(fieldEntry4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(fieldEntry4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(fieldEntry4.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAGenericMsgAll(fieldEntry4.genericMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry5 = iter.next();
	        checkResult(fieldEntry5.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry5.name(), "MY_MSG");
	        checkResult(fieldEntry5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(fieldEntry5.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(fieldEntry5.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAPostMsgAll(fieldEntry5.postMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry6 = iter.next();
	        checkResult(fieldEntry6.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry6.name(), "MY_MSG");
	        checkResult(fieldEntry6.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(fieldEntry6.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(fieldEntry6.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPAAckMsgAll(fieldEntry6.ackMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.thomsonreuters.ema.access.FieldEntry fieldEntry7 = iter.next();
	        checkResult(fieldEntry7.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry7.name(), "MY_MSG");
	        checkResult(fieldEntry7.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(fieldEntry7.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(fieldEntry7.code(), Data.DataCode.NO_CODE);
	        EmaDecode_UPARequestMsgAll(fieldEntry7.reqMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	    
	        checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
	}
	
    static void EmaDecode_UPAArrayAll(OmmArray array)
    {
        checkResult(array.dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ARRAY);
        checkResult(array.hasFixedWidth());
        checkResult(array.fixedWidth(), 8);
        
        Iterator<OmmArrayEntry> iter = array.iterator();
        checkResult(iter != null);
        
        // check first array entry
        checkResult(iter.hasNext());
        com.thomsonreuters.ema.access.OmmArrayEntry ae1 = iter.next();
        checkResult(ae1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        ByteBuffer buffer1Contents = ae1.buffer().buffer();
        byte[] byteArray1 = new byte[buffer1Contents.limit()];
        for (int i = 0; i < buffer1Contents.limit(); i++)
        {
            byteArray1[i] = buffer1Contents.get(i);
        }
        String byteArray1String = new String(byteArray1);
        checkResult(byteArray1String.equals("BUFFER 1"));
        
        // check second array entry
        checkResult(iter.hasNext());
        com.thomsonreuters.ema.access.OmmArrayEntry ae2 = iter.next();
        checkResult(ae2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        ByteBuffer buffer2Contents = ae2.buffer().buffer();
        byte[] byteArray2 = new byte[buffer2Contents.limit()];
        for (int i = 0; i < buffer2Contents.limit(); i++)
        {
            byteArray2[i] = buffer2Contents.get(i);
        }
        String byteArray2String = new String(byteArray2);
        checkResult(byteArray2String.equals("BUFFER 2"));
        
        // check third array entry
        checkResult(iter.hasNext());
        com.thomsonreuters.ema.access.OmmArrayEntry ae3 = iter.next();
        checkResult(ae3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        ByteBuffer buffer3Contents = ae3.buffer().buffer();
        byte[] byteArray3 = new byte[buffer3Contents.limit()];
        for (int i = 0; i < buffer3Contents.limit(); i++)
        {
            byteArray3[i] = buffer3Contents.get(i);
        }
        String byteArray3String = new String(byteArray3);
        checkResult(byteArray3String.equals("BUFFER 3"));
        
        // check fourth array entry
        checkResult(iter.hasNext());
        com.thomsonreuters.ema.access.OmmArrayEntry ae4 = iter.next();
        checkResult(ae4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.BUFFER);
        ByteBuffer buffer4Contents = ae4.buffer().buffer();
        byte[] byteArray4 = new byte[buffer4Contents.limit()];
        for (int i = 0; i < buffer4Contents.limit(); i++)
        {
            byteArray4[i] = buffer4Contents.get(i);
        }
        String byteArray4String = new String(byteArray4);
        checkResult(byteArray4String.equals("BUFFER 4"));
    }
	
	static void EmaDecode_UPAElementListAll(com.thomsonreuters.ema.access.ElementList el, int encodeOption)
	{
        checkResult(el.hasInfo());
        checkResult(el.infoElementListNum(), 7);

        Iterator<ElementEntry> elIter = el.iterator();
       	checkResult(elIter != null);
       	
       	if ( (encodeOption & EncodingTypeFlags.PRIMITIVE_TYPES ) != 0 )
       	{
	        // check first element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee1 = elIter.next();
	        checkResult(ee1.name(), "Element - Time");
	        checkResult(ee1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.code(), Data.DataCode.NO_CODE);
	        checkResult(ee1.time().hour(), 10);
	        checkResult(ee1.time().minute(), 21);
	        checkResult(ee1.time().second(), 16);
	
	        //check reseting iteration to start of elementlist
	        elIter = el.iterator();
	        // check first element entry again
	        checkResult(elIter.hasNext());
	        ee1 = elIter.next();
	        checkResult(ee1.name(), "Element - Time");
	        checkResult(ee1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.code(), Data.DataCode.NO_CODE);
	        checkResult(ee1.time().hour(), 10);
	        checkResult(ee1.time().minute(), 21);
	        checkResult(ee1.time().second(), 16);
	
	        // check second element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee2 = elIter.next();
	        checkResult(ee2.name(), "Element - Int");
	        checkResult(ee2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.INT);
	        checkResult(ee2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.INT);
	        checkResult(ee2.code(), Data.DataCode.NO_CODE);
	        checkResult(ee2.intValue(), 13);
	        
	        // check third element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee3 = elIter.next();
	        checkResult(ee3.name(), "Element - UInt");
	        checkResult(ee3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(ee3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT);
	        checkResult(ee3.code(), Data.DataCode.NO_CODE);
	        checkResult(ee3.uintValue(), 17);
	       
	        // check fourth element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee4 = elIter.next();
	        checkResult(ee4.name(), "Element - Real - Blank");
	        checkResult(ee4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(ee4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REAL);
	        checkResult(ee4.code(), Data.DataCode.BLANK);
	      
	        // check fifth element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee5 = elIter.next();
	        checkResult(ee5.name(), "Element - Float");
	        checkResult(ee5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(ee5.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(ee5.code(), Data.DataCode.NO_CODE);
	        checkResult(ee5.floatValue(), 1.34f);
	
	        // check sixth element entry
	        checkResult(elIter.hasNext());
	        com.thomsonreuters.ema.access.ElementEntry ee6 = elIter.next();
	        checkResult(ee6.name(), "Element - Double");
	        checkResult(ee6.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(ee6.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(ee6.code(), Data.DataCode.NO_CODE);
	        checkResult(ee6.doubleValue(), 1.345);
	        
	        if ( encodeOption == EncodingTypeFlags.PRIMITIVE_TYPES )
	        	checkResult(!elIter.hasNext()); //final forth() - no more element entries
       	}
       	
     	if ( (encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
       	{
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee7 = elIter.next();
 			checkResult(ee7.name(), "Element - FieldList");
 			checkResult(ee7.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
        	checkResult(ee7.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
        	EmaDecode_UPAFieldListAll(ee7.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
 		
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee8 = elIter.next();
 			checkResult(ee8.name(), "Element - ElementList");
 			checkResult(ee8.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
 			checkResult(ee8.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
 			EmaDecode_UPAElementListAll(ee8.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
     		
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee9 = elIter.next();
 			checkResult(ee9.name(), "Element - FilterList");
 			checkResult(ee9.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
 			checkResult(ee9.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
 			EmaDecode_UPAFilterListAll(ee9.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee10 = elIter.next();
 			checkResult(ee10.name(), "Element - Series");
 			checkResult(ee10.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
 			checkResult(ee10.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
 			EmaDecode_UPASeriesAll(ee10.series(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	        
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee11 = elIter.next();
 			checkResult(ee11.name(), "Element - Vector");
 			checkResult(ee11.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
 			checkResult(ee11.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
 			EmaDecode_UPAVectorAll(ee11.vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee12 = elIter.next();
 			checkResult(ee12.name(), "Element - Map");
 			checkResult(ee12.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
 			checkResult(ee12.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
 			EmaDecode_UPAMapKeyUIntAll(ee12.map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			 if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
 	        	checkResult(!elIter.hasNext()); //final forth() - no more element entries
       	}
     	
     	if ( (encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
       	{
     		checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee15 = elIter.next();
 			checkResult(ee15.name(), "Element - Msg");
 			checkResult(ee15.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
 			checkResult(ee15.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
 			EmaDecode_UPARefreshMsgAll(ee15.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee16 = elIter.next();
 			checkResult(ee16.name(), "Element - Msg");
 			checkResult(ee16.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
 			checkResult(ee16.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
 			EmaDecode_UPAUpdateMsgAll(ee16.updateMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee17 = elIter.next();
 			checkResult(ee17.name(), "Element - Msg");
 			checkResult(ee17.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
 			checkResult(ee17.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
 			EmaDecode_UPAStatusMsgAll(ee17.statusMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee18 = elIter.next();
 			checkResult(ee18.name(), "Element - Msg");
 			checkResult(ee18.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
 			checkResult(ee18.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
 			EmaDecode_UPAGenericMsgAll(ee18.genericMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee19 = elIter.next();
 			checkResult(ee19.name(), "Element - Msg");
 			checkResult(ee19.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
 			checkResult(ee19.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
 			EmaDecode_UPAPostMsgAll(ee19.postMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee20 = elIter.next();
 			checkResult(ee20.name(), "Element - Msg");
 			checkResult(ee20.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
 			checkResult(ee20.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
 			EmaDecode_UPAAckMsgAll(ee20.ackMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.thomsonreuters.ema.access.ElementEntry ee21 = elIter.next();
 			checkResult(ee21.name(), "Element - Msg");
 			checkResult(ee21.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
 			checkResult(ee21.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
 			EmaDecode_UPARequestMsgAll(ee21.reqMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(!elIter.hasNext());
       	}
	}
	
	static void EmaDecode_UPAFilterListAll(com.thomsonreuters.ema.access.FilterList filterList, int encodeOption)
	{
		checkResult(filterList.hasTotalCountHint());
		
		int expectedTotalCountHint = 0;
		
		if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
			expectedTotalCountHint = 6;
		
		if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
			expectedTotalCountHint += 6;
		
        checkResult(filterList.totalCountHint(), expectedTotalCountHint);

        Iterator<FilterEntry> filterIter = filterList.iterator();
       	checkResult(filterIter != null);
       	
       	if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
    	{
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry1 = filterIter.next();
	        
	        checkResult(filterEntry1.filterId(), 1);
	        checkResult(filterEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(filterEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(filterEntry1.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry1.hasPermissionData());
	        checkResult(filterEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAFieldListAll(filterEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry2 = filterIter.next();
	        
	        checkResult(filterEntry2.filterId(), 2);
	        checkResult(filterEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(filterEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(filterEntry2.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry2.hasPermissionData());
	        checkResult(filterEntry2.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAElementListAll(filterEntry2.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry3 = filterIter.next();
	        
	        checkResult(filterEntry3.filterId(), 3);
	        checkResult(filterEntry3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(filterEntry3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(filterEntry3.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry3.hasPermissionData());
	        checkResult(filterEntry3.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAFilterListAll(filterEntry3.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry4 = filterIter.next();
	        
	        checkResult(filterEntry4.filterId(), 4);
	        checkResult(filterEntry4.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
	        checkResult(filterEntry4.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
	        checkResult(filterEntry4.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry4.hasPermissionData());
	        checkResult(filterEntry4.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPASeriesAll(filterEntry4.series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry5 = filterIter.next();
	        
	        checkResult(filterEntry5.filterId(), 5);
	        checkResult(filterEntry5.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(filterEntry5.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(filterEntry5.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry5.hasPermissionData());
	        checkResult(filterEntry5.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAVectorAll(filterEntry5.vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry6 = filterIter.next();
	        
	        checkResult(filterEntry6.filterId(), 6);
	        checkResult(filterEntry6.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
	        checkResult(filterEntry6.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
	        checkResult(filterEntry6.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry6.hasPermissionData());
	        checkResult(filterEntry6.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAMapKeyUIntAll(filterEntry6.map(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	        
	        if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
 	        	checkResult(!filterIter.hasNext()); //final forth() - no more filter entries
    	}
       	
     	if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
    	{
     		checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry7 = filterIter.next();
	        
	        checkResult(filterEntry7.filterId(), 7);
	        checkResult(filterEntry7.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(filterEntry7.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(filterEntry7.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry7.hasPermissionData());
	        checkResult(filterEntry7.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPARefreshMsgAll(filterEntry7.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry8 = filterIter.next();
	        
	        checkResult(filterEntry8.filterId(), 8);
	        checkResult(filterEntry8.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(filterEntry8.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(filterEntry8.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry8.hasPermissionData());
	        checkResult(filterEntry8.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAUpdateMsgAll(filterEntry8.updateMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry9 = filterIter.next();
	        
	        checkResult(filterEntry9.filterId(), 9);
	        checkResult(filterEntry9.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(filterEntry9.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(filterEntry9.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry9.hasPermissionData());
	        checkResult(filterEntry9.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAStatusMsgAll(filterEntry9.statusMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry10 = filterIter.next();
	        
	        checkResult(filterEntry10.filterId(), 10);
	        checkResult(filterEntry10.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(filterEntry10.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(filterEntry10.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry10.hasPermissionData());
	        checkResult(filterEntry10.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAGenericMsgAll(filterEntry10.genericMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry11 = filterIter.next();
	        
	        checkResult(filterEntry11.filterId(), 11);
	        checkResult(filterEntry11.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(filterEntry11.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(filterEntry11.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry11.hasPermissionData());
	        checkResult(filterEntry11.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAPostMsgAll(filterEntry11.postMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.thomsonreuters.ema.access.FilterEntry filterEntry12 = filterIter.next();
	        
	        checkResult(filterEntry12.filterId(), 12);
	        checkResult(filterEntry12.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(filterEntry12.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(filterEntry12.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry12.hasPermissionData());
	        checkResult(filterEntry12.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPAAckMsgAll(filterEntry12.ackMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.thomsonreuters.ema.access.FilterEntry filterEntry13 = filterIter.next();
	        
	        checkResult(filterEntry13.filterId(), 13);
	        checkResult(filterEntry13.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(filterEntry13.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(filterEntry13.action(), com.thomsonreuters.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry13.hasPermissionData());
	        checkResult(filterEntry13.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_UPARequestMsgAll(filterEntry13.reqMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(!filterIter.hasNext());
    	}
	}
	
	static void EmaDecode_UPASeriesAll(com.thomsonreuters.ema.access.Series series, int containerType)
	{
		checkResult(series.hasTotalCountHint());
		
		int expectedTotalCountHint = 2;
		
		checkResult(series.totalCountHint(), expectedTotalCountHint);
		
		Iterator<SeriesEntry> seriesIter = series.iterator();
	    checkResult(seriesIter != null);
		
		switch( containerType )
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(series.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(seriesEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(seriesEntry2.fieldList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(series.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(seriesEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(seriesEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(series.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(seriesEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(seriesEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(series.summaryData().series(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(seriesEntry1.series(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(seriesEntry2.series(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(series.summaryData().vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(seriesEntry1.vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(seriesEntry2.vector(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(series.summaryData().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
						
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(seriesEntry1.map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(seriesEntry2.map(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(series.summaryData().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(seriesEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(seriesEntry1.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.thomsonreuters.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(seriesEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(seriesEntry2.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		}
	}
	
	static void EmaDecode_UPAVectorAll(com.thomsonreuters.ema.access.Vector vector, int containerType)
	{
		checkResult(vector.hasTotalCountHint());
		
		int expectedTotalCountHint = 2;
		
		checkResult(vector.totalCountHint(), expectedTotalCountHint);
		
		Iterator<VectorEntry> vectorIter = vector.iterator();
	    checkResult(vectorIter != null);
		
		switch( containerType )
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(vector.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(vectorEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(vectorEntry2.fieldList(), EncodingTypeFlags.MESSAGE_TYPES);
		}		
		break;
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(vector.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(vectorEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(vectorEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(vector.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(vectorEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(vectorEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(vector.summaryData().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(vectorEntry1.series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(vectorEntry2.series(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(vector.summaryData().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			//
						
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(vectorEntry1.vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(vectorEntry2.vector(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(vector.summaryData().map(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(vectorEntry1.map(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(vectorEntry2.map(), com.thomsonreuters.upa.codec.DataTypes.MSG);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(vector.summaryData().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(vectorEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(vectorEntry1.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(vectorIter.hasNext());
			com.thomsonreuters.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.thomsonreuters.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(vectorEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(vectorEntry2.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		}
	}
	
	static void EmaDecode_UPAMapKeyUIntAll(com.thomsonreuters.ema.access.Map map, int containerType)
	{
		checkResult(map.hasTotalCountHint());
		
		int expectedTotalCountHint = 3;
		
		checkResult(map.hasKeyFieldId());
		checkResult(map.keyFieldId(), 3426);
		
		checkResult(map.hasTotalCountHint());
		checkResult(map.totalCountHint(), expectedTotalCountHint);
		
		Iterator<MapEntry> mapIter = map.iterator();
	    checkResult(mapIter != null);
	    
		switch( containerType )
		{
		case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(map.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(mapEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_UPAFieldListAll(mapEntry2.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(map.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(mapEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_UPAElementListAll(mapEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(map.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(mapEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_UPAFilterListAll(mapEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.SERIES:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(map.summaryData().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(mapEntry1.series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_UPASeriesAll(mapEntry2.series(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(map.summaryData().vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(mapEntry1.vector(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_UPAVectorAll(mapEntry2.vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MAP:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(map.summaryData().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(mapEntry1.map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.MAP);
			EmaDecode_UPAMapKeyUIntAll(mapEntry2.map(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.thomsonreuters.upa.codec.DataTypes.MSG:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(map.summaryData().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(mapEntry1.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(mapEntry1.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.thomsonreuters.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(mapEntry2.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_UPARefreshMsgAll(mapEntry2.refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
		}	
		break;
		}
		
		checkResult(mapIter.hasNext());
	    com.thomsonreuters.ema.access.MapEntry mapEntry3 = mapIter.next();
	    checkResult(mapEntry3.action(), com.thomsonreuters.ema.access.MapEntry.MapAction.DELETE);
	    checkResult(mapEntry3.key().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.UINT );
	    checkResult(mapEntry3.key().uintValue(), 3 );
	    checkResult(mapEntry3.loadType(), com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA);
		checkResult(mapEntry3.load().dataType(), com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA);
	    
		checkResult(!mapIter.hasNext());
	}
	
	static void EmaEncodeFieldListAll( FieldList fl )
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING,

		fl.info( 1, 65 );

		fl.add(EmaFactory.createFieldEntry().uintValue( 1, 64));

		fl.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		fl.add(EmaFactory.createFieldEntry().intValue( -2, 32));

		fl.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));

		fl.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));

		fl.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));

		fl.add(EmaFactory.createFieldEntry().qos( MY_QOS, OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));

		fl.add(EmaFactory.createFieldEntry().state( MY_STATE, OmmState.StreamState.OPEN,
																			OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));

		fl.add(EmaFactory.createFieldEntry().ascii( 235, "ABCDEF"));
	}
	
	static void EmaEncodeElementListAll( ElementList el)
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, QOS, STATE, ASCII_STRING, RMTES_STRING, ENUM, FLOAT, DOUBLE, REALBLANK, BUFFER, UTF8
		
		el.info( 5 );

		//first entry
		el.add(EmaFactory.createElementEntry().uintValue("Element - UInt" , 64));

		//second entry
		el.add(EmaFactory.createElementEntry().real("Element - Real", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		//third entry
		el.add(EmaFactory.createElementEntry().intValue("Element - Int" , 32));

		//fourth entry
		el.add(EmaFactory.createElementEntry().date("Element - Date" , 1999, 11, 7));

		//fifth entry
		el.add(EmaFactory.createElementEntry().time("Element - Time" , 02, 03, 04, 005));

		//sixth entry
		el.add(EmaFactory.createElementEntry().dateTime("Element - DATETIME" , 1999, 11, 7, 01, 02, 03, 000));

		//seventh entry
		el.add(EmaFactory.createElementEntry().qos("Element - Qos" , OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));

		//eightth entry
		el.add(EmaFactory.createElementEntry().state("Element - State" , OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));

		//ninth entry
		el.add(EmaFactory.createElementEntry().ascii("Element - AsciiString" , "ABCDEF"));

		//tenth entry
		ByteBuffer value = (ByteBuffer)ByteBuffer.wrap("ABCDEF".getBytes());
		el.add(EmaFactory.createElementEntry().rmtes("Element - RmtesString" , value));

		//eleventh entry
		el.add(EmaFactory.createElementEntry().enumValue("Element - Enum" , 29));

		//twelfth entry
		el.add(EmaFactory.createElementEntry().floatValue("Element - Float" , 11.11f));

		//thirteenth entry
		el.add(EmaFactory.createElementEntry().doubleValue("Element - Double" , 22.22f));

		//fourteenth entry
		el.add(EmaFactory.createElementEntry().codeReal("Element - RealBlank"));

		//fifteenth entry
		value = (ByteBuffer)ByteBuffer.wrap("ABCDEFGH".getBytes());
		el.add(EmaFactory.createElementEntry().buffer("Element - Buffer", value));

		//sixteenth entry
		el.add(EmaFactory.createElementEntry().utf8("Element - Utf8String" , "ABCDEFGH"));
	}
	static void EmaEncodeMapAllWithFieldList( Map map)
	{
		// encoding order:  SummaryData(with FieldList),
		//                  Buffer, FieldList-Delete,
		//                  Buffer, FieldList-Add,
		//                  Buffer, FieldList-Add,
		//                  Buffer, FieldList-Update,

		map.totalCountHint(5);
		map.keyFieldId(3426);

		FieldList flEnc = EmaFactory.createFieldList();
		EmaEncodeFieldListAll(flEnc);

		map.summaryData(flEnc);

		ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =(ByteBuffer)ByteBuffer.wrap("ABCD".getBytes());
		
		flEnc.clear();
		EmaEncodeFieldListAll(flEnc);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, flEnc, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//third entry  //Add FieldList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//fourth entry  //Update FieldList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, flEnc, permission));
	}
	
	static void EmaEncodeFilterListAllWithFieldListElementList( FilterList filterList)
	{
		//                  FieldList-Clear,
		//                  FieldList-Set,
		//                  ElementList-Update,

		filterList.totalCountHint(3);

		FieldList flEnc = EmaFactory.createFieldList();
		EmaEncodeFieldListAll(flEnc);

		ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Clear FieldList
		filterList.add(EmaFactory.createFilterEntry().fieldList(1, FilterEntry.FilterAction.CLEAR, flEnc, permission));

		//second entry  //Add FieldList
		filterList.add(EmaFactory.createFilterEntry().fieldList(2, FilterEntry.FilterAction.SET, flEnc));

		ElementList elEnc = EmaFactory.createElementList();
		EmaEncodeElementListAll(elEnc);

		//third entry  //Add FieldList
		filterList.add(EmaFactory.createFilterEntry().elementList(3, FilterEntry.FilterAction.UPDATE, elEnc, permission));
	}
	
	static void EmaEncodeVectorAllWithFieldList( Vector vector)
	{
		// encoding order:  SummaryData(with FieldList),
		//                  FieldList-Delete,
		//                  FieldList-Set,
		//                  FieldList-Set,
		//                  FieldList-Update,

		vector.totalCountHint(5);

		FieldList flEnc = EmaFactory.createFieldList();
		EmaEncodeFieldListAll(flEnc);

		vector.summaryData(flEnc);

		ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Delete Buffer
		flEnc.clear();
		EmaEncodeFieldListAll(flEnc);
		
		vector.add(EmaFactory.createVectorEntry().fieldList(1, VectorEntry .VectorAction.DELETE, flEnc, permission));

		//second entry  //Add FieldList
		vector.add(EmaFactory.createVectorEntry ().fieldList(1, VectorEntry .VectorAction.SET, flEnc, permission));

		//third entry  //Add FieldList
		vector.add(EmaFactory.createVectorEntry ().fieldList(2, VectorEntry .VectorAction.SET, flEnc, permission));

		//fourth entry  //Update FieldList
		vector.add(EmaFactory.createVectorEntry ().fieldList(3, VectorEntry .VectorAction.UPDATE, flEnc, permission));
	}
	
	static void EmaEncodeMapAllWithFieldListWithUtf8Key( Map map)
	{
		// encoding order:  SummaryData(with FieldList),
		//                  UTF8, FieldList-Delete,
		//                  UTF8,FieldList-Add,
		//                  UTF8,FieldList-Add,
		//                  UTF8,FieldList-Update,

		map.totalCountHint(5);
		map.keyFieldId(3426);

		FieldList flEnc = EmaFactory.createFieldList();
		EmaEncodeFieldListAll(flEnc);

		map.summaryData(flEnc);

		ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =(ByteBuffer)ByteBuffer.wrap("ABC".getBytes());
		
		flEnc.clear();
		EmaEncodeFieldListAll(flEnc);
		
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.DELETE, flEnc, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//third entry  //Add FieldList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("DEFGH".getBytes());
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//fourth entry  //Update FieldList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("KLMNOPQRS".getBytes());
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.UPDATE, flEnc, permission));
	}

	static void EmaEncodeMapAllWithElementList( Map map)
	{
		// encoding order:  SummaryData(with ElementList),
		//                  Buffer, ElementList-Delete,
		//                  Buffer,ElementList-Add,
		//                  Buffer,ElementList-Add,
		//                  Buffer,ElementList-Update,

		map.totalCountHint(5);
		map.keyFieldId(3426);

		ElementList elEnc = EmaFactory.createElementList(); 
		EmaEncodeElementListAll(elEnc);

		map.summaryData(elEnc);

		ByteBuffer permission =  (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());;

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =(ByteBuffer)ByteBuffer.wrap("ABCD".getBytes());
		elEnc.clear();
		EmaEncodeElementListAll(elEnc);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, elEnc, permission));

		//second entry  //Add ElementList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, elEnc, permission));

		//third entry  //Add ElementList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, elEnc, permission));

		//fourth entry  //Update ElementList
		orderBuf =(ByteBuffer)ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, elEnc, permission));
	}
	
	static void EmaEncodeSeriesAllWithElementList( Series series)
	{
		// encoding order:  SummaryData(with ElementList),
		//                  ElementList,
		//                  ElementList,
		//                  ElementList,

		series.totalCountHint(5);

		ElementList elEnc = EmaFactory.createElementList(); 
		EmaEncodeElementListAll(elEnc);

		series.summaryData(elEnc);

		elEnc.clear();
		EmaEncodeElementListAll(elEnc);
		
		//first entry  //Add ElementList
		series.add(EmaFactory.createSeriesEntry().elementList(elEnc));

		//second entry  //Add ElementList
		series.add(EmaFactory.createSeriesEntry().elementList(elEnc));

		//third entry  //Add ElementList
		series.add(EmaFactory.createSeriesEntry().elementList(elEnc));
	}
	
	static void EmaEncodeMapAllWithMap( Map map)
	{
		// encoding order:  SummaryData(with FieldList),
		//                  Buffer, Map-Delete,
		//                  Buffer, Map-Add,
		//                  Buffer, Map-Add,
		//                  Buffer, Map-Update,

		map.totalCountHint(5);
		map.keyFieldId(3426);

		Map mapEncS = EmaFactory.createMap();
		EmaEncodeMapAllWithFieldList(mapEncS);
		
		map.summaryData(mapEncS);

		ByteBuffer permission = (ByteBuffer)ByteBuffer.wrap("PERMISSION DATA".getBytes());
		
		//first entry  //Delete Buffer
		ByteBuffer orderBuf = (ByteBuffer)ByteBuffer.wrap("ABCD".getBytes());
		
		mapEncS.clear();
		EmaEncodeMapAllWithFieldList(mapEncS);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, mapEncS, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, mapEncS, permission));

		//third entry  //Add FieldList
		orderBuf = (ByteBuffer)ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, mapEncS, permission));

		//fourth entry  //Update FieldList
		orderBuf = (ByteBuffer)ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, mapEncS, permission));
	}
	
	static void EmaDecodeFieldListAll(FieldList fl)
	{
		checkResult( fl.hasInfo(), "Decode FieldList - hasInfo()" );
		checkResult( fl.infoDictionaryId() == 1, "Decode FieldList - getInfoDictionaryId()" );
		checkResult( fl.infoFieldListNum() == 65, "Decode FieldList - getInfoFieldListNum()" );

		Iterator<FieldEntry> iter = fl.iterator();
       	checkResult(iter != null);

		checkResult( iter.hasNext(), "Decode FieldList - first fieldlist iter.hasNext()" );
		FieldEntry fe1 = iter.next();
		checkResult( fe1.fieldId() == 1, "FieldEntry::getFieldId()" );
		checkResult( fe1.name().equals("PROD_PERM"), "FieldEntry::getName()" );
		checkResult( fe1.loadType() == DataTypes.UINT, "FieldEntry.loadType() == DataTypes.UInt" );
		checkResult( fe1.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe1.uintValue() == 64, "FieldEntry::getUInt()" );

		checkResult( iter.hasNext(), "Decode FieldList - second fieldlist iter.hasNext()" );
		FieldEntry fe2 = iter.next();
		checkResult( fe2.fieldId() == 6, "FieldEntry::getFieldId()" );
		checkResult( fe2.name().equals("TRDPRC_1"), "FieldEntry::getName()" );
		checkResult( fe2.loadType() == DataTypes.REAL, "FieldEntry.loadType() == DataTypes.REAL" );
		checkResult( fe2.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe2.real().mantissa() == 11, "FieldEntry::getReal().mantissa()" );
		checkResult( fe2.real().magnitudeType() == 12, "FieldEntry::getReal().magnitudeType()" );
				
		checkResult( iter.hasNext(), "Decode FieldList - third fieldlist iter.hasNext()" );
		FieldEntry fe3 = iter.next();
		checkResult( fe3.fieldId() == -2, "FieldEntry::getFieldId()" );
		checkResult( fe3.name().equals("INTEGER"), "FieldEntry::getName()" );
		checkResult( fe3.loadType() == DataTypes.INT, "FieldEntry.loadType() == DataTypes.Int" );
		checkResult( fe3.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe3.intValue() == 32, "FieldEntry::getInt()" );

		checkResult( iter.hasNext(), "Decode FieldList - fourth fieldlist iter.hasNext()" );
		FieldEntry fe4 = iter.next();
		checkResult( fe4.fieldId() == 16, "FieldEntry::getFieldId()" );
		checkResult( fe4.name().equals("TRADE_DATE"), "FieldEntry::getName()" );
		checkResult( fe4.loadType() == DataTypes.DATE, "FieldEntry.loadType() == DataTypes.DATE" );
		checkResult( fe4.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe4.date().day() == 7, "FieldEntry::getDate().day()" );
		checkResult( fe4.date().month() == 11, "FieldEntry::getDate().month()" );
		checkResult( fe4.date().year() == 1999, "FieldEntry::getDate().year()" );

		checkResult( iter.hasNext(), "Decode FieldList - fifth fieldlist iter.hasNext()" );
		FieldEntry fe5 = iter.next();
		checkResult( fe5.fieldId() == 18, "FieldEntry::getFieldId()" );
		checkResult( fe5.name().equals("TRDTIM_1"), "FieldEntry::getName()" );
		checkResult( fe5.loadType() == DataTypes.TIME, "FieldEntry.loadType() == DataTypes.Time" );
		checkResult( fe5.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe5.time().hour() == 02, "FieldEntry::getTime().hour()" );
		checkResult( fe5.time().minute() == 03, "FieldEntry::getTime().minute()" );
		checkResult( fe5.time().second() == 04, "FieldEntry::getTime().second()" );
		checkResult( fe5.time().millisecond() == 005, "FieldEntry::getTime().millisecond()" );

		checkResult( iter.hasNext(), "Decode FieldList - sixth fieldlist iter.hasNext()" );
		FieldEntry fe6 = iter.next();
		checkResult( fe6.fieldId() == -3, "FieldEntry::getFieldId()" );
		checkResult( fe6.name().equals("TRADE_DATE"), "FieldEntry::getName()" );
		checkResult( fe6.loadType() == DataTypes.DATETIME, "FieldEntry.loadType() == DataTypes.dateTime" );
		checkResult( fe6.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe6.dateTime().day() == 7, "FieldEntry::getDATETIME().day()" );
		checkResult( fe6.dateTime().month() == 11, "FieldEntry::getDATETIME().month()" );
		checkResult( fe6.dateTime().year() == 1999, "FieldEntry::getDATETIME().year()" );
		checkResult( fe6.dateTime().hour() == 01, "FieldEntry::getDATETIME().hour()" );
		checkResult( fe6.dateTime().minute() == 02, "FieldEntry::getDATETIME().minute()" );
		checkResult( fe6.dateTime().second() == 03, "FieldEntry::getDATETIME().second()" );
		checkResult( fe6.dateTime().millisecond() == 000, "FieldEntry::getDATETIME().millisecond()" );

		checkResult( iter.hasNext(), "Decode FieldList - seventh fieldlist iter.hasNext()" );
		FieldEntry fe7 = iter.next();
		checkResult( fe7.fieldId() == MY_QOS, "FieldEntry::getFieldId()" );
		checkResult( fe7.name().equals("MY_QOS"), "FieldEntry::getName()" );
		checkResult( fe7.loadType() == DataTypes.QOS, "FieldEntry.loadType() == DataTypes.QOS" );
		checkResult( fe7.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe7.qos().timeliness() == OmmQos.Timeliness.REALTIME, "FieldEntry::getTime().timeliness()" );
		checkResult( fe7.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "FieldEntry::getTime().rate()" );

		checkResult( iter.hasNext(), "Decode FieldList - eigtth fieldlist iter.hasNext()" );
		FieldEntry fe8 = iter.next();
		checkResult( fe8.fieldId() == MY_STATE, "FieldEntry::getFieldId()" );
		checkResult( fe8.name().equals("MY_STATE"), "FieldEntry::getName()" );
		checkResult( fe8.loadType() == DataTypes.STATE, "FieldEntry.loadType() == DataTypes.STATE" );
		checkResult( fe8.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe8.state().streamState() == OmmState.StreamState.OPEN, "FieldEntry::getState().streamState()" );
		checkResult( fe8.state().dataState() == OmmState.DataState.OK, "FieldEntry::getState().dataState()" );
		checkResult( fe8.state().statusCode() == OmmState.StatusCode.NONE, "FieldEntry::getState().statusCode()" );
		checkResult( fe8.state().statusText().equals("Succeeded"), "FieldEntry::getState().statusText()" );
		checkResult( fe8.state().toString().equals("Open / Ok / None / 'Succeeded'"), "FieldEntry::getState().toString()" );
			
		checkResult( iter.hasNext(), "Decode FieldList - ninth fieldlist iter.hasNext()" );
		FieldEntry fe9 = iter.next();
		checkResult( fe9.fieldId() == 235, "FieldEntry::getFieldId()" );
		checkResult( fe9.name().equals("PNAC"), "FieldEntry::getName()" );
		checkResult( fe9.loadType() == DataTypes.ASCII, "FieldEntry.loadType() == DataTypes.ASCII" );
		checkResult( fe9.code() == DataCode.NO_CODE, "FieldEntry::code() == DataCode.NO_CODE" );
		checkResult( fe9.ascii().ascii().equals( "ABCDEF"), "FieldEntry.ascii()" );

		checkResult( !iter.hasNext(), "Decode FieldList - final fieldlist iter.hasNext()" );
	}


	//corresponds to TestUtilities::EmaEncodeElementListAll(ElementList el)
	static void EmaDecodeElementListAll(ElementList el)
	{
		checkResult( el.hasInfo(), "Decode ElementList - hasInfo()" );
		checkResult( el.infoElementListNum() == 5, "MapEntry ElementList within map- getInfoElementListNum()" );

		Iterator<ElementEntry> elIter = el.iterator();
       	checkResult(elIter != null);
        
		checkResult(elIter.hasNext(), "Decode ElementList within map - first elementlist elIter.hasNext()" );
		ElementEntry ee1 = elIter.next();
		checkResult( ee1.name().equals("Element - UInt"), "ElementEntry::getName()" );
		checkResult( ee1.loadType() == DataTypes.UINT, "ElementEntry.loadType() == DataTypes.UInt" );
		checkResult( ee1.uintValue() == 64, "ElementEntry::getUInt()" );

		checkResult(elIter.hasNext(), "Decode ElementList within map - second elementlist elIter.hasNext()" );
		ElementEntry ee2 = elIter.next();
		checkResult( ee2.name().equals("Element - Real"), "ElementEntry::getName()" );
		checkResult( ee2.loadType() == DataTypes.REAL, "ElementEntry.loadType() == DataTypes.REAL" );
		checkResult( ee2.real().mantissa() == 11, "ElementEntry::getReal().mantissa()" );
		checkResult( ee2.real().magnitudeType() == 12, "ElementEntry::getReal().magnitudeType()" );

		checkResult(elIter.hasNext(), "Decode Decode ElementList - third elementlist elIter.hasNext()" );
		ElementEntry ee3 = elIter.next();
		checkResult( ee3.name().equals("Element - Int"), "ElementEntry::getName()" );
		checkResult( ee3.loadType() == DataTypes.INT, "ElementEntry.loadType() == DataTypes.Int" );
		checkResult( ee3.load().dataType() == DataTypes.INT, "ElementEntry.load().dataType() == DataTypes.Int" );
		checkResult( ee3.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee3.intValue() == 32, "ElementEntry::getInt()" );

		checkResult(elIter.hasNext(), "Decode ElementList - fourth elementlist elIter.hasNext()" );
		ElementEntry ee4 = elIter.next();
		checkResult( ee4.name().equals("Element - Date"), "ElementEntry::getName()" );
		checkResult( ee4.loadType() == DataTypes.DATE, "ElementEntry.loadType() == DataTypes.DATE" );
		checkResult( ee4.load().dataType() == DataTypes.DATE, "ElementEntry.load().dataType() == DataTypes.DATE" );
		checkResult( ee4.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee4.date().day() == 7, "ElementEntry.date().day()" );
		checkResult( ee4.date().month() == 11, "ElementEntry.date().month()" );
		checkResult( ee4.date().year() == 1999, "ElementEntry.date().year()" );

		checkResult(elIter.hasNext(), "Decode ElementList - fifth elementlist elIter.hasNext()" );
		ElementEntry ee5 = elIter.next();
		checkResult( ee5.name().equals("Element - Time"), "ElementEntry::getName()" );
		checkResult( ee5.loadType() == DataTypes.TIME, "ElementEntry.loadType() == DataTypes.Time" );
		checkResult( ee5.load().dataType() == DataTypes.TIME, "ElementEntry.load().dataType() == DataTypes.Time" );
		checkResult( ee5.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee5.time().hour() == 02, "ElementEntry::getTime().hour()" );
		checkResult( ee5.time().minute() == 03, "ElementEntry::getTime().minute()" );
		checkResult( ee5.time().second() == 04, "ElementEntry::getTime().second()" );
		checkResult( ee5.time().millisecond() == 005, "ElementEntry::getTime().millisecond()" );

		checkResult(elIter.hasNext(), "Decode ElementList - sixth elementlist elIter.hasNext()" );
		ElementEntry ee6 = elIter.next();
		checkResult( ee6.name().equals("Element - DATETIME"), "ElementEntry::getName()" );
		checkResult( ee6.loadType() == DataTypes.DATETIME, "ElementEntry.loadType() == DataTypes.dateTime" );
//		checkResult( ee6.load().dataType() == DataTypes.DATETIME, "ElementEntry.load().dataType() == DataTypes.dateTime" );
		checkResult( ee6.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee6.dateTime().day() == 7, "ElementEntry.dateTime().day()" );
		checkResult( ee6.dateTime().month() == 11, "ElementEntry.dateTime().month()" );
		checkResult( ee6.dateTime().year() == 1999, "ElementEntry.dateTime().year()" );
		checkResult( ee6.dateTime().hour() == 01, "ElementEntry.dateTime().hour()" );
		checkResult( ee6.dateTime().minute() == 02, "ElementEntry.dateTime().minute()" );
		checkResult( ee6.dateTime().second() == 03, "ElementEntry.dateTime().second()" );
		checkResult( ee6.dateTime().millisecond() == 000, "ElementEntry.dateTime().millisecond()" );

		checkResult(elIter.hasNext(), "Decode ElementList - seventh elementlist elIter.hasNext()" );
		ElementEntry ee7 = elIter.next();
		checkResult( ee7.name().equals("Element - Qos"), "ElementEntry::getName()" );
		checkResult( ee7.loadType() == DataTypes.QOS, "ElementEntry.loadType() == DataTypes.QOS" );
		checkResult( ee7.load().dataType() == DataTypes.QOS, "ElementEntry.load().dataType() == DataTypes.QOS" );
		checkResult( ee7.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee7.qos().timeliness() == OmmQos.Timeliness.REALTIME, "ElementEntry::getTime().timeliness()" );
		checkResult( ee7.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "ElementEntry::getTime().rate()" );
//		checkResult( ee7..qos().rate() == 1, "ElementEntry::getTime().rate()" );

		checkResult(elIter.hasNext(), "Decode ElementList - eightth elementlist elIter.hasNext()" );
		ElementEntry ee8 = elIter.next();
		checkResult( ee8.name().equals("Element - State"), "ElementEntry::getName()" );
		checkResult( ee8.loadType() == DataTypes.STATE, "ElementEntry.loadType() == DataTypes.STATE" );
		checkResult( ee8.load().dataType() == DataTypes.STATE, "ElementEntry.load().dataType() == DataTypes.STATE" );
		checkResult( ee8.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee8.state().streamState() == OmmState.StreamState.OPEN, "ElementEntry::getState().streamState()" );
		checkResult( ee8.state().dataState() == OmmState.DataState.OK, "ElementEntry::getState().dataState()" );
		checkResult( ee8.state().statusCode() == OmmState.StatusCode.NONE, "ElementEntry::getState().statusCode()" );
		checkResult( ee8.state().statusText().equals("Succeeded"), "ElementEntry::getState().statusText()" );
		checkResult( ee8.state().toString().equals( "Open / Ok / None / 'Succeeded'"), "ElementEntry::getState().toString()" );

		checkResult(elIter.hasNext(), "Decode ElementList - ninth elementlist elIter.hasNext()" );
		ElementEntry ee9 = elIter.next();
		checkResult( ee9.name().equals("Element - AsciiString"), "ElementEntry::getName()" );
		checkResult( ee9.loadType() == DataTypes.ASCII, "ElementEntry.loadType() == DataTypes.ASCII" );
		checkResult( ee9.load().dataType() == DataTypes.ASCII, "ElementEntry.load().dataType() == DataTypes.ASCII" );
		checkResult( ee9.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee9.ascii().ascii().equals("ABCDEF"), "ElementEntry.ascii()" );

		checkResult(elIter.hasNext(), "Decode ElementList - tenth elementlist elIter.hasNext()" );
		ElementEntry ee10= elIter.next();
		checkResult( ee10.name().equals("Element - RmtesString"), "ElementEntry::getName()" );
		checkResult( ee10.loadType() == DataTypes.RMTES, "ElementEntry.loadType() == DataTypes.RMTES" );
		checkResult( ee10.load().dataType() == DataTypes.RMTES, "ElementEntry.load().dataType() == DataTypes.RMTES" );
		checkResult( ee10.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );

		checkResult(elIter.hasNext(), "Decode ElementList - eleventh elementlist elIter.hasNext()" );
		ElementEntry ee11 = elIter.next();
		checkResult( ee11.name().equals("Element - Enum"), "ElementEntry::getName()" );
		checkResult( ee11.loadType() == DataTypes.ENUM, "ElementEntry.loadType() == DataTypes." );
		checkResult( ee11.load().dataType() == DataTypes.ENUM, "ElementEntry.load().dataType() == DataTypes." );
		checkResult( ee11.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee11.enumValue() == 29, "ElementEntry.enumValue()" );

		checkResult(elIter.hasNext(), "Decode ElementList - twelfth elementlist elIter.hasNext()" );
		ElementEntry ee12 = elIter.next();
		checkResult( ee12.name().equals("Element - Float"), "ElementEntry::getName()" );
		checkResult( ee12.loadType() == DataTypes.FLOAT, "ElementEntry.loadType() == DataTypes.FLOAT" );
		checkResult( ee12.load().dataType() == DataTypes.FLOAT, "ElementEntry.load().dataType() == DataTypes.FLOAT" );
		checkResult( ee12.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee12.floatValue() == 11.11f, "ElementEntry::getFloat()" );

		checkResult(elIter.hasNext(), "Decode ElementList - thirteenth elementlist elIter.hasNext()" );
		ElementEntry ee13 = elIter.next();
		checkResult( ee13.name().equals("Element - Double"), "ElementEntry::getName()" );
		checkResult( ee13.loadType() == DataTypes.DOUBLE, "ElementEntry.loadType() == DataTypes.DOUBLE" );
		checkResult( ee13.load().dataType() == DataTypes.DOUBLE, "ElementEntry.load().dataType() == DataTypes.DOUBLE" );
		checkResult( ee13.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( ee13.doubleValue() == 22.22f, "ElementEntry.double()" );
			
		checkResult(elIter.hasNext(), "Decode ElementList - fourteenth elementlist elIter.hasNext()" );
		ElementEntry ee14 = elIter.next();
		checkResult( ee14.name().equals("Element - RealBlank"), "ElementEntry::getName()" );
		checkResult( ee14.loadType() == DataTypes.REAL, "ElementEntry.loadType() == DataTypes.REAL" );
		checkResult( ee14.load().dataType() == DataTypes.REAL, "ElementEntry.load().dataType() == DataTypes.REAL" );
		checkResult( ee14.code() == DataCode.BLANK, "ElementEntry::code() == DataCode.BLANK" );

		checkResult(elIter.hasNext(), "Decode ElementList - fifteenth elementlist elIter.hasNext()" );
		ElementEntry ee15 = elIter.next();
		checkResult( ee15.name().equals("Element - Buffer"), "ElementEntry::getName()" );
		checkResult( ee15.loadType() == DataTypes.BUFFER, "ElementEntry.loadType() == DataTypes.BUFFER" );
		checkResult( ee15.load().dataType() == DataTypes.BUFFER, "ElementEntry.load().dataType() == DataTypes.BUFFER" );
		checkResult( ee15.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult( Arrays.equals(ee15.buffer().buffer().array() , new String("ABCDEFGH").getBytes()), "ElementEntry.buffer()");

		checkResult(elIter.hasNext(), "Decode ElementList - sixteenth elementlist elIter.hasNext()" );
		ElementEntry ee16 = elIter.next();
		checkResult( ee16.name().equals("Element - Utf8String"), "ElementEntry::getName()" );
		checkResult( ee16.loadType() == DataTypes.UTF8, "ElementEntry.loadType() == DataTypes.Utf8" );
		checkResult( ee16.load().dataType() == DataTypes.UTF8, "ElementEntry.load().dataType() == DataTypes.Utf8" );
		checkResult( ee16.code() == DataCode.NO_CODE, "ElementEntry::code() == DataCode.NO_CODE" );
		checkResult(Arrays.equals( ee16.utf8().buffer().array(), new String("ABCDEFGH").getBytes()), "ElementEntry.utf8()" );

		checkResult( !elIter.hasNext(), "Decode ElementList - final elementlist elIter.hasNext()" );
	}

	//corresponds to TestUtilities::EmaEncodeMapAllWithFieldList(Map map)
	static void EmaDecodeMapAllWithFieldList(Map map)
	{
		checkResult( map.hasKeyFieldId(), "Decode Map contains FieldList - hasKeyFieldId()" );
		checkResult( map.keyFieldId() == 3426, "Decode Map contains FieldList - getKeyFieldId()" );
		checkResult( map.hasTotalCountHint(), "Decode Map contains FieldList - hasTotalCountHint()" );
		checkResult( map.totalCountHint() == 5, "Decode Map contains FieldList - getTotalCountHint()" );
			
		switch ( map.summaryData().dataType() )
		{
			case DataTypes.FIELD_LIST :
			{
				FieldList fl = map.summaryData().fieldList();
				EmaDecodeFieldListAll(fl);
			}
			break;
			default :
				checkResult( false, "Decode Map Decode Summary FieldList - map.summaryType() not expected" );
			break;
		}

		Iterator<MapEntry> mapIter = map.iterator();
			
		checkResult( mapIter.hasNext(), "Map contains FieldList - first map iter.hasNext()" );

		MapEntry me1 = mapIter.next();

		checkResult( me1.key().dataType() == DataTypes.BUFFER, "MapEntry::getKey().dataType() == DataTypes.BUFFER" );
			{
				checkResult( Arrays.equals(me1.key().buffer().buffer().array() , new String("ABCD").getBytes()), "MapEntry::getKey().buffer()" );
			}
		checkResult( me1.action() == MapAction.DELETE, "MapEntry.action() == MapAction.DELETE" );
		checkResult( me1.load().dataType() == DataTypes.NO_DATA, "MapEntry.load().dataType() == DataTypes.NoData" );


		checkResult( mapIter.hasNext(), "Decode Map contains FieldList - second map iter.hasNext()" );

		MapEntry me2 = mapIter.next();

		checkResult( me2.key().dataType() == DataTypes.BUFFER, "MapEntry::getKey().dataType() == DataTypes.BUFFER" );
			{
				checkResult( Arrays.equals(me2.key().buffer().buffer().array() , new String("ABCD").getBytes()),"MapEntry::getKey().buffer()");
			}
		checkResult( me2.action() == MapAction.ADD, "MapEntry.action() == MapAction.ADD" );
		checkResult( me2.load().dataType() == DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.FieldList" );
		{
			FieldList fl = me2.fieldList();
			EmaDecodeFieldListAll(fl);
		}


		checkResult( mapIter.hasNext(), "Map contains FieldList - third map iter.hasNext()" );

		MapEntry me3 = mapIter.next();

		checkResult( me3.key().dataType() == DataTypes.BUFFER, "MapEntry::getKey().dataType() == DataTypes.BUFFER" );
			{
				checkResult( Arrays.equals(me3.key().buffer().buffer().array() , new String("EFGHI").getBytes()), "MapEntry::getKey().buffer()" );
			}
		checkResult( me3.action() == MapAction.ADD, "MapEntry.action() == MapAction.ADD" );
		checkResult( me3.load().dataType() == DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.NoData" );
		{
			FieldList fl = me3.fieldList();
			EmaDecodeFieldListAll(fl);
		}

			
		checkResult( mapIter.hasNext(), "Map contains FieldList - fourth map iter.hasNext()" );

		MapEntry me4 = mapIter.next();

		checkResult( me4.key().dataType() == DataTypes.BUFFER, "MapEntry::getKey().dataType() == DataTypes.BUFFER" );
			{
				checkResult( Arrays.equals(me4.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()), "MapEntry::getKey().buffer()" );
			}
		checkResult( me4.action() == MapAction.UPDATE, "MapEntry.action() == MapAction.UPDATE" );
		checkResult( me4.load().dataType() == DataTypes.FIELD_LIST, "MapEntry.load().dataType() == DataTypes.FieldList" );

		checkResult( !mapIter.hasNext(), "Map contains FieldList - final map iter.hasNext()" );
		
		checkResult( true, "Map contains FieldList - exception not expected" );
	}
	
	static void EmaDecode_UPARequestMsgAll(ReqMsg reqMsg, int containerType)
	{
		System.out.println("Begin EMA ReqMsg Decoding");
		System.out.println(reqMsg);
		
		TestUtilities.checkResult(reqMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
		TestUtilities.checkResult(reqMsg.privateStream() == true, "ReqMsg.privateStream()");
		
		TestUtilities.checkResult(reqMsg.interestAfterRefresh() == true, "ReqMsg.interestAfterRefresh()");
		
		TestUtilities.checkResult(reqMsg.conflatedInUpdates() == true, "ReqMsg.conflatedInUpdates()");
		
		TestUtilities.checkResult(reqMsg.streamId() == 555, "ReqMsg.streamId()");
		
		TestUtilities.checkResult(reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(reqMsg.priorityClass() == 7, "ReqMsg.priorityClass()");
		
		TestUtilities.checkResult(reqMsg.priorityCount() == 5, "ReqMsg.priorityCount()");
		
		TestUtilities.checkResult(reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.qosRate() ==  com.thomsonreuters.ema.access.ReqMsg.Rate.TICK_BY_TICK, "ReqMsg.qosRate()");
		
		TestUtilities.checkResult(reqMsg.qosTimeliness() ==  com.thomsonreuters.ema.access.ReqMsg.Timeliness.REALTIME, "ReqMsg.qosTimeliness()");
		
		TestUtilities.checkResult(reqMsg.hasMsgKey(), "ReqMsg.hasMsgKey()");

		TestUtilities.checkResult(reqMsg.hasId(), "ReqMsg.hasId()");
		
		TestUtilities.checkResult(reqMsg.id() == 21, "ReqMsg.id()");

		TestUtilities.checkResult(reqMsg.hasFilter(), "ReqMsg.hasFilter()");
		
		TestUtilities.checkResult(reqMsg.filter() == 12 , "ReqMsg.hasFilter()");

		TestUtilities.checkResult(reqMsg.hasServiceId(), "ReqMsg.hasServiceId()");
		
		TestUtilities.checkResult(reqMsg.serviceId() == 5 , "ReqMsg.serviceId()");

		TestUtilities.checkResult(reqMsg.hasNameType(), "ReqMsg.hasNameType()");
		
		TestUtilities.checkResult(reqMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "ReqMsg.nameType()");

		TestUtilities.checkResult(reqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(reqMsg.name().compareTo("ABCDEF") == 0, "ReqMsg.name()");
		
		TestUtilities.checkResult(reqMsg.hasExtendedHeader(), "ReqMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( reqMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( reqMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( reqMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( reqMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( reqMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( reqMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( reqMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( reqMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( reqMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( reqMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( reqMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( reqMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "ReqMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( reqMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( reqMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		System.out.println("End EMA RequestMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPARefreshMsgAll(RefreshMsg refreshMsg, int containerType)
	{
		System.out.println("Begin EMA RefreshMsg Decoding");
		System.out.println(refreshMsg);
		
		TestUtilities.checkResult(refreshMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
		TestUtilities.checkResult(refreshMsg.streamId() == 15, "RefreshMsg.streamId()");

		TestUtilities.checkResult(refreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");
		
		TestUtilities.checkResult(refreshMsg.partNum() == 10, "RefreshMsg.partNum()");

		TestUtilities.checkResult(refreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");
		
		TestUtilities.checkResult(refreshMsg.seqNum() == 22, "RefreshMsg.seqNum()");

		TestUtilities.checkResult(refreshMsg.hasQos(), "RefreshMsg.hasQos()");
		
		TestUtilities.checkResult(refreshMsg.qos().timeliness() == OmmQos.Timeliness.REALTIME, "RefreshMsg.qos().timeliness()");
		
		TestUtilities.checkResult(refreshMsg.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "RefreshMsg.qos().rate()");
		
		TestUtilities.checkResult(refreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(refreshMsg.hasId(), "RefreshMsg.hasId()");
		
		TestUtilities.checkResult(refreshMsg.id() == 21, "RefreshMsg.id()");

		TestUtilities.checkResult(refreshMsg.hasFilter(), "RefreshMsg.hasFilter()");
		
		TestUtilities.checkResult(refreshMsg.filter() == 12 , "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(refreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");
		
		TestUtilities.checkResult(refreshMsg.serviceId() == 5 , "RefreshMsg.serviceId()");

		TestUtilities.checkResult(refreshMsg.hasNameType(), "RefreshMsg.hasNameType()");
		
		TestUtilities.checkResult(refreshMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(refreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(refreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");
		
		TestUtilities.checkResult(refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(refreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(refreshMsg.hasPermissionData(), "RefreshMsg.hasPermissionData()");
		TestUtilities.checkResult(refreshMsg.permissionData().equals(ByteBuffer.wrap("RefreshMsg.permissionData".getBytes())), "RefreshMsg.permissionData()");
		
		TestUtilities.checkResult(refreshMsg.hasExtendedHeader(), "RefreshMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( refreshMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( refreshMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( refreshMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( refreshMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( refreshMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( refreshMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( refreshMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( refreshMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( refreshMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( refreshMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( refreshMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( refreshMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "RefreshMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( refreshMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( refreshMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("refresh complete") == 0, "RefreshMsg.state().statusText()");
		
		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPAUpdateMsgAll(UpdateMsg updateMsg, int containerType)
	{
		System.out.println("Begin EMA UpdateMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(updateMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(updateMsg.streamId() == 15, "UpdateMsg.streamId()");

		TestUtilities.checkResult(updateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(updateMsg.seqNum() == 22, "UpdateMsg.seqNum()");

		TestUtilities.checkResult(updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(updateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(updateMsg.id() == 21, "UpdateMsg.id()");

		TestUtilities.checkResult(updateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(updateMsg.filter() == 12 , "UpdateMsg.hasFilter()");

		TestUtilities.checkResult(updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(updateMsg.serviceId() == 5 , "UpdateMsg.serviceId()");

		TestUtilities.checkResult(updateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(updateMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(updateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(updateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");
		
		TestUtilities.checkResult(updateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(updateMsg.doNotConflate(), "UpdateMsg.doNotconflate()");

		TestUtilities.checkResult(!updateMsg.doNotRipple(), "UpdateMsg.solicited()");
		
		TestUtilities.checkResult(updateMsg.hasConflated(), "UpdateMsg.hasConflated()");
	
		TestUtilities.checkResult(updateMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");

		TestUtilities.checkResult(updateMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");

		TestUtilities.checkResult(updateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(updateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(updateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(updateMsg.hasPermissionData(), "UpdateMsg.hasPermissionData()");
		TestUtilities.checkResult(updateMsg.permissionData().equals(ByteBuffer.wrap("UpdateMsg.permissionData".getBytes())), "UpdateMsg.permissionData()");
		
		TestUtilities.checkResult(updateMsg.hasExtendedHeader(), "UpdateMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( updateMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( updateMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( updateMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( updateMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( updateMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( updateMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( updateMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( updateMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( updateMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( updateMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( updateMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( updateMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "UpdateMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( updateMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( updateMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		System.out.println("Begin EMA UpdateMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPAStatusMsgAll(StatusMsg statusMsg, int containerType)
	{
		System.out.println("Begin EMA StatusMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(statusMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(statusMsg.streamId() == 15, "StatusMsg.streamId()");

		TestUtilities.checkResult(statusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(statusMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(statusMsg.id() == 21, "StatusMsg.id()");

		TestUtilities.checkResult(statusMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(statusMsg.filter() == 12, "StatusMsg.hasFilter()");

		TestUtilities.checkResult(statusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(statusMsg.serviceId() == 5, "StatusMsg.serviceId()");

		TestUtilities.checkResult(statusMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(statusMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
				"StatusMsg.nameType()");

		TestUtilities.checkResult(statusMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(statusMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

		TestUtilities.checkResult(statusMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(statusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(statusMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(statusMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(statusMsg.hasPermissionData(), "StatusMsg.hasPermissionData()");
		TestUtilities.checkResult(statusMsg.permissionData().equals(ByteBuffer.wrap("StatusMsg.permissionData".getBytes())), "StatusMsg.permissionData()");
		
		TestUtilities.checkResult(statusMsg.hasExtendedHeader(), "StatusMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( statusMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( statusMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( statusMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( statusMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( statusMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( statusMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( statusMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( statusMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( statusMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( statusMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( statusMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( statusMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "StatusMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( statusMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( statusMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		TestUtilities.checkResult(statusMsg.hasState(), "StatusMsg.hasState()");

		TestUtilities.checkResult(statusMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

		TestUtilities.checkResult(statusMsg.state().streamState() == OmmState.StreamState.OPEN,
				"StatusMsg.state().streamState()");

		TestUtilities.checkResult(statusMsg.state().dataState() == OmmState.DataState.OK,
				"StatusMsg.state().dataState()");

		TestUtilities.checkResult(statusMsg.state().statusText().compareTo("status complete") == 0,
				"StatusMsg.state().statusText()");

		System.out.println("End EMA StatusMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPAGenericMsgAll(GenericMsg genericMsg, int containerType)
	{
		System.out.println("Begin EMA GenericMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(genericMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
		TestUtilities.checkResult(genericMsg.streamId() == 15, "GenericMsg.streamId()");

		TestUtilities.checkResult(genericMsg.hasPartNum(), "GenericMsg.hasPartNum()");
		
		TestUtilities.checkResult(genericMsg.partNum() == 10, "GenericMsg.partNum()");

		TestUtilities.checkResult(genericMsg.hasSeqNum(), "GenericMsg.hasSeqNum()");
		
		TestUtilities.checkResult(genericMsg.seqNum() == 22, "GenericMsg.seqNum()");

		TestUtilities.checkResult(genericMsg.hasSecondarySeqNum(), "GenericMsg.hasSecondarySeqNum()");
		
		TestUtilities.checkResult(genericMsg.secondarySeqNum() == 123, "GenericMsg.secondarySeqNum()");

		TestUtilities.checkResult(genericMsg.complete(), "GenericMsg.complete()");
		
		TestUtilities.checkResult(genericMsg.hasMsgKey(), "GenericMsg.hasMsgKey()");

		TestUtilities.checkResult(genericMsg.hasId(), "GenericMsg.hasId()");
		
		TestUtilities.checkResult(genericMsg.id() == 21, "GenericMsg.id()");

		TestUtilities.checkResult(genericMsg.hasFilter(), "GenericMsg.hasFilter()");
		
		TestUtilities.checkResult(genericMsg.filter() == 12 , "GenericMsg.hasFilter()");

		TestUtilities.checkResult(genericMsg.hasServiceId(), "GenericMsg.hasServiceId()");
		
		TestUtilities.checkResult(genericMsg.serviceId() == 5 , "GenericMsg.serviceId()");

		TestUtilities.checkResult(genericMsg.hasNameType(), "GenericMsg.hasNameType()");
		
		TestUtilities.checkResult(genericMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(genericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(genericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(genericMsg.hasPermissionData(), "GenericMsg.hasPermissionData()");
		TestUtilities.checkResult(genericMsg.permissionData().equals(ByteBuffer.wrap("GenericMsg.permissionData".getBytes())), "GenericMsg.permissionData()");
		
		TestUtilities.checkResult(genericMsg.hasExtendedHeader(), "GenericMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( genericMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( genericMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( genericMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( genericMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( genericMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( genericMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( genericMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( genericMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( genericMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( genericMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( genericMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( genericMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "GenericMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( genericMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( genericMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		System.out.println("End EMA GenericMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPAPostMsgAll(PostMsg postMsg, int containerType)
	{
		System.out.println("Begin EMA PostMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(postMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(postMsg.streamId() == 15, "PostMsg.streamId()");

		TestUtilities.checkResult(postMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(postMsg.partNum() == 10, "PostMsg.partNum()");

		TestUtilities.checkResult(postMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(postMsg.seqNum() == 22, "PostMsg.seqNum()");

		TestUtilities.checkResult(postMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(postMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(postMsg.id() == 21, "PostMsg.id()");

		TestUtilities.checkResult(postMsg.hasPostUserRights(), "PostMsg.hasPostUserRights()");
		
		TestUtilities.checkResult(postMsg.postUserRights() == PostMsg.PostUserRights.CREATE, "PostMsg.postUserRights()");
		
		TestUtilities.checkResult(postMsg.hasPostId(), "PostMsg.hasPostId()");
		
		TestUtilities.checkResult(postMsg.postId() == 223,  "PostMsg.postId()");
		
		TestUtilities.checkResult(postMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(postMsg.filter() == 12 , "PostMsg.hasFilter()");

		TestUtilities.checkResult(postMsg.solicitAck(), "PostMsg.solicitAck()");
		
		TestUtilities.checkResult(postMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(postMsg.serviceId() == 5 , "PostMsg.serviceId()");

		TestUtilities.checkResult(postMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(postMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");
		
		TestUtilities.checkResult(postMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(postMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(postMsg.hasPermissionData(), "PostMsg.hasPermissionData()");
		TestUtilities.checkResult(postMsg.permissionData().equals(ByteBuffer.wrap("PostMsg.permissionData".getBytes())), "PostMsg.permissionData()");
		
		TestUtilities.checkResult(postMsg.hasExtendedHeader(), "PostMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( postMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( postMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( postMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( postMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( postMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( postMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( postMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( postMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( postMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( postMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( postMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( postMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "PostMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( postMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( postMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		System.out.println("End EMA PostMsg Decoding");
		System.out.println();
	}
	
	static void EmaDecode_UPAAckMsgAll(AckMsg ackMsg, int containerType)
	{
		System.out.println("Begin EMA AckMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(ackMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(ackMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(ackMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(ackMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(ackMsg.nackCode() == com.thomsonreuters.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

		TestUtilities.checkResult(ackMsg.hasText(), "AckMsg.hasText()");
		
		TestUtilities.checkResult(ackMsg.text().compareTo("denied by source") == 0, "AckMsg.text()");

		TestUtilities.checkResult(ackMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(ackMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(ackMsg.id() == 21, "AckMsg.id()");

		TestUtilities.checkResult(ackMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(ackMsg.filter() == 12 , "AckMsg.hasFilter()");

		TestUtilities.checkResult(ackMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(ackMsg.serviceId() == 5 , "AckMsg.serviceId()");

		TestUtilities.checkResult(ackMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(ackMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(ackMsg.hasExtendedHeader(), "AckMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( ackMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFieldListAll( ackMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.ELEMENT_LIST, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( ackMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAElementListAll( ackMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FILTER_LIST, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAFilterListAll( ackMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_UPAFilterListAll( ackMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.thomsonreuters.upa.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.SERIES, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPASeriesAll( ackMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPASeriesAll( ackMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.VECTOR, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAVectorAll( ackMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_UPAVectorAll( ackMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.MAP, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPAMapKeyUIntAll( ackMsg.attrib().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPAMapKeyUIntAll( ackMsg.payload().map(),  com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.thomsonreuters.upa.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.REFRESH_MSG, "AckMsg.payload().dataType()");
				
				com.thomsonreuters.upa.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.thomsonreuters.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_UPAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_UPARefreshMsgAll( ackMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				EmaDecode_UPARefreshMsgAll( ackMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
				
				break;
			}
		}
		
		System.out.println("End EMA AckMsg Decoding");
		System.out.println();
	}

	static void checkResult(boolean result, String description) 
	{
		System.out.println((result ? "passed" : "FAILED") + " - " + description);
		if ( result )
		{
			++_passNum;
		}
		else
		{
			++_failNum;
			assertFalse(true);
		}
	}

	static void checkResult(String description, boolean result)
	{
		checkResult(result, description);
	}
	
	static void checkResult(boolean result)
	{
		if ( result )
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	static void checkResult(long expected, long actual)
	{
		if (expected == actual)
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	static void checkResult(double expected, double actual)
	{
		if (expected == actual)
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	static void checkResult(String expect, String actual)
	{
		if (expect.equals(actual))
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
}
