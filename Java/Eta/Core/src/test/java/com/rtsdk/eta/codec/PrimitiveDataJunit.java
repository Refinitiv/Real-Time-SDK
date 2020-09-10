///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.math.BigInteger;

import org.junit.Test;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.BufferImpl;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.Date;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Enum;
import com.rtsdk.eta.codec.IntImpl;
import com.rtsdk.eta.codec.Qos;
import com.rtsdk.eta.codec.QosImpl;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.codec.RealImpl;
import com.rtsdk.eta.codec.State;
import com.rtsdk.eta.codec.StateImpl;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.codec.Time;
import com.rtsdk.eta.codec.UIntImpl;

public class PrimitiveDataJunit
{
    EncodeIterator _encIter = CodecFactory.createEncodeIterator();
    DecodeIterator _decIter = CodecFactory.createDecodeIterator();
    BufferImpl _buffer = (BufferImpl)CodecFactory.createBuffer();

    private void uintED(UIntImpl uint)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        uint.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        UIntImpl uint1 = (UIntImpl)CodecFactory.createUInt();
        uint1.decode(_decIter);

        assertEquals(uint.toLong(), uint1.toLong());
        assertEquals(uint.toBigInteger(), uint1.toBigInteger());
    }
    
    @Test
    public void uIntEncodeDecodeBlankTest()
    {
        UIntImpl uintv = (UIntImpl)CodecFactory.createUInt();
        
        assertFalse(uintv.isBlank());
        uintv.blank();
        assertTrue(uintv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, uintv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.UINT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        UIntImpl uint1 = (UIntImpl)CodecFactory.createUInt();
        assertFalse(uint1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, uint1.decode(_decIter));

        assertTrue(uint1.isBlank());
        assertEquals(uintv.toLong(), uint1.toLong());
    }

    @Test
    public void uIntSetTest()
    {
        UInt thisUInt = CodecFactory.createUInt();

        thisUInt.value(11223344);
        assertEquals(11223344, thisUInt.toLong());
    }

    @Test
    public void uIntStringTest()
    {
        UInt testUInt = CodecFactory.createUInt();
        
    	assertEquals(CodecReturnCodes.SUCCESS, testUInt.value("    "));
    	assertTrue(testUInt.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testUInt.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testUInt.value(""));
    	assertTrue(testUInt.isBlank());
        
    	assertEquals(CodecReturnCodes.SUCCESS, testUInt.value("11112222"));
    	assertEquals("11112222", testUInt.toString());
    	assertFalse(testUInt.isBlank());
    }

    @Test
    public void CopyUIntTest()
    {
    	 UIntImpl Left = (UIntImpl)CodecFactory.createUInt(); 
    	 UIntImpl Right = (UIntImpl)CodecFactory.createUInt();

    	 Left.value(13);
 	
    	 // Make sure they are NOT equal, by asserting if they ARE equal
    	 assertTrue(Left._longValue != Right._longValue);
    	 
    	 Left.copy(Right);
    	 //  Make sure they are Equal
    	 assertEquals(Left._isBlank, Right._isBlank);
    	 assertEquals(Left._longValue, Right._longValue);
    	 
         // Change it again to make sure they are now NOT equal 
         // ( this ensures we are doing a DEEP copy and not just copying references)
    	 Left.value(Right._longValue +1);
    	 assertTrue(Left._longValue != Right._longValue);
     }
	 
	@Test
    public void bigIntegerTest()
    {
        BigInteger expectedBi = new BigInteger("9223372036854775810"); // two values over LONG_MAX_VALUE
        UInt uint = CodecFactory.createUInt();
        uint.value(expectedBi);
        BigInteger actualBi = uint.toBigInteger();
        assertEquals(expectedBi.toString(), actualBi.toString());
    }

    
    @Test
    public void CopyEnumTest()
    {
    	 EnumImpl Left = (EnumImpl)CodecFactory.createEnum(); 
    	 EnumImpl Right = (EnumImpl)CodecFactory.createEnum();

    	 Left.value(13);
 	
    	 // Make sure they are NOT equal, by asserting if they ARE equal
    	 assertTrue(Left._enumValue != Right._enumValue);
    	 
    	 Left.copy(Right);
    	 //  Make sure they are Equal
    	 assertEquals(Left._isBlank, Right._isBlank);
    	 assertEquals(Left._enumValue, Right._enumValue);
    	 
         // Change it again to make sure they are now NOT equal 
         // ( this ensures we are doing a DEEP copy and not just copying references)
    	 if ( 65535 <Right._enumValue)Left.value(0); else Left.value(Right._enumValue +1);
    	 assertTrue(Left._enumValue != Right._enumValue);
     }
    
  
	@Test
    public void CopyFloatTest()
    {
    	 FloatImpl Left = (FloatImpl)CodecFactory.createFloat(); 
    	 FloatImpl Right = (FloatImpl)CodecFactory.createFloat();

    	 Left._floatValue = 13;
 	
    	 // Make sure they are NOT equal, by asserting if they ARE equal
    	 assertTrue(Left._floatValue != Right._floatValue);
    	 
    	 Left.copy(Right);
    	 //  Make sure they are Equal
    	 assertEquals(Left._isBlank, Right._isBlank);
    	 assertTrue(Left._floatValue == Right._floatValue);
    	 
         // Change it again to make sure they are now NOT equal 
         // ( this ensures we are doing a DEEP copy and not just copying references)
    	 Left.value(Right._floatValue +1);
    	 assertTrue(Left._floatValue != Right._floatValue);
     }
   
    
  
 	public void CopyDoubleTest()
     {
     	 DoubleImpl Left = (DoubleImpl)CodecFactory.createDouble(); 
     	 DoubleImpl Right = (DoubleImpl)CodecFactory.createDouble();

     	Left.value(13);
  	
     	 // Make sure they are NOT equal, by asserting if they ARE equal
     	 assertTrue(Left._doubleValue != Right._doubleValue);
     	 
     	Left.copy(Right);
     	 //  Make sure they are Equal
     	 assertEquals(Left._isBlank, Right._isBlank);
     	 assertTrue(Left._doubleValue == Right._doubleValue);
     	 
          // Change it again to make sure they are now NOT equal 
          // ( this ensures we are doing a DEEP copy and not just copying references)
     	Left.value(Right._doubleValue +1);
     	 assertTrue(Left._doubleValue != Right._doubleValue);
      }
    
    
 	public void CopyRealTest()
     {
    	RealImpl Left= (RealImpl)CodecFactory.createReal(); 
    	RealImpl Right = (RealImpl)CodecFactory.createReal();

    	Left._value =13;
  	
     	// Make sure they are NOT equal, by asserting if they ARE equal
     	assertTrue(Left._value != Right._value);
     	 
     	Left.copy(Right);
     	//  Make sure they are Equal
     	assertEquals(Left._isBlank, Right._isBlank);
     	assertEquals(Left._value, Right._value);
     	 
        // Change it again to make sure they are now NOT equal 
        // ( this ensures we are doing a DEEP copy and not just copying references)
     	Left._value =Right._value + 1;
     	assertTrue(Left._value != Right._value);
      }
   
    
    
    @Test
    public void uIntEncodeDecodeTest()
    {
        UIntImpl uint = (UIntImpl)CodecFactory.createUInt();
        long val;

        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        /* trailing 0's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
            uint.value(val);
            uintED(uint);
        }

        /* Highest hex number */
        val = 0xFFFFFFFFFFFFFFFFl;
        uint.value(val);
        uintED(uint);

        /* 0 */
        val = 0;
        uint.value(val);
        uintED(uint);
        
        /* Highest hex number */
        uint.value("-1");
        uintED(uint);

        /* 0 */
        uint.value("0");
        uintED(uint);
    }

    private void intED(IntImpl intv)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        intv.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        IntImpl int1 = (IntImpl)CodecFactory.createInt();
        int1.decode(_decIter);

        assertEquals(intv.toLong(), int1.toLong());
    }
   
    @Test
    public void CopyIntTest()
    {
    	IntImpl iLeft = (IntImpl)CodecFactory.createInt();
    	IntImpl iRight = (IntImpl)CodecFactory.createInt();
    	
    	iLeft.value(100);
    	assertTrue(iLeft.toLong()!=iRight.toLong());
 
    	iLeft.copy(iRight);
    	assertEquals(iLeft.toLong(), iRight.toLong());
    	
        // Change it again to make sure they are now NOT equal 
        // ( this ensures we are doing a DEEP copy and not just copying references)
    	iLeft.value(9);
    	assertTrue(iLeft.toLong()!=iRight.toLong());
    }
   
 
    @Test
    public void intEncodeDecodeBlankTest()
    {
        IntImpl intv = (IntImpl)CodecFactory.createInt();
        
        assertFalse(intv.isBlank());
        intv.blank();
        assertTrue(intv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, intv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.INT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        IntImpl int1 = (IntImpl)CodecFactory.createInt();
        assertFalse(int1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, int1.decode(_decIter));

        assertTrue(int1.isBlank());
        assertEquals(intv.toLong(), int1.toLong());
    }
    
    @Test
    public void intSetTest()
    {
        Int thisInt = CodecFactory.createInt();

        thisInt.value(111222);
        assertEquals(111222, thisInt.toLong());
    }

    @Test
    public void intStringTest()
    {
        Int testInt = CodecFactory.createInt();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testInt.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, testInt.value("    "));
    	assertTrue(testInt.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testInt.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testInt.value(""));
    	assertTrue(testInt.isBlank());
        
    	assertEquals(CodecReturnCodes.SUCCESS, testInt.value("1122"));
    	assertEquals("1122", testInt.toString());
    	assertFalse(testInt.isBlank());
    }

    @Test
    public void int64EncodeDecodeTest()
    {
        IntImpl intv = (IntImpl)CodecFactory.createInt();
        long val;

        /* leading 0's */
        for (val = 0x5555555555555555l; val != 0; val >>= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Negative numbers with leading 1's */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0xFFFFFFFFFFFFFFFFl; val >>= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Numbers with trailing zeros */
        for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
        {
            intv.value(val);
            intED(intv);
        }

        /* Highest hex number */
        val = 0xFFFFFFFFFFFFFFFFl;
        intv.value(val);
        intED(intv);

        /* 0 */
        val = 0;
        intv.value(val);
        intED(intv);

    }

    private void dateED(Date date)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        date.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Date date1 = CodecFactory.createDate();
        date1.decode(_decIter);

        assertEquals(date.day(), date1.day());
        assertEquals(date.month(), date1.month());
        assertEquals(date.year(), date1.year());
    }

    @Test
    public void dateEncodeDecodeTest()
    {
        Date date = CodecFactory.createDate();

        for (int i = 4095; i != 0; i >>= 1)
        {
            date.year(i);
            for (int j = 0; j <= 12; ++j)
            {
                date.month(j);
                for (int k = 0; k <= 31; ++k)
                    dateED(date);
            }
        }
    }

    @Test
    public void dateEncodeDecodeBlankTest()
    {
        Date date = CodecFactory.createDate();

        date.blank();
        assertTrue(date.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.SUCCESS, date.encode(_encIter));

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Date date1 = CodecFactory.createDate();
        date1.decode(_decIter);

        assertEquals(date.day(), date1.day());
        assertEquals(date.month(), date1.month());
        assertEquals(date.year(), date1.year());
        assertTrue(date1.isBlank());
    }
    
    @Test
    public void dateEqualsTest()
    {
        Date thisDate = CodecFactory.createDate();
        Date thatDate = CodecFactory.createDate();
        thisDate.blank();
        thatDate.blank();
        assertTrue("Date equals?", thisDate.equals(thatDate));
        thisDate.year(2012);
        thisDate.month(10);
        thisDate.day(15);
        thatDate.year(2012);
        thatDate.month(10);
        thatDate.day(15);
        assertTrue("Date equals?", thisDate.equals(thatDate));
        thatDate.blank();
        assertFalse("Date equals?", thisDate.equals(thatDate));
    }
    
    @Test
    public void dateSetTest()
    {
        Date thisDate = CodecFactory.createDate();

        assertEquals(CodecReturnCodes.SUCCESS, thisDate.year(0));
        assertEquals(0, thisDate.year());
        assertEquals(CodecReturnCodes.SUCCESS, thisDate.month(0));
        assertEquals(0, thisDate.month());
        assertEquals(CodecReturnCodes.SUCCESS, thisDate.day(0));
        assertEquals(0, thisDate.day());
        assertTrue(thisDate.isBlank());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.year(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.month(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.day(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.year(65536));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.month(13));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDate.day(32));
        assertEquals(CodecReturnCodes.SUCCESS, thisDate.year(65535));
        assertEquals(65535, thisDate.year());
        assertEquals(CodecReturnCodes.SUCCESS, thisDate.month(12));
        assertEquals(12, thisDate.month());
        assertEquals(CodecReturnCodes.SUCCESS, thisDate.day(31));
        assertEquals(31, thisDate.day());
    }

    @Test 
    public void dateStringTest()
    {
        Date date = CodecFactory.createDate();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, date.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, date.value("    "));
    	assertTrue(date.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, date.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, date.value(""));
    	assertTrue(date.isBlank());
        
        //1974/04/14
        date.value("1974/04/14");
        assertEquals("date.value(1974/04/14)", 1974, date.year());
        assertEquals("date.value(1974/04/14)", 4, date.month());
        assertEquals("date.value(1974/04/14)", 14, date.day());
    	assertFalse(date.isBlank());
        
        date.blank();
        
        //04/14/74
        date.value("04/14/74");
        assertEquals("date.value(04/14/74)", 1974, date.year());
        assertEquals("date.value(04/14/74)", 4, date.month());
        assertEquals("date.value(04/14/74)", 14, date.day());
    	assertFalse(date.isBlank());
      
        date.blank();
        
        //04/14/1974
        date.value("04/14/1974");
        assertEquals("date.value(04/14/1974)", 1974, date.year());
        assertEquals("date.value(04/14/1974)", 4, date.month());
        assertEquals("date.value(04/14/1974)", 14, date.day());
    	assertFalse(date.isBlank());

        date.blank();
        
        //1974 04 14
        date.value("1974 04 14");
        assertEquals("date.value(1974 04 14)", 1974, date.year());
        assertEquals("date.value(1974 04 14)", 4, date.month());
        assertEquals("date.value(1974 04 14)", 14, date.day());
    	assertFalse(date.isBlank());
        
        date.blank();
        
        //04 14 74
        date.value("04 14 74");
        assertEquals("date.value(04 14 74)", 1974, date.year());
        assertEquals("date.value(04 14 74)", 4, date.month());
        assertEquals("date.value(04 14 74)", 14, date.day());
    	assertFalse(date.isBlank());
      
        date.blank();
        
        //04 14 1974
        date.value("04 14 1974");
        assertEquals("date.value(04 14 1974)", 1974, date.year());
        assertEquals("date.value(04 14 1974)", 4, date.month());
        assertEquals("date.value(04 14 1974)", 14, date.day());
    	assertFalse(date.isBlank());

        date.blank();
        
        //04 JAN 74
        date.value("04 JAN 74");
        assertEquals("date.value(04 JAN 74)", 1974, date.year());
        assertEquals("date.value(04 JAN 74)", 1, date.month());
        assertEquals("date.value(04 JAN 74)", 4, date.day());
    	assertFalse(date.isBlank());
      
        date.blank();
        
        //04 JAN 1974
        date.value("04 JAN 1974");
        assertEquals("date.value(04 JAN 1974)", 1974, date.year());
        assertEquals("date.value(04 JAN 1974)", 1, date.month());
        assertEquals("date.value(04 JAN 1974)", 4, date.day());
    	assertFalse(date.isBlank());
        
        date.blank();
        
        //04 jan 74
        date.value("04 jan 74");
        assertEquals("date.value(04 JAN 1974)", 1974, date.year());
        assertEquals("date.value(04 JAN 1974)", 1, date.month());
        assertEquals("date.value(04 JAN 1974)", 4, date.day());
    	assertFalse(date.isBlank());
      
        date.blank();
        
        //04 jan 1974
        date.value("04 jan 1974");
        assertEquals("date.value(04 jan 1974)", 1974, date.year());
        assertEquals("date.value(04 jan 1974)", 1, date.month());
        assertEquals("date.value(04 jan 1974)", 4, date.day());
    	assertFalse(date.isBlank());
    }

    private void timeED(Time time)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        time.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Time time1 = CodecFactory.createTime();
        time1.decode(_decIter);

        assertEquals(time.hour(), time1.hour());
        assertEquals(time.minute(), time1.minute());
        assertEquals(time.second(), time1.second());
        assertEquals(time.millisecond(), time1.millisecond());
        assertEquals(time.microsecond(), time1.microsecond());
        assertEquals(time.nanosecond(), time1.nanosecond());
    }

    @Test 
    public void dateTimeisValidTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();
        
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.time().blank();
        assertEquals("DateTime.isValid() valid date, blank time", true, dateTime.isValid());
        
        dateTime.date().blank();
        dateTime.time().blank();
        assertEquals("DateTime.isValid() blank date, blank time", true, dateTime.isValid());
        
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(11);
        dateTime.minute(12);
        dateTime.second(2);
        dateTime.millisecond(2);
        assertEquals("DateTime.isValid() valid date, valid time", true, dateTime.isValid());
        
    }
    
    @Test 
    public void dateTimeToStringTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();
        
        //blank
        dateTime.blank();
        assertEquals("DateTime.toString() blank", "", dateTime.toString());
        
        //invalid
        dateTime.year(2012);
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, dateTime.month(13)); //invalid
        dateTime.day(2);
        
        //valid date, blank time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.time().blank();
        assertEquals("DateTime.toString() valid date, blank time", "02 DEC 2012", dateTime.toString());
        
        
        //valid date, valid time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(2);
        dateTime.minute(2);
        dateTime.second(2);
        dateTime.millisecond(2);
        dateTime.microsecond(3);
        dateTime.nanosecond(4);
        assertEquals("DateTime.toString() valid date, valid time", "02 DEC 2012 02:02:02:002:003:004", dateTime.toString());
        
        //valid date, valid time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(2);
        dateTime.minute(2);
        dateTime.second(2);
        dateTime.millisecond(2);
        dateTime.microsecond(3);
        assertEquals("DateTime.toString() valid date, valid time", "02 DEC 2012 02:02:02:002:003:000", dateTime.toString());
        
      //valid date, valid time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(2);
        dateTime.minute(2);
        dateTime.second(2);
        dateTime.millisecond(2);
        assertEquals("DateTime.toString() valid date, valid time", "02 DEC 2012 02:02:02:002:000:000", dateTime.toString());
        
      //valid date, valid time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(2);
        dateTime.minute(2);
        dateTime.second(2);
        assertEquals("DateTime.toString() valid date, valid time", "02 DEC 2012 02:02:02:000:000:000", dateTime.toString());
        
        //valid date, valid time
        dateTime.clear();
        dateTime.year(2012);
        dateTime.month(12);
        dateTime.day(2);
        dateTime.hour(2);
        dateTime.minute(2);
        assertEquals("DateTime.toString() valid date, valid time", "02 DEC 2012 02:02:00:000:000:000", dateTime.toString());
        
      //blank date, valid time
        dateTime.date().blank();
        dateTime.hour(2);
        dateTime.minute(2);
        dateTime.second(2);
        dateTime.millisecond(2);
        dateTime.microsecond(3);
        dateTime.nanosecond(4);              
        assertEquals("DateTime.toString() valid date, valid time", "02:02:02:002:003:004", dateTime.toString());
        
        // this will fail intentionally because of the blank in the middle
        dateTime.blank();
        dateTime.hour(2);
        dateTime.minute(255);
        dateTime.second(2);
        dateTime.millisecond(2);
        dateTime.microsecond(3);
        dateTime.nanosecond(4);  
        assertEquals(false,dateTime.isValid());
        
        // this will work and should only output up to second.
        dateTime.blank();
        dateTime.hour(2);
        dateTime.minute(3);
        dateTime.second(4);
        dateTime.millisecond(65535);
        dateTime.microsecond(2047);
        dateTime.nanosecond(2047);  
        assertEquals(true,dateTime.isValid());
        System.out.println(dateTime.toString());
        assertEquals("DateTime.toString() valid date, valid time", "02:03:04", dateTime.toString());
    }
    
    @Test 
    public void dateTimeGMTTimeTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();
        dateTime.gmtTime();
    }
    
    @Test
    public void dateTimeSetTest()
    {
        DateTime thisDateTime = CodecFactory.createDateTime();

        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.year(2012));
        assertEquals(2012, thisDateTime.year());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.month(10));
        assertEquals(10, thisDateTime.month());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.day(15));
        assertEquals(15, thisDateTime.day());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.hour(8));
        assertEquals(8, thisDateTime.hour());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.minute(47));
        assertEquals(47, thisDateTime.minute());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.second(43));
        assertEquals(43, thisDateTime.second());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.millisecond(123));
        assertEquals(123, thisDateTime.millisecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.microsecond(345));
        assertEquals(345, thisDateTime.microsecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.nanosecond(223));
        assertEquals(223, thisDateTime.nanosecond());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.year(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.month(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.day(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.hour(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.minute(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.second(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.millisecond(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.year(65536));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.month(13));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.day(32));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.hour(24));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.minute(60));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.second(61));
		assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.second(60)); // leap second
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.millisecond(1000));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.microsecond(1000));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisDateTime.nanosecond(1000));
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.year(65535));
        assertEquals(65535, thisDateTime.year());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.month(12));
        assertEquals(12, thisDateTime.month());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.day(31));
        assertEquals(31, thisDateTime.day());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.hour(23));
        assertEquals(23, thisDateTime.hour());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.minute(59));
        assertEquals(59, thisDateTime.minute());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.second(59));
        assertEquals(59, thisDateTime.second());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.millisecond(999));
        assertEquals(999, thisDateTime.millisecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.microsecond(999));
        assertEquals(999, thisDateTime.microsecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisDateTime.nanosecond(999));
        assertEquals(999, thisDateTime.nanosecond());
    }

    @Test 
    public void dateTimeStringTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, dateTime.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, dateTime.value("    "));
    	assertTrue(dateTime.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, dateTime.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, dateTime.value(""));
    	assertTrue(dateTime.isBlank());

    	//1974/04/14 02:02:02:200
        dateTime.value("1974/04/14 02:02:02:200");
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04/14/74 02:02:02:200
        dateTime.value("04/14/74 02:02:02:200");
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04/14/1974 02:02:02:200
        dateTime.value("04/14/1974 02:02:02:200");
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //1974/04/14 02 02 02 200
        dateTime.value("1974/04/14 02 02 02 200");
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04/14/74 02 02 02 200
        dateTime.value("04/14/74 02 02 02 200");
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04/14/1974 02 02 02 200
        dateTime.value("04/14/1974 02 02 02 200");
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
       
        dateTime.blank();
        
        //1974 04 14 02:02:02:200
        dateTime.value("1974 04 14 02:02:02:200");
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02:02:02:200
        dateTime.value("04 14 74 02:02:02:200");
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02:02:02:200
        dateTime.value("04 14 1974 02:02:02:200");
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //04 JAN 74 02:02:02:200
        dateTime.value("04 JAN 74 02:02:02:200");
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 JAN 1974 02:02:02:200
        dateTime.value("04 JAN 1974 02:02:02:200");
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 jan 74 02:02:02:200
        dateTime.value("04 jan 74 02:02:02:200");
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 jan 1974 02:02:02:200
        dateTime.value("04 jan 1974 02:02:02:200");
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //1974 04 14 02 02 02 200
        dateTime.value("1974 04 14 02 02 02 200");
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02 02 02 200
        dateTime.value("04 14 74 02 02 02 200");
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02 02 02 200
        dateTime.value("04 14 1974 02 02 02 200");
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //1974 04 14 02 02 02 200
        dateTime.value("1974 04 14 02 02 02 200");
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02 02 02 200
        dateTime.value("04 14 74 02 02 02 200");
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02 02 02 200
        dateTime.value("04 14 1974 02 02 02 200");
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //04 JAN 74 02 02 02 200
        dateTime.value("04 JAN 74 02 02 02 200");
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 JAN 1974 02 02 02 200
        dateTime.value("04 JAN 1974 02 02 02 200");
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 jan 74 02 02 02 200
        dateTime.value("04 jan 74 02 02 02 200");
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200)", 0, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 jan 1974 02 02 02 200
        dateTime.value("04 jan 1974 02 02 02 200");
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 0, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200)", 0, dateTime.nanosecond());
        
    	assertFalse(dateTime.isBlank());
    	
    	
    	
    	// round 2
    	//1974/04/14 02:02:02:200:300:400
        dateTime.value("1974/04/14 02:02:02:200:300:400");
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04/14/74 02:02:02:200:300:400
        dateTime.value("04/14/74 02:02:02:200:300:400");
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04/14/1974 02:02:02:200
        dateTime.value("04/14/1974 02:02:02:200:300:400");
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/1974 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //1974/04/14 02 02 02 200
        dateTime.value("1974/04/14 02 02 02 200 300 400");
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(1974/04/14 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04/14/74 02 02 02 200
        dateTime.value("04/14/74 02 02 02 200 300 400");
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/74 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04/14/1974 02 02 02 200
        dateTime.value("04/14/1974 02 02 02 200 300 400");
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04/14/1974 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04/14/74 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04/14/74 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
       
        dateTime.blank();
        
        //1974 04 14 02:02:02:200
        dateTime.value("1974 04 14 02:02:02:200:300:400");
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(1974/04/14 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02:02:02:200
        dateTime.value("04 14 74 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02:02:02:200
        dateTime.value("04 14 1974 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //04 JAN 74 02:02:02:200
        dateTime.value("04 JAN 74 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 74 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 JAN 1974 02:02:02:200
        dateTime.value("04 JAN 1974 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 jan 74 02:02:02:200
        dateTime.value("04 jan 74 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 jan 1974 02:02:02:200
        dateTime.value("04 jan 1974 02:02:02:200:300:400");
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 1974 02:02:02:200:300:400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //1974 04 14 02 02 02 200
        dateTime.value("1974 04 14 02 02 02 200 300 400");
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02 02 02 200
        dateTime.value("04 14 74 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02 02 02 200
        dateTime.value("04 14 1974 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //1974 04 14 02 02 02 200
        dateTime.value("1974 04 14 02 02 02 200 300 400");
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 2, dateTime.second());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(1974 04 14 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 14 74 02 02 02 200
        dateTime.value("04 14 74 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 74 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 14 1974 02 02 02 200
        dateTime.value("04 14 1974 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 4, dateTime.month());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 14, dateTime.day());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 14 1974 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());

        dateTime.blank();
        
        //04 JAN 74 02 02 02 200
        dateTime.value("04 JAN 74 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 74 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 JAN 1974 02 02 02 200
        dateTime.value("04 JAN 1974 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 JAN 1974 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
        
        dateTime.blank();
        
        //04 jan 74 02 02 02 200
        dateTime.value("04 jan 74 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 74 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
      
        dateTime.blank();
        
        //04 jan 1974 02 02 02 200
        dateTime.value("04 jan 1974 02 02 02 200 300 400");
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 1974, dateTime.year());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 1, dateTime.month());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 4, dateTime.day());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 2, dateTime.hour());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 2, dateTime.minute());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 2, dateTime.second());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 200, dateTime.millisecond());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 300, dateTime.microsecond());
        assertEquals("dateTime.value(04 jan 1974 02 02 02 200 300 400)", 400, dateTime.nanosecond());
    	assertFalse(dateTime.isBlank());
    	
    	
    	
    }

    @Test
    public void dateTimeEncodeDecodeTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();

        for (int i = 4095; i != 0; i >>= 1)
        {
            dateTime.year(i);
            for (int j = 0; j <= 12; ++j)
            {
                dateTime.month(j);
                for (int k = 0; k <= 31; ++k)
                {
                    dateTime.day(k);

                    // for each date, test different times.
                    for (int hour = 23; hour >= 0; hour = hour - 4)
                    {
                        dateTime.hour(hour);
                        for (int min = 59; min >= 0; min = min - 15)
                        {
                            dateTime.minute(min);
                            for (int sec = 59; sec >= 0; sec = sec - 20)
                            {
                                dateTime.second(sec);
                                for (int msec = 59; msec >= 0; msec = msec - 30)
                                {                                  
	                                dateTime.millisecond(msec);
	                                for (int usec = 59; usec >= 0; usec = usec - 25)
	                                {
	                                	dateTime.microsecond(usec);
	                                    for (int nsec = 59; nsec >= 0; nsec = nsec - 40)
	                                    {
	                                        dateTime.nanosecond(nsec);
	                                        dateTimeED(dateTime);
	                                    }
	                                }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    @Test
    public void dateTimeEncodeDecodeBlankTest()
    {
        DateTime dateTime = CodecFactory.createDateTime();
        
        dateTime.blank();
        assertTrue(dateTime.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        int ret = dateTime.encode(_encIter);
        assertEquals("DateTime Encode Success", CodecReturnCodes.SUCCESS, ret);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        DateTime dateTime1 = CodecFactory.createDateTime();
        ret = dateTime1.decode(_decIter);
        assertEquals("DateTime Decode Blank", CodecReturnCodes.BLANK_DATA, ret);
        assertEquals("DateTime isBlank", true, dateTime1.isBlank());
    }

    private void dateTimeED(DateTime dateTime)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        int ret = dateTime.encode(_encIter);
        assertEquals("DateTime Encode Success", CodecReturnCodes.SUCCESS, ret);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        DateTime dateTime1 = CodecFactory.createDateTime();
        ret = dateTime1.decode(_decIter);
        assertEquals("DateTime Decode Success", CodecReturnCodes.SUCCESS, ret);

        assertEquals(dateTime.day(), dateTime1.day());
        assertEquals(dateTime.month(), dateTime1.month());
        assertEquals(dateTime.year(), dateTime1.year());
        assertEquals(dateTime.hour(), dateTime1.hour());
        assertEquals(dateTime.minute(), dateTime1.minute());
        assertEquals(dateTime.second(), dateTime1.second());
        assertEquals(dateTime.millisecond(), dateTime1.millisecond());
        assertEquals(dateTime.microsecond(), dateTime1.microsecond());
        assertEquals(dateTime.nanosecond(), dateTime1.nanosecond());
        
    }
  
    @Test
    public void CopyDateTimeTest()
    {
 	   DateTime Left = CodecFactory.createDateTime();
 	   DateTime Right = CodecFactory.createDateTime();
 	   Left.localTime();
 	   
 	   Time tempTime = CodecFactory.createTime();
 	   Date tempDate = CodecFactory.createDate();
 	   Left.time().copy(tempTime);
 	   Left.date().copy(tempDate);
 	   
 	   addToTime((TimeImpl)tempTime,2,2,2,2,4,5);
 	   addToDate((DateImpl) tempDate,3,3,3);
 	   
 	   Right.nanosecond(tempTime.nanosecond());
 	   Right.microsecond(tempTime.microsecond());
 	   Right.millisecond(tempTime.millisecond());
 	   Right.second(tempTime.second());
 	   Right.minute(tempTime.minute());
 	   Right.hour(tempTime.hour());
 	   Right.day(tempDate.day());
 	   Right.month(tempDate.month());
 	   Right.year(tempDate.year());
  
 	   // test that they are NOT equal!
 	   assertTrue(Left.day()!= Right.day());
 	   assertTrue(Left.month() !=Right.month());
 	   assertTrue(Left.year()!= Right.year());
 	   assertTrue(Left.hour()!= Right.hour());
 	   assertTrue(Left.minute()!= Right.minute());
 	   assertTrue(Left.second()!=Right.second());
 	   assertTrue(Left.millisecond()!= Right.millisecond());
 	   assertTrue(Left.microsecond()!= Right.microsecond());
 	   assertTrue(Left.nanosecond()!=Right.nanosecond());
 	   
 	   Left.copy(Right);  // Testing this method
 	   
        assertEquals(Left.day(), Right.day());
        assertEquals(Left.month(), Right.month());
        assertEquals(Left.year(), Right.year());
        assertEquals(Left.hour(), Right.hour());
        assertEquals(Left.minute(), Right.minute());
        assertEquals(Left.second(), Right.second());
        assertEquals(Left.millisecond(), Right.millisecond());
        assertEquals(Left.microsecond(), Right.microsecond());
        assertEquals(Left.nanosecond(), Right.nanosecond());
        
        // Change it again to make sure they are now NOT equal 
        // ( this ensures we are doing a DEEP copy and not just copying references)
  	   addToTime((TimeImpl)tempTime,5,5,5,5,6,7);
  	   addToDate((DateImpl) tempDate,5,11,15);
  	   
  	   Right.nanosecond(tempTime.nanosecond());
  	   Right.microsecond(tempTime.microsecond());
  	   Right.millisecond(tempTime.millisecond());
  	   Right.second(tempTime.second());
  	   Right.minute(tempTime.minute());
  	   Right.hour(tempTime.hour());
  	   Right.day(tempDate.day());
  	   Right.month(tempDate.month());
  	   Right.year(tempDate.year());
      
 	   assertTrue(Left.day()!= Right.day());
 	   assertTrue(Left.month()!=Right.month());
 	   assertTrue(Left.year()!= Right.year());
 	   assertTrue(Left.hour()!= Right.hour());
 	   assertTrue(Left.minute()!= Right.minute());
 	   assertTrue(Left.second()!=Right.second());
 	   assertTrue(Left.millisecond()!= Right.millisecond());
 	   assertTrue(Left.microsecond()!= Right.microsecond());
 	   assertTrue(Left.nanosecond()!= Right.nanosecond());
    }
    
    private void addToDate(DateImpl date, int year, int month, int day)
    {
		// This function does NOT take into account adding very large numbers like if you want to increment day by 360 
    	//  It also assumes every month has ONLY 27 days

    	int tmpDay = date.day();
    	if (27 < tmpDay+day)
    	{
    		date.day(tmpDay+day-27);
    		month++;
    	}
    	else
    		date.day(tmpDay+day);
    	
    	
    	int tmpMonth = date.month();
    	if (12 < tmpMonth+month)
    	{
    		date.month(tmpMonth+month-12);
    		year++;
    	}
    	else
    		date.month(tmpMonth+month);
    	
    	date.year(date.year()+year);

    	
    }
    private void addToTime(TimeImpl time, int hour, int min, int sec, int mSec, int usec, int nsec)
    {
    	// This function does NOT take into account adding numbers greater than 59 to the to the minutes or Seconds,
    	// 999 to the mSec, or 23 to the hours
    	//   
    	
    	
    	 //  add nanosecond
    	int temp_nsec = time.nanosecond();
    	if (999 <= temp_nsec + nsec )
    	{
    		time.nanosecond(temp_nsec+nsec-999);
    		usec++;
    	}
    	else
    		time.nanosecond(temp_nsec+nsec);
    	
    //  add microsecond
    	int temp_usec = time.microsecond();
    	if (999 <= temp_usec + usec )
    	{
    		time.microsecond(temp_usec+usec-999);
    		mSec++;
    	}
    	else
    		time.microsecond(temp_usec+usec);
    	
    	//  add milliseconds
    	int temp_mSec = time.millisecond();
    	if (999 <= temp_mSec + mSec )
    	{
    		time.millisecond(temp_mSec+mSec-999);
    		sec++;
    	}
    	else
    		time.millisecond(temp_mSec+mSec);
    	
    	
    	//  add seconds
    	int temp_sec = time.second();
       	if (59 <= temp_sec + sec )
    	{
    		time.second(temp_sec+sec-59);
    		min++;
    	}
    	else
    		time.second(temp_sec+sec);

       	//  add minutes
       	int temp_min = time.minute();
       	if (59 <= temp_min + min )
    	{
    		time.minute(temp_min+min-59);
    		hour++;
    	}
    	else
    		time.minute(temp_min+min);

       	//  add hours
       	int temp_hour = time.hour();
       	if (23 < temp_hour + hour )
    	{
    		time.hour(temp_hour+hour-23);
    	}
    	else
    		time.hour(temp_hour+hour);
    }
    
    
    @Test
    public void CopyTimeTest()
    {
 	   TimeImpl Left = (TimeImpl)CodecFactory.createTime();
 	   TimeImpl Right = (TimeImpl)CodecFactory.createTime();
 	   DateTimeImpl temp = (DateTimeImpl)CodecFactory.createDateTime();
 	   temp.localTime();
 	   Left =(TimeImpl) temp._time;
 	   
 	   // test that they are NOT equal!
 	   Left.copy(Right);  // Testing this method
 	   addToTime(Right,2,2,2,2,3,4);
 	 
 	   assertTrue(Left.hour()!= Right.hour());
 	   assertTrue(Left.minute()!= Right.minute());
 	   assertTrue(Left.second()!=Right.second());
 	   assertTrue(Left.millisecond()!= Right.millisecond());
 	   assertTrue(Left.microsecond()!= Right.microsecond());
 	   assertTrue(Left.nanosecond()!=Right.nanosecond());
 	   
 	   Left.copy(Right);  // Testing this method
 	   
        
        assertEquals(Left.hour(), Right.hour());
        assertEquals(Left.minute(), Right.minute());
        assertEquals(Left.second(), Right.second());
        assertEquals(Left.millisecond(), Right.millisecond());
        assertEquals(Left.microsecond(),Right.microsecond());
        assertEquals(Left.nanosecond(), Right.nanosecond());
        
        // Change it again to make sure they are now NOT equal 
        // ( this ensures we are doing a DEEP copy and not just copying references)
        addToTime(Right,4,4,4,400,500,600);
       	   
 	   assertTrue(Left.hour()!= Right.hour());
 	   assertTrue(Left.minute()!= Right.minute());
 	   assertTrue(Left.second()!=Right.second());
 	   assertTrue(Left.millisecond()!= Right.millisecond());
 	   assertTrue(Left.microsecond()!= Right.microsecond());
 	   assertTrue(Left.nanosecond()!= Right.nanosecond());
    }    
    
    @Test
    public void CopyDateTest()
    {
 	   DateImpl Left = (DateImpl)CodecFactory.createDate();
 	   DateImpl Right = (DateImpl)CodecFactory.createDate();
 	   
 	   Left.value("3/17/1967");
 	   
 	   // test that they are NOT equal!
 	   assertTrue(Left.day()!= Right.day());
 	   assertTrue(Left.month() !=Right.month());
 	   assertTrue(Left.year()!= Right.year());
 	 
 	   
 	    Left.copy(Right);  // Testing this method
 	   
        assertEquals(Left.day(), Right.day());
        assertEquals(Left.month(), Right.month());
        assertEquals(Left.year(), Right.year());
      
        // Change it again to make sure they are now NOT equal 
        // ( this ensures we are doing a DEEP copy and not just copying references)
        int idt_day=Left.day();
        int idt_month=Left.month();
        int idt_year=Left.year();
        
        if (1==idt_day)        idt_day = 15; else idt_day--;
        if (1==idt_month)      idt_month=6; else idt_month--;
        if (1==idt_year)       idt_year=1967; else idt_year--;
       
        Right.day(idt_day);
        Right.month(idt_month);
        Right.year(idt_year);
 	  
      
 	   assertTrue(Left.day()!= Right.day());
 	   assertTrue(Left.month()!=Right.month());
 	   assertTrue(Left.year()!= Right.year());
 	   
    }

    @Test
    public void CopyQosTest()
    {
    	 QosImpl Left = (QosImpl)CodecFactory.createQos(); 
    	 QosImpl Right = (QosImpl)CodecFactory.createQos();

    	 Left.blank();
    	 Right._dynamic=true;
    	 Right._isBlank =false;
    	 Right._rate=QosRates.JIT_CONFLATED;
    	 Right._timeliness=QosTimeliness.DELAYED;
    	 Right._timeInfo=1;
    	 Right._rateInfo=1;
    	 
    	 // Make sure they are NOT equal, by asserting if they ARE equal
    	 assertTrue(Left._dynamic != Right._dynamic);
    	 assertTrue(Left._isBlank != Right._isBlank);
    	 assertTrue(Left._rate != Right._rate);
    	 assertTrue(Left._timeliness != Right._timeliness);
    	 assertTrue(Left._timeInfo != Right._timeInfo);
    	 assertTrue(Left._rateInfo != Right._rateInfo);
    	 
    	 Left.copy(Right);
    	 //  Make sure they are Equal
       	 assertEquals(Left._dynamic, Right._dynamic);
       	 assertEquals(Left._isBlank, Right._isBlank);
       	 assertEquals(Left._rate, Right._rate);
       	 assertEquals(Left._timeliness, Right._timeliness);
       	 assertEquals(Left._timeInfo, Right._timeInfo);
       	 assertEquals(Left._rateInfo, Right._rateInfo);
    	 
         // Change it again to make sure they are now NOT equal 
         // ( this ensures we are doing a DEEP copy and not just copying references)
    	 Right._dynamic=true;
    	 Right._isBlank =false;
    	 Right._rate=QosRates.JIT_CONFLATED;
    	 Right._timeliness=QosTimeliness.DELAYED;
    	 Right._timeInfo=1;
    	 Right._rateInfo=1;
    	 assertTrue(Left._dynamic != Right._dynamic);
    	 assertTrue(Left._isBlank != Right._isBlank);
    	 assertTrue(Left._rate != Right._rate);
    	 assertTrue(Left._timeliness != Right._timeliness);
    	 assertTrue(Left._timeInfo != Right._timeInfo);
    	 assertTrue(Left._rateInfo != Right._rateInfo);
    }
    
    
   @Test
    public void timeBlankTest()
    {
        Time time = CodecFactory.createTime();
        time.blank();
        assertTrue("Time isBlank?", time.isBlank());
    }
    
    @Test
    public void timeEqualsTest()
    {
        Time thisTime = CodecFactory.createTime();
        Time thatTime = CodecFactory.createTime();
        thisTime.blank();
        thatTime.blank();
        assertTrue("Time equals?", thisTime.equals(thatTime));
        thisTime.hour(23);
        thisTime.minute(59);
        thisTime.second(59);
        thisTime.millisecond(999);
        thatTime.hour(23);
        thatTime.minute(59);
        thatTime.second(59);
        thatTime.millisecond(999);
        assertTrue("Time equals?", thisTime.equals(thatTime));
        thatTime.blank();
        assertFalse("Time equals?", thisTime.equals(thatTime));
    }
    
    @Test
    public void timeSetTest()
    {
        Time thisTime = CodecFactory.createTime();

        assertEquals(CodecReturnCodes.SUCCESS, thisTime.hour(0));
        assertEquals(0, thisTime.hour());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.minute(0));
        assertEquals(0, thisTime.minute());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.second(0));
        assertEquals(0, thisTime.second());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.millisecond(0));
        assertEquals(0, thisTime.millisecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.microsecond(0));
        assertEquals(0, thisTime.microsecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.nanosecond(0));
        assertEquals(0, thisTime.nanosecond());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.hour(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.minute(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.second(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.millisecond(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.hour(24));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.minute(60));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.second(61));
		assertEquals(CodecReturnCodes.SUCCESS, thisTime.second(60)); // leap second
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.millisecond(1000));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.microsecond(1000));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisTime.nanosecond(1000));
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.hour(23));
        assertEquals(23, thisTime.hour());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.minute(59));
        assertEquals(59, thisTime.minute());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.second(59));
        assertEquals(59, thisTime.second());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.millisecond(999));
        assertEquals(999, thisTime.millisecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.microsecond(999));
        assertEquals(999, thisTime.microsecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.nanosecond(999));
        assertEquals(999, thisTime.nanosecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.hour(255));
        assertEquals(255, thisTime.hour());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.minute(255));
        assertEquals(255, thisTime.minute());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.second(255));
        assertEquals(255, thisTime.second());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.millisecond(65535));
        assertEquals(65535, thisTime.millisecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.microsecond(2047));
        assertEquals(2047, thisTime.microsecond());
        assertEquals(CodecReturnCodes.SUCCESS, thisTime.nanosecond(2047));
        assertEquals(2047, thisTime.nanosecond());
        assertTrue(thisTime.isBlank());
    }
    
    @Test 
    public void timeStringTest()
    {
        Time time = CodecFactory.createTime();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, time.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, time.value("    "));
    	assertTrue(time.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, time.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, time.value(""));
    	assertTrue(time.isBlank());

    	//02:02:02:200
        time.value("02:02:02:200:300:400");
        assertEquals("time.value(02:02:02:200:300:400)", 2, time.hour());
        assertEquals("time.value(02:02:02:200:300:400)", 2, time.minute());
        assertEquals("time.value(02:02:02:200:300:400)", 2, time.second());
        assertEquals("time.value(02:02:02:200:300:400)", 200, time.millisecond());
        assertEquals("time.value(02:02:02:200:300:400)", 300, time.microsecond());
        assertEquals("time.value(02:02:02:200:300:400)", 400, time.nanosecond());
    	assertFalse(time.isBlank());
        
        time.blank();

        //02 02 02 200
        time.value("02 02 02 200 300 400");
        assertEquals("time.value(02 02 02 200 300 400)", 2, time.hour());
        assertEquals("time.value(02 02 02 200 300 400)", 2, time.minute());
        assertEquals("time.value(02 02 02 200 300 400)", 2, time.second());
        assertEquals("time.value(02 02 02 200 300 400)", 200, time.millisecond());
        assertEquals("time.value(02 02 02 200 300 400)", 300, time.microsecond());
        assertEquals("time.value(02 02 02 200 300 400)", 400, time.nanosecond());
    	assertFalse(time.isBlank());

        time.blank();

        //02:02:02
        time.value("02:02:02");
        assertEquals("time.value(02:02:02)", 2, time.hour());
        assertEquals("time.value(02:02:02)", 2, time.minute());
        assertEquals("time.value(02:02:02)", 2, time.second());
        assertFalse("time.value(02:02:02)", 200 == time.millisecond());
        assertFalse("time.value(02:02:02)", 300 == time.microsecond());
        assertFalse("time.value(02:02:02)", 400 == time.nanosecond());
    	assertFalse(time.isBlank());
        
        time.blank();

        //02:02
        time.value("02:02");
        assertEquals("time.value(02:02)", 2, time.hour());
        assertEquals("time.value(02:02)", 2, time.minute());
        assertFalse("time.value(02:02)", 2 == time.second());
        assertFalse("time.value(02:02)", 200 == time.millisecond());
        assertFalse("time.value(02:02)", 300 == time.microsecond());
        assertFalse("time.value(02:02)", 400 == time.nanosecond());
    	assertFalse(time.isBlank());
    }
    
    @Test
    public void timeEncodeDecodeTest()
    {
        Time time = CodecFactory.createTime();

        for (int hour = 23; hour >= 0; hour = hour - 2)
        {
            time.hour(hour);
            for (int min = 59; min >= 0; min = min - 10)
            {
                time.minute(min);
                for (int sec = 59; sec >= 0; sec = sec - 10)
                {
                    time.second(sec);
                    for (int msec = 59; msec >= 0; msec = msec - 10)
                    {
                        time.millisecond(msec);
                        for (int usec = 59; usec >= 0; usec = usec - 10)
                        {
                            time.microsecond(usec);
                            for (int nsec = 59; nsec >= 0; nsec = nsec - 10)
                            {
                                time.nanosecond(nsec);
                                timeED(time);
                            }
                        }
                       
                    }
                }
            }
        }
    }
    
    
    @Test 
    public void newToOldTimeTest()
    {
    	Time time = new TimeImpl();
    	Time decTime = new TimeImpl()
    	{	@Override
    		public int decode(DecodeIterator diter)
    		{
            DecodeIteratorImpl iter = (DecodeIteratorImpl)diter;
            TimeImpl value = this;
            int ret = CodecReturnCodes.SUCCESS;
            int savePosition = iter._reader.position();

            switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
            {
                case 5:
                    try
                    {
                        iter._reader.position(iter._curBufPos);
                        value._hour = (iter._reader.readUnsignedByte());
                        value._minute = (iter._reader.readUnsignedByte());
                        value._second = (iter._reader.readUnsignedByte());
                        value._millisecond = (iter._reader.readUnsignedShort());
                        iter._reader.position(savePosition);
                    }
                    catch (Exception e)
                    {
                        return CodecReturnCodes.INCOMPLETE_DATA;
                    }
                    if (value.isBlank())
                    {
                    	ret = CodecReturnCodes.BLANK_DATA;
                    }
                    break;
                case 3:
                    try
                    {
                        iter._reader.position(iter._curBufPos);
                        value.clear();
                        value._hour = (iter._reader.readUnsignedByte());
                        value._minute = (iter._reader.readUnsignedByte());
                        value._second = (iter._reader.readUnsignedByte());
                        iter._reader.position(savePosition);
                    }
                    catch (Exception e)
                    {
                        return CodecReturnCodes.INCOMPLETE_DATA;
                    }
                    if (value._hour == TimeImpl.BLANK_HOUR)
                    {
                        value._millisecond = (TimeImpl.BLANK_MILLI);
                    }
                    if (value.isBlank())
                    {
                    	ret = CodecReturnCodes.BLANK_DATA;
                    }
                    break;
                case 2:
                    try
                    {
                        iter._reader.position(iter._curBufPos);
                        value.clear();
                        value._hour = (iter._reader.readUnsignedByte());
                        value._minute = (iter._reader.readUnsignedByte());
                        if (value._hour == TimeImpl.BLANK_HOUR)
                        {
                            value._second = (TimeImpl.BLANK_SECOND);
                            value._millisecond = (TimeImpl.BLANK_MILLI);
                        }
                        iter._reader.position(savePosition);
                    }
                    catch (Exception e)
                    {
                        return CodecReturnCodes.INCOMPLETE_DATA;
                    }
                    if (value.isBlank())
                    {
                    	ret = CodecReturnCodes.BLANK_DATA;
                    }
                    break;
                case 0:
                    value.blank();
                    ret = CodecReturnCodes.BLANK_DATA;
                    break;
                default:
                    ret = CodecReturnCodes.INCOMPLETE_DATA;
                    break;
            }
            return ret;
        }
    	};
    	
    	
    	
    	_encIter.clear();
    	_buffer.data(ByteBuffer.allocate(100));
    	_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
    	time.microsecond(654);
    	assertEquals(CodecReturnCodes.SUCCESS, time.encode(_encIter));
    	
    	_decIter.clear();
       	_decIter.setBufferAndRWFVersion(_buffer,  Codec.majorVersion(),  Codec.minorVersion());
    	assertEquals(CodecReturnCodes.INCOMPLETE_DATA, decTime.decode(_decIter));    	
    	
    }
    
    
    @Test 
    public void newToOldDateTimeTest()
    {
    	DateTime time = new DateTimeImpl();
    	DateTime decTime = new DateTimeImpl()
    	{	@Override
    		public int decode(DecodeIterator diter)
    		{
    		 DecodeIteratorImpl iter = (DecodeIteratorImpl)diter;
    	        DateTimeImpl value = this;
    	        int ret = CodecReturnCodes.SUCCESS;
    	        int savePosition = iter._reader.position();

    	        switch (iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos)
    	        {
    	            case 0:
    	                value.blank();
    	                ret = CodecReturnCodes.BLANK_DATA;
    	                return ret;
    	            case 1:
    	            case 2:
    	            case 3:
    	            case 4:
    	            case 5:
    	                return CodecReturnCodes.INCOMPLETE_DATA;
    	            case 6:
    	                try
    	                {
    	                    value.clear();
    	                    iter._reader.position(iter._curBufPos);
    	                    value.day(iter._reader.readUnsignedByte());
    	                    value.month(iter._reader.readUnsignedByte());
    	                    value.year(iter._reader.readUnsignedShort());
    	                    value.time().hour(iter._reader.readUnsignedByte());
    	                    value.time().minute(iter._reader.readUnsignedByte());

							ret = CodecReturnCodes.SUCCESS;

    	                    iter._reader.position(savePosition);

    	                    return ret;
    	                }
    	                catch (Exception e)
    	                {
    	                    return CodecReturnCodes.INCOMPLETE_DATA;
    	                }
    	            case 7:
    	                value.clear();
    	                try
    	                {
    	                    iter._reader.position(iter._curBufPos);
    	                    value.day(iter._reader.readUnsignedByte());
    	                    value.month(iter._reader.readUnsignedByte());
    	                    value.year(iter._reader.readUnsignedShort());
    	                    value.time().hour(iter._reader.readUnsignedByte());
    	                    value.time().minute(iter._reader.readUnsignedByte());
    	                    value.time().second(iter._reader.readUnsignedByte());
    	                    
    	                    ret = CodecReturnCodes.SUCCESS;

    	                    iter._reader.position(savePosition);

    	                    return ret;
    	                    
    	                }
    	                catch (Exception e)
    	                {
    	                    return CodecReturnCodes.INCOMPLETE_DATA;
    	                }
    	            case 8:
    	                return CodecReturnCodes.INCOMPLETE_DATA;
    	            case 9:
    	                value.clear();
    	                try
    	                {
    	                    iter._reader.position(iter._curBufPos);
    	                    value.day(iter._reader.readUnsignedByte());
    	                    value.month(iter._reader.readUnsignedByte());
    	                    value.year(iter._reader.readUnsignedShort());
    	                    value.time().hour(iter._reader.readUnsignedByte());
    	                    value.time().minute(iter._reader.readUnsignedByte());
    	                    value.time().second(iter._reader.readUnsignedByte());
    	                    value.time().millisecond(iter._reader.readUnsignedShort());
    	                    if ((value.date().day() == 0) && (value.date().year() == 0) && (value.date().month() == 0) &&
    	                            (value.time().hour() == 255) && (value.time().minute() == 255) && (value.time().second() == 255) &&
    	                            (value.time().millisecond() == 65535))
    	                            return CodecReturnCodes.BLANK_DATA;
    	                        else
    	                            return CodecReturnCodes.SUCCESS;
    	                }
    	                catch (Exception e)
    	                {
    	                    return CodecReturnCodes.INCOMPLETE_DATA;
    	                }
    	                default:
    	                    return CodecReturnCodes.INCOMPLETE_DATA;                    
    	        }
    	    }
    	};
    	
    	
    	
    	_encIter.clear();
    	_buffer.data(ByteBuffer.allocate(100));
    	_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
    	time.microsecond(654);
    	assertEquals(CodecReturnCodes.SUCCESS, time.encode(_encIter));
    	
    	_decIter.clear();
       	_decIter.setBufferAndRWFVersion(_buffer,  Codec.majorVersion(),  Codec.minorVersion());
    	assertEquals(CodecReturnCodes.INCOMPLETE_DATA, decTime.decode(_decIter));    	
    	
    }
    
    
    

    @Test
    public void timeEncodeDecodeBlankTest()
    {
        Time time = CodecFactory.createTime();

        time.blank();
        assertTrue(time.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.SUCCESS, time.encode(_encIter));

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Time time1 = CodecFactory.createTime();
        time1.decode(_decIter);

        assertEquals(time.hour(), time1.hour());
        assertEquals(time.minute(), time1.minute());
        assertEquals(time.second(), time1.second());
        assertEquals(time.millisecond(), time1.millisecond());
        assertEquals(time.microsecond(), time1.microsecond());
        assertEquals(time.nanosecond(), time1.nanosecond());
    }

    @Test
    public void enumEncodeDecodeTest()
    {
        Enum enumv = CodecFactory.createEnum();

        for (short e = 0; e <= 0xFF; e++)
        {
            enumv.value(e);

            _encIter.clear();
            _buffer.data(ByteBuffer.allocate(15));
            _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            enumv.encode(_encIter);

            _decIter.clear();
            _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

            Enum enumv1 = CodecFactory.createEnum();
            enumv1.decode(_decIter);

            assertEquals(enumv.toInt(), enumv1.toInt());
        }
    }
    
    @Test
    public void enumEncodeDecodeBlankTest()
    {
        EnumImpl enumv = (EnumImpl)CodecFactory.createEnum();
        
        assertFalse(enumv.isBlank());
        enumv.blank();
        assertTrue(enumv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, enumv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.INT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        EnumImpl enum1 = (EnumImpl)CodecFactory.createEnum();
        assertFalse(enum1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, enum1.decode(_decIter));

        assertTrue(enum1.isBlank());
        assertEquals(enumv.toInt(), enum1.toInt());
    }
    
    @Test
    public void enumSetTest()
    {
        Enum thisEnum = CodecFactory.createEnum();

        assertEquals(CodecReturnCodes.SUCCESS, thisEnum.value(0));
        assertEquals(0, thisEnum.toInt());
        assertEquals(CodecReturnCodes.SUCCESS, thisEnum.value(65535));
        assertEquals(65535, thisEnum.toInt());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisEnum.value(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisEnum.value(65536));
    }

    @Test
    public void enumStringTest()
    {
    	String enumStr1 = "0";
    	String enumStr2 = "65535";
    	String enumStr3 = "32000";
    	Enum testEnum = CodecFactory.createEnum();
    	
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testEnum.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.value("    "));
    	assertTrue(testEnum.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testEnum.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.value(""));
    	assertTrue(testEnum.isBlank());
        
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.value(enumStr1));
    	assertEquals(enumStr1, testEnum.toString());
    	assertFalse(testEnum.isBlank());
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.value(enumStr2));
    	assertEquals(enumStr2, testEnum.toString());
    	assertFalse(testEnum.isBlank());
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.value(enumStr3));
    	assertEquals(enumStr3, testEnum.toString());
    	assertFalse(testEnum.isBlank());
    }
    
    private void realED(RealImpl real)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        RealImpl real1 = (RealImpl)CodecFactory.createReal();
        real1.decode(_decIter);

        assertEquals(real.hint(), real1.hint());
        assertEquals(real.toLong(), real1.toLong());
        assertTrue(real.equals(real1));
    }

    @Test
    public void realEncodeDecodeTest()
    {
        RealImpl real = (RealImpl)CodecFactory.createReal();

        for (int hint = 0; hint <= 30; ++hint)
        {
            long val;
            /* leading 0's */
            for (val = 0x5555555555555555l; val != 0; val >>= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            for (val = 0xAAAAAAAAAAAAAAAAl; val != -1; val >>= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            /* trailing 0's */
            for (val = 0xAAAAAAAAAAAAAAAAl; val != 0; val <<= 1)
            {
                real.value(val, hint);
                realED(real);
            }

            real.value(0xFFFFFFFFFFFFFFFFl, hint);
            realED(real);
            
            real.value(0, RealHints.INFINITY);
            realED(real);
            
            real.value(0, RealHints.NEG_INFINITY);
            realED(real);
            
            real.value(0, RealHints.NOT_A_NUMBER);
            realED(real);
            

            real.value(0, hint);
            realED(real);
        }
    }

    @Test
    public void realEncodeDecodeBlankTest()
    {
        RealImpl real = (RealImpl)CodecFactory.createReal();

        real.blank();
        assertTrue(real.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.SUCCESS, real.encode(_encIter));

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        RealImpl real1 = (RealImpl)CodecFactory.createReal();
        real1.decode(_decIter);

        assertEquals(real.hint(), real1.hint());
        assertEquals(real.toLong(), real1.toLong());
        assertTrue(real.equals(real1));
        assertTrue(real1.isBlank());
    }
    
    @Test
    public void realSetTest()
    {
        Real thisReal = CodecFactory.createReal();

        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(-1000000000, RealHints.EXPONENT_14));
        assertEquals(-1000000000, thisReal.toLong());
        assertEquals(RealHints.EXPONENT_14, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(1000000000, RealHints.FRACTION_256));
        assertEquals(1000000000, thisReal.toLong());
        assertEquals(RealHints.FRACTION_256, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(-111222.333444, RealHints.EXPONENT_6));
        assertEquals(-111222.333444, thisReal.toDouble(), 0);
        assertEquals(RealHints.EXPONENT_6, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(111222.333444, RealHints.EXPONENT_6));
        assertEquals(111222.333444, thisReal.toDouble(), 0);
        assertEquals(RealHints.EXPONENT_6, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)-111222.333444, RealHints.EXPONENT_6));
        assertEquals((float)-111222.333444, (float)thisReal.toDouble(), 0);
        assertEquals(RealHints.EXPONENT_6, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)111222.333444, RealHints.EXPONENT_6));
        assertEquals((float)111222.333444, (float)thisReal.toDouble(), 0);
        assertEquals(RealHints.EXPONENT_6, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.EXPONENT_2));
        assertEquals(RealHints.EXPONENT_2, thisReal.hint());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.INFINITY));
        assertEquals(RealHints.INFINITY, thisReal.hint());
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.NEG_INFINITY));
        assertEquals(RealHints.NEG_INFINITY, thisReal.hint());
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.EXPONENT7));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.EXPONENT_12));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value(0, RealHints.EXPONENT_14));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((double)0, RealHints.EXPONENT_2));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((double)0, RealHints.EXPONENT7));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((double)0, RealHints.EXPONENT_12));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((double)0, RealHints.EXPONENT_14));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)0, RealHints.EXPONENT_2));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)0, RealHints.EXPONENT7));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)0, RealHints.EXPONENT_12));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value((float)0, RealHints.EXPONENT_14));
        assertEquals(0, thisReal.toLong());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value("Inf"));
        assertEquals(0, thisReal.toLong());
        assertEquals(RealHints.INFINITY, thisReal.hint());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value("-Inf"));
        assertEquals(0, thisReal.toLong());
        assertEquals(RealHints.NEG_INFINITY, thisReal.hint());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.SUCCESS, thisReal.value("NaN"));
        assertEquals(0, thisReal.toLong());
        assertEquals(RealHints.NOT_A_NUMBER, thisReal.hint());
        assertEquals(false, thisReal.isBlank());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value(-1000000000, -1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value(1000000000, 31));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value(-111222.333444, -1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value(111222.333444, 31));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value((float)-111222.333444, -1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisReal.value((float)111222.333444, 31));
    }
    
    @Test
    public void realStringTest()
    {
    	String realStr1 = "0.0";
    	String realStr2 = "1.1234567";
    	String realStr3 = "100000.0000001";
    	String realStr4 = "1";
    	String realStr5 = "0.0000001";
    	String realStr6 = "+0";
    	String realStr7 = "+0.0";
    	String realStr7a = "+0.0000000000000";
    	String realStr8 = "-123456.7654321";
    	String realStr9 = "-0.0";
    	String realStr10 = "-10";
    	String realStr11 = "1/4";
    	String realStr12 = "-1/4";
    	String realStr13 = "100 1/4";
    	String realStr14 = "-1000 1/4";
    	String realStr15 = "  -0.9999999             ";
    	String realStr16 = "-9223372036854775808";
    	String realStr17 = "9223372036854775807";
    	String realStr18 = "922337203685477.5807";
    	String realStr19 = "-922337203685477.5808";
    	String realStr20 = "92233720368547757 1/4";
    	String realStr21 = "-92233720368547758 1/4";
    	String realStr22 = "Inf";
    	String realStr23 = "-Inf";
    	String realStr24 = "NaN";
    	Real testReal = CodecFactory.createReal();
    	double tempDouble;

    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testReal.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, testReal.value("   "));
    	assertTrue(testReal.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testReal.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testReal.value(""));
    	assertTrue(testReal.isBlank());
    	
    	testReal.value(realStr1);
    	tempDouble = java.lang.Double.parseDouble(realStr1);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr2);
    	tempDouble = java.lang.Double.parseDouble(realStr2);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr3);
    	tempDouble = java.lang.Double.parseDouble(realStr3);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr4);
    	tempDouble = java.lang.Double.parseDouble(realStr4);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr5);
    	tempDouble = java.lang.Double.parseDouble(realStr5);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr6);
    	assertTrue(testReal.isBlank());
    	testReal.value(realStr7);
    	assertTrue(testReal.isBlank());
    	testReal.value(realStr7a);
    	assertTrue(testReal.isBlank());
    	testReal.value(realStr8);
    	tempDouble = java.lang.Double.parseDouble(realStr8);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr9);
    	tempDouble = java.lang.Double.parseDouble(realStr9);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr10);
    	tempDouble = java.lang.Double.parseDouble(realStr10);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr11);
    	tempDouble = 0.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr12);
    	tempDouble = -0.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr13);
    	tempDouble = 100.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr14);
    	tempDouble = -1000.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr15);
    	tempDouble = java.lang.Double.parseDouble(realStr15);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr16);
    	assertEquals(-9223372036854775808L, testReal.toLong());
    	assertEquals(RealHints.EXPONENT0, testReal.hint());
    	tempDouble = java.lang.Double.parseDouble(realStr16);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr17);
    	assertEquals(9223372036854775807L, testReal.toLong());
    	assertEquals(RealHints.EXPONENT0, testReal.hint());
    	tempDouble = java.lang.Double.parseDouble(realStr17);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr18);
    	assertEquals(9223372036854775807L, testReal.toLong());
    	assertEquals(RealHints.EXPONENT_4, testReal.hint());
    	tempDouble = java.lang.Double.parseDouble(realStr18);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr19);
    	assertEquals(-9223372036854775808L, testReal.toLong());
    	assertEquals(RealHints.EXPONENT_4, testReal.hint());
    	tempDouble = java.lang.Double.parseDouble(realStr19);
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr20);
    	tempDouble = 92233720368547757.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr21);
    	tempDouble = -92233720368547758.25;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr22);
    	tempDouble = java.lang.Double.POSITIVE_INFINITY;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr23);
    	tempDouble = java.lang.Double.NEGATIVE_INFINITY;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    	testReal.value(realStr24);
    	tempDouble = java.lang.Double.NaN;
    	assertEquals(testReal.toDouble(), tempDouble, 0);
    	assertFalse(testReal.isBlank());
    }

    private void qosED(QosImpl qos)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        qos.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Qos qos1 = CodecFactory.createQos();
        qos1.decode(_decIter);

        assertEquals(qos.rate(), qos1.rate());
        assertEquals(qos.rateInfo(), qos1.rateInfo());
        assertEquals(qos.timeInfo(), qos1.timeInfo());
        assertEquals(qos.timeliness(), qos1.timeliness());
        assertEquals(qos.isDynamic(), qos1.isDynamic());
    }
    
    @Test
    public void qosEncodeDecodeBlankTest()
    {
        QosImpl qosv = (QosImpl)CodecFactory.createQos();
        
        assertFalse(qosv.isBlank());
        qosv.blank();
        assertTrue(qosv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, qosv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.INT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        QosImpl qos1 = (QosImpl)CodecFactory.createQos();
        assertFalse(qos1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, qos1.decode(_decIter));

        assertTrue(qos1.isBlank());
    }
    
    @Test
    public void qosSetTest()
    {
        Qos thisQos = CodecFactory.createQos();

        assertEquals(CodecReturnCodes.SUCCESS, thisQos.rate(QosRates.UNSPECIFIED));
        assertEquals(QosRates.UNSPECIFIED, thisQos.rate());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.timeliness(QosTimeliness.UNSPECIFIED));
        assertEquals(QosTimeliness.UNSPECIFIED, thisQos.timeliness());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.rateInfo(0));
        assertEquals(0, thisQos.rateInfo());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.timeInfo(0));
        assertEquals(0, thisQos.timeInfo());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.rate(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.timeliness(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.rateInfo(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.timeInfo(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.rate(16));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.timeliness(8));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.rateInfo(65536));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisQos.timeInfo(65536));
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.rate(QosRates.TIME_CONFLATED));
        assertEquals(QosRates.TIME_CONFLATED, thisQos.rate());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.timeliness(QosTimeliness.DELAYED));
        assertEquals(QosTimeliness.DELAYED, thisQos.timeliness());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.rateInfo(65535));
        assertEquals(65535, thisQos.rateInfo());
        assertEquals(CodecReturnCodes.SUCCESS, thisQos.timeInfo(65535));
        assertEquals(65535, thisQos.timeInfo());
    }
    
    @Test
    public void qosEncodeDecodeTest()
    {
        int[] rates = { QosRates.UNSPECIFIED,
                QosRates.TICK_BY_TICK,
                QosRates.JIT_CONFLATED,
                QosRates.TIME_CONFLATED };
        int[] timeliness = { QosTimeliness.UNSPECIFIED,
                QosTimeliness.REALTIME,
                QosTimeliness.DELAYED_UNKNOWN,
                QosTimeliness.DELAYED };
        Qos qos = CodecFactory.createQos();
        int rateInfo = 0;
        int timeInfo = 0;

        for (int rate = 0; rate < rates.length; rate++)
        {
            for (int tm = 0; tm < timeliness.length; tm++)
            {
                if (rates[rate] == QosRates.TIME_CONFLATED)
                {
                    for (rateInfo = 0x007f; rateInfo == 0; rateInfo >>= 1)
                    {
                        if (timeliness[tm] == QosTimeliness.DELAYED)
                        {
                            for (timeInfo = 0x007f; timeInfo == 0; timeInfo >>= 1)
                            {
                                qos.rate(rates[rate]);
                                qos.timeliness(timeliness[tm]);
                                qos.rateInfo(rateInfo);
                                qos.timeInfo(timeInfo);
                                qos.dynamic(false);
                                qosED((QosImpl)qos);

                                qos.rate(rates[rate]);
                                qos.timeliness(timeliness[tm]);
                                qos.rateInfo(rateInfo);
                                qos.timeInfo(timeInfo);
                                qos.dynamic(true);
                                qosED((QosImpl)qos);
                            }
                        }
                    }
                }
            }
        }
    }
    
    @Test
    public void qosEqualsTest()
    {
        Qos qos1 = CodecFactory.createQos();
        Qos qos2 = CodecFactory.createQos();
        
        assertTrue(qos1.equals(qos2));
        qos1.rate(QosRates.TICK_BY_TICK);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.rate(QosRates.JIT_CONFLATED);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.rate(QosRates.TIME_CONFLATED);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.timeliness(QosTimeliness.REALTIME);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.timeliness(QosTimeliness.DELAYED_UNKNOWN);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.timeliness(QosTimeliness.DELAYED);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.rateInfo(1000);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
        qos1.timeInfo(2000);
        assertFalse(qos1.equals(qos2));
        qos1.clear();
        assertTrue(qos1.equals(qos2));
    }

    @Test
    public void qosIsBetterTest()
    {
        Qos qos1 = CodecFactory.createQos();
        Qos qos2 = CodecFactory.createQos();
        
        assertFalse(qos1.isBetter(qos2));
        qos1.rate(QosRates.TICK_BY_TICK);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.rate(QosRates.JIT_CONFLATED);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.rate(QosRates.TIME_CONFLATED);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.timeliness(QosTimeliness.REALTIME);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.timeliness(QosTimeliness.DELAYED_UNKNOWN);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.timeliness(QosTimeliness.DELAYED);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.rate(QosRates.TIME_CONFLATED);
        qos1.rateInfo(1000);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
        qos1.timeliness(QosTimeliness.DELAYED);
        qos1.timeInfo(2000);
        assertTrue(qos1.isBetter(qos2));
        qos1.clear();
        assertFalse(qos1.isBetter(qos2));
    }

    @Test
    public void qosIsInRangeTest()
    {
        Qos qos = CodecFactory.createQos();
        Qos bestQos = CodecFactory.createQos();
        Qos worstQos = CodecFactory.createQos();

        ((QosImpl)bestQos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)bestQos)._rateInfo = 65532;
        ((QosImpl)bestQos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)bestQos)._timeInfo = 65533;
        ((QosImpl)worstQos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)worstQos)._rateInfo = 65534;
        ((QosImpl)worstQos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)worstQos)._timeInfo = 65535;
        
        assertFalse(qos.isInRange(bestQos, worstQos));
        ((QosImpl)qos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)qos)._rateInfo = 65531;
        ((QosImpl)qos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)qos)._timeInfo = 65532;
        assertFalse(qos.isInRange(bestQos, worstQos));
        ((QosImpl)qos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)qos)._rateInfo = 65535;
        ((QosImpl)qos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)qos)._timeInfo = 65536;
        assertFalse(qos.isInRange(bestQos, worstQos));
        qos.clear();
        assertFalse(qos.isInRange(bestQos, worstQos));
        ((QosImpl)qos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)qos)._rateInfo = 65532;
        ((QosImpl)qos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)qos)._timeInfo = 65533;
        assertTrue(qos.isInRange(bestQos, worstQos));
        qos.clear();
        assertFalse(qos.isInRange(bestQos, worstQos));
        ((QosImpl)qos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)qos)._rateInfo = 65533;
        ((QosImpl)qos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)qos)._timeInfo = 65534;
        assertTrue(qos.isInRange(bestQos, worstQos));
        qos.clear();
        assertFalse(qos.isInRange(bestQos, worstQos));
        ((QosImpl)qos)._rate = QosRates.TIME_CONFLATED;
        ((QosImpl)qos)._rateInfo = 65534;
        ((QosImpl)qos)._timeliness = QosTimeliness.DELAYED;
        ((QosImpl)qos)._timeInfo = 65535;
        assertTrue(qos.isInRange(bestQos, worstQos));       
    }

    private void stateED(StateImpl state)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        state.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        State state1 = CodecFactory.createState();
        state1.decode(_decIter);

        assertEquals(state.code(), state1.code());
        assertEquals(state.dataState(), state1.dataState());
        assertEquals(state.streamState(), state1.streamState());
        assertEquals(state.text(), state1.text());
    }
    
    @Test
    public void stateEncodeDecodeBlankTest()
    {
        StateImpl statev = (StateImpl)CodecFactory.createState();
        
        assertFalse(statev.isBlank());
        statev.blank();
        assertTrue(statev.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, statev.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.INT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        StateImpl state1 = (StateImpl)CodecFactory.createState();
        assertFalse(state1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, state1.decode(_decIter));

        assertTrue(state1.isBlank());
    }
    
    @Test
    public void stateSetTest()
    {
        State thisState = CodecFactory.createState();
        Buffer buffer = CodecFactory.createBuffer();

        assertEquals(CodecReturnCodes.SUCCESS, thisState.code(StateCodes.NONE));
        assertEquals(StateCodes.NONE, thisState.code());
        assertEquals(CodecReturnCodes.SUCCESS, thisState.dataState(DataStates.NO_CHANGE));
        assertEquals(DataStates.NO_CHANGE, thisState.dataState());
        assertEquals(CodecReturnCodes.SUCCESS, thisState.streamState(StreamStates.UNSPECIFIED));
        assertEquals(StreamStates.UNSPECIFIED, thisState.streamState());
        buffer.data("");
        assertEquals(CodecReturnCodes.SUCCESS, thisState.text(buffer));
        assertEquals("", thisState.text().toString());
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.code(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.dataState(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.streamState(-1));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.text(null));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.code(128));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.dataState(8));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, thisState.streamState(32));
        assertEquals(CodecReturnCodes.SUCCESS, thisState.code(StateCodes.UNABLE_TO_REQUEST_AS_BATCH));
        assertEquals(StateCodes.UNABLE_TO_REQUEST_AS_BATCH, thisState.code());
        assertEquals(CodecReturnCodes.SUCCESS, thisState.dataState(DataStates.SUSPECT));
        assertEquals(DataStates.SUSPECT, thisState.dataState());
        assertEquals(CodecReturnCodes.SUCCESS, thisState.streamState(StreamStates.REDIRECTED));
        assertEquals(StreamStates.REDIRECTED, thisState.streamState());
        buffer.data("1234567890");
        assertEquals(CodecReturnCodes.SUCCESS, thisState.text(buffer));
        assertEquals("1234567890", thisState.text().toString());
    }

    @Test
    public void stateEncodeDecodeTest()
    {
        int[] ss = { StreamStates.UNSPECIFIED,
                StreamStates.OPEN,
                StreamStates.NON_STREAMING,
                StreamStates.CLOSED_RECOVER,
                StreamStates.CLOSED,
                StreamStates.REDIRECTED };
        int[] ds = { DataStates.NO_CHANGE,
                DataStates.OK,
                DataStates.SUSPECT };

        State state = CodecFactory.createState();
        int code; // -34 - 19
        Buffer s1 = CodecFactory.createBuffer();
        s1.data("abc");
        String s2 = "qwertyu";
        Buffer s = CodecFactory.createBuffer();
        s.data(s2);

        for (int i = 0; i < ss.length; i++)
        {
            for (int j = 0; j < ds.length; j++)
            {
                for (code = -34; code == 19; code++)
                {
                    state.code(code);
                    state.dataState(ds[j]);
                    state.streamState(ss[i]);
                    stateED((StateImpl)state);

                    state.text(s1);
                    stateED((StateImpl)state);

                    state.text(s);
                    stateED((StateImpl)state);
                }
            }
        }
    }

    @Test
    public void stateCopyTest()
    {
    	State testState  = CodecFactory.createState();
    	State copiedState = CodecFactory.createState();
    	testState.streamState(StreamStates.REDIRECTED);
    	testState.dataState(DataStates.SUSPECT);
    	testState.text().data("testString");
    	testState.code(StateCodes.FULL_VIEW_PROVIDED);
    	assertEquals(StreamStates.REDIRECTED, testState.streamState());
    	assertEquals(StateCodes.FULL_VIEW_PROVIDED, testState.code());
    	assertEquals(DataStates.SUSPECT, testState.dataState());
    	
    	testState.copy(copiedState);
    	assertEquals(StreamStates.REDIRECTED, copiedState.streamState());
    	assertEquals(DataStates.SUSPECT, copiedState.dataState());
    	assertEquals(StateCodes.FULL_VIEW_PROVIDED, copiedState.code());
    	assertEquals(testState.text().toString(), copiedState.text().toString());
    	assertNotSame(testState, copiedState);

    	testState.text().data("testString123");
        testState.copy(copiedState);
        assertEquals(StreamStates.REDIRECTED, copiedState.streamState());
        assertEquals(DataStates.SUSPECT, copiedState.dataState());
        assertEquals(StateCodes.FULL_VIEW_PROVIDED, copiedState.code());
        assertEquals(testState.text().toString(), copiedState.text().toString());
    }
    
    
    private void bufferED(Buffer buf)
    {
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        buf.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        Buffer buf1 = CodecFactory.createBuffer();
        buf1.data(ByteBuffer.allocate(15));
        buf1.decode(_decIter);

        assertEquals(buf.toString(), buf1.toString());
        assertTrue(buf.equals(buf1));
    }

    @Test
    public void bufEncodeDecodeBlankTest()
    {
        BufferImpl bufv = (BufferImpl)CodecFactory.createBuffer();
        
        assertFalse(bufv.isBlank());
        bufv.blank();
        assertTrue(bufv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, bufv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.INT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        BufferImpl buf1 = (BufferImpl)CodecFactory.createBuffer();
        assertFalse(buf1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, buf1.decode(_decIter));

        assertTrue(buf1.isBlank());
    }
    
    @Test
    public void bufferEncodeDecodeTest()
    {
        byte[] data = { 0, 1, 2, 3, 4, 5, 6, 7 };
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.wrap(data));
        bufferED(buf);

        byte[] data1 = { 0, 1, 2 };
        buf.data(ByteBuffer.wrap(data1));
        bufferED(buf);

        buf = CodecFactory.createBuffer();
        buf.data("abhytgcdty");
        bufferED(buf);

        buf = CodecFactory.createBuffer();
        buf.data("abhytgcdty");
        bufferED(buf);
    }
    
    @Test
    public void floatingPointTests()
    {	
    	float maxFloatConstant = 340282346638528860000000000000000000000.0f;
    	float minFloatConstant = 0.0000000000000000000000000000000000000000000014012984643248171f;
    	Float maxFloat = CodecFactory.createFloat();
    	maxFloat.value(340282346638528860000000000000000000000.0f);
    	Float minFloat = CodecFactory.createFloat();
    	minFloat.value(0.0000000000000000000000000000000000000000000014012984643248171f);
    	Float testFloat32 = CodecFactory.createFloat();
    	testFloat32.value(160141100000000000000000000000000000000.0f);
    	Float testFloat32min = CodecFactory.createFloat();
    	testFloat32min.value(0.0000000000000000000000000000000000000000000024012984643248171f);
    	Double testFloat64 = CodecFactory.createDouble();
    	testFloat64.value(440282346638528860000000000000000000000.0);
    	
    	Double testFloat64min = CodecFactory.createDouble();
    	testFloat64min.value(0.00000000000000000000000000000000000000000000040129846432481707);
    	
    	Double testCast64a = CodecFactory.createDouble();
    	testCast64a.value(160141100000000000000000000000000000000.0f);
    	
    	Double testCast64b = CodecFactory.createDouble();
    	testCast64b.value(440282346638528860000000000000000000000.0);

    	Float testFloat = CodecFactory.createFloat();
    	testFloat.value((float)testCast64b.toDouble()); //This is from a float that is too big
    	Float testFloat2 = CodecFactory.createFloat();
    	testFloat2.value((float)testCast64a.toDouble()); //This is from a valid 32 bit float

    	System.out.printf("Display float value to make sure this sets properly:\n");
    	System.out.printf("%f\n", maxFloat.toFloat());
    	System.out.printf("%e\n", maxFloat.toFloat());
    	System.out.printf("%2.64f\n", minFloat.toFloat());
    	System.out.printf("%e\n", minFloat.toFloat());
    	
    	System.out.printf("\nCompare maximum float values now:\n");
    	assertTrue("Greater Than test", maxFloat.toFloat()  > testFloat32.toFloat());
    	assertTrue("Greater Than Constant test", maxFloatConstant > testFloat32.toFloat());
    	assertTrue("Less Than test", testFloat32.toFloat() < maxFloat.toFloat());

    	System.out.printf("\nCompare minimum float values now:\n");
    	assertTrue("Less Than test", minFloat.toFloat() < testFloat32min.toFloat());
    	assertTrue("Less Than test 2", minFloat.toFloat() < testFloat32.toFloat());
    	assertTrue("Less Than Constant test", minFloatConstant < testFloat32min.toFloat());
    	assertTrue("Min 32 Greater Than Smaller 64", minFloat.toFloat() > testFloat64min.toDouble());

    	System.out.printf("\nCompare 64 and 32 bit values now:\n");
    	assertTrue("64 Greater Than 32 test", testFloat64.toDouble() > maxFloat.toFloat());
    	assertTrue("Min Float Less Than 64 test", minFloat.toFloat() < testFloat64.toDouble());

    	System.out.printf("\nDisplay Float Cast Tests\n");
    	System.out.printf("%f\n", testCast64a.toDouble());
    	System.out.printf("%f\n", ((float)testCast64a.toDouble()));
    	
    	System.out.printf("\nCompare cast float values now:\n");
    	assertTrue("Equality After Casting 64 that fits in 32 test ", testCast64a.toDouble() == testFloat2.toFloat());
    	assertTrue("Equality After Casting 64 that doesnt fit test", ((testCast64b.toDouble() == testFloat.toFloat()) ? false : true));
    }
    
    public ByteBuffer putString(String s) 
    {
    	ByteBuffer tmp = ByteBuffer.allocate(127);
    	tmp.order(ByteOrder.LITTLE_ENDIAN);
    	for (int i = 0; i < s.length(); ++i)
    	{
    		tmp.putChar(i, s.charAt(i));
    	}
    	return tmp;
    }
    
    @Test
    public void stringCompareTests()
    {
    	ByteBuffer buf1;
    	buf1 = putString("Test\0garbage");
    	Buffer buffer1 = CodecFactory.createBuffer();
    	buffer1.data(buf1, 0, 0);
    	ByteBuffer buf2;
    	buf2 = putString("tEST\0moreGarbage");
    	Buffer buffer2 = CodecFactory.createBuffer();
    	buffer2.data(buf2, 0, 0);
    	ByteBuffer buf3;
    	buf3 = putString("tESTer");
    	Buffer buffer3 = CodecFactory.createBuffer();
    	buffer3.data(buf3, 0, 0);
    	
    	/* All zero length */
    	assertTrue(buffer1.equals(buffer2));
    	assertTrue(buffer1.equals(buffer3));
    	assertTrue(buffer2.equals(buffer3));
    	
    	/* Length vs. No length */
    	buffer1.data(buf1, 0, 1);
    	assertTrue(!buffer1.equals(buffer2));
    	assertTrue(!buffer1.equals(buffer3));
    	assertTrue(!buffer2.equals(buffer1));
       	assertTrue(!buffer3.equals(buffer1));
       	
    	/* Test vs. teST vs. teST */
    	buffer1.data(buf1, 0, 4);
    	buffer2.data(buf2, 0, 4);
    	buffer3.data(buf3, 0, 4);
    	
    	assertTrue(buffer1.equals(buffer1));
    	assertTrue(buffer2.equals(buffer2));
    	assertTrue(buffer3.equals(buffer3));
    	
    	assertTrue(!buffer1.equals(buffer2));
    	assertTrue(!buffer1.equals(buffer3));
    	assertTrue(buffer2.equals(buffer3));
    	
    	/* Test\0 vs. teST\0 vs. teSTe */
    	buffer1.data(buf1, 0, 5);
    	buffer2.data(buf2, 0, 5);
    	buffer3.data(buf3, 0, 5);
    	
    	assertTrue(buffer1.equals(buffer1));
    	assertTrue(buffer2.equals(buffer2));
    	assertTrue(buffer3.equals(buffer3));
    	
    	assertTrue(!buffer1.equals(buffer2));
    	assertTrue(!buffer2.equals(buffer1));
    	assertTrue(!buffer1.equals(buffer3));
    	assertTrue(!buffer2.equals(buffer3));
    	assertTrue(!buffer3.equals(buffer1));
    	assertTrue(!buffer3.equals(buffer2));

    	/* Test\0g vs. teST\0m vs. teSTer */ 
    	buffer1.data(buf1, 0, 6);
    	buffer2.data(buf2, 0, 6);
    	buffer3.data(buf3, 0, 6);
    	
    	assertTrue(buffer1.equals(buffer1));
    	assertTrue(buffer2.equals(buffer2));
    	assertTrue(buffer3.equals(buffer3));
    	
    	assertTrue(!buffer1.equals(buffer2));
    	assertTrue(!buffer2.equals(buffer1));
    	assertTrue(!buffer1.equals(buffer3));
    	assertTrue(!buffer2.equals(buffer3));
    	assertTrue(!buffer3.equals(buffer1));
    	assertTrue(!buffer3.equals(buffer2));

    }
    
    @Test
    public void stringConversionTest()
    {
    	ByteBuffer testDataBuffer, testStrBuffer;
    	Buffer testDataBuf  = CodecFactory.createBuffer(), testStrBuf  = CodecFactory.createBuffer();
    	Buffer testDataOutBuf = CodecFactory.createBuffer();
    	char[] testData = new char[128];
    	char[] testString = new char[128];
    	char[] testStringSmall = new char[1];
    	char[] textData = "test state text".toCharArray();
    	
    	int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
    	
		EncodeIterator encIter = CodecFactory.createEncodeIterator();
		DecodeIterator decIter = CodecFactory.createDecodeIterator();
    	Int testInt = CodecFactory.createInt(), testIntOut  = CodecFactory.createInt();
    	UInt testUInt  = CodecFactory.createUInt(), testUIntOut  = CodecFactory.createUInt();
    	Float testFloat  = CodecFactory.createFloat(), testFloatOut  = CodecFactory.createFloat();
    	Double testDouble  = CodecFactory.createDouble(), testDoubleOut  = CodecFactory.createDouble();
    	Real testReal  = CodecFactory.createReal(), testRealOut  = CodecFactory.createReal();
    	Date testDate  = CodecFactory.createDate(), testDateOut  = CodecFactory.createDate();
    	Time testTime  = CodecFactory.createTime(), testTimeOut  = CodecFactory.createTime();
    	DateTime testDateTime  = CodecFactory.createDateTime(), testDateTimeOut  = CodecFactory.createDateTime();
    	Qos testQos  = CodecFactory.createQos();
    	State testState  = CodecFactory.createState();
    	Enum testEnum  = CodecFactory.createEnum(), testEnumOut  = CodecFactory.createEnum();
    	ByteBuffer buffer1;
    	buffer1 = putString("Refinitiv");
    	Buffer testBuffer = CodecFactory.createBuffer();
    	testBuffer.data(buffer1, 0, "Refinitiv".length());
    	
    	/* Int conversion test */
    	testInt.value(987654321);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testInt.encode(encIter));
    	decIter.clear();
        testInt.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testInt.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testInt, DataTypes.INT, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testInt, DataTypes.INT, testStrBuf));
    	testIntOut.value(testStrBuf.toString());
    	assertEquals(testInt.toString(), testIntOut.toString());
    	
    	/* UInt conversion test */
    	testUInt.value(1234567891);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testUInt.encode(encIter));
    	decIter.clear();
    	testUInt.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testUInt.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testUInt, DataTypes.UINT, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testUInt, DataTypes.UINT, testStrBuf));
    	testUIntOut.value(testStrBuf.toString());
    	assertEquals(testUInt.toString(), testUIntOut.toString());
    	
    	/* Float conversion test */
    	testFloat.value(123.456f);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testFloat.encode(encIter));
    	decIter.clear();
        testFloat.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testFloat.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testFloat, DataTypes.FLOAT, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testFloat, DataTypes.FLOAT, testStrBuf));
    	testFloatOut.value(testStrBuf.toString());
    	assertEquals(testFloat.toString(), testFloatOut.toString());
    	
    	/* Double conversion test */
    	testDouble.value(98765.4321);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDouble.encode(encIter));
    	decIter.clear();
        testDouble.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDouble.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDouble, DataTypes.DOUBLE, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDouble, DataTypes.DOUBLE, testStrBuf));
    	testDoubleOut.value(testStrBuf.toString());
    	assertEquals(testDouble.toString(), testDoubleOut.toString());
    	
    	/* Real conversion test */
    	testReal.value("12345.6789012");
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testReal.encode(encIter));
    	decIter.clear();
        testReal.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testReal.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testReal, DataTypes.REAL, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testReal, DataTypes.REAL, testStrBuf));
    	testRealOut.value(testStrBuf.toString());
    	assertEquals(testReal.toString(), testRealOut.toString());
    	
    	/* Date conversion test */
    	testDate.value("05 FEB 1900");
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDate.encode(encIter));
    	decIter.clear();
        testDate.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDate.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDate, DataTypes.DATE, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDate, DataTypes.DATE, testStrBuf));
    	testDateOut.value(testStrBuf.toString());
    	assertEquals(testDate.toString(), testDateOut.toString());
    	
    	/* Time conversion test */
    	testTime.value("01:02:03:04:05:06");
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testTime.encode(encIter));
    	decIter.clear();
        testTime.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testTime.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testTime, DataTypes.TIME, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testTime, DataTypes.TIME, testStrBuf));
    	testTimeOut.value(testStrBuf.toString());
    	assertEquals(testTime.toString(), testTimeOut.toString());
    	
    	
    	/* Invalid time conversions start */
    	/* Time conversion test */
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("-1:35:32:000:9:8"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("998:35:32:000:1000:1000"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("11:999:32:000:5:1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("11:-12:32:000:5:1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("11:2:999:000:5:1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("11:2:-0:000:5:1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value(":::000:5:1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("1:2:3:4::8191"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("23:35:32:000:"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("23:35:32:abc:111"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("00:00:00:000:000:-1"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("00:00:00:000:000:1000"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("23:35:32:000:-1:000"));
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testTime.value("23:35:32:000:1000:1000"));    	    	
    	/* Invalid time conversions end */
    	    	    	    
    	
    	/* DateTime conversion tests */
    	testDateTime.value("10 OCT 2010 01:02:03:04:00:05");
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDateTime.encode(encIter));
    	decIter.clear();
        testDateTime.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDateTime.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDateTime, DataTypes.DATETIME, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDateTime, DataTypes.DATETIME, testStrBuf));
    	testDateTimeOut.value(testStrBuf.toString());
    	assertEquals(testDateTime.toString(), testDateTimeOut.toString());
    	
    	
    	/* DateTime conversion tests */
    	testDateTime.value("10 OCT 2010 01:02:03:04");
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDateTime.encode(encIter));
    	decIter.clear();
        testDateTime.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDateTime.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDateTime, DataTypes.DATETIME, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDateTime, DataTypes.DATETIME, testStrBuf));
    	testDateTimeOut.value(testStrBuf.toString());
    	assertEquals(testDateTime.toString(), testDateTimeOut.toString());
    	
    	
    	/* Qos conversion test */
    	testQos.dynamic(true);
    	testQos.rate(QosRates.TIME_CONFLATED);
    	testQos.rateInfo(123);
    	testQos.timeInfo(456);
    	testQos.timeliness(QosTimeliness.DELAYED);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testQos.encode(encIter));
    	decIter.clear();
        testQos.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testQos.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testQos, DataTypes.QOS, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testQos, DataTypes.QOS, testStrBuf));
    	
    	/* State conversion test */
    	testState.streamState(StreamStates.REDIRECTED);
    	testState.dataState(DataStates.SUSPECT);
    	testState.code(StateCodes.FULL_VIEW_PROVIDED);
    	ByteBuffer textDataBuf;
    	textDataBuf = putString(textData.toString());
    	Buffer textDataBuffer = CodecFactory.createBuffer();
    	textDataBuffer.data(textDataBuf);
    	testState.text(textDataBuffer);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	ByteBuffer tmp = ByteBuffer.allocate(600);
    	tmp.order(ByteOrder.LITTLE_ENDIAN);
    	for (int i = 0; i < testData.toString().length(); ++i)
    	{
    		tmp.putChar(i, testData.toString().charAt(i));
    	}
    	testDataBuffer = tmp;
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testState.encode(encIter));
    	decIter.clear();
        testState.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testState.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testState, DataTypes.STATE, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testState, DataTypes.STATE, testStrBuf));

    	/* Enum conversion test */
    	testEnum.value(9);
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.encode(encIter));
    	decIter.clear();
        testEnum.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testEnum.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testEnum, DataTypes.ENUM, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testEnum, DataTypes.ENUM, testStrBuf));
    	testEnumOut.value(testStrBuf.toString());
    	assertEquals(testEnum.toString(), testEnumOut.toString());
    	
    	/* Buffer conversion test */
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString()); //if i set position to string length
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testBuffer.encode(encIter));
    	decIter.clear();
    	testDataOutBuf.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDataOutBuf.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.BUFFER, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.BUFFER, testStrBuf));
    	assertEquals(testBuffer, testStrBuf);
    	
    	/* ASCII_STRING conversion test */
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testBuffer.encode(encIter));
    	decIter.clear();
    	testDataOutBuf.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDataOutBuf.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.ASCII_STRING, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.ASCII_STRING, testStrBuf));
        assertEquals(testBuffer, testStrBuf);
    	
    	/* UTF8_STRING conversion test */
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testBuffer.encode(encIter));
    	decIter.clear();
    	testDataOutBuf.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDataOutBuf.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.UTF8_STRING, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.UTF8_STRING, testStrBuf));
    	assertEquals(testBuffer, testStrBuf);
    	
    	/* RMTES_STRING conversion test */
    	testDataBuf.clear();
    	testDataBuffer = putString(testData.toString());
    	testDataBuf.data(testDataBuffer);
    	encIter.clear();
    	assertEquals(CodecReturnCodes.SUCCESS, encIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testBuffer.encode(encIter));
    	decIter.clear();
    	testDataOutBuf.clear();
    	assertEquals(CodecReturnCodes.SUCCESS,  decIter.setBufferAndRWFVersion(testDataBuf, majorVersion, minorVersion));
    	assertEquals(CodecReturnCodes.SUCCESS, testDataOutBuf.decode(decIter));
    	testStrBuf.clear();
    	testStrBuffer = putString(testStringSmall.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.RMTES_STRING, testStrBuf));
    	/* should pass */
    	testStrBuf.clear();
    	testStrBuffer = putString(testString.toString());
    	testStrBuf.data(testStrBuffer);
    	assertEquals(CodecReturnCodes.SUCCESS, Decoders.primitiveToString(testDataOutBuf, DataTypes.RMTES_STRING, testStrBuf));
    	assertEquals(testBuffer, testStrBuf);
    }

    private void floatED(Float putVal)
    {
    	Float getVal = CodecFactory.createFloat();

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        putVal.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

    	assertEquals(CodecReturnCodes.SUCCESS, getVal.decode(_decIter));

        assertTrue(putVal.equals(getVal));
    }

    private void doubleED(Double putVal)
    {
        Double getVal = CodecFactory.createDouble();

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        putVal.encode(_encIter);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.SUCCESS, getVal.decode(_decIter));

        assertTrue(putVal.equals(getVal));
    }
    
    @Test
    public void doubleEncodeDecodeBlankTest()
    {
        DoubleImpl doublev = (DoubleImpl)CodecFactory.createDouble();
        
        assertFalse(doublev.isBlank());
        doublev.blank();
        assertTrue(doublev.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, doublev.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.UINT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        DoubleImpl double1 = (DoubleImpl)CodecFactory.createDouble();
        assertFalse(double1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, double1.decode(_decIter));

        assertTrue(double1.isBlank());
    }
    
    @Test
    public void doubleSetTest()
    {
        Double thisDouble = CodecFactory.createDouble();

        thisDouble.value(111222.333444);
        assertEquals(111222.333444, thisDouble.toDouble(), 0);
    }

    @Test 
    public void doubleStringTest()
    {
        Double testDouble = CodecFactory.createDouble();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testDouble.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, testDouble.value("    "));
    	assertTrue(testDouble.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testDouble.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testDouble.value(""));
    	assertTrue(testDouble.isBlank());
        
    	assertEquals(CodecReturnCodes.SUCCESS, testDouble.value("11.22"));
    	assertEquals("11.22", testDouble.toString());
    	assertFalse(testDouble.isBlank());
    }
    
    @Test
    public void floatEncDecTest()
    {
    	Double ii, jj;
    	ii = CodecFactory.createDouble();
    	jj = CodecFactory.createDouble();
    	
    	Float putVal32 = CodecFactory.createFloat();
    	Double putVal = CodecFactory.createDouble();
    	
    	for (ii.value(-50); ii.toDouble() <= 50; ii.value(ii.toDouble()+1))
    	{
    		for (jj.value(1.0); jj.toDouble() < 10.0; jj.value(jj.toDouble() + 0.0001111111))
    		{
    			// positive
    			putVal32.value((float)(jj.toDouble()*Math.pow(10, ii.toDouble())));
    			floatED(putVal32);

    			// negative
    			putVal32.value((float)(-jj.toDouble()*Math.pow(10, ii.toDouble())));
    			floatED(putVal32);
    		}
    	}
        
        // test effective exponent range
        for (ii.value(-308); ii.toDouble() <= 307; ii.value(ii.toDouble()+1))
        {
            for (jj.value(1.0); jj.toDouble() < 10.0; jj.value(jj.toDouble() + 0.0001111111))
            {
                // positive
                putVal.value(jj.toDouble()*Math.pow(10, ii.toDouble()));
                doubleED(putVal);

                // negative
                putVal.value(-jj.toDouble()*Math.pow(10, ii.toDouble()));
                doubleED(putVal);
            }
        }
    }
    
    @Test
    public void floatEncodeDecodeBlankTest()
    {
        FloatImpl floatv = (FloatImpl)CodecFactory.createFloat();
        
        assertFalse(floatv.isBlank());
        floatv.blank();
        assertTrue(floatv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, floatv.encode(_encIter));

        FieldList list = CodecFactory.createFieldList();
        list.applyHasStandardData();
        list.encodeInit(_encIter, null, 0);
        FieldEntry entry = CodecFactory.createFieldEntry();
        entry.fieldId(1);
        entry.dataType(DataTypes.UINT);
        entry.encodeBlank(_encIter);
        list.encodeComplete(_encIter, true);

        _decIter.clear();
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        list.decode(_decIter, null);
        entry.decode(_decIter);
        FloatImpl float1 = (FloatImpl)CodecFactory.createFloat();
        assertFalse(float1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, float1.decode(_decIter));

        assertTrue(float1.isBlank());
    }
    
    @Test
    public void floatSetTest()
    {
        Float thisFloat = CodecFactory.createFloat();

        thisFloat.value((float)111222.333444);
        assertEquals((float)111222.333444, thisFloat.toFloat(), 0);
    }

    @Test 
    public void floatStringTest()
    {
        Float testFloat = CodecFactory.createFloat();
        
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testFloat.value(null));
    	assertEquals(CodecReturnCodes.SUCCESS, testFloat.value("    "));
    	assertTrue(testFloat.isBlank());
    	assertEquals(CodecReturnCodes.INVALID_ARGUMENT, testFloat.value("sfbgksj"));
    	assertEquals(CodecReturnCodes.SUCCESS, testFloat.value(""));
    	assertTrue(testFloat.isBlank());
        
    	assertEquals(CodecReturnCodes.SUCCESS, testFloat.value("11.22"));
    	assertEquals("11.22", testFloat.toString());
    	assertFalse(testFloat.isBlank());
    }

    @Test 
    public void newToOldRealTest()
    {
    	Real real = new RealImpl();
    	Real decReal = new RealImpl()
    	{	@Override
    		public int decode(DecodeIterator diter)
    		{
    			 DecodeIteratorImpl iter = (DecodeIteratorImpl)diter;
    		        RealImpl value = this;
    		        int ret = CodecReturnCodes.SUCCESS;

    		        if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) > 1)
    		        {
    		            int savePosition = iter._reader.position();
    		            try
    		            {
    		                iter._reader.position(iter._curBufPos);
    		                int hint = iter._reader.readByte();
    		                if (!((hint & RealImpl.BLANK_REAL) > 0))
    		                {
    		                    int length = iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos - 1;
    		                    value.value(iter._reader.readLong64ls(length), ((byte)(hint & 0x1F)));
    		                }
    		                else
    		                {
    		                    value.blank();
    		                }
    		                iter._reader.position(savePosition);
    		            }
    		            catch (Exception e1)
    		            {
    		                return CodecReturnCodes.INVALID_ARGUMENT;
    		            }
    		        }
    		        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 1)
    		        {
    		            value.blank();
    		        }
    		        else if ((iter._levelInfo[iter._decodingLevel + 1]._endBufPos - iter._curBufPos) == 0)
    		        {
    		            value.blank();
    		            ret = CodecReturnCodes.BLANK_DATA;
    		        }
    		        else
    		        {
    		            ret = CodecReturnCodes.INCOMPLETE_DATA;
    		        }

    		        return ret;
    		}
    	};
    
    	
    	    	
    	
    	_encIter.clear();
    	_buffer.data(ByteBuffer.allocate(100));
    	_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
    	real.value(0, RealHints.INFINITY);
    	assertEquals(CodecReturnCodes.SUCCESS, real.encode(_encIter));
    	
    	_decIter.clear();
       	_decIter.setBufferAndRWFVersion(_buffer,  Codec.majorVersion(),  Codec.minorVersion());
    	assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(_decIter));
    	assertEquals(true, decReal.isBlank());
    	
    }
    
    
    @Test
    public void encodeReal4Test()
    {
    	Real real = new RealImpl()
    	{
    		@Override
    	    public int encode(EncodeIterator iter)
    	    {
    	        return Encoders.PrimitiveEncoder.encodeReal4((EncodeIteratorImpl) iter, this);
    	    }
    	};
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

       
        real.blank();
    	real.encode(_encIter);
    	
    	byte [] res = {0x20, 0};
        assertArrayEquals(res, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(1, RealHints.EXPONENT_5);
    	real.encode(_encIter);
    	
    	byte [] res1 = {9, 1};
        assertArrayEquals(res1, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x0100, RealHints.FRACTION_2);
    	real.encode(_encIter);
    	
    	byte hint = 23 + 64;
    	byte [] res2 = {hint, 1, 0};
        assertArrayEquals(res2, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x010000, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 128);
    	byte [] res3 = {hint, 1, 0, 0};
        assertArrayEquals(res3, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x01000000, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 192);
    	byte [] res4 = {hint, 1, 0, 0, 0};
        assertArrayEquals(res4, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x100000000l, RealHints.EXPONENT0);
    	assertEquals(CodecReturnCodes.VALUE_OUT_OF_RANGE, real.encode(_encIter));
    	
    }
    
    @Test
    public void encodeReal8Test()
    {
    	Real real = new RealImpl()
    	{
    		@Override
    	    public int encode(EncodeIterator iter)
    	    {
    	        return Encoders.PrimitiveEncoder.encodeReal8((EncodeIteratorImpl) iter, this);
    	    }
    	};
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.blank();
    	real.encode(_encIter);
    	
    	byte [] res = {0x20, 0};
        assertArrayEquals(res, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x10, RealHints.EXPONENT_5);
    	real.encode(_encIter);
    	
    	byte [] res1 = {9, 0x10};
        assertArrayEquals(res1, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x1000, RealHints.FRACTION_2);
    	real.encode(_encIter);
    	
    	byte hint = 23;
    	byte [] res2 = {hint, 0x10, 0};
        assertArrayEquals(res2, convertToByteArray(_buffer.data()));
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x100000, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 64);
    	byte [] res3 = {hint, 0x10, 0, 0};
        assertArrayEquals(res3, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x10000000, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 64);
    	byte [] res4 = {hint, 0x10, 0, 0, 0};
        assertArrayEquals(res4, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x1000000000l, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 128);
    	byte [] res5 = {hint, 0x10, 0, 0, 0, 0};
        assertArrayEquals(res5, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x100000000000l, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 128);
    	byte [] res6 = {hint, 0x10, 0, 0, 0, 0, 0};
        assertArrayEquals(res6, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

        real.value(0x10000000000000l, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 192);
    	byte [] res7 = {hint, 0x10, 0, 0, 0, 0, 0, 0};
        assertArrayEquals(res7, convertToByteArray(_buffer.data()));
    	
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
    	
        real.value(0x1000000000000000l, RealHints.EXPONENT0);
    	real.encode(_encIter);
    	
    	hint = (byte) (14 + 192);
    	byte [] res8 = {hint, 0x10, 0, 0, 0, 0, 0, 0, 0};
        assertArrayEquals(res8, convertToByteArray(_buffer.data()));
    }
    
    // copy encoded data into byte[]
    private byte[] convertToByteArray(ByteBuffer bb)
    {
        bb.flip(); // prepare for writing.
        byte[] ba = new byte[bb.limit()];
        bb.get(ba);
        return ba;
    }

    @Test
    public void dataTypesTest()
    {
    	// isPrimitiveType() test
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.ARRAY));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.ASCII_STRING));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.BUFFER));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DATE));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DATE_4));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DATETIME));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DATETIME_7));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DATETIME_9));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DOUBLE));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.DOUBLE_8));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.ENUM));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.FLOAT));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.FLOAT_4));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.INT));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.INT_1));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.INT_2));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.INT_4));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.INT_8));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.QOS));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.REAL));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.REAL_4RB));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.REAL_8RB));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.RMTES_STRING));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.STATE));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.TIME));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.TIME_3));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.TIME_5));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UINT));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UINT_1));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UINT_2));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UINT_4));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UINT_8));
    	assertEquals(true, DataTypes.isPrimitiveType(DataTypes.UTF8_STRING));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.ANSI_PAGE));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.ELEMENT_LIST));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.FIELD_LIST));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.FILTER_LIST));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.MAP));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.MSG));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.OPAQUE));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.SERIES));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.VECTOR));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.XML));
    	assertEquals(false, DataTypes.isPrimitiveType(DataTypes.JSON));
    	
    	// isContainerType() test
    	assertEquals(false, DataTypes.isContainerType(DataTypes.ARRAY));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.ASCII_STRING));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.BUFFER));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DATE));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DATE_4));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DATETIME));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DATETIME_7));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DATETIME_9));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DOUBLE));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.DOUBLE_8));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.ENUM));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.FLOAT));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.FLOAT_4));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.INT));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.INT_1));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.INT_2));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.INT_4));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.INT_8));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.QOS));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.REAL));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.REAL_4RB));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.REAL_8RB));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.RMTES_STRING));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.STATE));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.TIME));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.TIME_3));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.TIME_5));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UINT));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UINT_1));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UINT_2));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UINT_4));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UINT_8));
    	assertEquals(false, DataTypes.isContainerType(DataTypes.UTF8_STRING));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.ANSI_PAGE));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.ELEMENT_LIST));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.FIELD_LIST));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.FILTER_LIST));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.MAP));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.MSG));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.OPAQUE));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.SERIES));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.VECTOR));
    	assertEquals(true, DataTypes.isContainerType(DataTypes.XML));
    	
    	// primitiveTypeSize() test
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.ARRAY), 255);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.ASCII_STRING), 255);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.BUFFER), 255);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DATE), 5);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DATE_4), 4);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DATETIME), 10);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DATETIME_7), 7);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DATETIME_9), 9);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DOUBLE), 9);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.DOUBLE_8), 8);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.ENUM), 3);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.FLOAT), 5);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.FLOAT_4), 4);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.INT), 9);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.INT_1), 1);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.INT_2), 2);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.INT_4), 4);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.INT_8), 8);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.QOS), 6);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.REAL), 10);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.REAL_4RB), 5);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.REAL_8RB), 9);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.RMTES_STRING), 255);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.STATE), 255);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.TIME), 6);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.TIME_3), 3);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.TIME_5), 5);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UINT), 9);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UINT_1), 1);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UINT_2), 2);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UINT_4), 4);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UINT_8), 8);
    	assertEquals(DataTypes.primitiveTypeSize(DataTypes.UTF8_STRING), 255);
    }
    
}

