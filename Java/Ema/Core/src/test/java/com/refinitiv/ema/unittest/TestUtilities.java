///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.Data.DataCode;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.JUnitTestConnect;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.MapEntry.MapAction;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmArrayEntry;
import com.refinitiv.ema.access.OmmError;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmQos;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.Series;
import com.refinitiv.ema.access.SeriesEntry;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.Vector;
import com.refinitiv.ema.access.VectorEntry;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.ElementListFlags;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldListFlags;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.MapFlags;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;
import com.refinitiv.eta.codec.UInt;

import junit.framework.TestCase;


public final class TestUtilities extends TestCase
{
	
	private static String fieldDictionaryFileName = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/RDMTestDictionary";
	private static String enumTableFileName = "./src/test/resources/com/refinitiv/ema/unittest/DataDictionaryTest/testenumtype.def";
	
	public static class EncodingTypeFlags
	{
		public static final int PRIMITIVE_TYPES = 1;

		public static final int CONTAINER_TYPES = 2;

		public static final int MESSAGE_TYPES = 4;
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
	private static com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
	
	static
	{
		fieldDictionaryFileName = TestDictionaries.fieldDictionaryFileName;
		enumTableFileName = TestDictionaries.enumTableFileName;
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
	}
	
	private TestUtilities()
	{
		throw new AssertionError();
	}
	
	static com.refinitiv.eta.codec.DataDictionary getDataDictionary()
	{
		return dictionary;
	}
	public static  String getFieldDictionaryFileName()
	{
		return fieldDictionaryFileName;
	}

	public static  String getEnumTableFileName()
	{
		return enumTableFileName;
	}

	public static void printTestHead(String title, String detail)
	{
		_strBuilder.setLength(0);
		_strBuilder.append("\n\n" + "******************************************************************************" + "\n" + "Title: " + title + "\n" + "Detail: " + detail + "\n");
		System.out.println(_strBuilder.toString());
	}

	public static void eta_EncodeErrorFieldList( Buffer rsslBuf )
	{
		com.refinitiv.eta.codec.FieldList rsslFL= CodecFactory.createFieldList();
		com.refinitiv.eta.codec.EncodeIterator iter = CodecFactory.createEncodeIterator();
		com.refinitiv.eta.codec.FieldEntry rsslFEntry = CodecFactory.createFieldEntry();
		
		iter.setBufferAndRWFVersion(rsslBuf, Codec.majorVersion(), Codec.minorVersion());
		rsslFL.flags(FieldListFlags.HAS_FIELD_LIST_INFO | FieldListFlags.HAS_STANDARD_DATA );
		rsslFL.dictionaryId(dictionary.infoDictionaryId());
		rsslFL.fieldListNum( 65);

		rsslFL.encodeInit(iter, null, 0);

		// fid not found case (first)
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.fieldId( -100);
		UInt uint64 = CodecFactory.createUInt();
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );
		
		// corect fid found case (second)
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.fieldId( 1);
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// incorrect data type fid longer than expected (third)
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 1);
		Real real = CodecFactory.createReal();
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );
		
		// correct data type fid (fourth)
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 6);
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real);

		// incorrect data type fid shorter than expected (fifth)
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.fieldId( 6);
		uint64.value( 67);
		rsslFEntry.encode( iter, uint64 );

		// correct data type fid
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.fieldId( 6);
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );

		rsslFL.encodeComplete(iter, true);
	}

	public static void eta_EncodeErrorElementList( Buffer rsslBuf )
	{
		com.refinitiv.eta.codec.ElementList rsslFL= CodecFactory.createElementList();
		com.refinitiv.eta.codec.EncodeIterator iter = CodecFactory.createEncodeIterator();
		com.refinitiv.eta.codec.ElementEntry rsslFEntry = CodecFactory.createElementEntry();
        Buffer badDataBuffer = CodecFactory.createBuffer();
		
		badDataBuffer.data(ByteBuffer.allocate(1));
        badDataBuffer.data().put(0, (byte)0xff);

		iter.setBufferAndRWFVersion(rsslBuf, Codec.majorVersion(), Codec.minorVersion());
		rsslFL.flags(ElementListFlags.HAS_ELEMENT_LIST_INFO | ElementListFlags.HAS_STANDARD_DATA );
		rsslFL.elementListNum( 65);

		rsslFL.encodeInit(iter, null, 0);

		// first entry correct
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT");
		UInt uint64 = CodecFactory.createUInt();
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );
		
		// second entry correct
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT2");
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// bad data
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL");
		rsslFEntry.encodedData(badDataBuffer);
		rsslFEntry.encode( iter);
		
		// fourth entry correct
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.UINT);
		rsslFEntry.name().data("MY_UINT3");
		uint64.value( 64);
		rsslFEntry.encode( iter, uint64 );

		// bad data
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL2");
		rsslFEntry.encodedData(badDataBuffer);
		rsslFEntry.encode( iter);

		// sixth entry correct
		rsslFEntry.dataType( com.refinitiv.eta.codec.DataTypes.REAL);
		rsslFEntry.name().data("MY_REAL3");
		Real real = CodecFactory.createReal();
		real.value(11, RealHints.EXPONENT_2);
		rsslFEntry.encode( iter, real );

		rsslFL.encodeComplete(iter, true);
	}
	
	// Encode (with ETA) a basic field list with several primitives embedded in it
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeFieldListAll(com.refinitiv.eta.codec.Buffer etaBuf, int encodeOption)
	{
		// used to store and check return values
		int retVal;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		if ((retVal = eta_EncodeFieldListAll(encodeIter, encodeOption)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with eta_EncodeFieldListAll(encodeIter, dictionary). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
	
	private static int eta_EncodeFieldListAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int encodeFlag)
	{
		int retVal;
		
		// create and initialize field list structure
		com.refinitiv.eta.codec.FieldList etaFieldList = CodecFactory.createFieldList();

		// populate field list structure prior to call to EncodeFieldListInit
		// NOTE: some of the fieldId, dictionaryId and fieldListNum values used here do not correspond to actual id values

		// indicate that standard data will be encoded and that dictionaryId and fieldListNum are included
		etaFieldList.flags(FieldListFlags.HAS_STANDARD_DATA | FieldListFlags.HAS_FIELD_LIST_INFO);
		// populate dictionaryId and fieldListNum with info needed to cross-reference fieldIds and cache
		etaFieldList.dictionaryId(dictionary.infoDictionaryId()); 
		etaFieldList.fieldListNum(65);

		// begin encoding of field list - assumes that encodeIter is already populated with buffer and version information
		if ((retVal = etaFieldList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
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
			com.refinitiv.eta.codec.FieldEntry fieldEntry = CodecFactory.createFieldEntry();

			boolean success = true;
			
			System.out.println("\tETA FieldList Header Encoded");
			
		    if ( (encodeFlag & EncodingTypeFlags.PRIMITIVE_TYPES) != 0 )
		    {
				// create a date and populate {day, month, year}
				com.refinitiv.eta.codec.Date date = CodecFactory.createDate();
				date.month(1);
				date.day(31);
				date.year(2022);
				
				com.refinitiv.eta.codec.Real real = CodecFactory.createReal();
				com.refinitiv.eta.codec.Time time = CodecFactory.createTime();
				com.refinitiv.eta.codec.DateTime DATETIME = CodecFactory.createDateTime();
				com.refinitiv.eta.codec.Array array = CodecFactory.createArray();
				com.refinitiv.eta.codec.Qos qos = CodecFactory.createQos();
				com.refinitiv.eta.codec.State state = CodecFactory.createState();
				com.refinitiv.eta.codec.Enum enumValue  = CodecFactory.createEnum();
				com.refinitiv.eta.codec.Buffer buffer = CodecFactory.createBuffer();
			
				com.refinitiv.eta.codec.UInt uInt = CodecFactory.createUInt();
				uInt.value(23456);
				com.refinitiv.eta.codec.Int Int = CodecFactory.createInt();
				Int.value(65432);
				com.refinitiv.eta.codec.Float Float = CodecFactory.createFloat();
				Float.value(3.14f);
				com.refinitiv.eta.codec.Double Double = CodecFactory.createDouble();
				Double.value(3.1416);
	
				// Create a Buffer for UInt to encode into
				com.refinitiv.eta.codec.Buffer encUInt = CodecFactory.createBuffer();
				encUInt.data(ByteBuffer.allocate(10));
	
				// FIRST Field Entry
				// fid not found case (first)
				fieldEntry.fieldId(-100);
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.UINT);
				if ((retVal = fieldEntry.encode(encodeIter, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal));
					return retVal;
				}
	
				// SECOND Field Entry: encode entry from the Date primitive type
				// Populate and encode field entry with fieldId and dataType information for this field.
				fieldEntry.fieldId(16); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.DATE);
				if ((retVal = fieldEntry.encode(encodeIter, date)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Date: " + date.month() + "-" + date.day() + "-" + date.year());
	
				// THIRD Field Entry: encode entry from the UInt primitive type
				fieldEntry.fieldId(147); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.UINT);
				if ((retVal = fieldEntry.encode(encodeIter, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Unsigned Integer: " + uInt.toLong());
	
				// FOURTH Field Entry: encode entry from preencoded buffer containing an encoded UInt type
				// Populate and encode field entry with fieldId and dataType information for this field.
				// Because we are re-populating all values on FieldEntry, there is no need to clear it.
				fieldEntry.fieldId(1080); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.UINT);
	
				// assuming encUInt is a Buffer with length and data properly populated
				if ((retVal = eta_getPreEncodedUIntBuffer(encUInt, uInt)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_getPreEncodedUIntBuffer.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				// pre-encoded data
				fieldEntry.encodedData(encUInt);
				if ((retVal = fieldEntry.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
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
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL);
				if ((retVal = fieldEntry.encodeBlank(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real as blank.");
	
				// SIXTH Field Entry: encode entry for a Real primitive type
				fieldEntry.fieldId(24); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL); 
				real.value(227, RealHints.EXPONENT_2);
				if ((retVal = fieldEntry.encode(encodeIter, real)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real: hint: " + real.hint() + " value: " + real.toLong());
	
				// SEVENTH Field Entry: encode entry for another Real primitive type
				fieldEntry.fieldId(25); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL);  
				real.value(22801, RealHints.EXPONENT_4);
				if ((retVal = fieldEntry.encode(encodeIter, real)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real: hint: " + real.hint() + " value: " + real.toLong());
	
				// EIGHTH Field Entry: encode entry for another Time primitive type
				fieldEntry.fieldId(18); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.TIME);  
				time.hour(8);
				time.minute(39);
				time.second(24);
				if ((retVal = fieldEntry.encode(encodeIter, time)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Time: " + time.hour() + ":" + time.minute() + ":" + time.second());
	
				// NINETH Field Entry: encode entry from the Int primitive type
				fieldEntry.fieldId(INTEGER); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.INT);
				if ((retVal = fieldEntry.encode(encodeIter, Int)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded signed Integer: " + Int.toLong());
	
				// TENTH Field Entry: encode entry from the Float primitive type
				fieldEntry.fieldId(MY_FLOAT); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.FLOAT);
				if ((retVal = fieldEntry.encode(encodeIter, Float)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID "+ fieldEntry.fieldId() + " Encoded float: " + Float.toFloat());
	
				// ELEVENTH Field Entry: encode entry from the Double primitive type
				fieldEntry.fieldId(MY_DOUBLE); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.DOUBLE);
				if ((retVal = fieldEntry.encode(encodeIter, Double)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Double: " + Double.toDouble());
	
				// TWELVETH Field Entry: encode entry from the DATETIME primitive type
				fieldEntry.fieldId(TRADE_DATETIME); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.DATETIME);
				DATETIME.month(11);
				DATETIME.day(15);
				DATETIME.year(2011);
				DATETIME.hour(8);
				DATETIME.minute(39);
				DATETIME.second(24);
				if ((retVal = fieldEntry.encode(encodeIter, DATETIME)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId()
						+ " Encoded DATETIME: " + DATETIME.month() + "-" + DATETIME.day() + "-" + DATETIME.year()
											+ " " + DATETIME.hour() + ":" + DATETIME.minute() + ":" + DATETIME.second());
	
				// THIRTEENTH Field Entry: encode entry from the Qos primitive type
				fieldEntry.fieldId(MY_QOS); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.QOS);
				qos.timeliness(com.refinitiv.eta.codec.QosTimeliness.REALTIME);
				qos.rate(com.refinitiv.eta.codec.QosRates.TICK_BY_TICK);
				qos.dynamic(true);
				qos.rateInfo(0);
				qos.timeInfo(0);
				if ((retVal = fieldEntry.encode(encodeIter, qos)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId()
							+ " Encoded QOS: " + qos.timeliness() + " " + qos.rate()
										+ " " + (qos.isDynamic() ? 1 : 0)
										+ " " + qos.rateInfo() + " " + qos.timeInfo());
	
				// FOURTEENTH Field Entry: encode entry from the State primitive type
				fieldEntry.fieldId(MY_STATE); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.STATE);
				state.streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
				state.dataState(com.refinitiv.eta.codec.DataStates.OK);
				state.code(com.refinitiv.eta.codec.StateCodes.NONE);
				state.text().data("Succeeded");
				if ((retVal = fieldEntry.encode(encodeIter, state)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded State: " + state.streamState() + " " + state.dataState() + " " + state.code() + " " + state.text().toString());
	
				// FIFTEENTH Field Entry: encode entry from the Buffer primitive type
				fieldEntry.fieldId(MY_BUFFER); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.BUFFER);
				buffer.data("ABCDEFGH");
				if ((retVal = fieldEntry.encode(encodeIter, buffer)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Buffer: " + buffer.toString());
	
				// SIXTEENTH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ENUM);
				enumValue.value(29);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
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
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ARRAY);
				// begin complex field entry encoding, we are not sure of the approximate max encoding length
				if ((retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntryInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					// now encode nested container using its own specific encode methods
					// encode Real values into the array
					array.primitiveType(com.refinitiv.eta.codec.DataTypes.UINT);
					// values are variable length
					array.itemLength(2);
					// begin encoding of array - using same encIterator as field list
					if ((retVal = array.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						// print out message with return value string, value, and text
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						//----- Continue encoding array entries. ----
						com.refinitiv.eta.codec.UInt uInt1 = CodecFactory.createUInt();
						uInt1.value(10);
						com.refinitiv.eta.codec.UInt uInt2 = CodecFactory.createUInt();
						uInt2.value(20);
						com.refinitiv.eta.codec.UInt uInt3 = CodecFactory.createUInt();
						uInt3.value(30);
						com.refinitiv.eta.codec.UInt uInt4 = CodecFactory.createUInt();
						uInt4.value(40);
	
						// Array encoding was successful.
	
						System.out.print("\t\tFID " + fieldEntry.fieldId() + " Encoded Array: [");
						com.refinitiv.eta.codec.ArrayEntry ae = CodecFactory.createArrayEntry();
	
						// Encode first entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt1)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(uInt1.toLong());
	
						// Encode second entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt2)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(" " + uInt2.toLong());
	
						// Encode third entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt3)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
						System.out.print(" " + uInt3.toLong());
	
						// Encode forth entry from a UInt from a primitive type.
						if ((retVal = ae.encode(encodeIter, uInt4)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
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
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayEntry.  "
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
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeArrayComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntryComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				// EIGHTEENTH Field Entry: encode entry for the Enum type with blank value
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ENUM);
				if ((retVal = fieldEntry.encodeBlank(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + "Blank");
				
				// NINETEENTH Field Entry: encode entry for the Enum type with undefined display value
				fieldEntry.fieldId(4); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ENUM);
				enumValue.value(2999);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
				
				// TWENTIETH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(115); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ENUM);
				enumValue.value(0);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());
				
				// TWENTY FIFTH Field Entry: encode entry from the  primitive type
				fieldEntry.fieldId(8960); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ENUM);
				enumValue.value(2);
				if ((retVal = fieldEntry.encode(encodeIter, enumValue)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded : " + enumValue.toInt());

				// TWENTY SIXTH Field Entry: encode entry for corrupted Real primitive type
				// using Date FID
				fieldEntry.fieldId(16);
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL);
				real.value(22801, RealHints.EXPONENT_4);
				if ((retVal = fieldEntry.encode(encodeIter, real)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal));
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Real: hint: " + real.hint() + " value: " + real.toLong());

				// TWENTY SEVENTH Field Entry: encode entry from corrupted the Date
				// primitive type using Real FID
				fieldEntry.fieldId(6);
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.DATE);
				if ((retVal = fieldEntry.encode(encodeIter, date)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal));
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Date: " + date.month() + "-" + date.day() + "-" + date.year());

				// TWENTY EIGHTH Field Entry: encode entry from corrupted the DATE
				// primitive type using TIME FID
				fieldEntry.fieldId(5);
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.DATE);
				if ((retVal = fieldEntry.encode(encodeIter, date)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeFieldEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal));
					return retVal;
				}
				System.out.println("\t\tFID " + fieldEntry.fieldId() + " Encoded Date: " + date.month() + "-" + date.day() + "-" + date.year());
			}
			
			if ( (encodeFlag & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
			{
				fieldEntry.clear();
				fieldEntry.fieldId(MY_FIELDLIST); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_ELEMENTLIST); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_FILTERLIST); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_SERIES); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.SERIES);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_VECTOR); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.VECTOR);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MAP); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MAP);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
			}
			
			if ( (encodeFlag & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
			{
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeUpdateMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeStatusMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeGenericMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodePostMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeAckMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
				
				fieldEntry.clear();
				fieldEntry.fieldId(MY_MSG); 
				fieldEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
				if ( (retVal = fieldEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					// print out message with return value string, value, and text
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeInit().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
				else
				{
					if ( (retVal = eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRequestMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				
				// Complete encoding of complex field entry.
				// If any array encoding failed, success is false.
				if ((retVal = fieldEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldEntry.encodeComplete().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					return retVal;
				}
			}
		}

		// Complete fieldList encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeFieldListInit.
		if ((retVal = etaFieldList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FieldList.encodeCompelte().  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		System.out.println("\tETA FieldList Encoding Complete");
		
		return CodecReturnCodes.SUCCESS;
	}
	
	// Encode (with ETA) a basic array with several primitives embedded in it
    // We pass in the buffer to this method with the total length available.
    public static int eta_EncodeArrayAll(com.refinitiv.eta.codec.Buffer etaBuf)
    {
        // used to store and check return values
        int retVal;

        int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
        int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
    
        // Create and clear iterator to prepare for encoding
        com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
    
        // Associate buffer and iterator and set proper protocol version information on iterator.
        if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
                            + " Error Text: " + CodecReturnCodes.info(retVal)); 
            return retVal;
        }

        if ((retVal = eta_EncodeArrayAll(encodeIter)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with eta_EncodeFieldListAll(encodeIter, dictionary). "
                            + " Error Text: " + CodecReturnCodes.info(retVal)); 
            return retVal;
        }

        return CodecReturnCodes.SUCCESS;
    }
	
	private static int eta_EncodeArrayAll(EncodeIterator encodeIter)
    {
        int retVal;
        boolean success = true;
        
        // create and initialize Array structure
        com.refinitiv.eta.codec.Array array = CodecFactory.createArray();
        
        /* encode Buffer values into the array */
        array.primitiveType(com.refinitiv.eta.codec.DataTypes.BUFFER);
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

    // Encode (with ETA) a basic element list with several primitives embedded in it
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeElementListAll(com.refinitiv.eta.codec.Buffer etaBuf, int encodingFlag)
	{
        // used to store and check return values
        int retVal;
        
		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ((retVal = eta_EncodeElementListAll(encodeIter, encodingFlag)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with eta_EncodeElementListAll(encodeIter). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

        return CodecReturnCodes.SUCCESS;
	}

	public static int eta_EncodeElementListAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int encodeFlag)
	{
		int retVal;
		boolean success = true;
		
		// create a single ElementEntry and reuse for each entry
		com.refinitiv.eta.codec.ElementEntry elemEntry = CodecFactory.createElementEntry();
		
		// create and initialize element list structure
		com.refinitiv.eta.codec.ElementList etaElementList = CodecFactory.createElementList();
		
		// populate element list structure prior to call to EncodeElementListInit

		// indicate that standard data will be encoded and that elementListNum is included
		etaElementList.flags(ElementListFlags.HAS_STANDARD_DATA | ElementListFlags.HAS_ELEMENT_LIST_INFO);
		
		// populate elementListNum with info needed to cache
		etaElementList.elementListNum(7);
		
		// begin encoding of element list - assumes that encodeIter is already populated with buffer and version information
		if ((retVal = etaElementList.encodeInit(encodeIter, null, 0)) < CodecReturnCodes.SUCCESS)
		{
			// print out message with return value string, value, and text
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeElementListInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ( (encodeFlag & EncodingTypeFlags.PRIMITIVE_TYPES) != 0 )
	    {
			// various data types used during encoding
			com.refinitiv.eta.codec.Time time = CodecFactory.createTime();
			time.hour(10);
			time.minute(21);
			time.second(16);
			time.millisecond(777);
			com.refinitiv.eta.codec.UInt uInt = CodecFactory.createUInt();
			uInt.value(17);
			com.refinitiv.eta.codec.Int Int = CodecFactory.createInt();
			Int.value(13);
			
			com.refinitiv.eta.codec.Float floatVal = CodecFactory.createFloat();
			floatVal.value(1.34f);
			
			com.refinitiv.eta.codec.Double doubleVal = CodecFactory.createDouble();
			doubleVal.value(1.345);

			Real real = CodecFactory.createReal();
			real.value(22801, 31);

			// FIRST Element Entry: encode entry from the Time primitive type
			// Populate and encode element entry with name and dataType information.
			elemEntry.name().data("Element - Time");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.TIME);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, time);
			System.out.println("\t\tEncoded Time: " + time.hour() + ":" + time.minute() + ":" + time.second());
			
			// SECOND Element Entry: encode entry from the Int primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - Int");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.INT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, Int);
			System.out.println("\t\tEncoded signed Integer: " + Int.toLong());
	
			// THIRD Element Entry: encode entry from the UInt primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - UInt");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.UINT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, uInt);
			System.out.println("\t\tEncoded Unsigned Integer: " + uInt.toLong());
			
			// FOURTH Element Entry: encode entry from the Real primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.clear(); // clear this to ensure a blank element
			elemEntry.name().data("Element - Real - Blank");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			elemEntry.encodeBlank(encodeIter); // this encodes a blank
			System.out.println("\t\tEncoded Real: Blank");
	
			// FIFTH Element Entry: encode entry from the Float primitive type
			// Populate and encode element entry with name and dataType information for this element
			elemEntry.name().data("Element - Float");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.FLOAT);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, floatVal);
			System.out.println("\t\tEncoded Float: " + floatVal.toFloat());
			
			// SIXTH Element Entry: encode entry from the Double primitive type
			// Populate and encode element entry with name and dataType information
			elemEntry.name().data("Element - Double");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.DOUBLE);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, doubleVal);
			System.out.println("\t\tEncoded Float: " + doubleVal.toDouble());

//			// SEVENTH Element Entry: encode entry from the Real primitive type
//			// Populate and encode element entry with name and dataType information
			elemEntry.clear(); // clear this to ensure a blank element
			elemEntry.name().data("Element - Real - Invalid");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.REAL);
			System.out.println("\tEncoding Element Entry (name: " + elemEntry.name() + ")");
			retVal = elemEntry.encode(encodeIter, real);
			System.out.println("\t\tEncoded Real: " + real);
			encodeIter.buffer().data().put(encodeIter.buffer().data().position() - 2, (byte)0x1F);
	    }
		
		if ( (encodeFlag & EncodingTypeFlags.CONTAINER_TYPES) != 0 )
	    {
			success = true;
			
			elemEntry.clear();
			elemEntry.name().data("Element - FieldList");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - ElementList");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - FilterList");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Series");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.SERIES);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Vector"); 
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.VECTOR);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Map");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MAP);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
	    }
		
		if ( (encodeFlag & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
	    {
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeUpdateMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeStatusMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeGenericMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodePostMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeAckMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
			elemEntry.clear();
			elemEntry.name().data("Element - Msg");
			elemEntry.dataType(com.refinitiv.eta.codec.DataTypes.MSG);
			if ( (retVal = elemEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeInit().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				if ( (retVal = eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRequestMsgAll().  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			}
			
			// Complete encoding of complex element entry.
			// If any array encoding failed, success is false.
			if ((retVal = elemEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with ElementEntry.encodeComplete().  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
	    }
		
		// Complete elementList encoding.
		// If success parameter is true, this will finalize encoding.
		// If success parameter is false, this will roll back encoding prior to EncodeElementListInit.
		if ((retVal = etaElementList.encodeComplete(encodeIter, true)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeElementListComplete.  "
				+ "Error Text: " + CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println("\tETA ElementList Encoding Complete");
		
		return CodecReturnCodes.SUCCESS;
	}


	// Encode (with ETA) a map (key is UInt) with field lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyUIntAll(com.refinitiv.eta.codec.Buffer etaBuf, int containerType)
	{
		// use this to store and check return codes
		int retVal;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that etaBuf.data() points to sufficient memory and etaBuf.length()
		// indicates number of bytes available in etaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
		if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, containerType)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with eta_EncodeFieldListAll(encodeIter, dictionary). "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}
		
	public static int eta_EncodeMapKeyUIntAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(containerType);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.UINT);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.UInt entryKey = CodecFactory.createUInt();

			// Create a Buffer for uInt and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for FieldList encoding).
			com.refinitiv.eta.codec.Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();

			switch ( containerType )
			{
				case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");

						if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding field list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						// Now encode nested container using its own specific encode methods.
						// Clear, then begin encoding of field list - using same encIterator as map.
						if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding field list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
	
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
					if ((retVal = eta_getPreEncodedUIntBuffer(encUInt, entryKey)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
	
					// assuming encFieldList Buffer contains the pre-encoded payload with data and length populated
					if ((retVal = eta_getPreEncodedFieldListBuffer(encFieldList)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedFieldListBuffer.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						
						if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding element list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
					
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding filter list.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFilterListAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.SERIES:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding series.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeSeriesAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.VECTOR:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding vector.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeVectorAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.MAP:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
					System.out.println();
				}
				break;
				case com.refinitiv.eta.codec.DataTypes.MSG:
				{
					{
						System.out.println("\tEncoding Summary Data");
						if ((retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding msg.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}

						System.out.println("\tSummary Data Encoding Complete");
						System.out.println();
					}
					// complete encoding of summary data.  If any field list encoding failed, success is false.
					if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					System.out.println("\tEncoding Entry's payload type: " + com.refinitiv.eta.codec.DataTypes.toString(containerType));
					
					mapEntry.action(MapEntryActions.ADD);
					entryKey.value(1);
					if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back.
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					else
					{
						System.out.println("\tEncoding Map Entry (key: " + entryKey.toLong() + ")");
						if ((retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
						{
							// error condition - switch our success value to false so we can roll back
							success = false;
							System.out.println("Error encoding map.");
							System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
									+ "Error Text: " + CodecReturnCodes.info(retVal)); 
						}
					}
					
					if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
				}
				break;
				default:
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
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
	
				System.out.println("\tMap Entry (key: " + entryKey.toLong() + ") Encoding Complete");
				System.out.println();
			}
	
			// Complete map encoding.
			// If success parameter is true, this will finalize encoding.  
			// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
			if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
	        System.out.println("\tETA Map Encoding Complete");
	        
	        return CodecReturnCodes.SUCCESS;
		}
	
		// Encode (with ETA) a map (key is Int) with field lists
		// We pass in the buffer to this method with the total length available.
		public static int eta_EncodeMapKeyIntWithEmptyFLs(com.refinitiv.eta.codec.Buffer etaBuf)
		{
			// use this to store and check return codes
			int retVal;
			boolean success = true;

			int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
			int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
			// Create and clear iterator to prepare for encoding
			com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
			encodeIter.clear();
		
			// Associate buffer and iterator and set proper protocol version information on iterator.
			// It is assumed that etaBuf.data() points to sufficient memory and etaBuf.length()
			// indicates number of bytes available in etaBuf.data().
			if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
								+ " Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}

			//  create and initialize map structure
			com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

			// populate map structure prior to call to EncodeMapInit
			// NOTE: the key names used may not correspond to actual name values

			// indicate that summary data, a key field id, and a total count hint will be encoded
			etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
			// populate maps keyPrimitiveType and containerType
			etaMap.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.INT);
			// populate total count hint with approximate expected entry count
			etaMap.totalCountHint(1);
			etaMap.keyFieldId(3426);

			// Begin encoding of map - assumes that encodeIter is already populated with
			// buffer and version information, store return value to determine success or failure.
			// Expect summary data of approx. 256 bytes, no set definition data.
			if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				// print out message with return value string, value, and text
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			{
				// mapInit encoding was successful
				// create a single MapEntry and FieldList and reuse for each entry
				com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
				com.refinitiv.eta.codec.Int entryKeyInt = CodecFactory.createInt();

				// Create a Buffer for Int and field list to encode into
				// Here we are heap allocating the buffers (10 bytes is large enough for
				// Int encoding and 500 bytes is large enough for FieldList encoding).
				com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
				encBuffer.data(ByteBuffer.allocate(10));
				com.refinitiv.eta.codec.Buffer encFieldList = CodecFactory.createBuffer();
				encFieldList.data(ByteBuffer.allocate(500));

				System.out.println("\tMap Header Encoded");
				System.out.println();
				
				
				// Encode expected summary data, init for this was done by EncodeMapInit
			    // This type should match Map.containerType
				{
					System.out.println("\tEncoding Summary Data");

					// Now encode nested container using its own specific encode methods.
					// Begin encoding of field list - using same encIterator as map list.
					if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
				}
				// complete encoding of summary data.  If any field list encoding failed, success is false.
				if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				
				// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
				mapEntry.action(MapEntryActions.ADD);
				entryKeyInt.value(1);
				if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back.
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
				if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
				System.out.println();

			// Complete map encoding.
			// If success parameter is true, this will finalize encoding.  
			// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
			if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
			{
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				return retVal;
			}
			
	        System.out.println("\tETA Map Encoding Complete");
			}
	        
	        return CodecReturnCodes.SUCCESS;
		}


	// Encode (with ETA) a map (key is Int) with field lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyIntWithFLs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that etaBuf.data() points to sufficient memory and etaBuf.length()
		// indicates number of bytes available in etaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.INT);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Int entryKeyInt = CodecFactory.createInt();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is AsciiString) with field lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyAsciiStringWithFLs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that etaBuf.data() points to sufficient memory and etaBuf.length()
		// indicates number of bytes available in etaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.ASCII_STRING);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is Buffer) with field lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyBufferWithFLs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		// It is assumed that etaBuf.data() points to sufficient memory and etaBuf.length()
		// indicates number of bytes available in etaBuf.data().
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.BUFFER);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and FieldList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for Int and field list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// Int encoding and 500 bytes is large enough for FieldList encoding).
			com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encFieldList = CodecFactory.createBuffer();
			encFieldList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of field list - using same encIterator as map list.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any field list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained field list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of field list - using same encIterator as map.
				if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeFieldListAll(encodeIter, dictionary).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is UInt) with element lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyUIntWithELs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.UINT);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.UInt entryKeyUInt = CodecFactory.createUInt();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.refinitiv.eta.codec.Buffer encUInt = CodecFactory.createBuffer();
			encUInt.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyUInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyUInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyUInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
			if ((retVal = eta_getPreEncodedUIntBuffer(encUInt, entryKeyUInt)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// assuming encElementList Buffer contains the pre-encoded payload with data and length populated
			if ((retVal = eta_getPreEncodedElementListBuffer(encElementList)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_getPreEncodedElementListBuffer.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyUInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is Int) with element lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyIntWithELs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.INT);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Int entryKeyInt = CodecFactory.createInt();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.refinitiv.eta.codec.Buffer encInt = CodecFactory.createBuffer();
			encInt.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = eta_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKeyInt.value(1);
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKeyInt, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKeyInt.toLong() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter,EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
			if ((retVal = eta_getPreEncodedIntBuffer(encInt, entryKeyInt)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with getPreEncodedUIntBuffer.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			// assuming encElementList Buffer contains the pre-encoded payload with data and length populated
			if ((retVal = eta_getPreEncodedElementListBuffer(encElementList)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_getPreEncodedElementListBuffer.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKeyInt.toLong() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is Buffer) with element lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyBufferWithELs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.BUFFER);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter)).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}

	// Encode (with ETA) a map (key is AsciiString) with element lists
	// We pass in the buffer to this method with the total length available.
	public static int eta_EncodeMapKeyAsciiStringWithELs(com.refinitiv.eta.codec.Buffer etaBuf)
	{
		// use this to store and check return codes
		int retVal;
		boolean success = true;

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(etaBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		//  create and initialize map structure
		com.refinitiv.eta.codec.Map etaMap = CodecFactory.createMap();

		// populate map structure prior to call to EncodeMapInit
		// NOTE: the key names used may not correspond to actual name values

		// indicate that summary data, a key field id, and a total count hint will be encoded
		etaMap.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_TOTAL_COUNT_HINT);
		// populate maps keyPrimitiveType and containerType
		etaMap.containerType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		etaMap.keyPrimitiveType(com.refinitiv.eta.codec.DataTypes.ASCII_STRING);
		// populate total count hint with approximate expected entry count
		etaMap.totalCountHint(3);
		etaMap.keyFieldId(3426);

		// Begin encoding of map - assumes that encodeIter is already populated with
		// buffer and version information, store return value to determine success or failure.
		// Expect summary data of approx. 256 bytes, no set definition data.
		if ((retVal = etaMap.encodeInit(encodeIter, 256, 0 )) < CodecReturnCodes.SUCCESS)
		{
			// error condition - switch our success value to false so we can roll back
			success = false;
			// print out message with return value string, value, and text
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapInit.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
		}
		else
		{
			// mapInit encoding was successful
			// create a single MapEntry and ElementList and reuse for each entry
			com.refinitiv.eta.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
			com.refinitiv.eta.codec.Buffer entryKey = CodecFactory.createBuffer();

			// Create a Buffer for uInt and element list to encode into
			// Here we are heap allocating the buffers (10 bytes is large enough for
			// UInt encoding and 500 bytes is large enough for ElementList encoding).
			com.refinitiv.eta.codec.Buffer encBuffer = CodecFactory.createBuffer();
			encBuffer.data(ByteBuffer.allocate(10));
			com.refinitiv.eta.codec.Buffer encElementList = CodecFactory.createBuffer();
			encElementList.data(ByteBuffer.allocate(500));

			System.out.println("\tMap Header Encoded");
			System.out.println();
			
			
			// Encode expected summary data, init for this was done by EncodeMapInit
		    // This type should match Map.containerType
			{
				System.out.println("\tEncoding Summary Data");

				// Now encode nested container using its own specific encode methods.
				// Begin encoding of element list - using same encIterator as map list.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}

				System.out.println("\tSummary Data Encoding Complete");
				System.out.println();
			}
			// complete encoding of summary data.  If any element list encoding failed, success is false.
			if ((retVal = etaMap.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapSummaryDataComplete.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			
			// FIRST Map Entry: encode entry from non pre-encoded data and key.  Approx. encoded length unknown.
			mapEntry.action(MapEntryActions.UPDATE);
			entryKey.data("keyData1");
			if ((retVal = mapEntry.encodeInit(encodeIter, entryKey, 0)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back.
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding element list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryInit.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}
			else
			// encode contained element list - this type should match Map.containerType
			{
				System.out.println("\tEncoding Map Entry (key: " + entryKey.toString() + ") from non pre-encoded data and key");

				// Now encode nested container using its own specific encode methods.
				// Clear, then begin encoding of element list - using same encIterator as map.
				if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
				{
					// error condition - switch our success value to false so we can roll back
					success = false;
					System.out.println("Error encoding field list.");
					System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + " (" + retVal + ") encountered with eta_EncodeElementListAll(encodeIter)).  "
							+ "Error Text: " + CodecReturnCodes.info(retVal)); 
				}
			
			}
			if ((retVal = mapEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
			{
				// error condition - switch our success value to false so we can roll back
				success = false;
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntryComplete.  "
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
				System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapEntry.  "
						+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			}

			System.out.println("\tMap Entry (key: " + entryKey.toString() + ") Encoding Complete");
			System.out.println();
		}

		// Complete map encoding.
		// If success parameter is true, this will finalize encoding.  
		// If success parameter is false, this will roll back encoding prior to EncodeMapInit.
		if ((retVal = etaMap.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS) 
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with EncodeMapComplete.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}
		
        System.out.println("\tETA Map Encoding Complete");
        
        return CodecReturnCodes.SUCCESS;
	}
	
	public static int eta_EncodeRequestMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal = 0;
		boolean success;
		
		System.out.println();
		
		System.out.println("Begin ETA RequestMsg Set");
		com.refinitiv.eta.codec.RequestMsg requestMsg = (com.refinitiv.eta.codec.RequestMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		requestMsg.streamId(555);
		
		requestMsg.applyHasPriority();
		requestMsg.priority().count(5);
		requestMsg.priority().priorityClass(7);
		
		requestMsg.applyHasQos();
		requestMsg.qos().dynamic(false);
		requestMsg.qos().rate(com.refinitiv.eta.codec.QosRates.TICK_BY_TICK);
		requestMsg.qos().timeliness(com.refinitiv.eta.codec.QosTimeliness.REALTIME);
		
		requestMsg.applyPrivateStream();
		
		requestMsg.applyStreaming();
		
		requestMsg.applyConfInfoInUpdates();
		
		requestMsg.applyHasExtendedHdr();
		
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data( "ABCDEF" );
		
		requestMsg.msgKey().applyHasNameType();
		requestMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  requestMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA RequestMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeRefreshMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println();
		
		 System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		permissionData.data("RefreshMsg.permissionData");
		refreshMsg.applyHasPermData();
		refreshMsg.permData(permissionData);
	
		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  refreshMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA RefreshMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeUpdateMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		// Create a ETA Buffer to encode into
		System.out.println();
		
		System.out.println("Begin ETA UpdateMsg Set");
		com.refinitiv.eta.codec.UpdateMsg updateMsg = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		updateMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.UPDATE);
		
		updateMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		updateMsg.streamId( 15 );
		
		updateMsg.applyHasSeqNum();
		updateMsg.seqNum( 22 );

		updateMsg.applyHasMsgKey();

		updateMsg.msgKey().applyHasName();
		updateMsg.msgKey().name().data( "ABCDEF" );
		
		updateMsg.msgKey().applyHasNameType();
		updateMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		updateMsg.msgKey().applyHasServiceId();
		updateMsg.msgKey().serviceId(5);
		
		updateMsg.msgKey().applyHasFilter();
		updateMsg.msgKey().filter( 12 );
	
		updateMsg.msgKey().applyHasIdentifier();
		updateMsg.msgKey().identifier(21);
		
		updateMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.refinitiv.eta.codec.CodecFactory.createBuffer();
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  updateMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA UpdateMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeGenericMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin ETA GenericMsg Set");
		com.refinitiv.eta.codec.GenericMsg genericMsg = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
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
		genericMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.refinitiv.eta.codec.CodecFactory.createBuffer();
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  genericMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA GenericMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodePostMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		// Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        fieldListBuf.data(ByteBuffer.allocate(4048));

		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return retVal;
		}
		
		System.out.println();
		
		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
		System.out.println("Begin ETA PostMsg Set");
		com.refinitiv.eta.codec.PostMsg postMsg = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.POST);
		
		postMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.refinitiv.eta.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.applyHasExtendedHdr();
		Buffer permissionData = com.refinitiv.eta.codec.CodecFactory.createBuffer();
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  postMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA PostMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeAckMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin ETA AckMsg Set");
		com.refinitiv.eta.codec.AckMsg ackMsg = (com.refinitiv.eta.codec.AckMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.refinitiv.eta.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.refinitiv.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  ackMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA AckMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeStatusMsgAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType)
	{
		// use this to store and check return codes
		int retVal;
		boolean success;
		
		System.out.println("Begin ETA StatusMsg Set");
		com.refinitiv.eta.codec.StatusMsg statusMsg = (com.refinitiv.eta.codec.StatusMsg) com.refinitiv.eta.codec.CodecFactory
				.createMsg();
		statusMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.STATUS);

		statusMsg.domainType(com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE);

		statusMsg.streamId(15);

		statusMsg.applyHasMsgKey();

		statusMsg.msgKey().applyHasName();
		statusMsg.msgKey().name().data("ABCDEF");

		statusMsg.msgKey().applyHasNameType();
		statusMsg.msgKey().nameType(com.refinitiv.eta.rdm.InstrumentNameTypes.RIC);

		statusMsg.msgKey().applyHasServiceId();
		statusMsg.msgKey().serviceId(5);

		statusMsg.msgKey().applyHasFilter();
		statusMsg.msgKey().filter(12);

		statusMsg.msgKey().applyHasIdentifier();
		statusMsg.msgKey().identifier(21);
		
		statusMsg.applyHasExtendedHdr();
		
		Buffer permissionData = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		permissionData.data("StatusMsg.permissionData");
		statusMsg.applyHasPermData();
		statusMsg.permData(permissionData);

		statusMsg.applyHasState();
		statusMsg.state().code(com.refinitiv.eta.codec.StateCodes.NONE);
		statusMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		statusMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.SERIES:
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MAP:
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
			
		case com.refinitiv.eta.codec.DataTypes.MSG:
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeKeyAttribComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeKeyAttribComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			if ( (retVal =  statusMsg.encodeExtendedHeaderComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS )
			{
				 // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with encodeExtendedHeaderComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
			}
			
			if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
			
			break;
		default:
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
		
		System.out.println("End ETA StatusMsg Set");
		System.out.println();
		
		return retVal;
	}
	
	public static int eta_EncodeFilterListAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int encodeOption)
	{
		int retVal;
		boolean success;
		
		if ( ( encodeOption & EncodingTypeFlags.PRIMITIVE_TYPES ) != 0 )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize filter list structure
		com.refinitiv.eta.codec.FilterList etaFilterList = CodecFactory.createFilterList();
		
		etaFilterList.applyHasTotalCountHint();
		etaFilterList.applyHasPerEntryPermData();
		
		com.refinitiv.eta.codec.FilterEntry filterEntry = CodecFactory.createFilterEntry();
		
		int totalCountHint = 0;
		
		if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
			totalCountHint = 6;
		
		if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
			totalCountHint += 6;
		
		
		if ( totalCountHint != 0 )
		{
			etaFilterList.totalCountHint(totalCountHint);
			
			// begin encoding of filter list - assumes that encodeIter is already populated with buffer and version information
	        if ((retVal = etaFilterList.encodeInit(encodeIter) ) < CodecReturnCodes.SUCCESS)
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
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        success = true;
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode ElementList to filter entry
            filterEntry.id(2);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode FilterList to filter entry
            filterEntry.id(3);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Series to filter entry
            filterEntry.id(4);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.SERIES);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Vector to filter entry
            filterEntry.id(5);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.VECTOR);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }
            
            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
            // Encode Map to filter entry
            filterEntry.id(6);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MAP);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
            if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
                if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
            }

            // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
		{
			success = true;
			
			filterEntry.clear();
			filterEntry.id(7);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
            {
                // error condition - switch our success value to false so we can roll back
                success = false;
                // print out message with return value string, value, and text
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
            }
            else
            {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
            }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(8);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeUpdateMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(9);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeStatusMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(10);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeGenericMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(11);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodePostMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	        // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
	        
	        filterEntry.clear();
			filterEntry.id(12);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeAckMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	     // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
            
            filterEntry.clear();
			filterEntry.id(13);
	        filterEntry.action(com.refinitiv.eta.codec.FilterEntryActions.SET);
	        
	        filterEntry.applyHasContainerType();
	        filterEntry.containerType(com.refinitiv.eta.codec.DataTypes.MSG);
	        
	        filterEntry.applyHasPermData();
	        filterEntry.permData().data("PermissionData");
	        
	        if ( (retVal = filterEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
           {
               // error condition - switch our success value to false so we can roll back
               success = false;
               // print out message with return value string, value, and text
               System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeInit().  "
                       + "Error Text: " + CodecReturnCodes.info(retVal));
           }
           else
           {
		        // Encode primitive types to FieldList of RefreshMsg
		        if ( ( retVal = eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST )) < CodecReturnCodes.SUCCESS )
		        {
		        	// print out message with return value string, value, and text
		            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRequestMsgAll().  "
		                    + "Error Text: " + CodecReturnCodes.info(retVal));
		            return retVal;
		        }
           }
	        
	     // Complete encoding of complex filter entry.
            if ((retVal = filterEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding of FilterList.
		if ((retVal = etaFilterList.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with FilterList.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}
	
	public static int eta_EncodeSeriesAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType )
	{
		int retVal;
		boolean success;
		
		if ( containerType < com.refinitiv.eta.codec.DataTypes.FIELD_LIST )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize series structure
		com.refinitiv.eta.codec.Series etaSeries = CodecFactory.createSeries();
		
		int totalCountHint = 2; // Add two series entry
		
		etaSeries.containerType(containerType);
		
		etaSeries.applyHasTotalCountHint();
		etaSeries.totalCountHint(totalCountHint);
		
		etaSeries.applyHasSummaryData();
		
		// begin encoding of series - assumes that encodeIter is already populated with buffer and version information
        if ((retVal = etaSeries.encodeInit(encodeIter, 0, 0) ) < CodecReturnCodes.SUCCESS)
        {
            // print out message with return value string, value, and text
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeInit().  "
                    + "Error Text: " + CodecReturnCodes.info(retVal));
            return retVal;
        }

		com.refinitiv.eta.codec.SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
		
		success = true;
		int argument = 0;
		
		for ( int i = 0; i < totalCountHint; i++ )
		{
			 seriesEntry.clear();
			
			switch (containerType)
			{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
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
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeFieldListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding element list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
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
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeElementListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding filter list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
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
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeFilterListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.SERIES:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding series list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeSeriesAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeVectorAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.MAP:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding map.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.MSG:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding msg.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any series encoding failed, success is false.
					if ((retVal = etaSeries.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST;
				}
				
				 if ( (retVal = seriesEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
		         {
		                // error condition - switch our success value to false so we can roll back
		                success = false;
		                // print out message with return value string, value, and text
		                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeInit().  "
		                        + "Error Text: " + CodecReturnCodes.info(retVal));
		                break;
		         }
				
				if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			default:
				break;
			}
			
			 // Complete encoding of complex series entry.
            if ((retVal = seriesEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with SeriesEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding of series.
		if ((retVal = etaSeries.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Series.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}

	public static int eta_EncodeVectorAll(com.refinitiv.eta.codec.EncodeIterator encodeIter, int containerType )
	{
		int retVal;
		boolean success;
		
		if ( containerType < com.refinitiv.eta.codec.DataTypes.FIELD_LIST )
			return CodecReturnCodes.INVALID_ARGUMENT;
		
		// create and initialize vector structure
		com.refinitiv.eta.codec.Vector etaVector = CodecFactory.createVector();
		
		int totalCountHint = 2; // Add three filter entry
	
		etaVector.containerType(containerType);
		
		etaVector.applyHasTotalCountHint();
		etaVector.totalCountHint(totalCountHint);
		
		etaVector.applyHasSummaryData();
		
		// begin encoding of vector - assumes that encodeIter is already populated with buffer and version information
        if ((retVal = etaVector.encodeInit(encodeIter, 0, 0) ) < CodecReturnCodes.SUCCESS)
        {
            // print out message with return value string, value, and text
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeInit().  "
                    + "Error Text: " + CodecReturnCodes.info(retVal));
            return retVal;
        }
		
		com.refinitiv.eta.codec.VectorEntry vectorEntry = CodecFactory.createVectorEntry();
		
		success = true;
		int argument = 0;
		
		for ( int i = 0; i < totalCountHint; i++ )
		{
			vectorEntry.clear();
			
			switch (containerType)
			{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding field list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeFieldListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFieldListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding element list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.PRIMITIVE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeElementListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeElementListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeFilterListAll(encodeIter, EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding filter list.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				else if ( i == 1 )
				{
					argument = EncodingTypeFlags.MESSAGE_TYPES;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeFilterListAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeFilterListAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.SERIES:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeSeriesAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding series.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeSeriesAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeSeriesAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeVectorAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeVectorAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeVectorAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.MAP:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeMapKeyUIntAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding vector.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.MSG;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeMapKeyUIntAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
				
			case com.refinitiv.eta.codec.DataTypes.MSG:
				
				if ( i == 0 )
				{
					System.out.println("\tEncoding Summary Data");
					success = true;

					if ((retVal = eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
					{
						success = false;
						System.out.println("Error encoding msg.");
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeRefreshMsgAll().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}

					System.out.println("\tSummary Data Encoding Complete");
					System.out.println();
					
					// complete encoding of summary data.  If any vector encoding failed, success is false.
					if ((retVal = etaVector.encodeSummaryDataComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)	
					{
						// error condition - switch our success value to false so we can roll back
						success = false;
						System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeSummaryDataComplete().  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
					}
					
					argument = com.refinitiv.eta.codec.DataTypes.FIELD_LIST;
				}
				else if ( i == 1 )
				{
					argument = com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST;
				}
				
				vectorEntry.index(i+2);
				vectorEntry.action(com.refinitiv.eta.codec.VectorEntryActions.SET);
				vectorEntry.applyHasPermData();
				vectorEntry.permData().data("PermissionData");
	            if ( (retVal = vectorEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS )
	            {
	                // error condition - switch our success value to false so we can roll back
	                success = false;
	                // print out message with return value string, value, and text
	                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeInit().  "
	                        + "Error Text: " + CodecReturnCodes.info(retVal));
	                break;
	            }
				
				if ( (retVal = eta_EncodeRefreshMsgAll(encodeIter, argument )) < CodecReturnCodes.SUCCESS)
                {
                    // error condition - switch our success value to false so we can roll back
                    success = false;
                    System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with eta_EncodeMapKeyUIntAll().  "
                            + "Error Text: " + CodecReturnCodes.info(retVal));
                }
				
				break;
			default:
				break;
			}
			
			 // Complete encoding of complex vector entry.
            if ((retVal = vectorEntry.encodeComplete(encodeIter, success)) < CodecReturnCodes.SUCCESS)
            {
                System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with VectorEntry.encodeComplete().  "
                        + "Error Text: " + CodecReturnCodes.info(retVal));
                return retVal;
            }
		}
		
		// Complete encoding vector.
		if ((retVal = etaVector.encodeComplete(encodeIter, true) ) < CodecReturnCodes.SUCCESS)
	    {
	        // print out message with return value string, value, and text
	        System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ") encountered with Vector.encodeComplete().  "
	                + "Error Text: " + CodecReturnCodes.info(retVal));
	        return retVal;
	    }
	
		return 0;
	}

    public static void eta_encodeDictionaryMsg(com.refinitiv.eta.codec.DataDictionary dictionary)
    {
    	com.refinitiv.eta.codec.Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(12300));
        com.refinitiv.eta.codec.RefreshMsg msg = (com.refinitiv.eta.codec.RefreshMsg)CodecFactory.createMsg();
        com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        com.refinitiv.eta.transport.Error error = com.refinitiv.eta.transport.TransportFactory.createError();
        com.refinitiv.eta.codec.Buffer nameBuf = CodecFactory.createBuffer();
        com.refinitiv.eta.codec.Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field and Type dictionaries
        dictionary.clear();

        checkResult(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary(fieldDictionaryFileName, error));
        checkResult(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary(enumTableFileName, error));
        
        // set-up message
        msg.msgClass(MsgClasses.REFRESH);
        msg.domainType(com.refinitiv.eta.rdm.DomainTypes.DICTIONARY);
        msg.containerType(com.refinitiv.eta.codec.DataTypes.SERIES);
		msg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		msg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		msg.state().code(com.refinitiv.eta.codec.StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
		msg.applySolicited();
		msg.applyHasMsgKey();
    	msg.msgKey().filter(com.refinitiv.eta.rdm.Dictionary.VerbosityValues.NORMAL);
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
        										com.refinitiv.eta.rdm.Dictionary.VerbosityValues.NORMAL, error));        

        checkResult(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
    }

    
	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a ETA Buffer with length and data properly populated.
	private static int eta_getPreEncodedUIntBuffer(com.refinitiv.eta.codec.Buffer encUInt, com.refinitiv.eta.codec.UInt uInt)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encUInt.data() points to sufficient memory and encUInt.length()
		// indicates number of bytes available in encUInt.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encUInt, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = uInt.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeUInt.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	// This method returns a preencoded buffer containing an encoded Int type.
	// Assuming encInt is a ETA Buffer with length and data properly populated.
	private static int eta_getPreEncodedIntBuffer(com.refinitiv.eta.codec.Buffer encInt, com.refinitiv.eta.codec.Int Int)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encInt.data() points to sufficient memory and encInt.length()
		// indicates number of bytes available in encInt.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encInt, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = Int.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeInt.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

//	// This method returns a preencoded buffer containing an encoded Buffer type.
//	// Assuming encBuffer is a ETA Buffer with length and data properly populated.
//	private static int eta_getPreEncodedBufferBuffer(com.refinitiv.eta.codec.Buffer encBuffer, com.refinitiv.eta.codec.Buffer buffer)
//	{
//		int retVal;  //used to store and check the return value from the method calls
//
//		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
//		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
//		
//		// create and clear iterator to prepare for encoding
//		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
//		encodeIter.clear();
//
//		// associate buffer and iterator and set proper protocol version information on iterator
//		// assuming that encBuffer.data() points to sufficient memory and encBuffer.length()
//		// indicates number of bytes available in encBuffer.data()
//		if ((retVal = encodeIter.setBufferAndRWFVersion(encBuffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
//					+ "Error Text: " + CodecReturnCodes.info(retVal)); 
//
//			return retVal;
//		}
//
//		if ((retVal = buffer.encode(encodeIter)) < CodecReturnCodes.SUCCESS)
//		{
//			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with EncodeBuffer.  "
//								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
//			return retVal;
//		}
//
//		return CodecReturnCodes.SUCCESS;
//	}

	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a ETA Buffer with length and data properly populated.
	private static int eta_getPreEncodedFieldListBuffer(com.refinitiv.eta.codec.Buffer encFieldList)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encFieldList.data() points to sufficient memory and encFieldList.length()
		// indicates number of bytes available in encFieldList.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encFieldList, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = eta_EncodeFieldListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with eta_EncodeFieldListAll.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	// This method returns a preencoded buffer containing an encoded UInt type.
	// Assuming encUInt is a ETA Buffer with length and data properly populated.
	private static int eta_getPreEncodedElementListBuffer(com.refinitiv.eta.codec.Buffer encElementList)
	{
		int retVal;  //used to store and check the return value from the method calls

		int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		
		// create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();

		// associate buffer and iterator and set proper protocol version information on iterator
		// assuming that encFieldList.data() points to sufficient memory and encFieldList.length()
		// indicates number of bytes available in encFieldList.data()
		if ((retVal = encodeIter.setBufferAndRWFVersion(encElementList, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with setBufferAndRWFVersion.  "
					+ "Error Text: " + CodecReturnCodes.info(retVal)); 

			return retVal;
		}

		if ((retVal = eta_EncodeElementListAll(encodeIter, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("ETA error " + CodecReturnCodes.toString(retVal) + "(" + retVal + ")" + " encountered with eta_EncodeElementListAll.  "
								+ "Error Text: " + CodecReturnCodes.info(retVal)); 
			return retVal;
		}

		return CodecReturnCodes.SUCCESS;
	}

	
	
	public static void EmaDecode_ETAFieldListAll(com.refinitiv.ema.access.FieldList fl, int encodeOption)
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
	        com.refinitiv.ema.access.FieldEntry fe1 = iter.next();
	        checkResult(fe1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ERROR);
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
	        checkResult(fe1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ERROR);
	        checkResult(fe1.error().errorCode(), OmmError.ErrorCode.FIELD_ID_NOT_FOUND);
	
	        // check second field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe2 = iter.next();
	        checkResult(fe2.fieldId(), 16);
	        checkResult(fe2.name(), "TRADE_DATE");
	        checkResult(fe2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.DATE);
	        checkResult(fe2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.DATE);
	        checkResult(fe2.code(), Data.DataCode.NO_CODE);
	        checkResult(fe2.date().day(), 31);
	        checkResult(fe2.date().month(), 1);
	        checkResult(fe2.date().year(), 2022);
	       
	        // check third field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe3 = iter.next();
	        checkResult(fe3.fieldId(), 147);
	        checkResult(fe3.name(), "OFF_BND_NO");
	        checkResult(fe3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe3.code(), Data.DataCode.NO_CODE);
	        checkResult(fe3.uintValue(), 23456);
	
	        // check fourth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe4 = iter.next();
	        checkResult(fe4.fieldId(), 1080);
	        checkResult(fe4.name(), "PREF_DISP");
	        checkResult(fe4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(fe4.code(), Data.DataCode.NO_CODE);
	        checkResult(fe4.uintValue(), 23456);
	        
	        // check fifth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe5 = iter.next();
	        checkResult(fe5.fieldId(), 22);
	        checkResult(fe5.name(), "BID");
	        checkResult(fe5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe5.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe5.code(), Data.DataCode.BLANK);
	        
	        // check sixth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe6 = iter.next();
	        checkResult(fe6.fieldId(), 24);
	        checkResult(fe6.name(), "BID_2");
	        checkResult(fe6.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe6.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
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
	        com.refinitiv.ema.access.FieldEntry fe7 = iter.next();
	        checkResult(fe7.fieldId(), 25);
	        checkResult(fe7.name(), "ASK");
	        checkResult(fe7.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe7.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(fe7.code(), Data.DataCode.NO_CODE);
	        checkResult(fe7.real().mantissa(), 22801);
	        checkResult(fe7.real().magnitudeType(), 10);
	        
	        // check eighth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe8 = iter.next();
	        checkResult(fe8.fieldId(), 18);
	        checkResult(fe8.name(), "TRDTIM_1");
	        checkResult(fe8.loadType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
	        checkResult(fe8.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
	        checkResult(fe8.code(), Data.DataCode.NO_CODE);
	        checkResult(fe8.time().hour(), 8);
	        checkResult(fe8.time().minute(), 39);
	        checkResult(fe8.time().second(), 24);
	        
	        // check nineth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe9 = iter.next();
	        checkResult(fe9.fieldId(), TestUtilities.INTEGER);
	        checkResult(fe9.name(), "INTEGER");
	        checkResult(fe9.loadType(), com.refinitiv.ema.access.DataType.DataTypes.INT);
	        checkResult(fe9.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.INT);
	        checkResult(fe9.code(), Data.DataCode.NO_CODE);
	        checkResult(fe9.intValue(), 65432);
	        
	        // check tenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe10 = iter.next();
	        checkResult(fe10.fieldId(), TestUtilities.MY_FLOAT);
	        checkResult(fe10.name(), "MY_FLOAT");
	        checkResult(fe10.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(fe10.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(fe10.code(), Data.DataCode.NO_CODE);
	        checkResult(fe10.floatValue(), 3.14f);
	
	        // check eleventh field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe11 = iter.next();
	        checkResult(fe11.fieldId(), TestUtilities.MY_DOUBLE);
	        checkResult(fe11.name(), "MY_DOUBLE");
	        checkResult(fe11.loadType(), com.refinitiv.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(fe11.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(fe11.code(), Data.DataCode.NO_CODE);
	        checkResult(fe11.doubleValue(), 3.1416);
	
	        // check twelveth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe12 = iter.next();
	        checkResult(fe12.fieldId(), TestUtilities.TRADE_DATETIME);
	        checkResult(fe12.name(), "TRADE_DATE");
	        checkResult(fe12.loadType(), com.refinitiv.ema.access.DataType.DataTypes.DATETIME);
	        checkResult(fe12.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.DATETIME);
	        checkResult(fe12.code(), Data.DataCode.NO_CODE);
	        checkResult(fe12.dateTime().month(), 11);
	        checkResult(fe12.dateTime().day(), 15);
	        checkResult(fe12.dateTime().year(), 2011);
	        checkResult(fe12.dateTime().hour(), 8);
	        checkResult(fe12.dateTime().minute(), 39);
	        checkResult(fe12.dateTime().second(), 24);
	        
	        // check thirteenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe13 = iter.next();
	        checkResult(fe13.fieldId(), TestUtilities.MY_QOS);
	        checkResult(fe13.name(), "MY_QOS");
	        checkResult(fe13.loadType(), com.refinitiv.ema.access.DataType.DataTypes.QOS);
	        checkResult(fe13.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.QOS);
	        checkResult(fe13.code(), Data.DataCode.NO_CODE);
	        checkResult(fe13.qos().timeliness(), OmmQos.Timeliness.REALTIME);
	        checkResult(fe13.qos().rate(), OmmQos.Rate.TICK_BY_TICK);
	
	        // check fourteenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe14 = iter.next();
	        checkResult(fe14.fieldId(), TestUtilities.MY_STATE);
	        checkResult(fe14.name(), "MY_STATE");
	        checkResult(fe14.loadType(), com.refinitiv.ema.access.DataType.DataTypes.STATE);
	        checkResult(fe14.code(), Data.DataCode.NO_CODE);
	        checkResult(fe14.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.STATE);
	        checkResult(fe14.state().streamState(), OmmState.StreamState.OPEN);
	        checkResult(fe14.state().dataState(), OmmState.DataState.OK);
	        checkResult(fe14.state().statusCode(), OmmState.StatusCode.NONE);
	        checkResult(fe14.state().statusText(), "Succeeded");
	        checkResult(fe14.state().toString(), "Open / Ok / None / 'Succeeded'");
	
	        // check fifteenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe15 = iter.next();
	        checkResult(fe15.fieldId(), TestUtilities.MY_BUFFER);
	        checkResult(fe15.name(), "MY_BUFFER");
	        checkResult(fe15.loadType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
	        checkResult(fe15.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
	        checkResult(fe15.code(), Data.DataCode.NO_CODE);
	        com.refinitiv.eta.codec.Buffer Buf = CodecFactory.createBuffer();
	        Buf.data("ABCDEFGH");
	        checkResult( Arrays.equals(fe15.buffer().buffer().array(), Buf.data().array()) );
	
	        // check sixteenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe16 = iter.next();
	        checkResult(fe16.fieldId(), 4);
	        checkResult(fe16.name(), "RDN_EXCHID");
	        checkResult(fe16.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe16.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe16.code(), Data.DataCode.NO_CODE);
	        checkResult(fe16.enumValue() == 29, "enumValue() == 29");
	        checkResult(fe16.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe16.enumDisplay().equals("CSC") , "enumDisplay() == \"CSC\"");
	        
	        // check seventeenth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe17 = iter.next();
	        checkResult(fe17.fieldId(), TestUtilities.MY_ARRAY);
	        checkResult(fe17.name(), "MY_ARRAY");
	        checkResult(fe17.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ARRAY);
	        checkResult(fe17.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ARRAY);
	        checkResult(fe17.code(), Data.DataCode.NO_CODE);
	        OmmArray ar2 = fe17.array();
	        Iterator<OmmArrayEntry> ar2Iter = ar2.iterator();
	        
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae1 = ar2Iter.next();
	                     checkResult(ae1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae1.uintValue(), 10);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae2 = ar2Iter.next();
	                     checkResult(ae2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae2.uintValue(), 20);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae3 = ar2Iter.next();
	                     checkResult(ae3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae3.uintValue(), 30);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae4 = ar2Iter.next();
	                     checkResult(ae4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae4.uintValue(), 40);
	                checkResult(ar2Iter.hasNext());
	                     OmmArrayEntry ae5 = ar2Iter.next();
	                     checkResult(ae5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	                     checkResult(ae5.uintValue(), 23456);
	                checkResult(!ar2Iter.hasNext());
	                
	                
	        // Check eighteenth field entry
            checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe18 = iter.next();
	        checkResult(fe18.fieldId(), 4);
	        checkResult(fe18.name(), "RDN_EXCHID");
	        checkResult(fe18.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe18.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
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
	        com.refinitiv.ema.access.FieldEntry fe19 = iter.next();
	        checkResult(fe19.fieldId(), 4);
	        checkResult(fe19.name(), "RDN_EXCHID");
	        checkResult(fe19.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe19.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
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
	        com.refinitiv.ema.access.FieldEntry fe20 = iter.next();
	        checkResult(fe20.fieldId(), 115);
	        checkResult(fe20.name(), "BID_TICK_1");
	        checkResult(fe20.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe20.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe20.code(), Data.DataCode.NO_CODE);
	        checkResult(fe20.enumValue() == 0, "enumValue() == 0");
	        checkResult(fe20.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe20.enumDisplay().equals(" ") , "enumDisplay() == \" \"");
	        


	        
	        // check twenty fifth field entry
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fe25 = iter.next();
	        checkResult(fe25.fieldId(), 8960);
	        checkResult(fe25.name(), "GBLISS_IND");
	        checkResult(fe25.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe25.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ENUM);
	        checkResult(fe25.code(), Data.DataCode.NO_CODE);
	        checkResult(fe25.enumValue() == 2, "enumValue() == 2");
	        checkResult(fe25.hasEnumDisplay(), "hasEnumDisplay() == true");
	        checkResult(fe25.enumDisplay().equals("  EURO  ") , "enumDisplay() == \"  EURO  \"");

			// check twenty sixth entry
			checkResult(iter.hasNext());
			com.refinitiv.ema.access.FieldEntry fe26 = iter.next();
			checkResult(fe26.fieldId(), 16);
			checkResult(fe26.name(), "TRADE_DATE");
			checkResult(fe26.loadType(), DataTypes.ERROR);
			checkResult(fe26.load().dataType(), DataTypes.ERROR);
			checkResult(fe26.code(), Data.DataCode.NO_CODE);
			checkResult(fe26.error().errorCode(), OmmError.ErrorCode.INCOMPLETE_DATA);

			// check twenty seventh field entry
			checkResult(iter.hasNext());
			com.refinitiv.ema.access.FieldEntry fe27 = iter.next();
			checkResult(fe27.fieldId(), 6);
			checkResult(fe27.name(), "TRDPRC_1");
			checkResult(fe27.loadType(), DataTypes.ERROR);
			checkResult(fe27.load().dataType(), DataTypes.ERROR);
			checkResult(fe27.code(), Data.DataCode.NO_CODE);
			checkResult(fe27.error().errorCode(), OmmError.ErrorCode.INCOMPLETE_DATA);

			// check twenty eighth field entry
			checkResult(iter.hasNext());
			com.refinitiv.ema.access.FieldEntry fe28 = iter.next();
			checkResult(fe28.fieldId(), 5);
			checkResult(fe28.name(), "TIMACT");
			checkResult(fe28.loadType(), DataTypes.ERROR);
			checkResult(fe28.load().dataType(), DataTypes.ERROR);
			checkResult(fe28.code(), Data.DataCode.NO_CODE);
			checkResult(fe28.error().errorCode(), OmmError.ErrorCode.INCOMPLETE_DATA);

	        if ( encodeOption == EncodingTypeFlags.PRIMITIVE_TYPES )
	        	checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
       	
       	if ( ( encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
       	{
       		// check field entry which contains FieldList
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry1 = iter.next();
	        checkResult(fieldEntry1.fieldId(), TestUtilities.MY_FIELDLIST);
	        checkResult(fieldEntry1.name(), "MY_FIELDLIST");
	        checkResult(fieldEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(fieldEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(fieldEntry1.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAFieldListAll(fieldEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES );
	        
	        // check field entry which contains ElementList
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry2 = iter.next();
	        checkResult(fieldEntry2.fieldId(), TestUtilities.MY_ELEMENTLIST);
	        checkResult(fieldEntry2.name(), "MY_ELEMENTLIST");
	        checkResult(fieldEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(fieldEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(fieldEntry2.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAElementListAll(fieldEntry2.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES );
	        
	        // check field entry which contains FilterList
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry3 = iter.next();
	        checkResult(fieldEntry3.fieldId(), TestUtilities.MY_FILTERLIST);
	        checkResult(fieldEntry3.name(), "MY_FILTERLIST");
	        checkResult(fieldEntry3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(fieldEntry3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(fieldEntry3.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAFilterListAll(fieldEntry3.filterList(), EncodingTypeFlags.MESSAGE_TYPES );
	        
	        // check field entry which contains Series
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry4 = iter.next();
	        checkResult(fieldEntry4.fieldId(), TestUtilities.MY_SERIES);
	        checkResult(fieldEntry4.name(), "MY_SERIES");
	        checkResult(fieldEntry4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
	        checkResult(fieldEntry4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
	        checkResult(fieldEntry4.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETASeriesAll(fieldEntry4.series(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST );
	        
	        // check field entry which contains Vector
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry5 = iter.next();
	        checkResult(fieldEntry5.fieldId(), TestUtilities.MY_VECTOR);
	        checkResult(fieldEntry5.name(), "MY_VECTOR");
	        checkResult(fieldEntry5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(fieldEntry5.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(fieldEntry5.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAVectorAll(fieldEntry5.vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
	        
	        // check field entry which contains Map
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry6 = iter.next();
	        checkResult(fieldEntry6.fieldId(), TestUtilities.MY_MAP);
	        checkResult(fieldEntry6.name(), "MY_MAP");
	        checkResult(fieldEntry6.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
	        checkResult(fieldEntry6.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
	        checkResult(fieldEntry6.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAMapKeyUIntAll(fieldEntry6.map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
	        
	        if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
	        	checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
       	
    	if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
       	{
    		// check first field entry which contains FieldList
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry1 = iter.next();
	        checkResult(fieldEntry1.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry1.name(), "MY_MSG");
	        checkResult(fieldEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(fieldEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(fieldEntry1.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETARefreshMsgAll(fieldEntry1.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry2 = iter.next();
	        checkResult(fieldEntry2.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry2.name(), "MY_MSG");
	        checkResult(fieldEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(fieldEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(fieldEntry2.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAUpdateMsgAll(fieldEntry2.updateMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry3 = iter.next();
	        checkResult(fieldEntry3.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry3.name(), "MY_MSG");
	        checkResult(fieldEntry3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(fieldEntry3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(fieldEntry3.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAStatusMsgAll(fieldEntry3.statusMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry4 = iter.next();
	        checkResult(fieldEntry4.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry4.name(), "MY_MSG");
	        checkResult(fieldEntry4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(fieldEntry4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(fieldEntry4.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAGenericMsgAll(fieldEntry4.genericMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry5 = iter.next();
	        checkResult(fieldEntry5.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry5.name(), "MY_MSG");
	        checkResult(fieldEntry5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(fieldEntry5.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(fieldEntry5.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAPostMsgAll(fieldEntry5.postMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry6 = iter.next();
	        checkResult(fieldEntry6.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry6.name(), "MY_MSG");
	        checkResult(fieldEntry6.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(fieldEntry6.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(fieldEntry6.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETAAckMsgAll(fieldEntry6.ackMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(iter.hasNext());
	        com.refinitiv.ema.access.FieldEntry fieldEntry7 = iter.next();
	        checkResult(fieldEntry7.fieldId(), TestUtilities.MY_MSG);
	        checkResult(fieldEntry7.name(), "MY_MSG");
	        checkResult(fieldEntry7.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(fieldEntry7.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(fieldEntry7.code(), Data.DataCode.NO_CODE);
	        EmaDecode_ETARequestMsgAll(fieldEntry7.reqMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	    
	        checkResult(!iter.hasNext()); //final forth() - no more field entries
       	}
	}
	
    public static void EmaDecode_ETAArrayAll(OmmArray array)
    {
        checkResult(array.dataType(), com.refinitiv.ema.access.DataType.DataTypes.ARRAY);
        checkResult(array.hasFixedWidth());
        checkResult(array.fixedWidth(), 8);
        
        Iterator<OmmArrayEntry> iter = array.iterator();
        checkResult(iter != null);
        
        // check first array entry
        checkResult(iter.hasNext());
        com.refinitiv.ema.access.OmmArrayEntry ae1 = iter.next();
        checkResult(ae1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
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
        com.refinitiv.ema.access.OmmArrayEntry ae2 = iter.next();
        checkResult(ae2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
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
        com.refinitiv.ema.access.OmmArrayEntry ae3 = iter.next();
        checkResult(ae3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
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
        com.refinitiv.ema.access.OmmArrayEntry ae4 = iter.next();
        checkResult(ae4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
        checkResult(ae4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.BUFFER);
        ByteBuffer buffer4Contents = ae4.buffer().buffer();
        byte[] byteArray4 = new byte[buffer4Contents.limit()];
        for (int i = 0; i < buffer4Contents.limit(); i++)
        {
            byteArray4[i] = buffer4Contents.get(i);
        }
        String byteArray4String = new String(byteArray4);
        checkResult(byteArray4String.equals("BUFFER 4"));
    }
	
	public static void EmaDecode_ETAElementListAll(com.refinitiv.ema.access.ElementList el, int encodeOption)
	{
        checkResult(el.hasInfo());
        checkResult(el.infoElementListNum(), 7);

        Iterator<ElementEntry> elIter = el.iterator();
       	checkResult(elIter != null);
       	
       	if ( (encodeOption & EncodingTypeFlags.PRIMITIVE_TYPES ) != 0 )
       	{
	        // check first element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee1 = elIter.next();
	        checkResult(ee1.name(), "Element - Time");
	        checkResult(ee1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
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
	        checkResult(ee1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.TIME);
	        checkResult(ee1.code(), Data.DataCode.NO_CODE);
	        checkResult(ee1.time().hour(), 10);
	        checkResult(ee1.time().minute(), 21);
	        checkResult(ee1.time().second(), 16);
	
	        // check second element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee2 = elIter.next();
	        checkResult(ee2.name(), "Element - Int");
	        checkResult(ee2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.INT);
	        checkResult(ee2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.INT);
	        checkResult(ee2.code(), Data.DataCode.NO_CODE);
	        checkResult(ee2.intValue(), 13);
	        
	        // check third element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee3 = elIter.next();
	        checkResult(ee3.name(), "Element - UInt");
	        checkResult(ee3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(ee3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT);
	        checkResult(ee3.code(), Data.DataCode.NO_CODE);
	        checkResult(ee3.uintValue(), 17);
	       
	        // check fourth element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee4 = elIter.next();
	        checkResult(ee4.name(), "Element - Real - Blank");
	        checkResult(ee4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(ee4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REAL);
	        checkResult(ee4.code(), Data.DataCode.BLANK);
	      
	        // check fifth element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee5 = elIter.next();
	        checkResult(ee5.name(), "Element - Float");
	        checkResult(ee5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(ee5.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FLOAT);
	        checkResult(ee5.code(), Data.DataCode.NO_CODE);
	        checkResult(ee5.floatValue(), 1.34f);
	
	        // check sixth element entry
	        checkResult(elIter.hasNext());
	        com.refinitiv.ema.access.ElementEntry ee6 = elIter.next();
	        checkResult(ee6.name(), "Element - Double");
	        checkResult(ee6.loadType(), com.refinitiv.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(ee6.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.DOUBLE);
	        checkResult(ee6.code(), Data.DataCode.NO_CODE);
	        checkResult(ee6.doubleValue(), 1.345);

			// check seventh element entry
			checkResult(elIter.hasNext());
			com.refinitiv.ema.access.ElementEntry ee7 = elIter.next();
			checkResult(ee7.name(), "Element - Real - Invalid");
			checkResult(ee7.loadType(), DataTypes.ERROR);
			checkResult(ee7.load().dataType(), DataTypes.ERROR);
			checkResult(ee7.code(), Data.DataCode.NO_CODE);
			checkResult(ee7.error().errorCode(), OmmError.ErrorCode.INCOMPLETE_DATA);
	        
	        if ( encodeOption == EncodingTypeFlags.PRIMITIVE_TYPES )
	        	checkResult(!elIter.hasNext()); //final forth() - no more element entries
       	}
       	
     	if ( (encodeOption & EncodingTypeFlags.CONTAINER_TYPES ) != 0 )
       	{
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee7 = elIter.next();
 			checkResult(ee7.name(), "Element - FieldList");
 			checkResult(ee7.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
        	checkResult(ee7.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
        	EmaDecode_ETAFieldListAll(ee7.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
 		
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee8 = elIter.next();
 			checkResult(ee8.name(), "Element - ElementList");
 			checkResult(ee8.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
 			checkResult(ee8.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
 			EmaDecode_ETAElementListAll(ee8.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
     		
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee9 = elIter.next();
 			checkResult(ee9.name(), "Element - FilterList");
 			checkResult(ee9.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
 			checkResult(ee9.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
 			EmaDecode_ETAFilterListAll(ee9.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee10 = elIter.next();
 			checkResult(ee10.name(), "Element - Series");
 			checkResult(ee10.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
 			checkResult(ee10.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
 			EmaDecode_ETASeriesAll(ee10.series(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	        
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee11 = elIter.next();
 			checkResult(ee11.name(), "Element - Vector");
 			checkResult(ee11.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
 			checkResult(ee11.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
 			EmaDecode_ETAVectorAll(ee11.vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee12 = elIter.next();
 			checkResult(ee12.name(), "Element - Map");
 			checkResult(ee12.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
 			checkResult(ee12.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
 			EmaDecode_ETAMapKeyUIntAll(ee12.map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			 if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
 	        	checkResult(!elIter.hasNext()); //final forth() - no more element entries
       	}
     	
     	if ( (encodeOption & EncodingTypeFlags.MESSAGE_TYPES ) != 0 )
       	{
     		checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee15 = elIter.next();
 			checkResult(ee15.name(), "Element - Msg");
 			checkResult(ee15.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
 			checkResult(ee15.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
 			EmaDecode_ETARefreshMsgAll(ee15.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee16 = elIter.next();
 			checkResult(ee16.name(), "Element - Msg");
 			checkResult(ee16.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
 			checkResult(ee16.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
 			EmaDecode_ETAUpdateMsgAll(ee16.updateMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee17 = elIter.next();
 			checkResult(ee17.name(), "Element - Msg");
 			checkResult(ee17.loadType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
 			checkResult(ee17.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
 			EmaDecode_ETAStatusMsgAll(ee17.statusMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee18 = elIter.next();
 			checkResult(ee18.name(), "Element - Msg");
 			checkResult(ee18.loadType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
 			checkResult(ee18.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
 			EmaDecode_ETAGenericMsgAll(ee18.genericMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee19 = elIter.next();
 			checkResult(ee19.name(), "Element - Msg");
 			checkResult(ee19.loadType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
 			checkResult(ee19.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
 			EmaDecode_ETAPostMsgAll(ee19.postMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee20 = elIter.next();
 			checkResult(ee20.name(), "Element - Msg");
 			checkResult(ee20.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
 			checkResult(ee20.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
 			EmaDecode_ETAAckMsgAll(ee20.ackMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(elIter.hasNext());
 			com.refinitiv.ema.access.ElementEntry ee21 = elIter.next();
 			checkResult(ee21.name(), "Element - Msg");
 			checkResult(ee21.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
 			checkResult(ee21.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
 			EmaDecode_ETARequestMsgAll(ee21.reqMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
 			
 			checkResult(!elIter.hasNext());
       	}
	}
	
	public static void EmaDecode_ETAFilterListAll(com.refinitiv.ema.access.FilterList filterList, int encodeOption)
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
	        com.refinitiv.ema.access.FilterEntry filterEntry1 = filterIter.next();
	        
	        checkResult(filterEntry1.filterId(), 1);
	        checkResult(filterEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(filterEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
	        checkResult(filterEntry1.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry1.hasPermissionData());
	        checkResult(filterEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAFieldListAll(filterEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry2 = filterIter.next();
	        
	        checkResult(filterEntry2.filterId(), 2);
	        checkResult(filterEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(filterEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
	        checkResult(filterEntry2.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry2.hasPermissionData());
	        checkResult(filterEntry2.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAElementListAll(filterEntry2.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry3 = filterIter.next();
	        
	        checkResult(filterEntry3.filterId(), 3);
	        checkResult(filterEntry3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(filterEntry3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
	        checkResult(filterEntry3.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry3.hasPermissionData());
	        checkResult(filterEntry3.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAFilterListAll(filterEntry3.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry4 = filterIter.next();
	        
	        checkResult(filterEntry4.filterId(), 4);
	        checkResult(filterEntry4.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
	        checkResult(filterEntry4.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
	        checkResult(filterEntry4.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry4.hasPermissionData());
	        checkResult(filterEntry4.permissionData().equals(ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETASeriesAll(filterEntry4.series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry5 = filterIter.next();
	        
	        checkResult(filterEntry5.filterId(), 5);
	        checkResult(filterEntry5.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(filterEntry5.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
	        checkResult(filterEntry5.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry5.hasPermissionData());
	        checkResult(filterEntry5.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAVectorAll(filterEntry5.vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry6 = filterIter.next();
	        
	        checkResult(filterEntry6.filterId(), 6);
	        checkResult(filterEntry6.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
	        checkResult(filterEntry6.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
	        checkResult(filterEntry6.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry6.hasPermissionData());
	        checkResult(filterEntry6.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAMapKeyUIntAll(filterEntry6.map(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	        
	        if ( encodeOption == EncodingTypeFlags.CONTAINER_TYPES )
 	        	checkResult(!filterIter.hasNext()); //final forth() - no more filter entries
    	}
       	
     	if ( ( encodeOption & EncodingTypeFlags.MESSAGE_TYPES) != 0 )
    	{
     		checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry7 = filterIter.next();
	        
	        checkResult(filterEntry7.filterId(), 7);
	        checkResult(filterEntry7.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(filterEntry7.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
	        checkResult(filterEntry7.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry7.hasPermissionData());
	        checkResult(filterEntry7.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETARefreshMsgAll(filterEntry7.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry8 = filterIter.next();
	        
	        checkResult(filterEntry8.filterId(), 8);
	        checkResult(filterEntry8.loadType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(filterEntry8.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG);
	        checkResult(filterEntry8.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry8.hasPermissionData());
	        checkResult(filterEntry8.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAUpdateMsgAll(filterEntry8.updateMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry9 = filterIter.next();
	        
	        checkResult(filterEntry9.filterId(), 9);
	        checkResult(filterEntry9.loadType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(filterEntry9.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG);
	        checkResult(filterEntry9.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry9.hasPermissionData());
	        checkResult(filterEntry9.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAStatusMsgAll(filterEntry9.statusMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry10 = filterIter.next();
	        
	        checkResult(filterEntry10.filterId(), 10);
	        checkResult(filterEntry10.loadType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(filterEntry10.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.GENERIC_MSG);
	        checkResult(filterEntry10.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry10.hasPermissionData());
	        checkResult(filterEntry10.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAGenericMsgAll(filterEntry10.genericMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry11 = filterIter.next();
	        
	        checkResult(filterEntry11.filterId(), 11);
	        checkResult(filterEntry11.loadType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(filterEntry11.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.POST_MSG);
	        checkResult(filterEntry11.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry11.hasPermissionData());
	        checkResult(filterEntry11.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAPostMsgAll(filterEntry11.postMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
     		com.refinitiv.ema.access.FilterEntry filterEntry12 = filterIter.next();
	        
	        checkResult(filterEntry12.filterId(), 12);
	        checkResult(filterEntry12.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(filterEntry12.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG);
	        checkResult(filterEntry12.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry12.hasPermissionData());
	        checkResult(filterEntry12.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETAAckMsgAll(filterEntry12.ackMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(filterIter.hasNext());
	        com.refinitiv.ema.access.FilterEntry filterEntry13 = filterIter.next();
	        
	        checkResult(filterEntry13.filterId(), 13);
	        checkResult(filterEntry13.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(filterEntry13.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REQ_MSG);
	        checkResult(filterEntry13.action(), com.refinitiv.ema.access.FilterEntry.FilterAction.SET);
	        checkResult(filterEntry13.hasPermissionData());
	        checkResult(filterEntry13.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
	        EmaDecode_ETARequestMsgAll(filterEntry13.reqMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	        
	        checkResult(!filterIter.hasNext());
    	}
	}
	
	public static void EmaDecode_ETASeriesAll(com.refinitiv.ema.access.Series series, int containerType)
	{
		checkResult(series.hasTotalCountHint());
		
		int expectedTotalCountHint = 2;
		
		checkResult(series.totalCountHint(), expectedTotalCountHint);
		
		Iterator<SeriesEntry> seriesIter = series.iterator();
	    checkResult(seriesIter != null);
		
		switch( containerType )
		{
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(series.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(seriesEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(seriesEntry2.fieldList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(series.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(seriesEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(seriesEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(series.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(seriesEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(seriesEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.SERIES:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(series.summaryData().series(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(seriesEntry1.series(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(seriesEntry2.series(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(series.summaryData().vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(seriesEntry1.vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(seriesEntry2.vector(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MAP:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(series.summaryData().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
						
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(seriesEntry1.map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(seriesEntry2.map(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MSG:
		{
			// Check summary data
			checkResult(series.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(series.summaryData().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry1 = seriesIter.next();
			
			checkResult(seriesEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(seriesEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(seriesEntry1.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(seriesIter.hasNext());
			com.refinitiv.ema.access.SeriesEntry seriesEntry2 = seriesIter.next();
			
			checkResult(seriesEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(seriesEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(seriesEntry2.refreshMsg(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		default:
		break;
		}
	}
	
	public static void EmaDecode_ETAVectorAll(com.refinitiv.ema.access.Vector vector, int containerType)
	{
		checkResult(vector.hasTotalCountHint());
		
		int expectedTotalCountHint = 2;
		
		checkResult(vector.totalCountHint(), expectedTotalCountHint);
		
		Iterator<VectorEntry> vectorIter = vector.iterator();
	    checkResult(vectorIter != null);
		
		switch( containerType )
		{
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(vector.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(vectorEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(vectorEntry2.fieldList(), EncodingTypeFlags.MESSAGE_TYPES);
		}		
		break;
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(vector.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(vectorEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(vectorEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(vector.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(vectorEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(vectorEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.SERIES:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(vector.summaryData().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(vectorEntry1.series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(vectorEntry2.series(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(vector.summaryData().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			//
						
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(vectorEntry1.vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(vectorEntry2.vector(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MAP:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(vector.summaryData().map(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(vectorEntry1.map(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(vectorEntry2.map(), com.refinitiv.eta.codec.DataTypes.MSG);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MSG:
		{
			// Check summary data
			checkResult(vector.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(vector.summaryData().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry1 = vectorIter.next();
			
			checkResult(vectorEntry1.position(), 2);
			checkResult(vectorEntry1.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry1.hasPermissionData());
			checkResult(vectorEntry1.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(vectorEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(vectorEntry1.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(vectorIter.hasNext());
			com.refinitiv.ema.access.VectorEntry vectorEntry2 = vectorIter.next();
			
			checkResult(vectorEntry2.position(), 3);
			checkResult(vectorEntry2.action(), com.refinitiv.ema.access.VectorEntry.VectorAction.SET);
			checkResult(vectorEntry2.hasPermissionData());
			checkResult(vectorEntry2.permissionData().equals( ByteBuffer.wrap("PermissionData".getBytes()) ));
			checkResult(vectorEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(vectorEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(vectorEntry2.refreshMsg(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		default:
		break;
		}
	}
	
	public static void EmaDecode_ETAMapKeyUIntAll(com.refinitiv.ema.access.Map map, int containerType)
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
		case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(map.summaryData().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(mapEntry1.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST);
			EmaDecode_ETAFieldListAll(mapEntry2.fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(map.summaryData().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(mapEntry1.elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST);
			EmaDecode_ETAElementListAll(mapEntry2.elementList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(map.summaryData().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(mapEntry1.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST);
			EmaDecode_ETAFilterListAll(mapEntry2.filterList(), EncodingTypeFlags.MESSAGE_TYPES);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.SERIES:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(map.summaryData().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(mapEntry1.series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.SERIES);
			EmaDecode_ETASeriesAll(mapEntry2.series(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.VECTOR:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(map.summaryData().vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(mapEntry1.vector(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.VECTOR);
			EmaDecode_ETAVectorAll(mapEntry2.vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MAP:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(map.summaryData().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(mapEntry1.map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.MAP);
			EmaDecode_ETAMapKeyUIntAll(mapEntry2.map(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}
		break;
		case com.refinitiv.eta.codec.DataTypes.MSG:
		{
			//Check Summary data
			checkResult(map.summaryData().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(map.summaryData().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			//
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry1 = mapIter.next();
			checkResult(mapEntry1.action(), com.refinitiv.ema.access.MapEntry.MapAction.ADD);
			checkResult(mapEntry1.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry1.key().uintValue(), 1 ); 
			checkResult(mapEntry1.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(mapEntry1.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(mapEntry1.refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
			
			checkResult(mapIter.hasNext());
			com.refinitiv.ema.access.MapEntry mapEntry2 = mapIter.next();
			checkResult(mapEntry2.action(), com.refinitiv.ema.access.MapEntry.MapAction.UPDATE);
			checkResult(mapEntry2.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
			checkResult(mapEntry2.key().uintValue(), 2 ); 
			checkResult(mapEntry2.loadType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			checkResult(mapEntry2.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG);
			EmaDecode_ETARefreshMsgAll(mapEntry2.refreshMsg(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
		}	
		break;
		default:
		break;
		}
		
		checkResult(mapIter.hasNext());
	    com.refinitiv.ema.access.MapEntry mapEntry3 = mapIter.next();
	    checkResult(mapEntry3.action(), com.refinitiv.ema.access.MapEntry.MapAction.DELETE);
	    checkResult(mapEntry3.key().dataType(), com.refinitiv.ema.access.DataType.DataTypes.UINT );
	    checkResult(mapEntry3.key().uintValue(), 3 );
	    checkResult(mapEntry3.loadType(), com.refinitiv.ema.access.DataType.DataTypes.NO_DATA);
		checkResult(mapEntry3.load().dataType(), com.refinitiv.ema.access.DataType.DataTypes.NO_DATA);
	    
		checkResult(!mapIter.hasNext());
	}
	
	public static void EmaEncodeFieldListAll( FieldList fl )
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
	
	public static void EmaEncodeElementListAll( ElementList el)
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
		ByteBuffer value = ByteBuffer.wrap("ABCDEF".getBytes());
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
		value = ByteBuffer.wrap("ABCDEFGH".getBytes());
		el.add(EmaFactory.createElementEntry().buffer("Element - Buffer", value));

		//sixteenth entry
		el.add(EmaFactory.createElementEntry().utf8("Element - Utf8String" , "ABCDEFGH"));
	}
	public static void EmaEncodeMapAllWithFieldList( Map map)
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

		ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =ByteBuffer.wrap("ABCD".getBytes());
		
		flEnc.clear();
		EmaEncodeFieldListAll(flEnc);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, flEnc, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//third entry  //Add FieldList
		orderBuf =ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//fourth entry  //Update FieldList
		orderBuf =ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, flEnc, permission));
	}
	
	public static void EmaEncodeFilterListAllWithFieldListElementList( FilterList filterList)
	{
		//                  FieldList-Clear,
		//                  FieldList-Set,
		//                  ElementList-Update,

		filterList.totalCountHint(3);

		FieldList flEnc = EmaFactory.createFieldList();
		EmaEncodeFieldListAll(flEnc);

		ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Clear FieldList
		filterList.add(EmaFactory.createFilterEntry().fieldList(1, FilterEntry.FilterAction.CLEAR, flEnc, permission));

		//second entry  //Add FieldList
		filterList.add(EmaFactory.createFilterEntry().fieldList(2, FilterEntry.FilterAction.SET, flEnc));

		ElementList elEnc = EmaFactory.createElementList();
		EmaEncodeElementListAll(elEnc);

		//third entry  //Add FieldList
		filterList.add(EmaFactory.createFilterEntry().elementList(3, FilterEntry.FilterAction.UPDATE, elEnc, permission));
	}
	
	public static void EmaEncodeVectorAllWithFieldList( Vector vector)
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

		ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());

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
	
	public static void EmaEncodeMapAllWithFieldListWithUtf8Key( Map map)
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

		ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =ByteBuffer.wrap("ABC".getBytes());
		
		flEnc.clear();
		EmaEncodeFieldListAll(flEnc);
		
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.DELETE, flEnc, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//third entry  //Add FieldList
		orderBuf =ByteBuffer.wrap("DEFGH".getBytes());
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.ADD, flEnc, permission));

		//fourth entry  //Update FieldList
		orderBuf =ByteBuffer.wrap("KLMNOPQRS".getBytes());
		map.add(EmaFactory.createMapEntry().keyUtf8(orderBuf, MapEntry.MapAction.UPDATE, flEnc, permission));
	}

	public static void EmaEncodeMapAllWithElementList( Map map)
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

		ByteBuffer permission =  ByteBuffer.wrap("PERMISSION DATA".getBytes());;

		//first entry  //Delete Buffer
		ByteBuffer orderBuf =ByteBuffer.wrap("ABCD".getBytes());
		elEnc.clear();
		EmaEncodeElementListAll(elEnc);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, elEnc, permission));

		//second entry  //Add ElementList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, elEnc, permission));

		//third entry  //Add ElementList
		orderBuf =ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, elEnc, permission));

		//fourth entry  //Update ElementList
		orderBuf =ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, elEnc, permission));
	}
	
	public static void EmaEncodeSeriesAllWithElementList( Series series)
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
	
	public static void EmaEncodeMapAllWithMap( Map map)
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

		ByteBuffer permission = ByteBuffer.wrap("PERMISSION DATA".getBytes());
		
		//first entry  //Delete Buffer
		ByteBuffer orderBuf = ByteBuffer.wrap("ABCD".getBytes());
		
		mapEncS.clear();
		EmaEncodeMapAllWithFieldList(mapEncS);
		
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.DELETE, mapEncS, permission));

		//second entry  //Add FieldList
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, mapEncS, permission));

		//third entry  //Add FieldList
		orderBuf = ByteBuffer.wrap("EFGHI".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.ADD, mapEncS, permission));

		//fourth entry  //Update FieldList
		orderBuf = ByteBuffer.wrap("JKLMNOP".getBytes());
		map.add(EmaFactory.createMapEntry().keyBuffer(orderBuf, MapEntry.MapAction.UPDATE, mapEncS, permission));
	}
	
	public static void EmaDecodeFieldListAll(FieldList fl)
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
	public static void EmaDecodeElementListAll(ElementList el)
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
	public static void EmaDecodeMapAllWithFieldList(Map map)
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
	
	public static void EmaDecode_ETARequestMsgAll(ReqMsg reqMsg, int containerType)
	{
		System.out.println("Begin EMA ReqMsg Decoding");
		System.out.println(reqMsg);
		
		TestUtilities.checkResult(reqMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
		TestUtilities.checkResult(reqMsg.privateStream() == true, "ReqMsg.privateStream()");
		
		TestUtilities.checkResult(reqMsg.interestAfterRefresh() == true, "ReqMsg.interestAfterRefresh()");
		
		TestUtilities.checkResult(reqMsg.conflatedInUpdates() == true, "ReqMsg.conflatedInUpdates()");
		
		TestUtilities.checkResult(reqMsg.streamId() == 555, "ReqMsg.streamId()");
		
		TestUtilities.checkResult(reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(reqMsg.priorityClass() == 7, "ReqMsg.priorityClass()");
		
		TestUtilities.checkResult(reqMsg.priorityCount() == 5, "ReqMsg.priorityCount()");
		
		TestUtilities.checkResult(reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.qosRate() ==  com.refinitiv.ema.access.ReqMsg.Rate.TICK_BY_TICK, "ReqMsg.qosRate()");
		
		TestUtilities.checkResult(reqMsg.qosTimeliness() ==  com.refinitiv.ema.access.ReqMsg.Timeliness.REALTIME, "ReqMsg.qosTimeliness()");
		
		TestUtilities.checkResult(reqMsg.hasMsgKey(), "ReqMsg.hasMsgKey()");

		TestUtilities.checkResult(reqMsg.hasId(), "ReqMsg.hasId()");
		
		TestUtilities.checkResult(reqMsg.id() == 21, "ReqMsg.id()");

		TestUtilities.checkResult(reqMsg.hasFilter(), "ReqMsg.hasFilter()");
		
		TestUtilities.checkResult(reqMsg.filter() == 12 , "ReqMsg.hasFilter()");

		TestUtilities.checkResult(reqMsg.hasServiceId(), "ReqMsg.hasServiceId()");
		
		TestUtilities.checkResult(reqMsg.serviceId() == 5 , "ReqMsg.serviceId()");

		TestUtilities.checkResult(reqMsg.hasNameType(), "ReqMsg.hasNameType()");
		
		TestUtilities.checkResult(reqMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "ReqMsg.nameType()");

		TestUtilities.checkResult(reqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(reqMsg.name().compareTo("ABCDEF") == 0, "ReqMsg.name()");
		
		TestUtilities.checkResult(reqMsg.hasExtendedHeader(), "ReqMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( reqMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( reqMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( reqMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( reqMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( reqMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( reqMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( reqMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( reqMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( reqMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( reqMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( reqMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( reqMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "ReqMsg.attrib().dataType()");
				TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "ReqMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(reqMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( reqMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( reqMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		System.out.println("End EMA RequestMsg Decoding");
		System.out.println();
	}
	
	public static void EmaDecode_ETARefreshMsgAll(RefreshMsg refreshMsg, int containerType)
	{
		System.out.println("Begin EMA RefreshMsg Decoding");
		System.out.println(refreshMsg);
		
		TestUtilities.checkResult(refreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(refreshMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

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
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( refreshMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( refreshMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( refreshMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( refreshMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( refreshMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( refreshMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( refreshMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( refreshMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( refreshMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( refreshMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( refreshMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( refreshMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "RefreshMsg.attrib().dataType()");
				TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "RefreshMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(refreshMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( refreshMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( refreshMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("refresh complete") == 0, "RefreshMsg.state().statusText()");
		
		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}
	
	public static void EmaDecode_ETAUpdateMsgAll(UpdateMsg updateMsg, int containerType)
	{
		System.out.println("Begin EMA UpdateMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(updateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
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
		
		TestUtilities.checkResult(updateMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

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
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( updateMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( updateMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( updateMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( updateMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( updateMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( updateMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( updateMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( updateMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( updateMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( updateMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( updateMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( updateMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "UpdateMsg.attrib().dataType()");
				TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "UpdateMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(updateMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( updateMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( updateMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		System.out.println("Begin EMA UpdateMsg Decoding");
		System.out.println();
	}
	
	public static void EmaDecode_ETAStatusMsgAll(StatusMsg statusMsg, int containerType)
	{
		System.out.println("Begin EMA StatusMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(statusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
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

		TestUtilities.checkResult(statusMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
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
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( statusMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( statusMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				System.out.println(extendedBufferEl);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( statusMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( statusMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( statusMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( statusMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( statusMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( statusMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( statusMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( statusMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( statusMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( statusMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "StatusMsg.attrib().dataType()");
				TestUtilities.checkResult(statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "StatusMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(statusMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( statusMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( statusMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
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
	
	public static void EmaDecode_ETAGenericMsgAll(GenericMsg genericMsg, int containerType)
	{
		System.out.println("Begin EMA GenericMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(genericMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
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
		
		TestUtilities.checkResult(genericMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(genericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(genericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(genericMsg.hasPermissionData(), "GenericMsg.hasPermissionData()");
		TestUtilities.checkResult(genericMsg.permissionData().equals(ByteBuffer.wrap("GenericMsg.permissionData".getBytes())), "GenericMsg.permissionData()");
		
		TestUtilities.checkResult(genericMsg.hasExtendedHeader(), "GenericMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( genericMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( genericMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( genericMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( genericMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( genericMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( genericMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( genericMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( genericMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( genericMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( genericMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( genericMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( genericMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "GenericMsg.attrib().dataType()");
				TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "GenericMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(genericMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( genericMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( genericMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		System.out.println("End EMA GenericMsg Decoding");
		System.out.println();
	}
	
	public static void EmaDecode_ETAPostMsgAll(PostMsg postMsg, int containerType)
	{
		System.out.println("Begin EMA PostMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(postMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
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
		
		TestUtilities.checkResult(postMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");
		
		TestUtilities.checkResult(postMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(postMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(postMsg.hasPermissionData(), "PostMsg.hasPermissionData()");
		TestUtilities.checkResult(postMsg.permissionData().equals(ByteBuffer.wrap("PostMsg.permissionData".getBytes())), "PostMsg.permissionData()");
		
		TestUtilities.checkResult(postMsg.hasExtendedHeader(), "PostMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( postMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( postMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( postMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( postMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( postMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( postMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( postMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( postMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( postMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( postMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( postMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( postMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "PostMsg.attrib().dataType()");
				TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "PostMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(postMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( postMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( postMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		System.out.println("End EMA PostMsg Decoding");
		System.out.println();
	}
	
	public static void EmaDecode_ETAAckMsgAll(AckMsg ackMsg, int containerType)
	{
		System.out.println("Begin EMA AckMsg Decoding");
		System.out.println();
		
		TestUtilities.checkResult(ackMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(ackMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(ackMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(ackMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(ackMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

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
		
		TestUtilities.checkResult(ackMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(ackMsg.hasExtendedHeader(), "AckMsg.hasExtendedHeader()");
		
		switch (containerType)
		{
			case com.refinitiv.eta.codec.DataTypes.FIELD_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFl = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( ackMsg.attrib().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFieldListAll( ackMsg.payload().fieldList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ELEMENT_LIST, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.ElementList extendedBufferEl = JUnitTestConnect.createElementList();
				JUnitTestConnect.setRsslData(extendedBufferEl, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAElementListAll( extendedBufferEl, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( ackMsg.attrib().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAElementListAll( ackMsg.payload().elementList(), EncodingTypeFlags.PRIMITIVE_TYPES);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.FILTER_LIST:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FILTER_LIST, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAFilterListAll( ackMsg.attrib().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				EmaDecode_ETAFilterListAll( ackMsg.payload().filterList(), EncodingTypeFlags.MESSAGE_TYPES);
				
				break;
			}
			case com.refinitiv.eta.codec.DataTypes.SERIES:
			{	
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.SERIES, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETASeriesAll( ackMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETASeriesAll( ackMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.VECTOR:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.VECTOR, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAVectorAll( ackMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				EmaDecode_ETAVectorAll( ackMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MAP:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.MAP, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETAMapKeyUIntAll( ackMsg.attrib().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETAMapKeyUIntAll( ackMsg.payload().map(),  com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}	
			case com.refinitiv.eta.codec.DataTypes.MSG:
			{
				TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "AckMsg.attrib().dataType()");
				TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.REFRESH_MSG, "AckMsg.payload().dataType()");
				
				com.refinitiv.eta.codec.Buffer extendedBuffer = CodecFactory.createBuffer();
				extendedBuffer.data(ackMsg.extendedHeader());
				
				com.refinitiv.ema.access.FieldList extendedBufferFieldList = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(extendedBufferFieldList, extendedBuffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
				
				EmaDecode_ETAFieldListAll( extendedBufferFieldList, EncodingTypeFlags.PRIMITIVE_TYPES);
				EmaDecode_ETARefreshMsgAll( ackMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				EmaDecode_ETARefreshMsgAll( ackMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
				
				break;
			}
			default:
				break;
		}
		
		System.out.println("End EMA AckMsg Decoding");
		System.out.println();
	}

	public static void checkResult(boolean result, String description) 
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

	public static void checkResult(String description, boolean result)
	{
		checkResult(result, description);
	}
	
	public static void checkResult(boolean result)
	{
		if ( result )
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	public static void checkResult(long expected, long actual)
	{
		if (expected == actual)
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	public static void checkResult(double expected, double actual)
	{
		if (expected == actual)
			++_passNum;
		else
		{
			assertFalse(true);
			++_failNum;
		}
	}
	
	public static void checkResult(String expect, String actual)
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
