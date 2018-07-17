///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import static org.junit.Assert.assertEquals;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import org.junit.Test;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RealHints;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.ema.access.Data.DataCode;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;


public class ArrayTests
{

	@Test
	public void TestAll()
	{
		testArrayBlank_Decode( true );
		testArrayBlank_Decode( false );
		testArrayInt_Decode( true );
		testArrayInt_Decode( false );
		testArrayUInt_Decode( true );
		testArrayUInt_Decode( false );
		testArrayFloat_Decode( true );
		testArrayFloat_Decode( false );
		testArrayDouble_Decode( true );
		testArrayDouble_Decode( false );
		testArrayReal_Decode();
		testArrayRealOneBlankEntry_Decode();
		testArrayAscii_Decode();
		testArrayAsciiOneBlankEntry_Decode();
		testArrayDate_Decode();
		testArrayTime_Decode( true );
		testArrayTime_Decode( false );
		testArrayDateTime_Decode( true );
		testArrayDateTime_Decode( false );
		testArrayBuffer_Decode();
		testArrayQos_Decode();
		testArrayState_Decode();
		testArrayEnum_Decode( true );
		testArrayEnum_Decode( false );
		testArrayUtf8_Decode();
		
		testArrayInt_Encode( false );
		testArrayInt_Encode( true );
		testArrayBuffer_EncodeDecode(false);
		testArrayBuffer_EncodeDecode(true);

        testArray_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA();
	}
	
	void testArrayAscii_Decode()
	{
		TestUtilities.printTestHead("testArrayAscii_Decode","Decoding OmmArray of Ascii\n");

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.ASCII_STRING);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			Buffer bufText = CodecFactory.createBuffer();
			bufText.data("ABC");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
			
			bufText.clear();
			bufText.data( "DEFGH");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
				
			bufText.clear();
			bufText.data( "KLMNOPQRS");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);


			// Now do EMA decoding of OmmArray of Ascii
			OmmArray ar = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);
			
			TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Ascii - hasFixedWidth()" );
			TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Ascii - getFixedWidth()" );

			Iterator<OmmArrayEntry> arIter = ar.iterator();
			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - first next()" );
			 OmmArrayEntry ae1 = arIter.next();
			TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
			try {
				ae1.uintValue();
				TestUtilities.checkResult( false, "OmmArray with three Ascii - exception expected" );
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult( true, "OmmArray with three Ascii - exception expected: " +  excp.getMessage() );
			}
			TestUtilities.checkResult( ae1.ascii().ascii().equals( "ABC"), "OmmArrayEntry.ascii()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - second next()" );
			 OmmArrayEntry ae2 = arIter.next();
			TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
			TestUtilities.checkResult( ae2.ascii().ascii().equals("DEFGH"), "OmmArrayEntry.ascii()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - third next()" );
			 OmmArrayEntry ae3 = arIter.next();
			TestUtilities.checkResult( ae3.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
			TestUtilities.checkResult( ae3.ascii().ascii().equals("KLMNOPQRS"), "OmmArrayEntry.ascii()" );
	       
			TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Ascii - fourth next()" );
			
			arIter = ar.iterator();
			{
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Ascii - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Ascii - getFixedWidth()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - first next()" );
				 OmmArrayEntry ae = arIter.next();
				TestUtilities.checkResult( ae.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				try {
					ae.uintValue();
					TestUtilities.checkResult( false, "OmmArray with three Ascii - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Ascii - exception expected: " +  excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.ascii().ascii().equals("ABC"), "OmmArrayEntry.ascii()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - second next()" );
				ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				TestUtilities.checkResult( ae2.ascii().ascii().equals("DEFGH"), "OmmArrayEntry.ascii()" );
					
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii - third next()" );
				ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				TestUtilities.checkResult( ae3.ascii().ascii().equals("KLMNOPQRS"), "OmmArrayEntry.ascii()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Ascii - fourth next()" );
			}

			TestUtilities.checkResult( true, "OmmArray with three Ascii - exception not expected" );


		} catch (  OmmException excp  ) {
			TestUtilities.checkResult( false, "OmmArray with three Ascii - exception not expected" );
			System.out.println( excp);
		}
	}

	void testArrayAsciiOneBlankEntry_Decode()
	{
		TestUtilities.printTestHead("testArrayAsciiOneBlankEntry_Decode", "OmmArray Decode Ascii One Blank Entry\n" );
		
		try {
				Array array = CodecFactory.createArray();
				EncodeIterator iter = CodecFactory.createEncodeIterator();
				ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
	
				Buffer buf = CodecFactory.createBuffer();
				buf.data(ByteBuffer.allocate(1000));
	
				iter.clear();
				iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
	
				array.itemLength(0); //varying size only
				array.primitiveType(DataTypes.ASCII_STRING);
					
				assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
					
				Buffer bufText = CodecFactory.createBuffer();
				bufText.data("ABC");
				assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
				
				assertEquals(arrayEntry.encodeBlank(iter), CodecReturnCodes.SUCCESS);
					
				bufText.clear();
				bufText.data( "KLMNOPQRS");
				assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
	
				assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
				
				//Now do EMA decoding of OmmArray of Ascii On Blank
	           OmmArray ar = JUnitTestConnect.createOmmArray();
	           JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

	   			Iterator<OmmArrayEntry> arIter = ar.iterator();
	   		
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Ascii (one blank) - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Ascii (one blank) - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Ascii (one blank) - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Ascii (one blank) - exception expected: " +  excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.ascii().ascii().equals("ABC"), "OmmArrayEntry.ascii()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - second next()" );
				OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				TestUtilities.checkResult( ae2.code() == DataCode.BLANK, "OmmArrayEntry.code() == DataCode.BLANK" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
				TestUtilities.checkResult( ae3.ascii().ascii().equals("KLMNOPQRS"), "OmmArrayEntry.ascii()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Ascii (one blank) - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Ascii (one blank) - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Ascii (one blank) - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - first next()" );
					OmmArrayEntry ae = arIter.next();
					TestUtilities.checkResult( ae.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
					try {
							ae.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Ascii (one blank) - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Ascii (one blank) - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.ascii().ascii().equals("ABC"), "OmmArrayEntry.ascii()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
					TestUtilities.checkResult( ae2.code() == DataCode.BLANK, "OmmArrayEntry.code() == DataCode.BLANK" );
					
					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Ascii (one blank) - third next()" );
					 ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()==  DataType.DataTypes.ASCII, "OmmArrayEntry.loadType() ==  DataType.DataTypes.ASCII" );
					TestUtilities.checkResult( ae3.ascii().ascii().equals("KLMNOPQRS"), "OmmArrayEntry.ascii()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Ascii (one blank) - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Ascii (one blank) - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Ascii (one blank) - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayBlank_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayBlank_Decode", "Decoding Blank OmmArray with " + appendText);
		
		try {
			Array array = CodecFactory.createArray();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			array.itemLength(fixedSize ? 4 : 0); //varying size only
			array.primitiveType(DataTypes.UINT);
				
			//Now do EMA decoding of OmmArray
			OmmArray ar = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

	 		Iterator<OmmArrayEntry> arIter = ar.iterator();
	 		
			TestUtilities.checkResult( !ar.hasFixedWidth(), "Blank OmmArray - hasFixedWidth()" );
			TestUtilities.checkResult( ar.fixedWidth() == 0, "Blank OmmArray - getFixedWidth()" );

			TestUtilities.checkResult( !arIter.hasNext(), "Blank OmmArray - iter.hasNext() is false" );
			
			TestUtilities.checkResult( true, "Blank OmmArray - exception not expected" );

		} catch (  OmmException excp  ) {
			TestUtilities.checkResult( false, "Blank OmmArray - exception not expected" );
			System.out.println( excp);
		}
	}

	void testArrayDate_Decode()
	{
		TestUtilities.printTestHead("testArrayDate_Decode",  "Decoding OmmArray of Date\n" );
	

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.DATE);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			Date date = CodecFactory.createDate();
			date.year(1111);
			date.month(11);
			date.day( 1);
			assertEquals(arrayEntry.encode(iter,date), CodecReturnCodes.SUCCESS);
			
			date.year(2222);
			date.month(2);
			date.day( 2);
			assertEquals(arrayEntry.encode(iter,date), CodecReturnCodes.SUCCESS);

			date.year(3333);
			date.month(3);
			date.day( 3);
			assertEquals(arrayEntry.encode(iter,date), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
				
				//Now do EMA decoding of OmmArray of Dates
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Date - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Date - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Date - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Date - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.date().year() == 1111, "OmmArrayEntry.date().year()" );
				TestUtilities.checkResult( ae1.date().month() == 11, "OmmArrayEntry.date().month()" );
				TestUtilities.checkResult( ae1.date().day() == 1, "OmmArrayEntry.date().day()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
				TestUtilities.checkResult( ae2.date().year() == 2222, "OmmArrayEntry.date().year()" );
				TestUtilities.checkResult( ae2.date().month() == 2, "OmmArrayEntry.date().month()" );
				TestUtilities.checkResult( ae2.date().day() == 2, "OmmArrayEntry.date().day()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
				TestUtilities.checkResult( ae3.date().year() == 3333, "OmmArrayEntry.date().year()" );
				TestUtilities.checkResult( ae3.date().month() == 3, "OmmArrayEntry.date().month()" );
				TestUtilities.checkResult( ae3.date().day() == 3, "OmmArrayEntry.date().day()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Date - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Date - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Date - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Date - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Date - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.date().year() == 1111, "OmmArrayEntry.date().year()" );
					TestUtilities.checkResult( ae1.date().month() == 11, "OmmArrayEntry.date().month()" );
					TestUtilities.checkResult( ae1.date().day() == 1, "OmmArrayEntry.date().day()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
					TestUtilities.checkResult( ae2.date().year() == 2222, "OmmArrayEntry.date().year()" );
					TestUtilities.checkResult( ae2.date().month() == 2, "OmmArrayEntry.date().month()" );
					TestUtilities.checkResult( ae2.date().day() == 2, "OmmArrayEntry.date().day()" );
					
					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Date - third next()" );
					 ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DATE, "OmmArrayEntry.loadType() == DataType.DataTypes.DATE" );
					TestUtilities.checkResult( ae3.date().year() == 3333, "OmmArrayEntry.date().year()" );
					TestUtilities.checkResult( ae3.date().month() == 3, "OmmArrayEntry.date().month()" );
					TestUtilities.checkResult( ae3.date().day() == 3, "OmmArrayEntry.date().day()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Date - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Date - exception not expected" );
				
				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Date - exception not expected" );
					System.out.println( excp);
	       }
	}

	void arrayEncodeDouble( Buffer buf, boolean fixedSize, double[] values, int numValues )
	{
		Array array = CodecFactory.createArray();
		EncodeIterator iter = CodecFactory.createEncodeIterator();
		ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

		buf.data(ByteBuffer.allocate(1000));

		iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

		array.itemLength(fixedSize ? 8 : 0); 
		array.primitiveType(DataTypes.DOUBLE);
		com.thomsonreuters.upa.codec.Double value = CodecFactory.createDouble();

		assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
		

		for ( int i = 0; i < numValues; i++ )
		{
			value.value(values[i]);
			assertEquals(arrayEntry.encode(iter, value ), CodecReturnCodes.SUCCESS);
		}

		assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
	}

	void testArrayDouble_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayDouble_Decode",  "Decoding OmmArray of Double with "+ appendText);

		try {
				//encode 3 Doubles
				Buffer buf = CodecFactory.createBuffer();
				double values[] = { -11.1111, 22.2222, -33.3333 };
				arrayEncodeDouble( buf, fixedSize, values, 3);


				//Now do EMA decoding of OmmArray of Doubles
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Double - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Double - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Double - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Double - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.doubleValue() == -11.1111, "OmmArrayEntry.doubleValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
				TestUtilities.checkResult( ae2.doubleValue() == 22.2222, "OmmArrayEntry.doubleValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
				TestUtilities.checkResult( ae3.doubleValue() == -33.3333, "OmmArrayEntry.doubleValue()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Double - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Double - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Double - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - first next()" );
					 ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Double - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Double - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.doubleValue() == -11.1111, "OmmArrayEntry.doubleValue()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
					TestUtilities.checkResult( ae2.doubleValue() == 22.2222, "OmmArrayEntry.doubleValue()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Double - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DOUBLE, "OmmArrayEntry.loadType() == DataType.DataTypes.DOUBLE" );
					TestUtilities.checkResult( ae3.doubleValue() == -33.3333, "OmmArrayEntry.doubleValue()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Double - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Double - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Double - exception not expected" );
					System.out.println( excp);
	       }
	}

	void arrayEncodeFloat( Buffer buf, boolean fixedSize, float[] values, int numValues )
	{
		Array array = CodecFactory.createArray();
		EncodeIterator iter = CodecFactory.createEncodeIterator();
		ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

		buf.data(ByteBuffer.allocate(1000));

		iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

		array.itemLength(fixedSize ? 4 : 0); 
		array.primitiveType(DataTypes.FLOAT);
		com.thomsonreuters.upa.codec.Float value = CodecFactory.createFloat();

		assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
		
		for ( int i = 0; i < numValues; i++ )
		{
			value.value(values[i]);
			assertEquals(arrayEntry.encode(iter, value ), CodecReturnCodes.SUCCESS);
		}

		assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
	}

	void testArrayFloat_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayFloat_Decode",  "Decoding OmmArray of Float with " + appendText);
	

		try {
				//encode 3 Floats
				Buffer buf = CodecFactory.createBuffer();
				float values[] = { -11.11f, 22.22f, -33.33f };
				arrayEncodeFloat( buf, fixedSize, values, 3);


				//Now do EMA decoding of OmmArray of Floats
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Float - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three Float - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Float - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Float - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.floatValue() == -11.11f, "OmmArrayEntry.floatValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
				TestUtilities.checkResult( ae2.floatValue() == 22.22f, "OmmArrayEntry.floatValue()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
				TestUtilities.checkResult( ae3.floatValue() == -33.33f, "OmmArrayEntry.floatValue()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Float - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Float - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three Float - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
					try {
							ae1.intValue();
							TestUtilities.checkResult( false, "OmmArray with three Float - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Float - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.floatValue() == -11.11f, "OmmArrayEntry.floatValue()" );


					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
					TestUtilities.checkResult( ae2.floatValue() == 22.22f, "OmmArrayEntry.floatValue()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Float - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.FLOAT, "OmmArrayEntry.loadType() == DataType.DataTypes.FLOAT" );
					TestUtilities.checkResult( ae3.floatValue() == -33.33f, "OmmArrayEntry.floatValue()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Float - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Float - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Float - exception not expected" );
					System.out.println( excp);
	       }
	}

	void arrayEncodeInt(Buffer buf, boolean fixedSize, long values[], int numValues )
	{
		Array array = CodecFactory.createArray();
		EncodeIterator iter = CodecFactory.createEncodeIterator();
		ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

		buf.data(ByteBuffer.allocate(1000));

		iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

		array.itemLength(fixedSize ? 4 : 0); 
		array.primitiveType(DataTypes.INT);
		com.thomsonreuters.upa.codec.Int value = CodecFactory.createInt();

		assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
		
		for ( int i = 0; i < numValues; i++ )
		{
			value.value(values[i]);
			assertEquals(arrayEntry.encode(iter, value ), CodecReturnCodes.SUCCESS);
		}

		assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
	}

	void testArrayInt_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayInt_Decode",  "Decoding OmmArray of Int with "+ appendText);
	

		try {
				//encode 3 Ints
				Buffer buf = CodecFactory.createBuffer();
				long values[] = { -11, 22, -33 };
				arrayEncodeInt( buf, fixedSize, values, 3);


				//Now do EMA decoding of OmmArray of Ints
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

	            TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Int - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three Int - getFixedWidth()" );

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Int - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Int - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.intValue() == -11, "OmmArrayEntry.intValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
				TestUtilities.checkResult( ae2.intValue() == 22, "OmmArrayEntry.intValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
				TestUtilities.checkResult( ae3.intValue() == -33, "OmmArrayEntry.intValue()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Int - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Int - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three Int - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Int - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Int - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.intValue() == -11, "OmmArrayEntry.intValue()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
					TestUtilities.checkResult( ae2.intValue() == 22, "OmmArrayEntry.intValue()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Int - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.INT, "OmmArrayEntry.loadType() == DataType.DataTypes.INT" );
					TestUtilities.checkResult( ae3.intValue() == -33, "OmmArrayEntry.intValue()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Int - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Int - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Int - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayReal_Decode()
	{
		TestUtilities.printTestHead("testArrayReal_Decode",  "Decoding OmmArray of Real" );
	

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); 
			array.primitiveType(DataTypes.REAL);
			
			com.thomsonreuters.upa.codec.Real real = CodecFactory.createReal();

			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
			
			real.value(11, RealHints.EXPONENT_2);
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			real.value(22, RealHints.FRACTION_2);
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			real.value(-33, RealHints.FRACTION_2);
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);

				//Now do EMA decoding of OmmArray
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Real - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Real - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Real - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Real - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.real().mantissa() == 11, "OmmArrayEntry.real().mantissa()" );
				TestUtilities.checkResult( ae1.real().magnitudeType() == OmmReal.MagnitudeType.EXPONENT_NEG_2, "OmmArrayEntry.real().magnitudeType()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				TestUtilities.checkResult( ae2.real().mantissa() == 22, "OmmArrayEntry.real().mantissa()" );
				TestUtilities.checkResult( ae2.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				TestUtilities.checkResult( ae3.real().mantissa() == -33, "OmmArrayEntry.real().mantissa()" );
				TestUtilities.checkResult( ae3.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Real - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Real - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Real - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - first next()" );
					 ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Real - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Real - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.real().mantissa() == 11, "OmmArrayEntry.real().mantissa()" );
					TestUtilities.checkResult( ae1.real().magnitudeType() == OmmReal.MagnitudeType.EXPONENT_NEG_2, "OmmArrayEntry.real().magnitudeType()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					TestUtilities.checkResult( ae2.real().mantissa() == 22, "OmmArrayEntry.real().mantissa()" );
					TestUtilities.checkResult( ae2.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real - third next()" );
					 ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					TestUtilities.checkResult( ae3.real().mantissa() == -33, "OmmArrayEntry.real().mantissa()" );
					TestUtilities.checkResult( ae3.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Real - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Real - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Real - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayRealOneBlankEntry_Decode()
	{
		TestUtilities.printTestHead("testArrayRealOneBlankEntry_Decode",  "Decoding OmmArray of Real (one blank)" );
	

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); 
			array.primitiveType(DataTypes.REAL);
			
			com.thomsonreuters.upa.codec.Real real = CodecFactory.createReal();

			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
			
			real.blank();
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			real.value(22, RealHints.FRACTION_2);
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			real.value(-33, RealHints.FRACTION_2);
			assertEquals(arrayEntry.encode(iter, real), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);


				//Now do EMA decoding of OmmArray of Reals
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Real (one blank) - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Real (one blank) - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Real (one blank) - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Real (one blank) - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				TestUtilities.checkResult( ae1.code() == DataCode.BLANK, "OmmArrayEntry.code() == DataCode.BLANK" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				TestUtilities.checkResult( ae2.real().mantissa() == 22, "OmmArrayEntry.real().mantissa()" );
				TestUtilities.checkResult( ae2.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
				TestUtilities.checkResult( ae3.real().mantissa() == -33, "OmmArrayEntry.real().mantissa()" );
				TestUtilities.checkResult( ae3.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Real (one blank) - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Real (one blank) - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Real (one blank) - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - first next()" );
					 OmmArrayEntry ae = arIter.next();
					TestUtilities.checkResult( ae.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					try {
							ae.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Real (one blank) - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Real (one blank) - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					TestUtilities.checkResult( ae1.code() == DataCode.BLANK, "OmmArrayEntry.code() == DataCode.BLANK" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					TestUtilities.checkResult( ae2.real().mantissa() == 22, "OmmArrayEntry.real().mantissa()" );
					TestUtilities.checkResult( ae2.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
					
					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Real (one blank) - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.REAL, "OmmArrayEntry.loadType() == DataType.DataTypes.REAL" );
					TestUtilities.checkResult( ae3.real().mantissa() == -33, "OmmArrayEntry.real().mantissa()" );
					TestUtilities.checkResult( ae3.real().magnitudeType() == OmmReal.MagnitudeType.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Real (one blank) - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Real (one blank) - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Real (one blank) - exception not expected" );
					System.out.println( excp);
	       }
	}

	void arrayEncodeUInt( Buffer buf, boolean fixedSize, long[] values, int numValues )
	{
		Array array = CodecFactory.createArray();
		EncodeIterator iter = CodecFactory.createEncodeIterator();
		ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

		buf.data(ByteBuffer.allocate(1000));

		iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

		array.itemLength(fixedSize ? 4 : 0); 
		array.primitiveType(DataTypes.UINT);
		com.thomsonreuters.upa.codec.UInt value = CodecFactory.createUInt();

		assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
		

		for ( int i = 0; i < numValues; i++ )
		{
			value.value(values[i]);
			assertEquals(arrayEntry.encode(iter, value ), CodecReturnCodes.SUCCESS);
		}

		assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
	}

	void testArrayUInt_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayUInt_Decode",  "Decoding OmmArray of UInt with " + appendText);
	

		try {
				//encode 3 UInts
				Buffer buf = CodecFactory.createBuffer();
				long values[] = { 11, 22, 33 };
				arrayEncodeUInt( buf, fixedSize, values, 3);


				//Now do EMA decoding of OmmArray of UInts
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three UInt - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three UInt - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
				try {
						ae1.intValue();
						TestUtilities.checkResult( false, "OmmArray with three UInt - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three UInt - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.uintValue() == 11, "OmmArrayEntry.getUInt()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
				TestUtilities.checkResult( ae2.uintValue() == 22, "OmmArrayEntry.getUInt()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
				TestUtilities.checkResult( ae3.uintValue() == 33, "OmmArrayEntry.getUInt()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three UInt - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three UInt - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 4 : 0 ), "OmmArray with three UInt - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
					try {
							ae1.intValue();
							TestUtilities.checkResult( false, "OmmArray with three UInt - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three UInt - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.uintValue() == 11, "OmmArrayEntry.getUInt()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
					TestUtilities.checkResult( ae2.uintValue() == 22, "OmmArrayEntry.getUInt()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three UInt - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.UINT, "OmmArrayEntry.loadType() == DataType.DataTypes.UINT" );
					TestUtilities.checkResult( ae3.uintValue() == 33, "OmmArrayEntry.getUInt()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three UInt - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three UInt - exception not expected" );

				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three UInt - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayTime_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayTime_Decode",  "Decoding OmmArray of Time with "+ appendText);
	
		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(fixedSize ? 5 : 0); //varying size only
			array.primitiveType(DataTypes.TIME);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);

			Time time = CodecFactory.createTime();
			time.hour(02);
			time.minute(03);
			time.second( 04);
			time.millisecond(05);
			assertEquals(arrayEntry.encode(iter,time), CodecReturnCodes.SUCCESS);
				
			time.hour(04);
			time.minute(05);
			time.second( 06);
			time.millisecond(07);
			assertEquals(arrayEntry.encode(iter,time), CodecReturnCodes.SUCCESS);

			time.hour(14);
			time.minute(15);
			time.second( 16);
			time.millisecond(17);
			assertEquals(arrayEntry.encode(iter,time), CodecReturnCodes.SUCCESS);
			
			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
			

				//Now do EMA decoding of OmmArray of Times
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( ar.hasFixedWidth()  == fixedSize, "OmmArray with three Time - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 5 : 0 ), "OmmArray with three Time - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Time - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Time - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.time().hour() == 02, "OmmArrayEntry.time().hour()" );
				TestUtilities.checkResult( ae1.time().minute() == 03, "OmmArrayEntry.time().minute()" );
				TestUtilities.checkResult( ae1.time().second() == 04, "OmmArrayEntry.time().second()" );
				TestUtilities.checkResult( ae1.time().millisecond() == 05, "OmmArrayEntry.time().millisecond()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				TestUtilities.checkResult( ae2.time().hour() == 04, "OmmArrayEntry.time().hour()" );
				TestUtilities.checkResult( ae2.time().minute() == 05, "OmmArrayEntry.time().minute()" );
				TestUtilities.checkResult( ae2.time().second() == 06, "OmmArrayEntry.time().second()" );
				TestUtilities.checkResult( ae2.time().millisecond() == 07, "OmmArrayEntry.time().millisecond()" );
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				TestUtilities.checkResult( ae3.time().hour() == 14, "OmmArrayEntry.time().hour()" );
				TestUtilities.checkResult( ae3.time().minute() == 15, "OmmArrayEntry.time().minute()" );
				TestUtilities.checkResult( ae3.time().second() == 16, "OmmArrayEntry.time().second()" );
				TestUtilities.checkResult( ae3.time().millisecond() == 17, "OmmArrayEntry.time().millisecond()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Time - fourth next()" );
			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth()  == fixedSize, "OmmArray with three Time - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 5 : 0 ), "OmmArray with three Time - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Time - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Time - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.time().hour() == 02, "OmmArrayEntry.time().hour()" );
					TestUtilities.checkResult( ae1.time().minute() == 03, "OmmArrayEntry.time().minute()" );
					TestUtilities.checkResult( ae1.time().second() == 04, "OmmArrayEntry.time().second()" );
					TestUtilities.checkResult( ae1.time().millisecond() == 05, "OmmArrayEntry.time().millisecond()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
					TestUtilities.checkResult( ae2.time().hour() == 04, "OmmArrayEntry.time().hour()" );
					TestUtilities.checkResult( ae2.time().minute() == 05, "OmmArrayEntry.time().minute()" );
					TestUtilities.checkResult( ae2.time().second() == 06, "OmmArrayEntry.time().second()" );
					TestUtilities.checkResult( ae2.time().millisecond() == 07, "OmmArrayEntry.time().millisecond()" );


					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Time - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.TIME, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
					TestUtilities.checkResult( ae3.time().hour() == 14, "OmmArrayEntry.time().hour()" );
					TestUtilities.checkResult( ae3.time().minute() == 15, "OmmArrayEntry.time().minute()" );
					TestUtilities.checkResult( ae3.time().second() == 16, "OmmArrayEntry.time().second()" );
					TestUtilities.checkResult( ae3.time().millisecond() == 17, "OmmArrayEntry.time().millisecond()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Time - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Time - exception not expected" );
				
				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Time - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayDateTime_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayDateTime_Decode",   "Decoding OmmArray of DateTime with " + appendText);
	

		try {
				Array array = CodecFactory.createArray();
				EncodeIterator iter = CodecFactory.createEncodeIterator();
				ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
	
				Buffer buf = CodecFactory.createBuffer();
				buf.data(ByteBuffer.allocate(1000));
	
				iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
	
				array.itemLength(fixedSize ? 9 : 0); //varying size only
				array.primitiveType(DataTypes.DATETIME);
					
				assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
	
				DateTime dateTime = CodecFactory.createDateTime();
				dateTime.year(1111);
				dateTime.month(11);
				dateTime.day( 1);
				dateTime.hour(14);
				dateTime.minute(15);
				dateTime.second( 16);
				dateTime.millisecond(17);
				assertEquals(arrayEntry.encode(iter,dateTime), CodecReturnCodes.SUCCESS);
					
				dateTime.year(2222);
				dateTime.month(2);
				dateTime.day( 2);
				dateTime.hour(14);
				dateTime.minute(15);
				dateTime.second( 16);
				dateTime.millisecond(17);
				assertEquals(arrayEntry.encode(iter,dateTime), CodecReturnCodes.SUCCESS);
				
				dateTime.year(3333);
				dateTime.month(3);
				dateTime.day( 3);
				dateTime.hour(14);
				dateTime.minute(15);
				dateTime.second( 16);
				dateTime.millisecond(17);
				assertEquals(arrayEntry.encode(iter,dateTime), CodecReturnCodes.SUCCESS);
			
				assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
			

				//Now do EMA decoding of OmmArray of DateTimes
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three DateTime - hasFixedWidth()" );
				TestUtilities.checkResult(  ar.fixedWidth() == (fixedSize ? 9 : 0 ), "OmmArray with three DateTime - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three DateTime - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three DateTime - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.dateTime().year() == 1111, "OmmArrayEntry.dateTime().year()" );
				TestUtilities.checkResult( ae1.dateTime().month() == 11, "OmmArrayEntry.dateTime().month()" );
				TestUtilities.checkResult( ae1.dateTime().day() == 1, "OmmArrayEntry.dateTime().day()" );
				TestUtilities.checkResult( ae1.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
				TestUtilities.checkResult( ae1.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
				TestUtilities.checkResult( ae1.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
				TestUtilities.checkResult( ae1.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );


				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
				TestUtilities.checkResult( ae2.dateTime().year() == 2222, "OmmArrayEntry.dateTime().year()" );
				TestUtilities.checkResult( ae2.dateTime().month() == 2, "OmmArrayEntry.dateTime().month()" );
				TestUtilities.checkResult( ae2.dateTime().day() == 2, "OmmArrayEntry.dateTime().day()" );
				TestUtilities.checkResult( ae2.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
				TestUtilities.checkResult( ae2.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
				TestUtilities.checkResult( ae2.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
				TestUtilities.checkResult( ae2.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );


				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
				TestUtilities.checkResult( ae3.dateTime().year() == 3333, "OmmArrayEntry.dateTime().year()" );
				TestUtilities.checkResult( ae3.dateTime().month() == 3, "OmmArrayEntry.dateTime().month()" );
				TestUtilities.checkResult( ae3.dateTime().day() == 3, "OmmArrayEntry.dateTime().day()" );
				TestUtilities.checkResult( ae3.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
				TestUtilities.checkResult( ae3.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
				TestUtilities.checkResult( ae3.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
				TestUtilities.checkResult( ae3.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three DateTime - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three DateTime - hasFixedWidth()" );
					TestUtilities.checkResult(  ar.fixedWidth() == (fixedSize ? 9 : 0 ), "OmmArray with three DateTime - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three DateTime - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three DateTime - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.dateTime().year() == 1111, "OmmArrayEntry.dateTime().year()" );
					TestUtilities.checkResult( ae1.dateTime().month() == 11, "OmmArrayEntry.dateTime().month()" );
					TestUtilities.checkResult( ae1.dateTime().day() == 1, "OmmArrayEntry.dateTime().day()" );
					TestUtilities.checkResult( ae1.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
					TestUtilities.checkResult( ae1.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
					TestUtilities.checkResult( ae1.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
					TestUtilities.checkResult( ae1.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );


					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
					TestUtilities.checkResult( ae2.dateTime().year() == 2222, "OmmArrayEntry.dateTime().year()" );
					TestUtilities.checkResult( ae2.dateTime().month() == 2, "OmmArrayEntry.dateTime().month()" );
					TestUtilities.checkResult( ae2.dateTime().day() == 2, "OmmArrayEntry.dateTime().day()" );
					TestUtilities.checkResult( ae2.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
					TestUtilities.checkResult( ae2.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
					TestUtilities.checkResult( ae2.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
					TestUtilities.checkResult( ae2.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three DateTime - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.DATETIME, "OmmArrayEntry.loadType() == DataType.DataTypes.DATETIME" );
					TestUtilities.checkResult( ae3.dateTime().year() == 3333, "OmmArrayEntry.dateTime().year()" );
					TestUtilities.checkResult( ae3.dateTime().month() == 3, "OmmArrayEntry.dateTime().month()" );
					TestUtilities.checkResult( ae3.dateTime().day() == 3, "OmmArrayEntry.dateTime().day()" );
					TestUtilities.checkResult( ae3.dateTime().hour() == 14, "OmmArrayEntry.dateTime().hour()" );
					TestUtilities.checkResult( ae3.dateTime().minute() == 15, "OmmArrayEntry.dateTime().minute()" );
					TestUtilities.checkResult( ae3.dateTime().second() == 16, "OmmArrayEntry.dateTime().second()" );
					TestUtilities.checkResult( ae3.dateTime().millisecond() == 17, "OmmArrayEntry.dateTime().millisecond()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three DateTime - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three DateTime - exception not expected" );
				
				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three DateTime - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayBuffer_Decode()
	{
		TestUtilities.printTestHead("testArrayBuffer_Decode",  "Decoding OmmArray of Buffer with varying size\n");

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.BUFFER);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			Buffer bufText = CodecFactory.createBuffer();
			bufText.data("ABC");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
			
			bufText.clear();
			bufText.data( "DEFGH");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
				
			bufText.clear();
			bufText.data( "KLMNOPQRS");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);

				
			//Now do EMA decoding of OmmArray of Buffers
	       OmmArray ar = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

			System.out.println( ar);

			Iterator<OmmArrayEntry> arIter = ar.iterator();
			
	        TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Buffer - hasFixedWidth()" );
			TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Buffer - getFixedWidth()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - first next()" );
			 OmmArrayEntry ae1 = arIter.next();
			TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
			try {
				ae1.uintValue();
				TestUtilities.checkResult( false, "OmmArray with three Buffer - exception expected" );
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult( true, "OmmArray with three Buffer - exception expected: "  + excp.getMessage() );
			}

			TestUtilities.checkResult( Arrays.equals(ae1.buffer().buffer().array(), new String("ABC").getBytes()), "OmmArrayEntry.buffer()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - second next()" );
			 OmmArrayEntry ae2 = arIter.next();
			TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult(Arrays.equals(ae2.buffer().buffer().array(), new String("DEFGH").getBytes()), "OmmArrayEntry.buffer()" );
				
			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - third next()" );
			 OmmArrayEntry ae3 = arIter.next();
			TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
			TestUtilities.checkResult(Arrays.equals(ae3.buffer().buffer().array(), new String("KLMNOPQRS").getBytes()), "OmmArrayEntry.buffer()" );
	       
			TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Buffer - fourth next()" );

		
			arIter = ar.iterator();
			{
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Buffer - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Buffer - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - first next()" );
				ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				try {
					ae1.uintValue();
					TestUtilities.checkResult( false, "OmmArray with three Buffer - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Buffer - exception expected: "  + excp.getMessage() );
				}

				TestUtilities.checkResult(Arrays.equals(ae1.buffer().buffer().array(), new String("ABC").getBytes()), "OmmArrayEntry.buffer()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - second next()" );
				ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult(Arrays.equals(ae2.buffer().buffer().array(), new String("DEFGH").getBytes()), "OmmArrayEntry.buffer()" );
									
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Buffer - third next()" );
				ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				TestUtilities.checkResult(Arrays.equals(ae3.buffer().buffer().array(), new String("KLMNOPQRS").getBytes()), "OmmArrayEntry.buffer()" );

				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Buffer - fourth next()" );
			}

			TestUtilities.checkResult( true, "OmmArray with three Buffer - exception not expected" );
				
			
	    } catch (  OmmException excp  ) {
			TestUtilities.checkResult( false, "OmmArray with three Buffer - exception not expected" );
			System.out.println( excp);
	    }
	}

	void testArrayQos_Decode()
	{
		TestUtilities.printTestHead("testArrayQos_Decode",  "Decoding OmmArray of Qos\n" );
	

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.QOS);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			Qos qos = CodecFactory.createQos();
			qos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.REALTIME);
			qos.rate(com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK);
			qos.dynamic(true);
			qos.rateInfo(0);
			qos.timeInfo( 0);
			assertEquals(arrayEntry.encode(iter,qos), CodecReturnCodes.SUCCESS);

			qos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.REALTIME);
			qos.rate(com.thomsonreuters.upa.codec.QosRates.TIME_CONFLATED);
			qos.dynamic(true);
			qos.rateInfo(9);
			qos.timeInfo( 0);
			assertEquals(arrayEntry.encode(iter,qos), CodecReturnCodes.SUCCESS);
			
			qos.timeliness(com.thomsonreuters.upa.codec.QosTimeliness.DELAYED);
			qos.rate(com.thomsonreuters.upa.codec.QosRates.JIT_CONFLATED);
			qos.dynamic(true);
			qos.rateInfo(0);
			qos.timeInfo( 15);
			assertEquals(arrayEntry.encode(iter,qos), CodecReturnCodes.SUCCESS);
		
			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);

				//Now do EMA decoding of OmmArray of Qos
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Qos - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Qos - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Qos - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three Qos - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.qos().timeliness() == OmmQos.Timeliness.REALTIME, "OmmArrayEntry.qos().timeliness()" );
				TestUtilities.checkResult( ae1.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "OmmArrayEntry.qos().rate()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
				TestUtilities.checkResult( ae2.qos().timeliness() == OmmQos.Timeliness.REALTIME, "OmmArrayEntry.qos().timeliness()" );
				TestUtilities.checkResult( ae2.qos().rate() == 9, "OmmArrayEntry.qos().rate()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
				TestUtilities.checkResult( ae3.qos().timeliness() == 15, "OmmArrayEntry.qos().timeliness()" );
				TestUtilities.checkResult( ae3.qos().rate() == OmmQos.Rate.JUST_IN_TIME_CONFLATED, "OmmArrayEntry.qos().rate()" );
				TestUtilities.checkResult( ae3.qos().toString().equals("Timeliness: 15/JustInTimeConflated"), "ae3.qos().toString()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Qos - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Qos - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Qos - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three Qos - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three Qos - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.qos().timeliness() == OmmQos.Timeliness.REALTIME, "OmmArrayEntry.qos().timeliness()" );
					TestUtilities.checkResult( ae1.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "OmmArrayEntry.qos().rate()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
					TestUtilities.checkResult( ae2.qos().timeliness() == OmmQos.Timeliness.REALTIME, "OmmArrayEntry.qos().timeliness()" );
					TestUtilities.checkResult( ae2.qos().rate() == 9, "OmmArrayEntry.qos().rate()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Qos - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.QOS, "OmmArrayEntry.loadType() == DataType.DataTypes.QOS" );
					TestUtilities.checkResult( ae3.qos().timeliness() == 15, "OmmArrayEntry.qos().timeliness()" );
					TestUtilities.checkResult( ae3.qos().rate() == OmmQos.Rate.JUST_IN_TIME_CONFLATED, "OmmArrayEntry.qos().rate()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Qos - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three Qos - exception not expected" );
		
				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three Qos - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayState_Decode()
	{
		TestUtilities.printTestHead("testArrayState_Decode",  "Decoding OmmArray of State\n" );

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.STATE);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			State state = CodecFactory.createState();
			state.streamState( com.thomsonreuters.upa.codec.StreamStates.OPEN);
			state.dataState(com.thomsonreuters.upa.codec.DataStates.OK);
			state.code(com.thomsonreuters.upa.codec.StateCodes.NONE);
			state.text().data( "Succeeded");
			assertEquals(arrayEntry.encode(iter,state), CodecReturnCodes.SUCCESS);

			state.streamState( com.thomsonreuters.upa.codec.StreamStates.CLOSED_RECOVER);
			state.dataState(com.thomsonreuters.upa.codec.DataStates.SUSPECT);
			state.code(com.thomsonreuters.upa.codec.StateCodes.TIMEOUT);
			state.text().data("Suspect Data");
			assertEquals(arrayEntry.encode(iter,state), CodecReturnCodes.SUCCESS);

			state.streamState( com.thomsonreuters.upa.codec.StreamStates.CLOSED);
			state.dataState(com.thomsonreuters.upa.codec.DataStates.SUSPECT);
			state.code(com.thomsonreuters.upa.codec.StateCodes.USAGE_ERROR);
			state.text().data("Usage Error");
			assertEquals(arrayEntry.encode(iter,state), CodecReturnCodes.SUCCESS);
		
			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);


				//Now do EMA decoding of OmmArray of States
	           OmmArray ar = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> arIter = ar.iterator();
				
	            TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three State - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three State - getFixedWidth()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - first next()" );
				 OmmArrayEntry ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three State - exception expected" );
				}
				catch ( OmmException excp )
				{
					
					TestUtilities.checkResult( true,  "OmmArray with three State - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.state().streamState() == OmmState.StreamState.OPEN, "OmmArrayEntry.state().streamState()" );
				TestUtilities.checkResult( ae1.state().dataState() == OmmState.DataState.OK, "OmmArrayEntry.state().dataState()" );
				TestUtilities.checkResult( ae1.state().statusCode() == OmmState.StatusCode.NONE, "OmmArrayEntry.state().statusCode()" );
				TestUtilities.checkResult( ae1.state().statusText().equals( "Succeeded"), "OmmArrayEntry.state().statusText()" );
				TestUtilities.checkResult( ae1.state().toString().equals( "Open / Ok / None / 'Succeeded'"), "OmmArrayEntry.state().toString()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - second next()" );
				 OmmArrayEntry ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
				TestUtilities.checkResult( ae2.state().streamState() == OmmState.StreamState.CLOSED_RECOVER, "OmmArrayEntry.state().streamState()" );
				TestUtilities.checkResult( ae2.state().dataState() == OmmState.DataState.SUSPECT, "OmmArrayEntry.state().dataState()" );
				TestUtilities.checkResult( ae2.state().statusCode() == OmmState.StatusCode.TIMEOUT, "OmmArrayEntry.state().statusCode()" );
				TestUtilities.checkResult( ae2.state().statusText().equals( "Suspect Data"), "OmmArrayEntry.state().statusText()" );
				TestUtilities.checkResult( ae2.state().toString().equals( "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'"), "OmmArrayEntry.state().toString()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - third next()" );
				 OmmArrayEntry ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
				TestUtilities.checkResult( ae3.state().streamState() == OmmState.StreamState.CLOSED, "OmmArrayEntry.state().streamState()" );
				TestUtilities.checkResult( ae3.state().dataState() == OmmState.DataState.SUSPECT, "OmmArrayEntry.state().dataState()" );
				TestUtilities.checkResult( ae3.state().statusCode() == OmmState.StatusCode.USAGE_ERROR, "OmmArrayEntry.state().statusCode()" );
				TestUtilities.checkResult( ae3.state().statusText().equals( "Usage Error"), "OmmArrayEntry.state().statusText()" );
				TestUtilities.checkResult( ae3.state().toString().equals( "Closed / Suspect / Usage error / 'Usage Error'"), "OmmArrayEntry.state().toString()" );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three State - fourth next()" );

			
				arIter = ar.iterator();
				{
					TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three State - hasFixedWidth()" );
					TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three State - getFixedWidth()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - first next()" );
					ae1 = arIter.next();
					TestUtilities.checkResult( ae1.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
					try {
							ae1.uintValue();
							TestUtilities.checkResult( false, "OmmArray with three State - exception expected" );
					}
					catch ( OmmException excp )
					{
						
						TestUtilities.checkResult( true,  "OmmArray with three State - exception expected: "  + excp.getMessage() );
					}
					TestUtilities.checkResult( ae1.state().streamState() == OmmState.StreamState.OPEN, "OmmArrayEntry.state().streamState()" );
					TestUtilities.checkResult( ae1.state().dataState() == OmmState.DataState.OK, "OmmArrayEntry.state().dataState()" );
					TestUtilities.checkResult( ae1.state().statusCode() == OmmState.StatusCode.NONE, "OmmArrayEntry.state().statusCode()" );
					TestUtilities.checkResult( ae1.state().statusText().equals( "Succeeded"), "OmmArrayEntry.state().statusText()" );
					TestUtilities.checkResult( ae1.state().toString().equals( "Open / Ok / None / 'Succeeded'"), "OmmArrayEntry.state().toString()" );

					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - second next()" );
					ae2 = arIter.next();
					TestUtilities.checkResult( ae2.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
					TestUtilities.checkResult( ae2.state().streamState() == OmmState.StreamState.CLOSED_RECOVER, "OmmArrayEntry.state().streamState()" );
					TestUtilities.checkResult( ae2.state().dataState() == OmmState.DataState.SUSPECT, "OmmArrayEntry.state().dataState()" );
					TestUtilities.checkResult( ae2.state().statusCode() == OmmState.StatusCode.TIMEOUT, "OmmArrayEntry.state().statusCode()" );
					TestUtilities.checkResult( ae2.state().statusText().equals( "Suspect Data"), "OmmArrayEntry.state().statusText()" );
					TestUtilities.checkResult( ae2.state().toString().equals( "Closed, Recoverable / Suspect / Timeout / 'Suspect Data'"), "OmmArrayEntry.state().toString()" );
					
					TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three State - third next()" );
					ae3 = arIter.next();
					TestUtilities.checkResult( ae3.loadType()==DataType.DataTypes.STATE, "OmmArrayEntry.loadType() ==DataType.DataTypes.STATE" );
					TestUtilities.checkResult( ae3.state().streamState() == OmmState.StreamState.CLOSED, "OmmArrayEntry.state().streamState()" );
					TestUtilities.checkResult( ae3.state().dataState() == OmmState.DataState.SUSPECT, "OmmArrayEntry.state().dataState()" );
					TestUtilities.checkResult( ae3.state().statusCode() == OmmState.StatusCode.USAGE_ERROR, "OmmArrayEntry.state().statusCode()" );
					TestUtilities.checkResult( ae3.state().statusText().equals( "Usage Error"), "OmmArrayEntry.state().statusText()" );
					TestUtilities.checkResult( ae3.state().toString().equals( "Closed / Suspect / Usage error / 'Usage Error'"), "OmmArrayEntry.state().toString()" );
	       
					TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three State - fourth next()" );
				}

				TestUtilities.checkResult( true, "OmmArray with three State - exception not expected" );
				
				
	        } catch (  OmmException excp  ) {
					TestUtilities.checkResult( false, "OmmArray with three State - exception not expected" );
					System.out.println( excp);
	       }
	}

	void testArrayEnum_Decode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayEnum_Decode",  "Decoding OmmArray of Enum with " + appendText);
	

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(fixedSize ? 2 : 0); //varying size only
			array.primitiveType(DataTypes.ENUM);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
		   com.thomsonreuters.upa.codec.Enum testEnum = CodecFactory.createEnum();
			testEnum.value(29);
			assertEquals(arrayEntry.encode(iter,testEnum), CodecReturnCodes.SUCCESS);

			testEnum.value(5300);
			assertEquals(arrayEntry.encode(iter,testEnum), CodecReturnCodes.SUCCESS);
			
			testEnum.value(8100);
			assertEquals(arrayEntry.encode(iter,testEnum), CodecReturnCodes.SUCCESS);
		
			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);


			//Now do EMA decoding of OmmArray of Enums
	       OmmArray ar = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

			Iterator<OmmArrayEntry> arIter = ar.iterator();
			
			TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Enum - hasFixedWidth()" );
			TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 2 : 0 ), "OmmArray with three Enum - getFixedWidth()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - first next()" );
			 OmmArrayEntry ae1 = arIter.next();
			TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
			try {
				ae1.uintValue();
				TestUtilities.checkResult( false, "OmmArray with three Enum - exception expected" );
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult( true, "OmmArray with three Enum - exception expected: "  + excp.getMessage() );
			}

			TestUtilities.checkResult( ae1.enumValue() == 29, "OmmArrayEntry.enumValue()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - second next()" );
			 OmmArrayEntry ae2 = arIter.next();
			TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
			TestUtilities.checkResult( ae2.enumValue() == 5300, "OmmArrayEntry.enumValue()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - third next()" );
			 OmmArrayEntry ae3 = arIter.next();
			TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
			TestUtilities.checkResult( ae3.enumValue() == 8100, "OmmArrayEntry.enumValue()" );

	       	TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Enum - fourth next()" );

		
			arIter = ar.iterator();
			{
				TestUtilities.checkResult( ar.hasFixedWidth() == fixedSize, "OmmArray with three Enum - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == (fixedSize ? 2 : 0 ), "OmmArray with three Enum - getFixedWidth()" );

				arIter = ar.iterator();
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - first next()" );
				ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				try {
					ae1.uintValue();
					TestUtilities.checkResult( false, "OmmArray with three Enum - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Enum - exception expected: "  + excp.getMessage() );
				}
				TestUtilities.checkResult( ae1.enumValue() == 29, "OmmArrayEntry.enumValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - second next()" );
				ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				TestUtilities.checkResult( ae2.enumValue() == 5300, "OmmArrayEntry.enumValue()" );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Enum - third next()" );
				ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.ENUM, "OmmArrayEntry.loadType() == DataType.DataTypes.ENUM" );
				TestUtilities.checkResult( ae3.enumValue() == 8100, "OmmArrayEntry.enumValue()" );

	       		TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Enum - fourth next()" );
			}

			TestUtilities.checkResult( true, "OmmArray with three Enum - exception not expected" );

			
	    } catch (  OmmException excp  ) {
			TestUtilities.checkResult( false, "OmmArray with three Enum - exception not expected" );
			System.out.println( excp);
	    }
	}

	void testArrayUtf8_Decode()
	{
		TestUtilities.printTestHead("testArrayUtf8_Decode",  "Decoding OmmArray of Utf8" );

		try {
			Array array = CodecFactory.createArray();
			EncodeIterator iter = CodecFactory.createEncodeIterator();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			Buffer buf = CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1000));

			iter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());

			array.itemLength(0); //varying size only
			array.primitiveType(DataTypes.UTF8_STRING);
				
			assertEquals(array.encodeInit(iter), CodecReturnCodes.SUCCESS);
				
			Buffer bufText = CodecFactory.createBuffer();
			bufText.data("ABC");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
			
			bufText.clear();
			bufText.data( "DEFGH");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);
				
			bufText.clear();
			bufText.data( "KLMNOPQRS");
			assertEquals(arrayEntry.encode(iter,bufText), CodecReturnCodes.SUCCESS);

			assertEquals(array.encodeComplete(iter, true), CodecReturnCodes.SUCCESS);
			
			//Now do EMA decoding of OmmArray of Utf8
	       OmmArray ar = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(ar, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

			Iterator<OmmArrayEntry> arIter = ar.iterator();
			
			TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Utf8 - hasFixedWidth()" );
			TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Utf8 - getFixedWidth()" );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - first next()" );
			 OmmArrayEntry ae1 = arIter.next();
			TestUtilities.checkResult( ae1.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
			try {
				ae1.uintValue();
				TestUtilities.checkResult( false, "OmmArray with three Utf8 - exception expected" );
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult( true, "OmmArray with three Utf8 - exception expected: "  + excp.getMessage() );
			}

			TestUtilities.checkResult( Arrays.equals(ae1.utf8().buffer().array(),  new String("ABC").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - second next()" );
			 OmmArrayEntry ae2 = arIter.next();
			TestUtilities.checkResult( ae2.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
			TestUtilities.checkResult( Arrays.equals(ae2.utf8().buffer().array() ,  new String("DEFGH").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );

			TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - third next()" );
			 OmmArrayEntry ae3 = arIter.next();
			TestUtilities.checkResult( ae3.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
			TestUtilities.checkResult( Arrays.equals(ae3.utf8().buffer().array() ,  new String("KLMNOPQRS").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );

			TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Utf8 - fourth next()" );

		
			arIter = ar.iterator();
			{
				TestUtilities.checkResult( !ar.hasFixedWidth(), "OmmArray with three Utf8 - hasFixedWidth()" );
				TestUtilities.checkResult( ar.fixedWidth() == 0, "OmmArray with three Utf8 - getFixedWidth()" );

				arIter = ar.iterator();
				
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - first next()" );
				ae1 = arIter.next();
				TestUtilities.checkResult( ae1.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
				try {
					ae1.uintValue();
					TestUtilities.checkResult( false, "OmmArray with three Utf8 - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Utf8 - exception expected: "  + excp.getMessage() );
				}

				TestUtilities.checkResult( Arrays.equals(ae1.utf8().buffer().array() ,  new String("ABC").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );
				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - second next()" );
				ae2 = arIter.next();
				TestUtilities.checkResult( ae2.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
				TestUtilities.checkResult( Arrays.equals(ae2.utf8().buffer().array() ,  new String("DEFGH").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );

				TestUtilities.checkResult( arIter.hasNext(), "OmmArray with three Utf8 - third next()" );
				ae3 = arIter.next();
				TestUtilities.checkResult( ae3.loadType()==DataType.DataTypes.UTF8, "OmmArrayEntry.loadType() ==DataType.DataTypes.UTF8" );
				TestUtilities.checkResult( Arrays.equals(ae3.utf8().buffer().array() ,  new String("KLMNOPQRS").getBytes()), "OmmArrayEntry.utf8().buffer().array() " );
	       
				TestUtilities.checkResult( !arIter.hasNext(), "OmmArray with three Utf8 - fourth next()" );
			}

			TestUtilities.checkResult( true, "OmmArray with three Utf8 - exception not expected" );

			
	    } catch (  OmmException excp  ) {
			TestUtilities.checkResult( false, "OmmArray with three Utf8 - exception not expected" );
			System.out.println( excp);
	    }
	}

	void testArrayBlank_Encode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayBlank_Encode",  "Encoding Blank OmmArray with "+ appendText); 
		
	

		
	}

	public void testArrayInt_Encode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayInt_Encode",  "Encoding Int OmmArray with " + appendText); 

		OmmArray encArray = EmaFactory.createOmmArray();
		try {
			if ( fixedSize )
				encArray.fixedWidth( 4 );

			
			encArray.add(EmaFactory.createOmmArrayEntry().intValue( -11 ));
			encArray.add(EmaFactory.createOmmArrayEntry().intValue( 22 ));
			encArray.add(EmaFactory.createOmmArrayEntry().intValue( -33 ));

			TestUtilities.checkResult( true, "Encode OmmArray Int - exception not expected" );
		}
		catch (  OmmException excp )
		{
			TestUtilities.checkResult( false, "Encode OmmArray Int - exception not expected" );
			System.out.println( excp);
		}

		Buffer buf = CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1000));
		JUnitTestConnect.setRsslData(buf, encArray);
		
		// Now do UPA decoding of OmmArray of Ints
		Array array = CodecFactory.createArray();
		ArrayEntry arEntry = CodecFactory.createArrayEntry();
		Int intValue = CodecFactory.createInt();
			
		DecodeIterator decodeIter = CodecFactory.createDecodeIterator() ;
		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
		array.decode(decodeIter);
		
		TestUtilities.checkResult( array.primitiveType() == DataTypes.INT, "decoded array.primitiveType == _DT_INT" );
		TestUtilities.checkResult( array.itemLength() == (fixedSize ? 4 : 0), "decoded array.itemLength == 4" );

		arEntry.decode(decodeIter);
		intValue.decode(decodeIter);
		TestUtilities.checkResult( intValue.toLong() == -11, "decoded Int == -11" );

		arEntry.decode(decodeIter);
		intValue.decode(decodeIter);
		TestUtilities.checkResult( intValue.toLong() == 22, "decoded Int == 22" );
		
		arEntry.decode(decodeIter);
		intValue.decode(decodeIter);
		TestUtilities.checkResult( intValue.toLong()  == -33, "decoded Int == -33" );
	}

	void testArrayBuffer_EncodeDecode( boolean fixedSize )
	{
		String appendText = fixedSize ?  "fixed size\n"  : "varying size\n";
		TestUtilities.printTestHead("testArrayBuffer_EncodeDecode",  "Encode and Decode OmmArray Buffer  with " + appendText);

		OmmArray encArray = EmaFactory.createOmmArray();
		try {
			if ( fixedSize )
				encArray.fixedWidth( 8 );

			OmmArrayEntry ae = EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("ABC".getBytes()));
			TestUtilities.checkResult("OmmArrayEntry.toString() == toString() not supported", ae.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
			
			encArray.add(ae);
			TestUtilities.checkResult("OmmArray.toString() == toString() not supported", encArray.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
			
			encArray.add(EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("DEFGH".getBytes())));
			TestUtilities.checkResult("OmmArray.toString() == toString() not supported", encArray.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
			
			encArray.add(EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("KLMNOPQRS".getBytes())));
			TestUtilities.checkResult("OmmArray.toString() == toString() not supported", encArray.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

			TestUtilities.checkResult( true, "Encode OmmArray Int - exception not expected" );

			OmmArray decArray = JUnitTestConnect.createOmmArray();
			JUnitTestConnect.setRsslData(decArray, encArray, Codec.majorVersion(), Codec.minorVersion(), null, null);
			// check that we can still get the toString on encoded/decoded container.
			TestUtilities.checkResult("OmmArray.toString() != toString() not supported", !(decArray.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));			
			
	        TestUtilities.checkResult( decArray.hasFixedWidth() == fixedSize, "OmmArray with three Buffer - hasFixedWidth()" );
			TestUtilities.checkResult( decArray.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Buffer - getFixedWidth()" );

			Iterator<OmmArrayEntry> iter = decArray.iterator();
			TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - first next()" );
			 OmmArrayEntry ae1 = iter.next();
			TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
			try {
				ae1.uintValue();
				TestUtilities.checkResult( false, "OmmArray with three Buffer - exception expected" );
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult( true, "OmmArray with three Buffer - exception expected: "  + excp.getMessage() );
			}
			{
				TestUtilities.checkResult( Arrays.equals(new String("ABC").getBytes(),  ae1.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
			}

			TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - second next()" );
			 OmmArrayEntry ae2 = iter.next();
			TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
			{
				TestUtilities.checkResult( Arrays.equals(new String("DEFGH").getBytes(),  ae2.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
			}

			TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - third next()" );
			 OmmArrayEntry ae3 = iter.next();
			{
				TestUtilities.checkResult( Arrays.equals(new String("KLMNOPQRS").getBytes(),  ae3.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
			}
	       
			TestUtilities.checkResult( !iter.hasNext(), "OmmArray with three Buffer - fourth next()" );

		
			iter = decArray.iterator();
			{
				TestUtilities.checkResult( decArray.hasFixedWidth() == fixedSize, "OmmArray with three Buffer - hasFixedWidth()" );
				TestUtilities.checkResult( decArray.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Buffer - getFixedWidth()" );

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - first next()" );
				ae1 = iter.next();

				// check that we can still get the toString on encoded/decoded entry.
				TestUtilities.checkResult("OmmArrayEntry.toString() != toString() not supported", !(ae1.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));
				
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				try {
					ae1.uintValue();
					TestUtilities.checkResult( false, "OmmArray with three Buffer - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Buffer - exception expected: "  + excp.getMessage() );
				}
				{
					TestUtilities.checkResult( Arrays.equals(new String("ABC").getBytes(),  ae1.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - second next()" );
				ae2 = iter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				{
					TestUtilities.checkResult( Arrays.equals(new String("DEFGH").getBytes(),  ae2.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer - third next()" );
				ae3 = iter.next();
				{
					TestUtilities.checkResult( Arrays.equals(new String("KLMNOPQRS").getBytes(),  ae3.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}
	        
				TestUtilities.checkResult( !iter.hasNext(), "OmmArray with three Buffer - fourth next()" );
			}

			TestUtilities.checkResult( true, "Encode OmmArray Buffer - exception not expected" );
		}
		catch (  OmmException excp )
		{
			System.out.println( excp);

			if ( !fixedSize )
				TestUtilities.checkResult( false, "Encode OmmArray Buffer with blanks - exception not expected" );
			else
				TestUtilities.checkResult( true, "Encode OmmArray Buffer with blanks - exception expected" );
		}


		encArray.clear();

		try {
			//Encoding (including blanks)
			if ( fixedSize )
				encArray.fixedWidth( 8 );

				encArray.add(EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("ABC".getBytes())));
				encArray.add(EmaFactory.createOmmArrayEntry().codeBuffer());
				encArray.add(EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("DEFGH".getBytes())));
				encArray.add(EmaFactory.createOmmArrayEntry().codeBuffer());
				encArray.add(EmaFactory.createOmmArrayEntry().buffer(ByteBuffer.wrap("KLMNOPQRS".getBytes())));
	
	
				OmmArray decArray = JUnitTestConnect.createOmmArray();
				JUnitTestConnect.setRsslData(decArray, encArray, Codec.majorVersion(), Codec.minorVersion(), null, null);

				Iterator<OmmArrayEntry> iter = decArray.iterator();
				
	            TestUtilities.checkResult( decArray.hasFixedWidth() == fixedSize, "OmmArray with three Buffer (with 2 blanks) - hasFixedWidth()" );
				TestUtilities.checkResult( decArray.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Buffer (with 2 blanks) - getFixedWidth()" );

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - first next()" );
				OmmArrayEntry ae1 = iter.next();
				TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Buffer (with 2 blanks) - exception expected" );
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult( true, "OmmArray with three Buffer (with 2 blanks) - exception expected: "  + excp.getMessage() );
				}
				{
					TestUtilities.checkResult( Arrays.equals(new String("ABC").getBytes(),  ae1.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer  (with 2 blanks)- second next()" );
				 OmmArrayEntry ae1b = iter.next();
				TestUtilities.checkResult( ae1b.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - third next()" );
				 OmmArrayEntry ae2 = iter.next();
				TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				{
					TestUtilities.checkResult( Arrays.equals(new String("DEFGH").getBytes(),  ae2.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - fourth next()" );
				 OmmArrayEntry ae2b = iter.next();
				TestUtilities.checkResult( ae2b.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );

				TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - fifth next()" );
				 OmmArrayEntry ae3 = iter.next();
				TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
				{
					TestUtilities.checkResult( Arrays.equals(new String("KLMNOPQRS").getBytes(),  ae3.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
				}
	        
				TestUtilities.checkResult( !iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - sixth next()" );

			
				iter = decArray.iterator();
				{
					TestUtilities.checkResult( decArray.hasFixedWidth() == fixedSize, "OmmArray with three Buffer (with 2 blanks) - hasFixedWidth()" );
					TestUtilities.checkResult( decArray.fixedWidth() == (fixedSize ? 8 : 0 ), "OmmArray with three Buffer (with 2 blanks) - getFixedWidth()" );

					TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - first next()" );
					ae1 = iter.next();
					TestUtilities.checkResult( ae1.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
					try {
						ae1.uintValue();
						TestUtilities.checkResult( false, "OmmArray with three Buffer (with 2 blanks) - exception expected" );
					}
					catch ( OmmException excp )
					{
						TestUtilities.checkResult( true, "OmmArray with three Buffer (with 2 blanks) - exception expected: "  + excp.getMessage() );
					}
					{
						TestUtilities.checkResult( Arrays.equals(new String("ABC").getBytes(),  ae1.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
					}

					TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - second next()" );
					ae1b = iter.next();
					TestUtilities.checkResult( ae1b.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );

					TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - third next()" );
					ae2 = iter.next();
					TestUtilities.checkResult( ae2.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
					{
						TestUtilities.checkResult( Arrays.equals(new String("DEFGH").getBytes(),  ae2.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
					}

					TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - fourth next()" );
					ae2b = iter.next();
					TestUtilities.checkResult( ae2b.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );

					TestUtilities.checkResult( iter.hasNext(), "OmmArray with three Buffer (with 2 blanks) - fifth next()" );
					ae3 = iter.next();
					TestUtilities.checkResult( ae3.loadType()== DataType.DataTypes.BUFFER, "OmmArrayEntry.loadType() == DataType.DataTypes.BUFFER" );
					{
						TestUtilities.checkResult( Arrays.equals(new String("KLMNOPQRS").getBytes(),  ae3.buffer().buffer().array()), "OmmArrayEntry.buffer()" );
					}
	        
					TestUtilities.checkResult( !iter.hasNext(), "OmmArray with three Buffer  (with 2 blanks) - sixth next()" );
				}

			if ( !fixedSize )
				TestUtilities.checkResult( true, "Encode OmmArray Buffer with blanks - exception not expected" );
			else
				TestUtilities.checkResult( false, "Encode OmmArray Buffer with blanks - exception expected" );
		}
		catch (  OmmException excp )
		{
			System.out.println( excp);

			if ( !fixedSize )
				TestUtilities.checkResult( false, "Encode OmmArray Buffer with blanks - exception not expected" );
			else
				TestUtilities.checkResult( true, "Encode OmmArray Buffer with blanks - exception expected" );
		}
	}
	
   void testArray_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA()
   {
       TestUtilities.printTestHead("testArray_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA", "Encode Array with UPA, Decode Array with EMA, Encode Array with EMA, Decode Array with UPA");
       
       // Create a UPA Buffer to encode into
       com.thomsonreuters.upa.codec.Buffer buf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
       buf.data(ByteBuffer.allocate(1024));
       
       // Encode Array with UPA.
       int retVal;
       if ((retVal = TestUtilities.upa_EncodeArrayAll(buf)) < CodecReturnCodes.SUCCESS)
       {
           System.out.println("Error encoding array.");
           System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                   + ") encountered with TestUtilities.upa_EncodeArrayAll.  " + "Error Text: "
                   + CodecReturnCodes.info(retVal));
           return;
       }

       // Decode Array with EMA.
       OmmArray array = JUnitTestConnect.createOmmArray();
       JUnitTestConnect.setRsslData(array, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

       TestUtilities.EmaDecode_UPAArrayAll(array);
       
       // Copy decoded entries into a different Array with EMA
       OmmArray arrayCopy = EmaFactory.createOmmArray();
       if (array.hasFixedWidth())
       {
           arrayCopy.fixedWidth(array.fixedWidth());
       }
       Iterator<OmmArrayEntry> iterator = array.iterator();
       while (iterator.hasNext())
       {
           OmmArrayEntry arrayEntry = iterator.next();
           arrayCopy.add(arrayEntry);
       }
       
       // decode array copy
       OmmArray arrayDecCopy = JUnitTestConnect.createOmmArray();
       JUnitTestConnect.setRsslData(arrayDecCopy, arrayCopy, Codec.majorVersion(), Codec.minorVersion(), null, null);
       
       // compare with original
       TestUtilities.EmaDecode_UPAArrayAll(arrayDecCopy);
       
       System.out.println("\ntestArray_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA passed");
   }
}
