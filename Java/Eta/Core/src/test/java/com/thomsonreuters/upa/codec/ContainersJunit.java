///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.codec;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;

import org.junit.Test;

import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;

public class ContainersJunit 
{

	EncodeIterator _encIter = CodecFactory.createEncodeIterator();
	DecodeIterator _decIter = CodecFactory.createDecodeIterator();
	BufferImpl _buffer = (BufferImpl) CodecFactory.createBuffer();

	void testBufferArray(Buffer [] testBuffers, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */
		for (int type = DataTypes.BUFFER; type<= DataTypes.RMTES_STRING; type++)
		{
			_encIter.clear();
			_buffer.data(ByteBuffer.allocate(100));
			_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			Array array = CodecFactory.createArray();
			ArrayEntry ae = CodecFactory.createArrayEntry();
			array.itemLength(itemLength);
			array.primitiveType(type);		
			array.encodeInit(_encIter);
			for (int i = 0; i<count; i++)
			{
			    if (!blank)
			        ae.encode(_encIter, testBuffers[i]);
			    else
			        ae.encodeBlank(_encIter);
			}
			array.encodeComplete(_encIter, true);	

			_decIter.clear();		
			_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			Array array1 = CodecFactory.createArray();
			array1.decode(_decIter);
			assertEquals(type, array.primitiveType());
			assertEquals(itemLength, array.itemLength());

			int ret = 0;
			int i =0;
			Buffer tmpBuffer = CodecFactory.createBuffer();
			while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
			{
				if (ret == CodecReturnCodes.SUCCESS)
				{
					tmpBuffer.clear();
					ret = tmpBuffer.decode(_decIter);
					if (ret == CodecReturnCodes.SUCCESS)
					{
    					assertEquals(testBuffers[i].toString(), tmpBuffer.toString());
					}
					else if (ret == CodecReturnCodes.BLANK_DATA)
					{
					    assertTrue(tmpBuffer.isBlank());
					    if (!blank)
					    	assertTrue(testBuffers[i].isBlank());
					}
					if (blank)
					    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
				}
				i++;
			}
			assertEquals(count, i);
		}
	}
	
	void testEnumArray(Enum [] testEnums, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.ENUM);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		        ae.encode(_encIter, testEnums[i]);
		    else
		        ae.encodeBlank(_encIter);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.ENUM, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Enum tmpEnum = CodecFactory.createEnum();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
            	tmpEnum.clear();
                ret = tmpEnum.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testEnums[i].toInt(), tmpEnum.toInt());
                    assertEquals(testEnums[i].toString(), tmpEnum.toString());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpEnum.isBlank());
                    if (!blank)
                    	assertTrue(testEnums[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}
	
	void testStateArray(State [] testStates, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.STATE);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		        ae.encode(_encIter, testStates[i]);
		    else
		        ae.encodeBlank(_encIter);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.STATE, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		State tmpState = CodecFactory.createState();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
            	tmpState.clear();
                ret = tmpState.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testStates[i].code(), tmpState.code());
                    assertEquals(testStates[i].dataState(), tmpState.dataState());
                    assertEquals(testStates[i].streamState(), tmpState.streamState());
                    assertTrue(testStates[i].text().equals(tmpState.text()));
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpState.isBlank());
                    if (!blank)
                    	assertTrue(testStates[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}
	
	void testQosArray(Qos [] testQoss, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.QOS);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		        ae.encode(_encIter, testQoss[i]);
		    else
		        ae.encodeBlank(_encIter);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.QOS, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Qos tmpQos = CodecFactory.createQos();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpQos.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testQoss[i].rate(), tmpQos.rate());
                    assertEquals(testQoss[i].rateInfo(), tmpQos.rateInfo());
                    assertEquals(testQoss[i].timeliness(), tmpQos.timeliness());
                    assertEquals(testQoss[i].timeInfo(), tmpQos.timeInfo());
                    assertEquals(testQoss[i].isDynamic(), tmpQos.isDynamic());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpQos.isBlank());
                    if (!blank)
                    	assertTrue(testQoss[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}
	
	void testDateArray(Date [] testDates, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.DATE);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		        ae.encode(_encIter, testDates[i]);
		    else
		        ae.encodeBlank(_encIter);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.DATE, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Date tmpDate = CodecFactory.createDate();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpDate.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testDates[i].year(), tmpDate.year());
                    assertEquals(testDates[i].month(), tmpDate.month());
                    assertEquals(testDates[i].day(), tmpDate.day());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpDate.isBlank());
                    if (!blank)
                    	assertTrue(testDates[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}
	
	void testTimeArray(Time [] testTimes, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.TIME);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		        ae.encode(_encIter, testTimes[i]);
		    else
		        ae.encodeBlank(_encIter);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.TIME, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Time tmpTime = CodecFactory.createTime();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpTime.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testTimes[i].hour(), tmpTime.hour());
                    assertEquals(testTimes[i].minute(), tmpTime.minute());
                    assertEquals(testTimes[i].second(), tmpTime.second());
                    assertEquals(testTimes[i].millisecond(), tmpTime.millisecond());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpTime.isBlank());
                    if (!blank)
                    	assertTrue(testTimes[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}
	
    void testDateTimeArray(DateTime [] testDateTimes, int count, int itemLength, boolean blank)
    {
        /* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(100));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
        
        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        array.itemLength(itemLength);
        array.primitiveType(DataTypes.DATETIME);        
        array.encodeInit(_encIter);
        for (int i = 0; i<count; i++)
        {
            if (!blank)
                ae.encode(_encIter, testDateTimes[i]);
            else
                ae.encodeBlank(_encIter);
        }
        array.encodeComplete(_encIter, true);   

        _decIter.clear();       
        _decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
        
        Array array1 = CodecFactory.createArray();
        array1.decode(_decIter);
        assertEquals(DataTypes.DATETIME, array.primitiveType());
        assertEquals(itemLength, array.itemLength());

        int ret = 0;
        int i =0;
        DateTime tmpDateTime = CodecFactory.createDateTime();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpDateTime.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                    assertEquals(testDateTimes[i].year(), tmpDateTime.year());
                    assertEquals(testDateTimes[i].month(), tmpDateTime.month());
                    assertEquals(testDateTimes[i].day(), tmpDateTime.day());
                    assertEquals(testDateTimes[i].hour(), tmpDateTime.hour());
                    assertEquals(testDateTimes[i].minute(), tmpDateTime.minute());
                    assertEquals(testDateTimes[i].second(), tmpDateTime.second());
                    assertEquals(testDateTimes[i].millisecond(), tmpDateTime.millisecond());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpDateTime.isBlank());
                    if (!blank)
                        assertTrue(testDateTimes[i].isBlank());
                }
                if (blank)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
        assertEquals(count, i);
    }

    void testRealArray(Real [] testReals, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.REAL);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		    {
		        ae.encode(_encIter, testReals[i]);
		    }
		    else
		    {
		        ae.encodeBlank(_encIter);
		    }
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.REAL, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Real tmpReal = CodecFactory.createReal();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpReal.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
	                assertEquals(testReals[i].toLong(), tmpReal.toLong());
	                assertEquals(testReals[i].hint(), tmpReal.hint());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpReal.isBlank());
                    if (!blank)
                    	assertTrue(testReals[i].isBlank());
                }
                if (blank)
                {
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
                }
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}

	void testFloatArray(Float [] testFloats, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.FLOAT);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		    {
		        ae.encode(_encIter, testFloats[i]);
		    }
		    else
		    {
		        ae.encodeBlank(_encIter);
		    }
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.FLOAT, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Float tmpFloat = CodecFactory.createFloat();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpFloat.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                	assertTrue(testFloats[i].toFloat() == tmpFloat.toFloat());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpFloat.isBlank());
                    if (!blank)
                    	assertTrue(testFloats[i].isBlank());
                }
                if (blank)
                {
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
                }
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ingored
            }
            i++;
        }
		assertEquals(count, i);
	}

	void testDoubleArray(Double [] testDoubles, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.DOUBLE);		
		array.encodeInit(_encIter);
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		    {
		        ae.encode(_encIter, testDoubles[i]);
		    }
		    else
		    {
		        ae.encodeBlank(_encIter);
		    }
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.DOUBLE, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Double tmpDouble = CodecFactory.createDouble();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret == CodecReturnCodes.SUCCESS)
            {
                ret = tmpDouble.decode(_decIter);
                if (ret == CodecReturnCodes.SUCCESS)
                {
                	assertTrue(testDoubles[i].toDouble() == tmpDouble.toDouble());
                }
                else if (ret == CodecReturnCodes.BLANK_DATA)
                {
                    assertTrue(tmpDouble.isBlank());
                    if (!blank)
                    	assertTrue(testDoubles[i].isBlank());
                }
                if (blank)
                {
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
                }
            }
            else if (ret == CodecReturnCodes.BLANK_DATA)
            {
                // ignored
            }
            i++;
        }
		assertEquals(count, i);
	}

	void testUIntArray(long [] testUInts, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.UINT);		
		array.encodeInit(_encIter);
		UInt data = CodecFactory.createUInt();
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		    {
    			data.clear();
    			data.value(testUInts[i]);
    			ae.encode(_encIter, data);
		    }
		    else
		    {
                if (testUInts[i] == 0)
                    ae.encodeBlank(_encIter);
                else
                {
                    data.clear();
                    data.value(testUInts[i]);
                    ae.encode(_encIter, data);
                }
		    }
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.UINT, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		UInt tmpUInt = CodecFactory.createUInt();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            assertEquals(ret, CodecReturnCodes.SUCCESS);

            ret = tmpUInt.decode(_decIter);
            if (!blank)
            {
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(testUInts[i], tmpUInt.toLong());
            }
            else
            {
                if (testUInts[i] == 0)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
                else
                {
                    assertEquals(CodecReturnCodes.SUCCESS, ret);
                    assertEquals(testUInts[i], tmpUInt.toLong());
                }
            }
            i++;
        }
		assertEquals(count, i);
	}

	void testIntArray(long [] testInts, int count, int itemLength, boolean blank)
	{
		/* Encode and decode the primitive types given by 'primitives' into and out of an array. Encode as blank if specified. Use a set definition if given. */

		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(itemLength);
		array.primitiveType(DataTypes.INT);		
		array.encodeInit(_encIter);
		Int data = CodecFactory.createInt();
		for (int i = 0; i<count; i++)
		{
		    if (!blank)
		    {
    			data.clear();
    			data.value(testInts[i]);
    			ae.encode(_encIter, data);
		    }
		    else
		    {
		        // When the blank option on this test is true,
		        // zero values in the testInts[] will be encoded
		        // as blank, while non-zero values will be encoded
		        // as non-blank data. This allows mixing blank
		        // and non-blank in a test, depending on the contents
		        // of testInts[].
		        if (testInts[i] == 0)
		            ae.encodeBlank(_encIter);
		        else
		        {
		            data.clear();
		            data.value(testInts[i]);
		            ae.encode(_encIter, data);
		        }
		    }
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.INT, array.primitiveType());
		assertEquals(itemLength, array.itemLength());

		int ret = 0;
		int i =0;
		Int tmpInt = CodecFactory.createInt();
        while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            assertEquals(ret, CodecReturnCodes.SUCCESS);
            
            ret = tmpInt.decode(_decIter);
            if (!blank)
            {
                assertEquals(CodecReturnCodes.SUCCESS, ret);
                assertEquals(testInts[i], tmpInt.toLong());
            }
            else
            {
                // If test using blank option: expect blank entry
                // if the testInts[] value was zero. Else expect non-blank.
                if (testInts[i] == 0)
                    assertEquals(ret, CodecReturnCodes.BLANK_DATA);
                else
                {
                    assertEquals(CodecReturnCodes.SUCCESS, ret);
                    assertEquals(testInts[i], tmpInt.toLong());
                }
            }

            i++;
        }
		assertEquals(count, i);
	}
	
	@Test
	public void arrayUnitTests()
	{
		/* Tests encoding arrays of different types and itemLengths, ensuring that it succeeds or fails as appropriate */

		/* Indexed by encoded size(e.g. testInts[4] is a 4-byte value) */
		long testInts[] = { 0x0, 0x7f, 0x7fff, 0x7fffff, 0x7fffffff, 0x7fffffffffL, 0x7fffffffffffL, 0x7fffffffffffffL, 0x7fffffffffffffffL };
		testIntArray(testInts, 9, 8, false);
		testIntArray(testInts, 9, 0, false);
		testIntArray(testInts, 9, 0, true /* blank */);
		testIntArray(testInts, 3, 2, false);
		testIntArray(testInts, 5, 4, false);
		
        // when blank == true, test will encode 0x0 Ints as blank
        long testInts2[] = { 0x0, 0x0, 0x7fff, 0x7fffff, 0x0, 0x7fffffffffL, 0x0, 0x7fffffffffffffL, 0x0 };
        testIntArray(testInts2, 9, 8, false);
        testIntArray(testInts2, 9, 0, false);
        testIntArray(testInts2, 9, 0, true /* blank */);
        testIntArray(testInts2, 3, 2, false);
        testIntArray(testInts2, 5, 4, false);

		long testUInts[] = { 0x0, 0xff, 0xffff, 0xffffff, 0xffffffffL, 0xffffffffffL, 0xffffffffffffL, 0xffffffffffffffL, 0xffffffffffffffffL };
		testUIntArray(testUInts, 9, 8, false);
		testUIntArray(testUInts, 9, 0, false);
		testUIntArray(testUInts, 9, 0, true /* blank */);
		testUIntArray(testUInts, 3, 2, false);
        testUIntArray(testUInts, 5, 4, false);
		
        long testUInts2[] = { 0x0, 0x0, 0xffff, 0xffffff, 0x0, 0xffffffffffL, 0x0, 0xffffffffffffffL, 0x0 };
        testUIntArray(testUInts2, 9, 0, false);
        testUIntArray(testUInts2, 9, 0, true);
        testUIntArray(testUInts2, 9, 8, false);
        testUIntArray(testUInts2, 3, 2, false);
        testUIntArray(testUInts2, 5, 4, false);

		Real testReals [] = new Real[testInts.length];
		int hint = RealHints.EXPONENT5;		
		for (int i=0; i<testInts.length; i++)
		{
			testReals[i] = CodecFactory.createReal();
			testReals[i].value(testInts[i], hint);
		}
		testRealArray(testReals, 9, 0, false);
		testRealArray(testReals, 9, 0, true);

		int testDT [][]= new int[3][7]; // test for 3 entries
		DateTime testDateTime [] = new DateTime[3];
		Date testDate [] = new Date[3];
		Time testTime [] = new Time[3];

		for (int i=0; i<3; i++)
		{
			testDT[i][0] = 1990 + 2*i;	// year
			testDT[i][1] = 1 + 3*i;		// month
			testDT[i][2] = 7*i+2;			// day
			testDT[i][3] = 11 - 2*i;	// hour
			testDT[i][4] = 60 - 10*i;	// minute
			testDT[i][5] = 60 - 11*i;	// second
			testDT[i][6] = 1000 - 60*i;	// milisecond

			testDateTime[i] = CodecFactory.createDateTime();
			testDateTime[i].year(testDT[i][0]);
			testDateTime[i].month(testDT[i][1]);
			testDateTime[i].day(testDT[i][2]);
			testDateTime[i].hour(testDT[i][3]);
			testDateTime[i].minute(testDT[i][4]);
			testDateTime[i].second(testDT[i][5]);
			testDateTime[i].millisecond(testDT[i][6]);
			
			testDate[i] = CodecFactory.createDate();
			testDate[i].year(testDT[i][0]);
			testDate[i].month(testDT[i][1]);
			testDate[i].day(testDT[i][2]);
			
			testTime[i] = CodecFactory.createTime();
			testTime[i].hour(testDT[i][3]);
			testTime[i].minute(testDT[i][4]);
			testTime[i].second(testDT[i][5]);
			testTime[i].millisecond(testDT[i][6]);
		}
		testDateArray(testDate, 3, 0, false);
		testDateArray(testDate, 3, 0, true);
		testDateArray(testDate, 3, 4, false);
		testTimeArray(testTime, 3, 0, false);
		testTimeArray(testTime, 3, 0, true);
		testTimeArray(testTime, 1, 3, false);
		testTimeArray(testTime, 3, 5, false);
		testDateTimeArray(testDateTime, 3, 0, false);
        testDateTimeArray(testDateTime, 3, 0, true);
        testDateTimeArray(testDateTime, 1, 7, false);
        testDateTimeArray(testDateTime, 3, 9, false);
        
		Qos testQos3 = CodecFactory.createQos();
		testQos3.dynamic(false);
		testQos3.rate(QosRates.TICK_BY_TICK);
		testQos3.timeliness(QosTimeliness.REALTIME);

		Qos testQos1 = CodecFactory.createQos();
		testQos1.dynamic(true);
		testQos1.rate(QosRates.TICK_BY_TICK);
		testQos1.timeliness(QosTimeliness.DELAYED);
		testQos1.timeInfo(567);

		Qos testQos2 = CodecFactory.createQos();
		testQos2.dynamic(false);
		testQos2.rate(QosRates.TIME_CONFLATED);
		testQos2.rateInfo(897);
		testQos2.timeliness(QosTimeliness.REALTIME);
		
		Qos [] testQos = {testQos2, testQos1, testQos3};
		testQosArray(testQos, 3, 0, false);
		testQosArray(testQos, 3, 0, true);

		State testState1 = CodecFactory.createState();
		testState1.code(StateCodes.INVALID_VIEW);
		testState1.dataState(DataStates.NO_CHANGE);
		testState1.streamState(StreamStates.OPEN);
		Buffer stateText = CodecFactory.createBuffer();
		stateText.data("Illinois");
		testState1.text(stateText);
		
		State testState2 = CodecFactory.createState();
		testState2.code(StateCodes.NONE);
		testState2.dataState(DataStates.NO_CHANGE);
		testState2.streamState(StreamStates.OPEN);
		
		State testState3 = CodecFactory.createState();
		testState3.code(StateCodes.INVALID_ARGUMENT);
		testState3.dataState(DataStates.NO_CHANGE);
		testState3.streamState(StreamStates.NON_STREAMING);
		Buffer stateText3 = CodecFactory.createBuffer();
		stateText3.data("bla bla");
		testState3.text(stateText3);
		
		State [] testState = {testState1, testState2, testState3};
		testStateArray(testState, 3, 0, false);
		testStateArray(testState, 3, 0, true);


		Enum [] testEnum = new Enum[3];
		for (int i =0; i<3; i++)
		{
			testEnum[i] = CodecFactory.createEnum();
			testEnum[i].value((short)testInts[i]);
		}
		testEnumArray(testEnum, 3, 0, false); 
		testEnumArray(testEnum, 3, 0, true);
		testEnumArray(testEnum, 2, 1, false);
        testEnumArray(testEnum, 3, 2, false);
		
		Buffer testBuffer1 = CodecFactory.createBuffer();
		ByteBuffer bb = ByteBuffer.allocate(7);
		byte [] bts = {1, 2, 0, 10, 57, 0x7F, 4}; 
		bb.put(bts);
		testBuffer1.data(bb,0,7);

		Buffer testBuffer2 = CodecFactory.createBuffer();
		bb = ByteBuffer.allocate(5);
		byte [] bts1 = {27, 48, 0, 10, 57}; 
		bb.put(bts1);
		testBuffer2.data(bb,0,5);

		Buffer testBuffer3 = CodecFactory.createBuffer();
		bb = ByteBuffer.allocate(4);
		byte [] bts2 = {10, 57, 0x7F, 4}; 
		bb.put(bts2);
		testBuffer3.data(bb,0,4);
		
		Buffer [] testBuffer = {testBuffer1, testBuffer2, testBuffer3};
		testBufferArray(testBuffer, 3, 0, false);
		testBufferArray(testBuffer, 3, 0, true);
		
		// FLOAT
		Float [] testFloats = new Float[10];
		for (int i=0; i < 10; i++)
		{
			testFloats[i] = CodecFactory.createFloat();
			testFloats[i].value((float)3.3 * i);
		}
		testFloatArray(testFloats, 10, 0, false);
		testFloatArray(testFloats, 10, 0, true);
		testFloatArray(testFloats, 10, 4, false);

		// DOUBLE
		Double [] testDoubles = new Double[10];
		for (int i=0; i < 10; i++)
		{
			testDoubles[i] = CodecFactory.createDouble();
			testDoubles[i].value(4.321 * i);
		}
		testDoubleArray(testDoubles, 10, 0, false);
		testDoubleArray(testDoubles, 10, 0, true);
		testDoubleArray(testDoubles, 10, 8, false);
	}
	
    @Test
    public void arrayEncodeDecodeBlankTest()
    {
        ArrayImpl arrayv = (ArrayImpl)CodecFactory.createArray();
        
        assertFalse(arrayv.isBlank());
        arrayv.blank();
        assertTrue(arrayv.isBlank());
        
        _encIter.clear();
        _buffer.data(ByteBuffer.allocate(15));
        _encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

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
        ArrayImpl array1 = (ArrayImpl)CodecFactory.createArray();
        assertFalse(array1.isBlank());
        assertEquals(CodecReturnCodes.BLANK_DATA, array1.decode(_decIter));

        assertTrue(array1.isBlank());
    }
	
	@Test
	public void BufferEntryTest()
	{
		Buffer buffer1 = CodecFactory.createBuffer();
		ByteBuffer bb = ByteBuffer.allocate(7);
		byte [] bts = {41, 42, 40, 50, 57, 32, 33}; 
		bb.put(bts);
		buffer1.data(bb,0,7);

		Buffer buffer2 = CodecFactory.createBuffer();
		bb = ByteBuffer.allocate(6);
		byte [] bts1 = {37, 48, 40, 50, 57, 32}; 
		bb.put(bts1);
		buffer2.data(bb,0,6);

		Buffer buffer3 = CodecFactory.createBuffer();
		bb = ByteBuffer.allocate(4);
		byte [] bts2 = {50, 57, 48, 34}; 
		bb.put(bts2);
		buffer3.data(bb,0,4);
		
		Buffer buffer4 = CodecFactory.createBuffer();
		bb = ByteBuffer.allocate(4);
		byte [] bts3 = {10, 57, 0x7F, 4}; 
		bb.put(bts3);
		buffer4.data(bb,0,0);

		Buffer [] buffers = {buffer1, buffer2, buffer3, buffer4};		
				
		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(100));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(6);
		array.primitiveType(DataTypes.BUFFER);		
		array.encodeInit(_encIter);
		for (int i = 0; i<4; i++)
		{
			ae.encode(_encIter, buffers[i]);
		}
		array.encodeComplete(_encIter, true);	

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		Array array1 = CodecFactory.createArray();
		array1.decode(_decIter);
		assertEquals(DataTypes.BUFFER, array.primitiveType());
		assertEquals(6, array.itemLength());
		
		int ret = 0;
		int i =0;
		Buffer tmpBuffer = CodecFactory.createBuffer();
		ae.clear();
		byte [] bytes;
		byte [] testBytes;
		while ((ret = ae.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret == CodecReturnCodes.SUCCESS)
			{
				tmpBuffer.clear();
				ret = tmpBuffer.decode(_decIter);
				assertEquals(CodecReturnCodes.SUCCESS, ret);
				assertEquals(6, tmpBuffer.length());
				bytes = tmpBuffer.data().array();
				testBytes = buffers[i].data().array();
				if (array.itemLength() <= buffers[i].length())
				{
					for (int j=0; j<array.itemLength(); j++)
					{
						assertEquals(testBytes[j], bytes[tmpBuffer.position()+j]);
					}
				}
				else
				{
					for (int j=0; j<buffers[i].length(); j++)
					{
						assertEquals(testBytes[j], bytes[tmpBuffer.position()+j]);
					}
					for (int j=buffers[i].length(); j<array.itemLength(); j++)
					{
						assertEquals(0x00, bytes[tmpBuffer.position()+j]);						
					}
				}
			}
			else if (ret == CodecReturnCodes.BLANK_DATA)
			{
				//
			}
			i++;
		}
		assertEquals(4, i);
		
	}
	
	private void decodeElementList(int elements, int fi)
	{
		int [] flags = {ElementListFlags.HAS_STANDARD_DATA, ElementListFlags.HAS_ELEMENT_LIST_INFO+ElementListFlags.HAS_STANDARD_DATA};
		// These arrays for fids and dataTypes make up our "Dictionary"

		ElementList decElementList = CodecFactory.createElementList();				
		ElementEntry decEntry = CodecFactory.createElementEntry();
		Buffer [] names = {CodecFactory.createBuffer(), 
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer()
				};
		names[0].data("INT");
		names[1].data("UINT");
		names[2].data("REAL");
		names[3].data("DATE");
		names[4].data("TIME");

		int dataTypes[] = 
			{
				DataTypes.INT,
				DataTypes.UINT,
				DataTypes.REAL,
				DataTypes.DATE,
				DataTypes.TIME
			};

		Int decInt = CodecFactory.createInt();
		UInt decUInt = CodecFactory.createUInt();
		Real decReal = CodecFactory.createReal();
		Date decDate = CodecFactory.createDate();
		Time decTime = CodecFactory.createTime();
		DateTime decDateTime = CodecFactory.createDateTime();
		Array decArray = CodecFactory.createArray();
		ArrayEntry decArrayEntry = CodecFactory.createArrayEntry();

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		decElementList.decode(_decIter, null);
		assertEquals(decElementList.flags(), flags[fi]);
		if (flags[fi] == ElementListFlags.HAS_ELEMENT_LIST_INFO)
			assertEquals(decElementList.setId(), 256);
		int eCount = 0;
		int ret;
		while ((ret = decEntry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			assertEquals(CodecReturnCodes.SUCCESS, ret);
			assertTrue(decEntry.name().equals(names[eCount]));
			assertEquals(dataTypes[eCount], decEntry.dataType());
			switch(decEntry.dataType())
			{
			case DataTypes.INT:
				assertEquals(CodecReturnCodes.SUCCESS, decInt.decode(_decIter));
				assertEquals(-2049, decInt.toLong());
				break;
			case DataTypes.UINT:
				assertEquals(CodecReturnCodes.SUCCESS, decUInt.decode(_decIter));
				assertEquals(2049, decUInt.toLong());
				break;
			case DataTypes.REAL:
				assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(_decIter));
				assertEquals(0xFFFFF, decReal.toLong());
				assertEquals(1, decReal.hint());
				break;
			case DataTypes.DATE:
				assertEquals(CodecReturnCodes.SUCCESS, decDate.decode(_decIter));
				assertEquals(3, decDate.day());
				assertEquals(8, decDate.month());
				assertEquals(1892, decDate.year());
				break;
			case DataTypes.TIME:
				assertEquals(CodecReturnCodes.SUCCESS, decTime.decode(_decIter));
				assertEquals(23, decTime.hour());
				assertEquals(59, decTime.minute());
				assertEquals(59, decTime.second());
				assertEquals(999, decTime.millisecond());
				break;
			case DataTypes.DATETIME:
				assertEquals(CodecReturnCodes.SUCCESS, decDateTime.decode(_decIter));
				assertEquals(3, decDateTime.day());
				assertEquals(8, decDateTime.month());
				assertEquals(1892, decDateTime.year());
				assertEquals(23, decDateTime.hour());
				assertEquals(59, decDateTime.minute());
				assertEquals(59, decDateTime.second());
				assertEquals(999, decDateTime.millisecond());
				break;
			case DataTypes.ARRAY:
				assertEquals(CodecReturnCodes.SUCCESS, decArray.decode(_decIter));
				assertEquals(DataTypes.INT, decArray.primitiveType());
				assertEquals(0, decArray.itemLength());

				int i =0;
				while ((ret = decArrayEntry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
				{
					assertEquals(CodecReturnCodes.SUCCESS, ret);
					ret = decInt.decode(_decIter);
					assertEquals(CodecReturnCodes.SUCCESS, ret);
					assertEquals(0xDEEEDEEE, decInt.toLong());
					i++;
					decArrayEntry.clear();
				}
				assertEquals(3, i);
				break;
			default:
				fail();
			}
			decEntry.clear();
			eCount++;
		}
		assertEquals(elements, eCount);

	}

	@Test
	public void elementListUnitTests()
	{
		ElementList container = CodecFactory.createElementList();
		ElementEntry entry = CodecFactory.createElementEntry();

		int [] flags = {ElementListFlags.HAS_STANDARD_DATA, ElementListFlags.HAS_ELEMENT_LIST_INFO+ElementListFlags.HAS_STANDARD_DATA};

		Buffer [] names = {CodecFactory.createBuffer(), 
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer(),
				CodecFactory.createBuffer()
				};
		names[0].data("INT");
		names[1].data("UINT");
		names[2].data("REAL");
		names[3].data("DATE");
		names[4].data("TIME");


		int dataTypes[] = 
			{
				DataTypes.INT,
				DataTypes.UINT,
				DataTypes.REAL,
				DataTypes.DATE,
				DataTypes.TIME,
			};

		Int paylInt = CodecFactory.createInt();
		paylInt.value(-2049);		

		UInt paylUInt = CodecFactory.createUInt();
		paylUInt.value(2049);		

		Real paylReal = CodecFactory.createReal();
		paylReal.value(0xFFFFF, 1);

		Date paylDate = CodecFactory.createDate();
		paylDate.day(3);
		paylDate.month(8);
		paylDate.year(1892);

		Time paylTime = CodecFactory.createTime();
		paylTime.hour(23);
		paylTime.minute(59);
		paylTime.second(59);
		paylTime.millisecond(999);

		DateTime paylDateTime = CodecFactory.createDateTime();
		paylDateTime.day(3);
		paylDateTime.month(8);
		paylDateTime.year(1892);
		paylDateTime.hour(23);
		paylDateTime.minute(59);
		paylDateTime.second(59);
		paylDateTime.millisecond(999);

		/* Pre-encode array */
		Buffer preencodedArray = CodecFactory.createBuffer();
		preencodedArray.data(ByteBuffer.allocate(20));

		_encIter.clear();
		_encIter.setBufferAndRWFVersion(preencodedArray, Codec.majorVersion(), Codec.minorVersion());

		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(0);
		array.primitiveType(DataTypes.INT);		
		array.encodeInit(_encIter);
		Int data = CodecFactory.createInt();
		data.value(0xDEEEDEEE);
		for (int i = 0; i<3; i++)
		{
			ae.encode(_encIter, data);
		}
		array.encodeComplete(_encIter, true);					

		for (int i = 0; i< dataTypes.length; i++)
		{
			for (int k = 0; k<flags.length; k++)
			{
				_encIter.clear();
				_buffer.data(ByteBuffer.allocate(100));
				_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
				container.flags(flags[k]);
				if ( (flags[k] & ElementListFlags.HAS_ELEMENT_LIST_INFO) > 0)
					container.setId(256);
				container.encodeInit(_encIter, null, 0);					
				for (int j = 0; j<= i; j++)
				{
					entry.clear();
					entry.name(names[j]);
					entry.dataType(dataTypes[j]);
					switch(dataTypes[j])
					{
					case DataTypes.INT:
						entry.encode(_encIter, paylInt);
						break;
					case DataTypes.UINT:
						entry.encode(_encIter, paylUInt);
						break;
					case DataTypes.REAL:
						entry.encode(_encIter, paylReal);
						break;
					case DataTypes.DATE:
						entry.encode(_encIter, paylDate);
						break;
					case DataTypes.TIME:
						entry.encode(_encIter, paylTime);
						break;
					case DataTypes.DATETIME:
						entry.encode(_encIter, paylDateTime);
						break;
					case DataTypes.ARRAY:
						// encode two entries with the same array but different way of encoding
						entry.encode(_encIter, preencodedArray);

						entry.clear();
						entry.name(names[j-1]);
						entry.dataType(DataTypes.ARRAY);
						entry.encodeInit(_encIter, 0);
						array.itemLength(0);
						array.primitiveType(DataTypes.INT);		
						array.encodeInit(_encIter);
						for (int l = 0; l<3; l++)
						{
							ae.encode(_encIter, data);
						}
						array.encodeComplete(_encIter, true);					
						entry.encodeComplete(_encIter, true);
						break;
					default:
						break;
					}
				}
				container.encodeComplete(_encIter, true);

				// decode encoded element list and compare
				decodeElementList(i+1,k);
			}			
		}
	}

	private void decodeFieldList(int elements, int fi)
	{
		int [] flags = {FieldListFlags.HAS_STANDARD_DATA, FieldListFlags.HAS_FIELD_LIST_INFO+FieldListFlags.HAS_STANDARD_DATA};
		
		int fids[] = {1, 2, 3, 4, 5};

		FieldList decFieldList = CodecFactory.createFieldList();				
		FieldEntry decEntry = CodecFactory.createFieldEntry();

		Int decInt = CodecFactory.createInt();
		UInt decUInt = CodecFactory.createUInt();
		Real decReal = CodecFactory.createReal();
		Date decDate = CodecFactory.createDate();
		Time decTime = CodecFactory.createTime();
		DateTime decDateTime = CodecFactory.createDateTime();
		Array decArray = CodecFactory.createArray();
		ArrayEntry decArrayEntry = CodecFactory.createArrayEntry();

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		decFieldList.decode(_decIter, null);
		assertEquals(decFieldList.flags(), flags[fi]);
		if ((flags[fi] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
		{
			assertEquals(decFieldList.fieldListNum(), 256);
			assertEquals(decFieldList.dictionaryId(), 257);
		}
		int eCount = 0;
		int ret;
		while ((ret = decEntry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			assertEquals(CodecReturnCodes.SUCCESS, ret);
			assertEquals(fids[eCount],decEntry.fieldId());
			switch(decEntry.dataType())
			{
			case DataTypes.INT:
				assertEquals(CodecReturnCodes.SUCCESS, decInt.decode(_decIter));
				assertEquals(-2049, decInt.toLong());
				break;
			case DataTypes.UINT:
				assertEquals(CodecReturnCodes.SUCCESS, decUInt.decode(_decIter));
				assertEquals(2049, decUInt.toLong());
				break;
			case DataTypes.REAL:
				assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(_decIter));
				assertEquals(0xFFFFF, decReal.toLong());
				assertEquals(1, decReal.hint());
				break;
			case DataTypes.DATE:
				assertEquals(CodecReturnCodes.SUCCESS, decDate.decode(_decIter));
				assertEquals(3, decDate.day());
				assertEquals(8, decDate.month());
				assertEquals(1892, decDate.year());
				break;
			case DataTypes.TIME:
				assertEquals(CodecReturnCodes.SUCCESS, decTime.decode(_decIter));
				assertEquals(23, decTime.hour());
				assertEquals(59, decTime.minute());
				assertEquals(59, decTime.second());
				assertEquals(999, decTime.millisecond());
				break;
			case DataTypes.DATETIME:
				assertEquals(CodecReturnCodes.SUCCESS, decDateTime.decode(_decIter));
				assertEquals(3, decDateTime.day());
				assertEquals(8, decDateTime.month());
				assertEquals(1892, decDateTime.year());
				assertEquals(23, decDateTime.hour());
				assertEquals(59, decDateTime.minute());
				assertEquals(59, decDateTime.second());
				assertEquals(999, decDateTime.millisecond());
				break;
			case DataTypes.ARRAY:
				assertEquals(CodecReturnCodes.SUCCESS, decArray.decode(_decIter));
				assertEquals(DataTypes.INT, decArray.primitiveType());
				assertEquals(0, decArray.itemLength());

				int i =0;
				while ((ret = decArrayEntry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
				{
					assertEquals(CodecReturnCodes.SUCCESS, ret);
					ret = decInt.decode(_decIter);
					assertEquals(CodecReturnCodes.SUCCESS, ret);
					assertEquals(0xDEEEDEEE, decInt.toLong());
					i++;
					decArrayEntry.clear();
				}
				assertEquals(3, i);
				break;
			default:
				break;
			}
			decEntry.clear();
			eCount++;
		}
		assertEquals(elements, eCount);

	}

	@Test
	public void FieldListUnitTests()
	{
		FieldList container = CodecFactory.createFieldList();
		FieldEntry entry = CodecFactory.createFieldEntry();

		int [] flags = {FieldListFlags.HAS_STANDARD_DATA, FieldListFlags.HAS_FIELD_LIST_INFO+FieldListFlags.HAS_STANDARD_DATA};
		

		int fids[] = {1, 2, 3, 4, 5};

		int dataTypes[] = 
			{
				DataTypes.INT,
				DataTypes.UINT,
				DataTypes.REAL,
				DataTypes.DATE,
				DataTypes.TIME,
			};

		Int paylInt = CodecFactory.createInt();
		paylInt.value(-2049);		

		UInt paylUInt = CodecFactory.createUInt();
		paylUInt.value(2049);		

		Real paylReal = CodecFactory.createReal();
		paylReal.value(0xFFFFF, 1);

		Date paylDate = CodecFactory.createDate();
		paylDate.day(3);
		paylDate.month(8);
		paylDate.year(1892);

		Time paylTime = CodecFactory.createTime();
		paylTime.hour(23);
		paylTime.minute(59);
		paylTime.second(59);
		paylTime.millisecond(999);

		DateTime paylDateTime = CodecFactory.createDateTime();
		paylDateTime.day(3);
		paylDateTime.month(8);
		paylDateTime.year(1892);
		paylDateTime.hour(23);
		paylDateTime.minute(59);
		paylDateTime.second(59);
		paylDateTime.millisecond(999);

		/* Pre-encode array */
		Buffer preencodedArray = CodecFactory.createBuffer();
		preencodedArray.data(ByteBuffer.allocate(20));

		_encIter.clear();
		_encIter.setBufferAndRWFVersion(preencodedArray, Codec.majorVersion(), Codec.minorVersion());

		Array array = CodecFactory.createArray();
		ArrayEntry ae = CodecFactory.createArrayEntry();
		array.itemLength(0);
		array.primitiveType(DataTypes.INT);		
		array.encodeInit(_encIter);
		Int data = CodecFactory.createInt();
		data.value(0xDEEEDEEE);
		for (int i = 0; i<3; i++)
		{
			ae.encode(_encIter, data);
		}
		array.encodeComplete(_encIter, true);					

		for (int i = 0; i< dataTypes.length; i++)
		{
			for (int k = 0; k<flags.length; k++)
			{
				_encIter.clear();
				_buffer.data(ByteBuffer.allocate(100));
				_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
				container.flags(flags[k]);
				if ( (flags[k] & FieldListFlags.HAS_FIELD_LIST_INFO) > 0)
				{
					container.dictionaryId(257);
					container.fieldListNum(256);
				}
				container.encodeInit(_encIter, null, 0);					
				for (int j = 0; j<= i; j++)
				{
					entry.clear();
					entry.fieldId(fids[j]);
					entry.dataType(dataTypes[j]);
					
					switch(dataTypes[j])
					{
					case DataTypes.INT:
						entry.encode(_encIter, paylInt);
						break;
					case DataTypes.UINT:
						entry.encode(_encIter, paylUInt);
						break;
					case DataTypes.REAL:
						entry.encode(_encIter, paylReal);
						break;
					case DataTypes.DATE:
						entry.encode(_encIter, paylDate);
						break;
					case DataTypes.TIME:
						entry.encode(_encIter, paylTime);
						break;
					case DataTypes.DATETIME:
						entry.encode(_encIter, paylDateTime);
						break;
					case DataTypes.ARRAY:
						// encode two entries with the same array but different way of encoding
						entry.encode(_encIter, preencodedArray);

						entry.clear();
						entry.fieldId(fids[j-1]);
						entry.dataType(DataTypes.ARRAY);
						entry.encodeInit(_encIter, 0);
						array.itemLength(0);
						array.primitiveType(DataTypes.INT);		
						array.encodeInit(_encIter);
						for (int l = 0; l<3; l++)
						{
							ae.encode(_encIter, data);
						}
						array.encodeComplete(_encIter, true);					
						entry.encodeComplete(_encIter, true);
						break;
					default:
						break;
					}
				}
				container.encodeComplete(_encIter, true);

				// decode encoded element list and compare
				decodeFieldList(i+1,k);
			}			
		}
	}
	
	private void decodeFieldList()
	{
		int fids[] = {1, 2, 3, 4, 5};

		FieldList decFieldList = CodecFactory.createFieldList();				
		FieldEntry decEntry = CodecFactory.createFieldEntry();

		Int decInt = CodecFactory.createInt();
		UInt decUInt = CodecFactory.createUInt();
		Real decReal = CodecFactory.createReal();
		Date decDate = CodecFactory.createDate();
		Time decTime = CodecFactory.createTime();

		decFieldList.decode(_decIter, null);
		assertEquals(FieldListFlags.HAS_STANDARD_DATA, decFieldList.flags());
		int eCount = 0;
		int ret;
		while ((ret = decEntry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			assertEquals(CodecReturnCodes.SUCCESS, ret);
			assertEquals(fids[eCount],decEntry.fieldId());
			switch(decEntry.dataType())
			{
			case DataTypes.INT:
				assertEquals(CodecReturnCodes.SUCCESS, decInt.decode(_decIter));
				assertEquals(-2049, decInt.toLong());
				break;
			case DataTypes.UINT:
				assertEquals(CodecReturnCodes.SUCCESS, decUInt.decode(_decIter));
				assertEquals(2049, decUInt.toLong());
				break;
			case DataTypes.REAL:
				assertEquals(CodecReturnCodes.SUCCESS, decReal.decode(_decIter));
				assertEquals(0xFFFFF, decReal.toLong());
				assertEquals(1, decReal.hint());
				break;
			case DataTypes.DATE:
				assertEquals(CodecReturnCodes.SUCCESS, decDate.decode(_decIter));
				assertEquals(3, decDate.day());
				assertEquals(8, decDate.month());
				assertEquals(1892, decDate.year());
				break;
			case DataTypes.TIME:
				assertEquals(CodecReturnCodes.SUCCESS, decTime.decode(_decIter));
				assertEquals(23, decTime.hour());
				assertEquals(59, decTime.minute());
				assertEquals(59, decTime.second());
				assertEquals(999, decTime.millisecond());
				break;
			default:
				break;
			}
			decEntry.clear();
			eCount++;
		}
		assertEquals(1, eCount);

	}

	private void encodeFieldList()
	{
		FieldList container = CodecFactory.createFieldList();
		FieldEntry entry = CodecFactory.createFieldEntry();

		int fids[] = {1, 2, 3, 4, 5};

		int dataTypes[] = 
			{
				DataTypes.INT,
				DataTypes.UINT,
				DataTypes.REAL,
				DataTypes.DATE,
				DataTypes.TIME,
			};

		Int paylInt = CodecFactory.createInt();
		paylInt.value(-2049);		

		UInt paylUInt = CodecFactory.createUInt();
		paylUInt.value(2049);		

		Real paylReal = CodecFactory.createReal();
		paylReal.value(0xFFFFF, 1);

		Date paylDate = CodecFactory.createDate();
		paylDate.day(3);
		paylDate.month(8);
		paylDate.year(1892);

		Time paylTime = CodecFactory.createTime();
		paylTime.hour(23);
		paylTime.minute(59);
		paylTime.second(59);
		paylTime.millisecond(999);

		container.flags(FieldListFlags.HAS_STANDARD_DATA);
		assertEquals(CodecReturnCodes.SUCCESS, container.encodeInit(_encIter, null, 0));					
		for (int j = 0; j< 1; j++)
		{
			entry.clear();
			entry.fieldId(fids[j]);
			entry.dataType(dataTypes[j]);

			switch(dataTypes[j])
			{
			case DataTypes.INT:
				assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylInt));
				break;
			case DataTypes.UINT:
				assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylUInt));
				break;
			case DataTypes.REAL:
				assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylReal));
				break;
			case DataTypes.DATE:
				assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylDate));
				break;
			case DataTypes.TIME:
				assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylTime));
				break;
			default:
				break;
			}
		}
		assertEquals(CodecReturnCodes.SUCCESS, container.encodeComplete(_encIter, true));
	}

	private void decodeMap(int ai, int fi, boolean hasPermData)
	{
		Map container = CodecFactory.createMap();
		MapEntry entry = CodecFactory.createMapEntry();
		Buffer deckey = CodecFactory.createBuffer();
		deckey.data(ByteBuffer.allocate(10));

		int [] flags = {MapFlags.HAS_SUMMARY_DATA, 
				MapFlags.HAS_PER_ENTRY_PERM_DATA,
				MapFlags.HAS_TOTAL_COUNT_HINT,
				MapFlags.HAS_KEY_FIELD_ID
		};
		
		int [] entryActions = {MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE};
		Buffer keyData = CodecFactory.createBuffer();	
		keyData.data("keyData");
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data("permission");
		Buffer summaryData = CodecFactory.createBuffer();
		summaryData.data("$");
		Buffer delName = CodecFactory.createBuffer();
		delName.data("Del");

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		container.decode(_decIter);
		if ((fi == MapFlags.HAS_SUMMARY_DATA) || (fi == MapFlags.HAS_TOTAL_COUNT_HINT) || 
				(hasPermData) || (fi == MapFlags.HAS_KEY_FIELD_ID))
			assertEquals(flags[fi], container.flags());
		assertEquals(DataTypes.FIELD_LIST, container.containerType());
		assertEquals(DataTypes.ASCII_STRING, container.keyPrimitiveType());
		if ( (flags[fi] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
		{
			assertEquals(5, container.totalCountHint());
		}
		
		if ( (flags[fi] & MapFlags.HAS_SUMMARY_DATA) > 0)
		{
			assertTrue(summaryData.equals(container.encodedSummaryData()));
		}
		
		int eCount = 0;
		int ret;
		while ((ret = entry.decode(_decIter, deckey)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			assertEquals(CodecReturnCodes.SUCCESS, ret);
			if (hasPermData)
			{
				assertEquals(MapEntryFlags.HAS_PERM_DATA, entry.flags());
				assertTrue(permissionData.equals(entry.permData()));
			}
			else
			{
				assertEquals(MapEntryFlags.NONE, entry.flags());				
			}				
			assertEquals(entryActions[ai], entry.action());
			
			if (entry.action() == MapEntryActions.DELETE)
			{
				assertTrue(delName.equals(entry.encodedKey()));
			}
			else
			{
				assertTrue(keyData.equals(deckey));
				//assertEquals(keyData, entry.encodedKey());
				decodeFieldList();
			}
			entry.clear();
			eCount++;
		}
		assertEquals(fi*ai +1, eCount);
	}

	@Test
	public void MapUnitTests()
	{
		Map container = CodecFactory.createMap();
		MapEntry entry = CodecFactory.createMapEntry();

		int [] flags = {MapFlags.HAS_SUMMARY_DATA, 
				MapFlags.HAS_PER_ENTRY_PERM_DATA,
				MapFlags.HAS_TOTAL_COUNT_HINT,
				MapFlags.HAS_KEY_FIELD_ID
		};
		
		int [] entryActions = {MapEntryActions.ADD, MapEntryActions.DELETE, MapEntryActions.UPDATE};
		Buffer keyData = CodecFactory.createBuffer();	
		keyData.data("keyData");
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data("permission");
		Buffer summaryData = CodecFactory.createBuffer();
		summaryData.data("$");
		Buffer delName = CodecFactory.createBuffer();
		delName.data("Del");


		for (int i = 0; i< entryActions.length; i++)
		{
			for (int k = 0; k<flags.length; k++)
			{
				// for each entryAction/mapFlag test with entry with and with no permission data
				_encIter.clear();
				_buffer.data(ByteBuffer.allocate(500));
				_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
				container.clear();
				container.flags(flags[k]);
				if ( (flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
				{
					container.encodedSummaryData(summaryData);
				}
				if ( (flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
				{
					container.totalCountHint(5);
				}
		        container.containerType(DataTypes.FIELD_LIST);
		        container.keyPrimitiveType(DataTypes.ASCII_STRING);

				container.encodeInit(_encIter, 0, 0);	
				
				// encode entries
				for (int j = 0; j<=k*i; j++)
				{
					entry.clear();
					entry.flags(MapEntryFlags.NONE);
					entry.action(entryActions[i]);
					if (entry.action() == MapEntryActions.DELETE)
					{
						entry.encode(_encIter, delName);
					}
					else
					{
						entry.encodeInit(_encIter, keyData, 0);
						encodeFieldList();
						entry.encodeComplete(_encIter, true);
					}
				}
				container.encodeComplete(_encIter, true);

				// decode encoded element list and compare
				decodeMap(i,k, false);
				container.clear();
				
				if ((flags[k] & MapFlags.HAS_PER_ENTRY_PERM_DATA) > 0)
				{
					// for each entryAction/mapFlag test with entry with and with no permission data
					_encIter.clear();
					_buffer.data(ByteBuffer.allocate(500));
					_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
					container.flags(flags[k]);
					if ( (flags[k] & MapFlags.HAS_SUMMARY_DATA) > 0)
					{
						container.encodedSummaryData(summaryData);
					}
					if ( (flags[k] & MapFlags.HAS_TOTAL_COUNT_HINT) > 0)
					{
						container.totalCountHint(5);
					}
			        container.containerType(DataTypes.FIELD_LIST);
			        container.keyPrimitiveType(DataTypes.ASCII_STRING);

					container.encodeInit(_encIter, 0, 0);	
					
					// encode entries
					for (int j = 0; j<=k*i; j++)
					{
						entry.clear();
						entry.flags(MapEntryFlags.HAS_PERM_DATA);
						entry.permData(permissionData);
						entry.action(entryActions[i]);
						if (entry.action() == MapEntryActions.DELETE)
						{
							entry.encode(_encIter, delName);
						}
						else
						{
							entry.encodeInit(_encIter, keyData, 0);
							encodeFieldList();
							entry.encodeComplete(_encIter, true);
						}
					}
					container.encodeComplete(_encIter, true);

					// decode encoded element list and compare
					decodeMap(i,k, true);
					container.clear();
				}
			}			
		}
	}

	private void decodeFilterList(int ai, int fi, int ef, boolean hasPermData)
	{
		FilterList container = CodecFactory.createFilterList();
		FilterEntry entry = CodecFactory.createFilterEntry();

		int [] flags = {FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
				FilterListFlags.HAS_TOTAL_COUNT_HINT
		};
		
		int [] entryActions = {FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE};
		int [] entryFlags = {FilterEntryFlags.NONE, FilterEntryFlags.HAS_CONTAINER_TYPE};
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data("permission");

		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

		container.decode(_decIter);
		if ((fi == FilterListFlags.HAS_TOTAL_COUNT_HINT) || (hasPermData))
			assertEquals(flags[fi], container.flags());
		assertEquals(DataTypes.FIELD_LIST, container.containerType());
		if ( (flags[fi] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
		{
			assertEquals(5, container.totalCountHint());
		}
		
		int eCount = 0;
		int ret;
		while ((ret = entry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			assertEquals(CodecReturnCodes.SUCCESS, ret);
			if (hasPermData)
			{
				int f = FilterEntryFlags.HAS_PERM_DATA;
				if (ef == 1)
					f = f + FilterEntryFlags.HAS_CONTAINER_TYPE;
				assertEquals(f, entry.flags());
				assertTrue(permissionData.equals(entry.permData()));
			}
			else
			{
				assertEquals(entryFlags[ef], entry.flags());				
			}				
			assertEquals(entryActions[ai], entry.action());
			
			if (entry.action() != FilterEntryActions.CLEAR)
			{
				decodeFieldList();
			}
			entry.clear();
			eCount++;
		}
		assertEquals((fi+1)*(ai+1)*(ef+1) , eCount);
	}

	@Test
	public void FilterListUnitTests()
	{
		FilterList container = CodecFactory.createFilterList();
		FilterEntry entry = CodecFactory.createFilterEntry();

		int [] flags = {FilterListFlags.HAS_PER_ENTRY_PERM_DATA,
				FilterListFlags.HAS_TOTAL_COUNT_HINT
		};
		
		int [] entryActions = {FilterEntryActions.CLEAR, FilterEntryActions.SET, FilterEntryActions.UPDATE};
		int [] entryFlags = {FilterEntryFlags.NONE, FilterEntryFlags.HAS_CONTAINER_TYPE};
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data("permission");

		for (int i = 0; i< entryActions.length; i++)
		{
			for (int k = 0; k<flags.length; k++)
			{
				for (int l = 0; l<entryFlags.length; l++)
				{
					// for each entryAction/flag test with entry with no permission data
					_encIter.clear();
					_buffer.data(ByteBuffer.allocate(200));
					_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
					container.clear();
					container.flags(flags[k]);
					container.containerType(DataTypes.FIELD_LIST);
					if ( (flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
					{
						container.totalCountHint(5);
					}
					container.containerType(DataTypes.FIELD_LIST);
					assertEquals(CodecReturnCodes.SUCCESS, container.encodeInit(_encIter));	

					// encode entries
					for (int j = 0; j<(k+1)*(i+1)*(l+1); j++)
					{
						entry.clear();
						entry.flags(entryFlags[l]);
						entry.action(entryActions[i]);
						entry.id(k*i+1);
						if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
						{
							entry.containerType(DataTypes.FIELD_LIST);
						}
						assertEquals(CodecReturnCodes.SUCCESS, entry.encodeInit(_encIter, 0));
						if (entry.action() != FilterEntryActions.CLEAR)
						{
							encodeFieldList();
						}
						assertEquals(CodecReturnCodes.SUCCESS, entry.encodeComplete(_encIter, true));
					}
					assertEquals(CodecReturnCodes.SUCCESS, container.encodeComplete(_encIter, true));

					// decode encoded element list and compare
					decodeFilterList(i,k, l, false);
					container.clear();

					// for each entryAction/flag test with entry with permission data
					if (k == 0)
					{
						_encIter.clear();
						_buffer.data(ByteBuffer.allocate(200));
						_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
						container.clear();
						container.flags(flags[k]);
						if ( (flags[k] & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0)
						{
							container.totalCountHint(5);
						}
						container.containerType(DataTypes.FIELD_LIST);
						assertEquals(CodecReturnCodes.SUCCESS, container.encodeInit(_encIter));	

						// encode entries
						for (int j = 0; j<(k+1)*(i+1)*(l+1); j++)
						{
							entry.clear();
							entry.flags(entryFlags[l] + FilterEntryFlags.HAS_PERM_DATA);
							entry.permData(permissionData);
							entry.action(entryActions[i]);
							entry.id(k*i+1);
							if ((entryFlags[l] & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0)
							{
								entry.containerType(DataTypes.FIELD_LIST);
							}
							assertEquals(CodecReturnCodes.SUCCESS, entry.encodeInit(_encIter, 0));
							if (entry.action() != FilterEntryActions.CLEAR)
							{
								encodeFieldList();
							}
							assertEquals(CodecReturnCodes.SUCCESS, entry.encodeComplete(_encIter, true));
						}
						assertEquals(CodecReturnCodes.SUCCESS, container.encodeComplete(_encIter, true));

						// decode encoded element list and compare
						decodeFilterList(i,k, l, true);
						container.clear();
					}
				}
			}			
		}
	}
	

	@Test
	public void dataDictionaryTest()
	{
		DataDictionary dictionary = CodecFactory.createDataDictionary();
	    final String dictionaryFileName1 = "src/test/resources/com/thomsonreuters/upa/data/Codec/enumtype1.def";
	    final String dictionaryFileName2 = "src/test/resources/com/thomsonreuters/upa/data/Codec/enumtype2.def";
        com.thomsonreuters.upa.transport.Error error = TransportFactory.createError();

        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary(dictionaryFileName1, error));        
        assertEquals(196, dictionary.enumTableCount());
        
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary(dictionaryFileName2, error));        
        assertEquals(124, dictionary.enumTableCount());
		
	}
	
	@Test
	public void basicFieldListNestingTest()
	{
		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(200));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		FieldList container = CodecFactory.createFieldList();
		FieldList container1 = CodecFactory.createFieldList();
		FieldEntry entry = CodecFactory.createFieldEntry();
		FieldEntry entry1 = CodecFactory.createFieldEntry();
		
		Real paylReal = CodecFactory.createReal();
		paylReal.value(97, 2);

		Date paylDate = CodecFactory.createDate();
		paylDate.day(21);
		paylDate.month(10);
		paylDate.year(1978);
		
		container.applyHasStandardData();
		container.setId(0);
		assertEquals(CodecReturnCodes.SUCCESS, container.encodeInit(_encIter, null, 0));
		
		entry.clear();
		entry.dataType(DataTypes.DATE);
		entry.fieldId(1);
		assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylDate));
		
		entry.clear();
		entry.fieldId(2);
		assertEquals(CodecReturnCodes.SUCCESS, entry.encodeInit(_encIter, 100));
		
		container1.applyHasStandardData();
		assertEquals(CodecReturnCodes.SUCCESS, container1.encodeInit(_encIter, null, 0));
		entry1.clear();
		entry1.fieldId(3);
		entry1.dataType(DataTypes.REAL);
		assertEquals(CodecReturnCodes.SUCCESS, entry1.encode(_encIter, paylReal));
		assertEquals(CodecReturnCodes.SUCCESS, container1.encodeComplete(_encIter, true));
		assertEquals(CodecReturnCodes.SUCCESS, entry.encodeComplete(_encIter, true));
		
		entry.clear();
		entry.fieldId(4);
		entry.dataType(DataTypes.DATE);
		assertEquals(CodecReturnCodes.SUCCESS, entry.encode(_encIter, paylDate));

		assertEquals(CodecReturnCodes.SUCCESS, container.encodeComplete(_encIter, true));
		
		// now decode
		_decIter.clear();		
		_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		// reuse the containers
		container.clear();
		assertEquals(CodecReturnCodes.SUCCESS, container.decode(_decIter, null));
		assertEquals(0, container.setId());
		assertEquals(FieldListFlags.HAS_STANDARD_DATA, container.flags());
		
		int ret;
		entry.clear();
		entry1.clear();
		while ( (ret = entry.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if ((entry.fieldId() == 1) || (entry.fieldId() == 4))
			{
				assertEquals(CodecReturnCodes.SUCCESS, ret);
				paylDate.clear();
				assertEquals(CodecReturnCodes.SUCCESS, paylDate.decode(_decIter));
				assertEquals(21, paylDate.day());
				assertEquals(10, paylDate.month());
				assertEquals(1978, paylDate.year());
			}
			else if (entry.fieldId() == 2)
			{
				container1.clear();
				assertEquals(CodecReturnCodes.SUCCESS, container1.decode(_decIter, null));
				
				entry1.clear();
				while ( (ret = entry1.decode(_decIter)) != CodecReturnCodes.END_OF_CONTAINER)
				{
					assertEquals(CodecReturnCodes.SUCCESS, ret);
					if (entry1.fieldId() == 3)
					{
						paylReal.clear();
						assertEquals(CodecReturnCodes.SUCCESS, paylReal.decode(_decIter));
						assertEquals(97, paylReal.toLong());
						assertEquals(2, paylReal.hint());
					}
				}
			}
			entry.clear();
		}
	}
	
	private void encodeNestedStructure(int type, int nested)
	{
		FieldList fieldList = CodecFactory.createFieldList();
		FieldEntry fieldEntry = CodecFactory.createFieldEntry();
		Map map = CodecFactory.createMap();
		MapEntry mapEntry = CodecFactory.createMapEntry();
		FilterList filterList = CodecFactory.createFilterList();
		FilterEntry filterEntry = CodecFactory.createFilterEntry();
		UInt key = CodecFactory.createUInt();
		key.value(nested);
		
		if (nested == 0)
		{
			encodeFieldList();
		}
		else
		{
			switch(type)
			{
			case DataTypes.FIELD_LIST:
				fieldList.applyHasStandardData();
				assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(_encIter, null, 0));
				fieldEntry.clear();
				fieldEntry.fieldId(2);
				assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeInit(_encIter, 100));
				encodeNestedStructure(DataTypes.FIELD_LIST, nested-1);
				assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(_encIter, true));
				assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(_encIter, true));				
				break;
			case DataTypes.MAP:
				map.containerType(DataTypes.MAP);
				if (nested == 1)
					map.containerType(DataTypes.FIELD_LIST);
				map.keyPrimitiveType(DataTypes.UINT);
				assertEquals(CodecReturnCodes.SUCCESS, map.encodeInit(_encIter, 0, 0));
				mapEntry.clear();
				mapEntry.action(MapEntryActions.ADD);
				key.value(nested*2);
				assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeInit(_encIter, key, 100));
				encodeNestedStructure(DataTypes.MAP, nested-1);
				assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(_encIter, true));
				assertEquals(CodecReturnCodes.SUCCESS, map.encodeComplete(_encIter, true));				
				break;
			case DataTypes.FILTER_LIST:
				filterList.containerType(DataTypes.MAP);
				assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeInit(_encIter));
				filterEntry.clear();
				filterEntry.applyHasContainerType();
				if (nested == 1)
					filterEntry.containerType(DataTypes.FIELD_LIST);
				else
					filterEntry.containerType(DataTypes.FILTER_LIST);					
				filterEntry.action(FilterEntryActions.SET);
				filterEntry.id(nested+1);
				assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeInit(_encIter, 100));
				encodeNestedStructure(DataTypes.FILTER_LIST, nested-1);
				assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeComplete(_encIter, true));
				assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeComplete(_encIter, true));				
				break;
				default:
					System.out.println("not implemented ");
			}
		}
		
	}

	private void decodeNestedStructure(int type, int nested)
	{
		FieldList fieldList = CodecFactory.createFieldList();
		FieldEntry fieldEntry = CodecFactory.createFieldEntry();
		Map map = CodecFactory.createMap();
		MapEntry mapEntry = CodecFactory.createMapEntry();
		FilterList filterList = CodecFactory.createFilterList();
		FilterEntry filterEntry = CodecFactory.createFilterEntry();
		UInt key = CodecFactory.createUInt();
		key.value(nested);
		
		if (nested == 0)
		{
			decodeFieldList();
		}
		else
		{
			switch(type)
			{
			case DataTypes.FIELD_LIST:
				assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(_decIter, null));
				assertTrue(fieldList.checkHasStandardData());
				fieldEntry.clear();
				assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(_decIter));
				assertEquals(2, fieldEntry.fieldId());
				decodeNestedStructure(DataTypes.FIELD_LIST, nested-1);
				break;
			case DataTypes.MAP:
				assertEquals(CodecReturnCodes.SUCCESS, map.decode(_decIter));
				if (nested == 1)
					assertEquals(DataTypes.FIELD_LIST, map.containerType());
				else
					assertEquals(DataTypes.MAP, map.containerType());
				assertEquals(DataTypes.UINT, map.keyPrimitiveType());
				mapEntry.clear();
				assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(_decIter, key));
				assertEquals(nested*2, key.toLong());
				decodeNestedStructure(DataTypes.MAP, nested-1);
				break;
			case DataTypes.FILTER_LIST:
				assertEquals(CodecReturnCodes.SUCCESS, filterList.decode(_decIter));
				assertEquals(0, filterList.flags());
				filterEntry.clear();
				assertEquals(DataTypes.MAP, filterList.containerType());
				assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(_decIter));
				if (nested == 1)
					assertEquals(DataTypes.FIELD_LIST, filterEntry.containerType());
				else
					assertEquals(DataTypes.FILTER_LIST, filterEntry.containerType());				
				assertEquals(nested+1, filterEntry.id());
				decodeNestedStructure(DataTypes.FILTER_LIST, nested-1);
				break;
				default:
					System.out.println("not implemented ");
			}
		}
		
	}

	@Test
	public void nestedEncDecTest()
	{
		int nested = 5;
		_encIter.clear();
		_buffer.data(ByteBuffer.allocate(200));
		_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
		
		for (int i = 0; i<nested; i++)
		{
			_encIter.clear();
			_buffer.data(ByteBuffer.allocate(200));
			_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			encodeNestedStructure(DataTypes.FIELD_LIST, nested);
			// now decode
			_decIter.clear();		
			_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			decodeNestedStructure(DataTypes.FIELD_LIST, nested);
			
			_encIter.clear();
			_buffer.data(ByteBuffer.allocate(200));
			_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			encodeNestedStructure(DataTypes.MAP, nested);
			// now decode
			_decIter.clear();		
			_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			decodeNestedStructure(DataTypes.MAP, nested);
			
			_encIter.clear();
			_buffer.data(ByteBuffer.allocate(200));
			_encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			encodeNestedStructure(DataTypes.FILTER_LIST, nested);
			// now decode
			_decIter.clear();		
			_decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

			decodeNestedStructure(DataTypes.FILTER_LIST, nested);

		}
	}

	@Test
	public void entrySkippingTest()
	{
		ByteBuffer bufBig = ByteBuffer.allocate(16000000);
		Buffer tbufBig = CodecFactory.createBuffer();
		Map map = CodecFactory.createMap(); MapEntry mapEntry = CodecFactory.createMapEntry();
		FieldList fieldList = CodecFactory.createFieldList(); FieldEntry fieldEntry = CodecFactory.createFieldEntry();
		int iMapEntries, iFields, iSkipField;

		/* Some values to use - total 4 mapEntries with 4 fields in each entry. */
		final int totalMapEntries = 4;
		final int totalFields = 4;
		Buffer[] mapKeyStrings = new BufferImpl[4];
		mapKeyStrings[0] = CodecFactory.createBuffer();
		mapKeyStrings[0].data("GROUCHO");
		mapKeyStrings[1] = CodecFactory.createBuffer();
		mapKeyStrings[1].data("ZEPPO");
		mapKeyStrings[2] = CodecFactory.createBuffer();
		mapKeyStrings[2].data("HARPO");
		mapKeyStrings[3] = CodecFactory.createBuffer();
		mapKeyStrings[3].data("GUMMO");
		int fids[] = { 22, 26, 30, 31 };
		Real[] reals = new RealImpl[4];
		reals[0] = CodecFactory.createReal();
		reals[0].value(3194, RealHints.EXPONENT_2);
		reals[1] = CodecFactory.createReal();
		reals[1].value(3196, RealHints.EXPONENT_2);
		reals[2] = CodecFactory.createReal();
		reals[2].value(4, RealHints.EXPONENT_2);
		reals[3] = CodecFactory.createReal();
		reals[3].value(5, RealHints.EXPONENT_2);

		tbufBig.data(bufBig);

		_encIter.clear();
		_encIter.setBufferAndRWFVersion(tbufBig, Codec.majorVersion(), Codec.minorVersion());

		/* Encode a map with a field list in each entry */

		/* Map */
		map.clear();
		map.flags(MapFlags.NONE);
		map.keyPrimitiveType(DataTypes.ASCII_STRING);
		map.containerType(DataTypes.FIELD_LIST);
		assertEquals(CodecReturnCodes.SUCCESS, map.encodeInit(_encIter, 0, 0));

		/* MapEntry */
		for(iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
		{
			mapEntry.clear();
			mapEntry.flags(MapEntryFlags.NONE);
			mapEntry.action(MapEntryActions.ADD);
			assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeInit(_encIter, mapKeyStrings[iMapEntries], 0));

			/* FieldList */
			fieldList.clear();
			fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
			assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(_encIter, null, 0));
			for (iFields = 0; iFields < totalFields; ++iFields)
			{
				fieldEntry.clear();
				fieldEntry.fieldId(fids[iFields]); fieldEntry.dataType(DataTypes.REAL); 
				assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(_encIter, reals[iFields]));
			}
			assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(_encIter, true));

			assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(_encIter, true));
		}

		assertEquals(CodecReturnCodes.SUCCESS, map.encodeComplete(_encIter, true));

		/* Decode all map entries, skipping at various points */
		for (iSkipField = 0; iSkipField <= totalFields /* Includes decoding all fields before 'skipping' */; ++iSkipField)
		{
	    	Real real = CodecFactory.createReal();
			Buffer mapKey = CodecFactory.createBuffer();;
			_decIter.clear();
			_decIter.setBufferAndRWFVersion(tbufBig, Codec.majorVersion(), Codec.minorVersion());

			/* Decode map */
			map.clear();
			assertEquals(CodecReturnCodes.SUCCESS, map.decode(_decIter));
			assertEquals(MapFlags.NONE, map.flags());
			assertEquals(DataTypes.ASCII_STRING, map.keyPrimitiveType());

			for (iMapEntries = 0; iMapEntries < totalMapEntries; ++iMapEntries)
			{
				/* Decode mapEntry */
				mapEntry.clear();
				assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(_decIter, mapKey));
				assertTrue(mapKey.equals(mapKeyStrings[iMapEntries]));

				fieldList.clear();
	    		assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(_decIter, null));

				for (iFields = 0; iFields < iSkipField; ++iFields)
				{
					/* Decode fieldEntry and check value. */
					fieldEntry.clear();
					assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(_decIter));
					assertEquals(fids[iFields], fieldEntry.fieldId());
					assertEquals(CodecReturnCodes.SUCCESS, real.decode(_decIter));

					assertEquals(reals[iFields].hint(), real.hint());
					assertEquals(reals[iFields].isBlank(), real.isBlank());
					assertEquals(reals[iFields].toLong(), real.toLong());
				}
				assertEquals(CodecReturnCodes.END_OF_CONTAINER, _decIter.finishDecodeEntries());
			}
		}
	}

    /* Data and helper functions for containerOverrunTest. */

    int[] containerOverrunTest_fieldIds = new int[]{22, 25, 30};
    long[] containerOverrunTest_fieldValues = new long[]{0 /* Not used, first entry is blank */, 4200, 22};
    int[] containerOverrunTest_fieldHints = new int[]{0 /* Not used, first entry is blank */, RealHints.EXPONENT2, RealHints.EXPONENT0};

    /* Encoded length of the field list with the given number of entries. */
    int[] containerOverrunTest_fieldListEncodedSizes = new int[]{3, 6, 12, 17};

    /* Encoded length of the field with the given number of entries (using the set definition). */
    int[] containerOverrunTest_fieldListWithSetDefEncodedSizes = new int[]{1, 2, 6, 9};
    
    /* Encoded length of the field list set definition with the given number of entries. */
    int[] containerOverrunTest_fieldSetDefEncodedSizes = new int[]{4, 7, 10, 13 };
        
    /* Populates a sample set definition payload for use with containerOverrunTest. */
    void containerOverrunTest_createFieldSetDb(int fieldEntryCount, LocalFieldSetDefDb setDb)
    {
        setDb.clear();
        setDb.definitions()[0].setId(0);
        setDb.definitions()[0].count(fieldEntryCount);
        
        for (int i = 0; i < fieldEntryCount; ++i)
        {
            setDb.definitions()[0].entries()[i].fieldId(containerOverrunTest_fieldIds[i]);
            setDb.definitions()[0].entries()[i].dataType(DataTypes.REAL);
        }
    }

    /* Encodes the given set definition for use with containerOverrunTest. */
    int containerOverrunTest_encodeFieldSetDb(EncodeIterator eIter, LocalFieldSetDefDb setDb)
    {
        int ret;
        int startEncodePos = ((EncodeIteratorImpl)eIter)._curBufPos;
        int startEncodingLevel = ((EncodeIteratorImpl)eIter)._encodingLevel;
               
        if ((ret = setDb.encode(eIter)) != CodecReturnCodes.SUCCESS)
        {
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, ret);
            assertEquals(startEncodePos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startEncodingLevel, ((EncodeIteratorImpl)eIter)._encodingLevel);
            return ret;
        }
        
        return CodecReturnCodes.SUCCESS;
    }
 
    
    /* Encodes a sample field list payload for use with containerOverrunTest. */
    int containerOverrunTest_encodeFieldList(EncodeIterator eIter, int fieldEntryCount, LocalFieldSetDefDb setDb)
    {
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        int ret;
        int startEncodePos = ((EncodeIteratorImpl)eIter)._curBufPos;
        int startEncodingLevel = ((EncodeIteratorImpl)eIter)._encodingLevel;
        
        assertTrue(fieldEntryCount <= containerOverrunTest_fieldIds.length);
        
        if (setDb == null)
            fieldList.applyHasStandardData();
        else
            fieldList.applyHasSetData();
        
        if ((ret = fieldList.encodeInit(eIter, setDb, 0)) != CodecReturnCodes.SUCCESS)
        {
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, ret);
            assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, false));
            assertEquals(startEncodePos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startEncodingLevel, ((EncodeIteratorImpl)eIter)._encodingLevel);
            return ret;
        }

        assertEquals(startEncodingLevel + 1, ((EncodeIteratorImpl)eIter)._encodingLevel);

        /* Encode entries. */
        for (int i = 0; i < fieldEntryCount; ++i)
        {
            fieldEntry.clear();
            fieldEntry.fieldId(containerOverrunTest_fieldIds[i]);
            fieldEntry.dataType(DataTypes.REAL);
            real.value(containerOverrunTest_fieldValues[i], containerOverrunTest_fieldHints[i]);
            
            if ((ret = fieldEntry.encode(eIter, 
                            (i == 0) ? null : real /* Encode first entry as blank. */
                            )) == CodecReturnCodes.BUFFER_TOO_SMALL)
            {
                assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, false));
                assertEquals(startEncodePos, ((EncodeIteratorImpl)eIter)._curBufPos);
                assertEquals(startEncodingLevel, ((EncodeIteratorImpl)eIter)._encodingLevel);
                return ret;
            }

            
            if (i == fieldEntryCount - 1 && setDb != null)
                assertEquals(CodecReturnCodes.SET_COMPLETE, ret);
            else
                assertEquals(CodecReturnCodes.SUCCESS, ret);
        }
        
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, true));
        assertEquals(startEncodingLevel, ((EncodeIteratorImpl)eIter)._encodingLevel);
        
        return CodecReturnCodes.SUCCESS;
    }
    
    /* Decodes the sample field set definition database encoded in containerOverrunTest. */
    public void containerOverrunTest_decodeFieldSetDb(int fieldEntryCount, DecodeIterator dIter, LocalFieldSetDefDb setDb)
    {
        assertEquals(CodecReturnCodes.SUCCESS, setDb.decode(dIter));
        assertEquals(fieldEntryCount, setDb.definitions()[0].count());
        for (int i = 0; i < fieldEntryCount; ++i)
        {
            assertEquals(containerOverrunTest_fieldIds[i], setDb.definitions()[0].entries()[i].fieldId());
            assertEquals(DataTypes.REAL, setDb.definitions()[0].entries()[i].dataType());
        }
    }

    /* Decodes the sample field list encoded in containerOverrunTest. */
    public void containerOverrunTest_decodeFieldList(DecodeIterator dIter, int fieldEntryCount, LocalFieldSetDefDb setDb)
    {
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();

        assertTrue(fieldEntryCount <= containerOverrunTest_fieldIds.length);

        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, setDb));

        assertEquals(setDb != null, fieldList.checkHasSetData());
        assertEquals(setDb == null, fieldList.checkHasStandardData());

        for (int i = 0; i < fieldEntryCount; ++i)
        {
            assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
            assertEquals(containerOverrunTest_fieldIds[i], fieldEntry.fieldId());
            assertEquals((setDb != null) ? DataTypes.REAL : DataTypes.UNKNOWN, fieldEntry.dataType());
            if (i == 0) /* First entry should be blank */
                assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
            else
            {
                assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
                assertEquals(containerOverrunTest_fieldValues[i], real.toLong());
                assertEquals(containerOverrunTest_fieldHints[i], real.hint());
            }
        }

        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
    }
    
    /* Write sample bytes for an opaque buffer into a ByteBuffer. */
    public int containerOverrunTest_writeOpaqueByteBuffer(int dataSize, ByteBuffer sampleDataBuf, ByteBuffer outBuf)
    {
        assertTrue(dataSize <= outBuf.remaining());
            
        sampleDataBuf.limit(dataSize);
        outBuf.put(sampleDataBuf);
        sampleDataBuf.position(0);
        
        return CodecReturnCodes.SUCCESS;
    }
    
    /* Encode sample bytes for an opaque buffer inline. */
    public int containerOverrunTest_encodeOpaque(EncodeIterator eIter, ByteBuffer sampleDataBuf, int dataSize)
    {
        Buffer inlineBuffer = CodecFactory.createBuffer();
        int ret;

        assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));

        if (inlineBuffer.length() < dataSize)
        {
            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, false));
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if ((ret = containerOverrunTest_writeOpaqueByteBuffer(dataSize, sampleDataBuf, inlineBuffer.data())) != CodecReturnCodes.SUCCESS)
        {
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, ret);
            return ret;
        }
        
        assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
        return CodecReturnCodes.SUCCESS;
    }
    
    /* Verify the bytes received in an opaque buffer. */
    public void containerOverrunTest_decodeOpaque(Buffer encData, ByteBuffer sampleDataBuf, int dataSize)
    {
        assertEquals(dataSize, encData.length());
        sampleDataBuf.limit(dataSize);
        
        ByteBuffer tmpByteBuf = encData.data().duplicate();
        
        tmpByteBuf.position(encData.position());
        tmpByteBuf.limit(encData.position() + encData.length());
        
        assertTrue(tmpByteBuf.equals(sampleDataBuf));
    }

    /* Enumerates the ways the test will try to encode data (namely entries/summaryData/setDefinitions) */
    private enum ContainerOverrunTest_EncodeMethod
    {
        INLINE_ENCODE, /* Encode the entries/summaryData/setDefinitions by using the appropriate init & complete methods. */
        PRE_ENCODE, /* Encode the entries/summaryData/setDefinitions using the appropriate encode() method with pre-encoded data. */
        NONE, /* Don't encode at all. */
    }
    
    static int _serverPort = 40001;
    
    ReadArgs readArgs = TransportFactory.createReadArgs();
    WriteArgs writeArgs = TransportFactory.createWriteArgs();
    Error error = TransportFactory.createError();
    
    /* Sends an encoded buffer from one channel to the other. Used with containerOverrunTest */
    TransportBuffer containerOverrunTest_sendMessage(Channel writeChannel, TransportBuffer buffer, Channel readChannel)
    {
        TransportBuffer readBuffer = null;
        int ret;

        ret = writeChannel.write(buffer, writeArgs, error);
        assertTrue (ret >= 0);
        while (ret > 0) assertTrue((ret = writeChannel.flush(error)) >= 0);
        while ((readBuffer = readChannel.read(readArgs, error)) == null)
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
        
        return readBuffer;
    }

    @Test
    public void containerOverrunTest()
    {
        /* Test various scenarios of running out of space when encoding data, as well as rollback when this 
         * happens.
         * 
         * The test calculates the expected sizes needed to encode the data for each scenario, verifying that
         * encoding succeeds or fails with BUFFER_TOO_SMALL when expected. The encode buffer's size starts small and 
         * is increased by 1 until all data is successfully encoded.  
         * 
         * When encoding fails, the test rolls back encoding and verifies that the iterator moved to the expected position.
         * When encoding succeeds, the test then decodes it to verify the data (decodes are also done in cases where 
         * an entry runs out of space and we only roll back that last entry).
         * 
         * Tests different combinations of pre-encoding & inline-encoding set definitions, summary data, and 
         * entries for each of Vector, Map, Series, FilterList, FieldList, ElementList, and Array.
         * 
         * The buffers under test may have their bytebuffer's limit moved during encoding due to reserving bytes, so this 
         * test uses TransportBuffers from a channel and either writes & reads them or releases them.
         * 
         * See ETA-2214, ETA-2340. */

        /* Connect a channel so we can encode into a TransportBuffer. */
        Error error = TransportFactory.createError();
        BindOptions bindOpts = TransportFactory.createBindOptions();
        InitArgs initArgs = TransportFactory.createInitArgs();
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Server server = null;
        Channel clientChannel, serverChannel;
        Selector selector;

        bindOpts.serviceName(Integer.toString(_serverPort));
        bindOpts.tcpOpts().tcpNoDelay(true);
        connectOpts.unifiedNetworkInfo().address("localhost");
        connectOpts.unifiedNetworkInfo().serviceName(Integer.toString(_serverPort));
		try
		{
			assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
			assertNotNull(server = Transport.bind(bindOpts, error));
			connectOpts.tcpOpts().tcpNoDelay(true);
			assertNotNull(clientChannel = Transport.connect(connectOpts, error));
			
			selector = Selector.open();
			
			AcceptOptions acceptOpts = TransportFactory.createAcceptOptions();
			server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, null);
			assertTrue(selector.select(2000) > 0);
			assertNotNull(serverChannel = server.accept(acceptOpts, error));
			server.selectableChannel().keyFor(selector).cancel();
			
			while (clientChannel.state() != ChannelState.ACTIVE || serverChannel.state() != ChannelState.ACTIVE)
			{
				if (clientChannel.state() != ChannelState.ACTIVE)
				{
					int ret = clientChannel.init(inProg, error);
					assertTrue(ret == TransportReturnCodes.SUCCESS || ret == TransportReturnCodes.CHAN_INIT_IN_PROGRESS);
				}

				if (serverChannel.state() != ChannelState.ACTIVE)
				{
					int ret = serverChannel.init(inProg, error);
					assertTrue(ret == TransportReturnCodes.SUCCESS || ret == TransportReturnCodes.CHAN_INIT_IN_PROGRESS);
				}
			}
					
			EncodeIterator eIter = CodecFactory.createEncodeIterator();
			EncodeIterator preEncodeIter = CodecFactory.createEncodeIterator();
			DecodeIterator dIter = CodecFactory.createDecodeIterator();
			
			LocalFieldSetDefDb setDb = CodecFactory.createLocalFieldSetDefDb();
			
			/* Sizes to use when encoding opaque data. */
			int[] opaqueDataSizes = new int[] {0x10, 0xFD, 0xFE, 0xFF};
			
			final int byteBufferSize = 6144;
			ByteBuffer preEncEntryByteBuf = ByteBuffer.allocateDirect(byteBufferSize);
			ByteBuffer preEncSetDefByteBuf = ByteBuffer.allocateDirect(byteBufferSize);
			ByteBuffer preEncSummaryDataByteBuf = ByteBuffer.allocateDirect(byteBufferSize);
			ByteBuffer preEncMapKeyBuf = ByteBuffer.allocate(10);
			ByteBuffer expectedOpaqueData = ByteBuffer.allocateDirect(byteBufferSize);
			
			Buffer preEncodeBuffer = CodecFactory.createBuffer();
			TransportBuffer mainEncodeBuffer;
			WriteArgs writeArgs = TransportFactory.createWriteArgs();

			Vector vector = CodecFactory.createVector();
			VectorEntry vectorEntry = CodecFactory.createVectorEntry();
			Map map = CodecFactory.createMap();
			MapEntry mapEntry = CodecFactory.createMapEntry();
			Series series = CodecFactory.createSeries();
			SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
			FilterList filterList = CodecFactory.createFilterList();
			FilterEntry filterEntry = CodecFactory.createFilterEntry();
			FieldList fieldList = CodecFactory.createFieldList();
			FieldEntry fieldEntry = CodecFactory.createFieldEntry();
			ElementList elementList = CodecFactory.createElementList();
			ElementEntry elementEntry = CodecFactory.createElementEntry();
			Array array = CodecFactory.createArray();
			ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

			
			/* Set the expected data of any opaque encoding. */
			for (int i = 0; i < byteBufferSize; ++i)
				expectedOpaqueData.put((byte)0xDC);
			expectedOpaqueData.flip();
			
			/* All tests encode at most three entries. */
			final int maxInnerEntryCount = 3;

			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
			
			/* Tests for Map, Vector, and Series. */

			/* Test rolling back everything upon failing to encode an entry, or just rolling back the failed entry and validating the remaining data. */
			for (boolean rollbackContainerOnInnerEntryEncodeFail : new boolean[]{false, true})
			{
				/* Test pre-encoding entries, and encoding them inline. */
				for (ContainerOverrunTest_EncodeMethod entryEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.INLINE_ENCODE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
				{
					/* Test encoding OPAQUE and FIELD_LIST data as summary data and as entry payloads. */
					for (int innerContainerType : new int[]{DataTypes.OPAQUE, DataTypes.FIELD_LIST})
					{
						/* Test encoding, or not encoding a totalCountHint. */
						for (boolean encodeTotalCountHint : new boolean[]{false, true})
						{
							/* Test not encoding set defs, pre-encoding them, and encoding them inline. */
							for (ContainerOverrunTest_EncodeMethod setDefsEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.NONE, ContainerOverrunTest_EncodeMethod.INLINE_ENCODE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
							{
								/* Test not encoding summary data, pre-encoding it, and encoding it inline. */
								for (ContainerOverrunTest_EncodeMethod summaryDataEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.NONE, ContainerOverrunTest_EncodeMethod.INLINE_ENCODE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
								{
									for (int sizeIndex = 0; sizeIndex < ((innerContainerType == DataTypes.OPAQUE) ? opaqueDataSizes.length : containerOverrunTest_fieldIds.length); ++sizeIndex)
									{
										int payloadEncodedSize = 0;
										int setDefEncodedSize = 0;

										boolean rollbackEntireContainer = false;
										int innerEntryEncodeCount = 0;

										switch(innerContainerType)
										{
											case DataTypes.FIELD_LIST: 
												payloadEncodedSize = 
													(setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? 
													containerOverrunTest_fieldListWithSetDefEncodedSizes[sizeIndex] : containerOverrunTest_fieldListEncodedSizes[sizeIndex];
												setDefEncodedSize = (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ?
													containerOverrunTest_fieldSetDefEncodedSizes[sizeIndex] : 0;
												break;
											case DataTypes.OPAQUE: 
												payloadEncodedSize = opaqueDataSizes[sizeIndex];
												setDefEncodedSize = opaqueDataSizes[sizeIndex];
												break;

											default: fail();
										}

										/* Vector */
										final int vectorHeaderEncodedSize = 1 /* flags */ + 1 /* Container type */ + 2 /* Entry count */;
										final int vectorEntryHeaderEncodedSize = 1 /* Action & flags */ + 2 /* index */; 

										/* Keep increasing the buffer size until we successfully encode all intended data
										 * (three entries, plus set definitions and/or summary data if they are part of the test) */
										boolean encodeTestPassed = false;
										for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
										{                
											int bufferSizeNeeded = 0;

											rollbackEntireContainer = false;

											mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
											assertNotNull(mainEncodeBuffer);

											/* Encode */
											eIter.clear();
											eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

											vector.clear();

											if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												vector.applyHasSetDefs();

											if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												vector.applyHasSummaryData();

											if (encodeTotalCountHint)
											{
												vector.applyHasTotalCountHint();
												vector.totalCountHint(3);
												bufferSizeNeeded += 4;
											}

											vector.containerType(innerContainerType);

											if (innerContainerType == DataTypes.FIELD_LIST && setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												containerOverrunTest_createFieldSetDb(sizeIndex, setDb);

											/* If pre-encoding set definitions, encode and set them now. */
											if (setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
											{
												preEncSetDefByteBuf.clear();
												switch(innerContainerType)
												{   
													case DataTypes.FIELD_LIST:
														preEncodeBuffer.clear();
														preEncodeBuffer.data(preEncSetDefByteBuf);
														preEncodeIter.clear();
														preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(preEncodeIter, setDb));
														vector.encodedSetDefs(preEncodeBuffer);
														break;

													case DataTypes.OPAQUE:
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSetDefByteBuf));
														preEncSetDefByteBuf.flip();
														vector.encodedSetDefs().data(preEncSetDefByteBuf);
														break;

													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* If pre-encoding summary data, encode and set it now. */
											if (summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
											{
												preEncSummaryDataByteBuf.clear();
												switch(innerContainerType)
												{   
													case DataTypes.FIELD_LIST:
														preEncodeBuffer.clear();
														preEncodeBuffer.data(preEncSummaryDataByteBuf);
														preEncodeIter.clear();
														preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
														vector.encodedSummaryData(preEncodeBuffer);
														break;

													case DataTypes.OPAQUE:
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSummaryDataByteBuf));
														preEncSummaryDataByteBuf.flip();
														vector.encodedSummaryData().data(preEncSummaryDataByteBuf);
														break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* Determine size needed for vector.encodeInit to succeed. */
											bufferSizeNeeded += vectorHeaderEncodedSize;
											switch(setDefsEncodeMethod)
											{
												case NONE: break;
												case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
												case PRE_ENCODE: bufferSizeNeeded += (setDefEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + setDefEncodedSize; break;
												default: fail();
											}

											switch(summaryDataEncodeMethod)
											{
												case NONE: break;
												case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
												case PRE_ENCODE: bufferSizeNeeded += (payloadEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + payloadEncodedSize; break;
												default: fail();
											}

											/* Start encoding vector. */
											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, vector.encodeInit(eIter, 0, 0));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, vector.encodeInit(eIter, 0, 0));
												rollbackEntireContainer = true;
											}

											if (!rollbackEntireContainer && setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
											{
												int setDefEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

												bufferSizeNeeded += setDefEncodedSize;

												/* If inline-encoding set definitions, encode them now. */
												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST:
														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
															rollbackEntireContainer = true;
														}

														assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSetDefsComplete(eIter, !rollbackEntireContainer));
														break;

													case DataTypes.OPAQUE:
														if (bufferSizeNeeded <= encodeBufSize)
														{
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSetDefsComplete(eIter, true));
														}
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSetDefsComplete(eIter, false));
															rollbackEntireContainer = true;
														}
														break;

													default: fail("No testing implemented for this containerType"); break;
												}

												if (rollbackEntireContainer)
												{
													assertEquals(setDefEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
													assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
												}
											}

											if (!rollbackEntireContainer && summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
											{
												int summaryDataEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

												bufferSizeNeeded += payloadEncodedSize;

												/* If inline-encoding summary data, encode it now. */
												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST:
														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
															rollbackEntireContainer = true;
														}

														assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSummaryDataComplete(eIter, !rollbackEntireContainer));
														break;

													case DataTypes.OPAQUE:
														if (bufferSizeNeeded <= encodeBufSize)
														{
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSummaryDataComplete(eIter, true));
														}
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, vector.encodeSummaryDataComplete(eIter, false));
															rollbackEntireContainer = true;
														}
														break;

													default: fail("No testing implemented for this containerType"); break;
												}

												if (rollbackEntireContainer)
												{
													assertEquals(summaryDataEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
													assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
												}
											}


											if (encodeTotalCountHint)
											{
												/* The Encoder reserves the worst-case size (4) for the totalCountHint, but in reality we only need 1 byte.
												 * At this point the totalCountHint has been encoded, so the remaining bytes should be available. */
												bufferSizeNeeded -= 3;
											}

											boolean innerEntryEncodeFailed = false;
											innerEntryEncodeCount = 0;
											if (!rollbackEntireContainer)
											{
												while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
												{
													int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

													/* Now try to encode VectorEntries, using inline encoding or pre-encoded data. 
													 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
													vectorEntry.clear();
													vectorEntry.index(1025);
													vectorEntry.action(VectorEntryActions.INSERT);

													if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
													{
														bufferSizeNeeded += vectorEntryHeaderEncodedSize + 3 /* reserved size bytes */;

														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encodeInit(eIter, 0));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, vectorEntry.encodeInit(eIter, 0));
															assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encodeComplete(eIter, false));
															innerEntryEncodeFailed = true;
															break;
														}

														bufferSizeNeeded += payloadEncodedSize;

														switch(innerContainerType)
														{
															case DataTypes.FIELD_LIST:
																if (bufferSizeNeeded <= encodeBufSize)
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																	innerEntryEncodeFailed = true;
																}

																assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
																break;

															case DataTypes.OPAQUE:
																if (bufferSizeNeeded <= encodeBufSize)
																{
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																	assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encodeComplete(eIter, true));
																}
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																	assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encodeComplete(eIter, false));
																	innerEntryEncodeFailed = true;
																}
																break;

															default: fail("No testing implemented for this containerType"); break;
														}

													}
													else
													{
														/* Pre-encode entry payload. */
														preEncEntryByteBuf.clear();
														preEncodeBuffer.clear();
														switch(innerContainerType)
														{   
															case DataTypes.FIELD_LIST:
																preEncodeBuffer.data(preEncEntryByteBuf);
																preEncodeIter.clear();
																preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																break;
															case DataTypes.OPAQUE:
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
																preEncEntryByteBuf.flip();
																preEncodeBuffer.data(preEncEntryByteBuf);
																break;
															default: fail("No testing implemented for this containerType"); break;
														}

														bufferSizeNeeded += vectorEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

														vectorEntry.encodedData(preEncodeBuffer);

														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.encode(eIter));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, vectorEntry.encode(eIter));
															innerEntryEncodeFailed = true;
														}
													}

													if (innerEntryEncodeFailed)
													{
														assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
														assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
													}
													else
														++innerEntryEncodeCount;
												}
											}

											/* If vector header,set defs, or summary data failed to encode, roll it back.
											 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
											 * Otherwise move onto the decode anyway to verify the partial data. */
											if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
											{
												assertEquals(CodecReturnCodes.SUCCESS, vector.encodeComplete(eIter, false));

												/* Iterator should be at start of buffer. */
												assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
												assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
												assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
												continue;
											}

											assertEquals(CodecReturnCodes.SUCCESS, vector.encodeComplete(eIter, true));

											/* Send buffer over connection. */
											mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);
	 
											/* Decode vector. */
											dIter.clear();
											dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
										 
											assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
											assertEquals(innerContainerType, vector.containerType());

											if (encodeTotalCountHint)
											{
												assertTrue(vector.checkHasTotalCountHint());
												assertEquals(3, vector.totalCountHint());
											}

											/* Decode set defs if present. */
											if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
											{
												assertTrue(vector.checkHasSetDefs());

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldSetDb(sizeIndex, dIter, setDb); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(vector.encodedSetDefs(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* Decode summary data if present. */
											if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
											{
												assertTrue(vector.checkHasSummaryData());

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(vector.encodedSummaryData(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* Decode all entries present. */
											for (int i = 0; i < innerEntryEncodeCount; ++i)
											{
												assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.decode(dIter));
												assertEquals(1025, vectorEntry.index());
												assertEquals(VectorEntryActions.INSERT, vectorEntry.action());

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(vectorEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											assertEquals(CodecReturnCodes.END_OF_CONTAINER, vectorEntry.decode(dIter));

											/* Stop on successful encode & decode of three vector entries (and summaryData/setDefs if present). */
											if (innerEntryEncodeCount == maxInnerEntryCount)
											{
												encodeTestPassed = true;
												break;
											}
										}
										assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
										/* End Vector */

										/* Test both method of encoding mapKey: None(passed in as an argument), or pre-encoded. */
										for (ContainerOverrunTest_EncodeMethod mapKeyEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.NONE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
										{
											/* Map */
											final int mapHeaderEncodedSize = 1 /* flags */ + 1 /* Container type */ + 1 /* Key primitive type */ + 2 /* Entry count */;
											final int mapEntryHeaderEncodedSize = 1 /* Action & flags */ + 3 /* key */; 
											Int key = CodecFactory.createInt();

											/* Keep increasing the buffer size until we successfully encode all intended data
											 * (three entries, plus set definitions and/or summary data if they are part of the test) */
											encodeTestPassed = false;
											for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
											{                
												int bufferSizeNeeded = 0;

												rollbackEntireContainer = false;

												mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
												assertNotNull(mainEncodeBuffer);

												/* Encode */
												eIter.clear();
												eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

												map.clear();

												if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
													map.applyHasSetDefs();

												if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
													map.applyHasSummaryData();

												if (encodeTotalCountHint)
												{
													map.applyHasTotalCountHint();
													map.totalCountHint(3);
													bufferSizeNeeded += 4;
												}

												map.containerType(innerContainerType);
												map.keyPrimitiveType(DataTypes.INT);

												if (innerContainerType == DataTypes.FIELD_LIST && setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
													containerOverrunTest_createFieldSetDb(sizeIndex, setDb);

												/* If pre-encoding set definitions, encode and set them now. */
												if (setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
												{
													preEncSetDefByteBuf.clear();
													switch(innerContainerType)
													{   
														case DataTypes.FIELD_LIST:
															preEncodeBuffer.clear();
															preEncodeBuffer.data(preEncSetDefByteBuf);
															preEncodeIter.clear();
															preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(preEncodeIter, setDb));
															map.encodedSetDefs(preEncodeBuffer);
															break;

														case DataTypes.OPAQUE:
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSetDefByteBuf));
															preEncSetDefByteBuf.flip();
															map.encodedSetDefs().data(preEncSetDefByteBuf);
															break;

														default: fail("No testing implemented for this containerType"); break;
													}
												}

												/* If pre-encoding summary data, encode and set it now. */
												if (summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
												{
													preEncSummaryDataByteBuf.clear();
													switch(innerContainerType)
													{   
														case DataTypes.FIELD_LIST:
															preEncodeBuffer.clear();
															preEncodeBuffer.data(preEncSummaryDataByteBuf);
															preEncodeIter.clear();
															preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
															map.encodedSummaryData(preEncodeBuffer);
															break;

														case DataTypes.OPAQUE:
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSummaryDataByteBuf));
															preEncSummaryDataByteBuf.flip();
															map.encodedSummaryData().data(preEncSummaryDataByteBuf);
															break;
														default: fail("No testing implemented for this containerType"); break;
													}
												}


												/* Determine size needed for map.encodeInit to succeed. */
												bufferSizeNeeded += mapHeaderEncodedSize;
												switch(setDefsEncodeMethod)
												{
													case NONE: break;
													case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
													case PRE_ENCODE: bufferSizeNeeded += (setDefEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + setDefEncodedSize; break;
													default: fail();
												}

												switch(summaryDataEncodeMethod)
												{
													case NONE: break;
													case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
													case PRE_ENCODE: bufferSizeNeeded += (payloadEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + payloadEncodedSize; break;
													default: fail();
												}

												/* Start encoding map. */
												if (bufferSizeNeeded <= encodeBufSize)
													assertEquals(CodecReturnCodes.SUCCESS, map.encodeInit(eIter, 0, 0));
												else
												{
													assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, map.encodeInit(eIter, 0, 0));
													rollbackEntireContainer = true;
												}

												if (!rollbackEntireContainer && setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
												{
													int setDefEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

													bufferSizeNeeded += setDefEncodedSize;

													/* If inline-encoding set definitions, encode them now. */
													switch(innerContainerType)
													{
														case DataTypes.FIELD_LIST:
															if (bufferSizeNeeded <= encodeBufSize)
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
															else
															{
																assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
																rollbackEntireContainer = true;
															}

															assertEquals(CodecReturnCodes.SUCCESS, map.encodeSetDefsComplete(eIter, !rollbackEntireContainer));
															break;

														case DataTypes.OPAQUE:
															if (bufferSizeNeeded <= encodeBufSize)
															{
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																assertEquals(CodecReturnCodes.SUCCESS, map.encodeSetDefsComplete(eIter, true));
															}
															else
															{
																assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																assertEquals(CodecReturnCodes.SUCCESS, map.encodeSetDefsComplete(eIter, false));
																rollbackEntireContainer = true;
															}
															break;

														default: fail("No testing implemented for this containerType"); break;
													}

													if (rollbackEntireContainer)
													{
														assertEquals(setDefEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
														assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
													}
												}

												if (!rollbackEntireContainer && summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
												{
													int summaryDataEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

													bufferSizeNeeded += payloadEncodedSize;

													/* If inline-encoding summary data, encode it now. */
													switch(innerContainerType)
													{
														case DataTypes.FIELD_LIST:
															if (bufferSizeNeeded <= encodeBufSize)
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
															else
															{
																assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																rollbackEntireContainer = true;
															}

															assertEquals(CodecReturnCodes.SUCCESS, map.encodeSummaryDataComplete(eIter, !rollbackEntireContainer));
															break;

														case DataTypes.OPAQUE:
															if (bufferSizeNeeded <= encodeBufSize)
															{
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																assertEquals(CodecReturnCodes.SUCCESS, map.encodeSummaryDataComplete(eIter, true));
															}
															else
															{
																assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																assertEquals(CodecReturnCodes.SUCCESS, map.encodeSummaryDataComplete(eIter, false));
																rollbackEntireContainer = true;
															}
															break;

														default: fail("No testing implemented for this containerType"); break;
													}

													if (rollbackEntireContainer)
													{
														assertEquals(summaryDataEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
														assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
													}
												}

												if (encodeTotalCountHint)
												{
													/* The Encoder reserves the worst-case size (4) for the totalCountHint, but in reality we only need 1 byte.
													 * At this point the totalCountHint has been encoded, so the remaining bytes should be available. */
													bufferSizeNeeded -= 3;
												}

												boolean innerEntryEncodeFailed = false;
												innerEntryEncodeCount = 0;
												if (!rollbackEntireContainer)
												{
													while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
													{
														int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

														/* Now try to encode MapEntries, using inline encoding or pre-encoded data. 
														 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
														mapEntry.clear();
														key.value(1025);
														mapEntry.action(MapEntryActions.ADD);

														if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
														{
															bufferSizeNeeded += mapEntryHeaderEncodedSize + 3 /* reserved size bytes */;

															if (mapKeyEncodeMethod == ContainerOverrunTest_EncodeMethod.NONE)
															{
																/* Encode mapEntry with key included as data. */
																if (bufferSizeNeeded <= encodeBufSize)
																	assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeInit(eIter, key, 0));
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, mapEntry.encodeInit(eIter, key, 0));
																	assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, false));
																	innerEntryEncodeFailed = true;
																	break;
																}
															}
															else /* Pre-encode. */
															{
																/* Pre-encode mapEntry key */
																preEncMapKeyBuf.clear();
																preEncodeBuffer.clear();
																preEncodeBuffer.data(preEncMapKeyBuf);
																preEncodeIter.clear();
																preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
																assertEquals(CodecReturnCodes.SUCCESS, key.encode(preEncodeIter));
																mapEntry.encodedKey(preEncodeBuffer);

																if (bufferSizeNeeded <= encodeBufSize)
																	assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeInit(eIter, 0));
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, mapEntry.encodeInit(eIter, 0));
																	assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, false));
																	innerEntryEncodeFailed = true;
																	break;
																}
															}

															bufferSizeNeeded += payloadEncodedSize;

															switch(innerContainerType)
															{
																case DataTypes.FIELD_LIST:
																	if (bufferSizeNeeded <= encodeBufSize)
																		assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																	else
																	{
																		assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																		innerEntryEncodeFailed = true;
																	}

																	assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
																	break;

																case DataTypes.OPAQUE:
																	if (bufferSizeNeeded <= encodeBufSize)
																	{
																		assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																		assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, true));
																	}
																	else
																	{
																		assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																		assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, false));
																		innerEntryEncodeFailed = true;
																	}
																	break;

																default: fail("No testing implemented for this containerType"); break;
															}

														}
														else
														{
															/* Pre-encode entry payload. */
															preEncEntryByteBuf.clear();
															preEncodeBuffer.clear();
															switch(innerContainerType)
															{   
																case DataTypes.FIELD_LIST:
																	preEncodeBuffer.data(preEncEntryByteBuf);
																	preEncodeIter.clear();
																	preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																	break;
																case DataTypes.OPAQUE:
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
																	preEncEntryByteBuf.flip();
																	preEncodeBuffer.data(preEncEntryByteBuf);
																	break;
																default: fail("No testing implemented for this containerType"); break;
															}

															bufferSizeNeeded += mapEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

															mapEntry.encodedData(preEncodeBuffer);

															if (bufferSizeNeeded <= encodeBufSize)
																assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encode(eIter, key));
															else
															{
																assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, mapEntry.encode(eIter, key));
																innerEntryEncodeFailed = true;
															}
														}

														if (innerEntryEncodeFailed)
														{
															assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
															assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
														}
														else
															++innerEntryEncodeCount;
													}
												}

												/* If map header,set defs, or summary data failed to encode, roll it back.
												 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
												 * Otherwise move onto the decode anyway to verify the partial data. */
												if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
												{
													assertEquals(CodecReturnCodes.SUCCESS, map.encodeComplete(eIter, false));

													/* Iterator should be at start of buffer. */
													assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
													assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
													assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
													continue;
												}

												assertEquals(CodecReturnCodes.SUCCESS, map.encodeComplete(eIter, true));

												/* Send buffer over connection. */
												mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

												/* Decode map. */
												dIter.clear();
												dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

												assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
												assertEquals(innerContainerType, map.containerType());

												if (encodeTotalCountHint)
												{
													assertTrue(map.checkHasTotalCountHint());
													assertEquals(3, map.totalCountHint());
												}

												/* Decode set defs if present. */
												if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												{
													assertTrue(map.checkHasSetDefs());

													switch(innerContainerType)
													{
														case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldSetDb(sizeIndex, dIter, setDb); break;
														case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(map.encodedSetDefs(), expectedOpaqueData, payloadEncodedSize); break;
														default: fail("No testing implemented for this containerType"); break;
													}
												}

												/* Decode summary data if present. */
												if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												{
													assertTrue(map.checkHasSummaryData());

													switch(innerContainerType)
													{
														case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
														case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(map.encodedSummaryData(), expectedOpaqueData, payloadEncodedSize); break;
														default: fail("No testing implemented for this containerType"); break;
													}
												}

												/* Decode all entries present. */
												for (int i = 0; i < innerEntryEncodeCount; ++i)
												{
													key.clear();
													assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, key));
													assertEquals(1025, key.toLong());
													assertEquals(MapEntryActions.ADD, mapEntry.action());

													switch(innerContainerType)
													{
														case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
														case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(mapEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
														default: fail("No testing implemented for this containerType"); break;
													}
												}

												assertEquals(CodecReturnCodes.END_OF_CONTAINER, mapEntry.decode(dIter, key));

												/* Stop on successful encode & decode of three map entries (and summaryData/setDefs if present). */
												if (innerEntryEncodeCount == maxInnerEntryCount)
												{
													encodeTestPassed = true;
													break;
												}
											}
										} /* End mapKey encode method. */
										assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
										/* End Map */

										/* Series */
										final int seriesHeaderEncodedSize = 1 /* flags */ + 1 /* Container type */ + 2 /* Entry count */;
										final int seriesEntryHeaderEncodedSize = 0 /* Nothing */;

										/* Keep increasing the buffer size until we successfully encode all intended data
										 * (three entries, plus set definitions and/or summary data if they are part of the test) */
										encodeTestPassed = false;
										for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
										{                
											int bufferSizeNeeded = 0;

											rollbackEntireContainer = false;

											mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
											assertNotNull(mainEncodeBuffer);

											/* Encode */
											eIter.clear();
											eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

											series.clear();

											if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												series.applyHasSetDefs();

											if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												series.applyHasSummaryData();

											if (encodeTotalCountHint)
											{
												series.applyHasTotalCountHint();
												series.totalCountHint(3);
												bufferSizeNeeded += 4;
											}

											series.containerType(innerContainerType);

											if (innerContainerType == DataTypes.FIELD_LIST && setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
												containerOverrunTest_createFieldSetDb(sizeIndex, setDb);

											/* If pre-encoding set definitions, encode and set them now. */
											if (setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
											{
												preEncSetDefByteBuf.clear();
												switch(innerContainerType)
												{   
													case DataTypes.FIELD_LIST:
														preEncodeBuffer.clear();
														preEncodeBuffer.data(preEncSetDefByteBuf);
														preEncodeIter.clear();
														preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(preEncodeIter, setDb));
														series.encodedSetDefs(preEncodeBuffer);
														break;

													case DataTypes.OPAQUE:
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSetDefByteBuf));
														preEncSetDefByteBuf.flip();
														series.encodedSetDefs().data(preEncSetDefByteBuf);
														break;

													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* If pre-encoding summary data, encode and set it now. */
											if (summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.PRE_ENCODE)
											{
												preEncSummaryDataByteBuf.clear();
												switch(innerContainerType)
												{   
													case DataTypes.FIELD_LIST:
														preEncodeBuffer.clear();
														preEncodeBuffer.data(preEncSummaryDataByteBuf);
														preEncodeIter.clear();
														preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
														series.encodedSummaryData(preEncodeBuffer);
														break;

													case DataTypes.OPAQUE:
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncSummaryDataByteBuf));
														preEncSummaryDataByteBuf.flip();
														series.encodedSummaryData().data(preEncSummaryDataByteBuf);
														break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}


											/* Determine size needed for series.encodeInit to succeed. */
											bufferSizeNeeded += seriesHeaderEncodedSize;
											switch(setDefsEncodeMethod)
											{
												case NONE: break;
												case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
												case PRE_ENCODE: bufferSizeNeeded += (setDefEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + setDefEncodedSize; break;
												default: fail();
											}

											switch(summaryDataEncodeMethod)
											{
												case NONE: break;
												case INLINE_ENCODE: bufferSizeNeeded += 2 /* reserved size byte */; break;
												case PRE_ENCODE: bufferSizeNeeded += (payloadEncodedSize >= 0x80 ? 2 : 1) /* size byte(s) */ + payloadEncodedSize; break;
												default: fail();
											}

											/* Start encoding series. */
											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, series.encodeInit(eIter, 0, 0));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, series.encodeInit(eIter, 0, 0));
												rollbackEntireContainer = true;
											}

											if (!rollbackEntireContainer && setDefsEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
											{
												int setDefEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

												bufferSizeNeeded += setDefEncodedSize;

												/* If inline-encoding set definitions, encode them now. */
												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST:
														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldSetDb(eIter, setDb));
															rollbackEntireContainer = true;
														}

														assertEquals(CodecReturnCodes.SUCCESS, series.encodeSetDefsComplete(eIter, !rollbackEntireContainer));
														break;

													case DataTypes.OPAQUE:
														if (bufferSizeNeeded <= encodeBufSize)
														{
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, series.encodeSetDefsComplete(eIter, true));
														}
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, series.encodeSetDefsComplete(eIter, false));
															rollbackEntireContainer = true;
														}
														break;

													default: fail("No testing implemented for this containerType"); break;
												}

												if (rollbackEntireContainer)
												{
													assertEquals(setDefEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
													assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
												}
											}

											if (!rollbackEntireContainer && summaryDataEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
											{
												int summaryDataEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

												bufferSizeNeeded += payloadEncodedSize;

												/* If inline-encoding summary data, encode it now. */
												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST:
														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
															rollbackEntireContainer = true;
														}

														assertEquals(CodecReturnCodes.SUCCESS, series.encodeSummaryDataComplete(eIter, !rollbackEntireContainer));
														break;

													case DataTypes.OPAQUE:
														if (bufferSizeNeeded <= encodeBufSize)
														{
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, series.encodeSummaryDataComplete(eIter, true));
														}
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, series.encodeSummaryDataComplete(eIter, false));
															rollbackEntireContainer = true;
														}
														break;

													default: fail("No testing implemented for this containerType"); break;
												}

												if (rollbackEntireContainer)
												{
													assertEquals(summaryDataEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
													assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
												}
											}

											if (encodeTotalCountHint)
											{
												/* The Encoder reserves the worst-case size (4) for the totalCountHint, but in reality we only need 1 byte.
												 * At this point the totalCountHint has been encoded, so the remaining bytes should be available. */
												bufferSizeNeeded -= 3;
											}

											boolean innerEntryEncodeFailed = false;
											innerEntryEncodeCount = 0;
											if (!rollbackEntireContainer)
											{
												while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
												{
													int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

													/* Now try to encode SeriesEntries, using inline encoding or pre-encoded data. 
													 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
													seriesEntry.clear();

													if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
													{
														bufferSizeNeeded += seriesEntryHeaderEncodedSize + 3 /* reserved size bytes */;

														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeInit(eIter, 0));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, seriesEntry.encodeInit(eIter, 0));
															assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, false));
															innerEntryEncodeFailed = true;
															break;
														}

														bufferSizeNeeded += payloadEncodedSize;

														switch(innerContainerType)
														{
															case DataTypes.FIELD_LIST:
																if (bufferSizeNeeded <= encodeBufSize)
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																	innerEntryEncodeFailed = true;
																}

																assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
																break;

															case DataTypes.OPAQUE:
																if (bufferSizeNeeded <= encodeBufSize)
																{
																	assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																	assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, true));
	 
																}
																else
																{
																	assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
																	assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, false));
																	innerEntryEncodeFailed = true;
																}
																break;

															default: fail("No testing implemented for this containerType"); break;
														}

													}
													else
													{
														/* Pre-encode entry payload. */
														preEncEntryByteBuf.clear();
														preEncodeBuffer.clear();
														switch(innerContainerType)
														{   
															case DataTypes.FIELD_LIST:
																preEncodeBuffer.data(preEncEntryByteBuf);
																preEncodeIter.clear();
																preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null));
																break;
															case DataTypes.OPAQUE:
																assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
																preEncEntryByteBuf.flip();
																preEncodeBuffer.data(preEncEntryByteBuf);
																break;
															default: fail("No testing implemented for this containerType"); break;
														}

														bufferSizeNeeded += seriesEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

														seriesEntry.encodedData(preEncodeBuffer);

														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encode(eIter));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, seriesEntry.encode(eIter));
															innerEntryEncodeFailed = true;
														}
													}

													if (innerEntryEncodeFailed)
													{
														assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
														assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
													}
													else
														++innerEntryEncodeCount;
												}
											}

											/* If series header,set defs, or summary data failed to encode, roll it back.
											 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
											 * Otherwise move onto the decode anyway to verify the partial data. */
											if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
											{
												assertEquals(CodecReturnCodes.SUCCESS, series.encodeComplete(eIter, false));

												/* Iterator should be at start of buffer. */
												assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
												assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
												assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
												continue;
											}

											assertEquals(CodecReturnCodes.SUCCESS, series.encodeComplete(eIter, true));

											/* Send buffer over connection. */
											mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

											/* Decode series. */
											dIter.clear();
											dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

											assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
											assertEquals(innerContainerType, series.containerType());

											if (encodeTotalCountHint)
											{
												assertTrue(series.checkHasTotalCountHint());
												assertEquals(3, series.totalCountHint());
											}

											/* Decode set defs if present. */
											if (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
											{
												assertTrue(series.checkHasSetDefs());

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldSetDb(sizeIndex, dIter, setDb); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(series.encodedSetDefs(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* Decode summary data if present. */
											if (summaryDataEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE)
											{
												assertTrue(series.checkHasSummaryData());

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(series.encodedSummaryData(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											/* Decode all entries present. */
											for (int i = 0; i < innerEntryEncodeCount; ++i)
											{
												assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, (setDefsEncodeMethod != ContainerOverrunTest_EncodeMethod.NONE) ? setDb : null); break;
													case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(seriesEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
													default: fail("No testing implemented for this containerType"); break;
												}
											}

											assertEquals(CodecReturnCodes.END_OF_CONTAINER, seriesEntry.decode(dIter));

											/* Stop on successful encode & decode of three series entries (and summaryData/setDefs if present). */
											if (innerEntryEncodeCount == maxInnerEntryCount)
											{
												encodeTestPassed = true;
												break;
											}
										}
										assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
										/* End Series */


									} /* End sizeIndex loop */
								} /* End summaryData encode loop */
							} /* End set definition encode loop */
						} /* End totalCountHint loop */
					} /* End containerType loop */
				} /* End entry encode loop */
			} /* End rollback on entry-encode failure loop */

			/* Tests for FilterList (it does not encode set definitions or summaryData).*/

			/* Test rolling back everything upon failing to encode an entry, or just rolling back the failed entry and validating the remaining data. */
			for (boolean rollbackContainerOnInnerEntryEncodeFail : new boolean[]{false, true})
			{
				/* Test pre-encoding entries, and encoding them inline. */
				for (ContainerOverrunTest_EncodeMethod entryEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.INLINE_ENCODE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
				{
					/* Test encoding OPAQUE and FIELD_LIST data as summary data and as entry payloads. */
					for (int innerContainerType : new int[]{DataTypes.OPAQUE, DataTypes.FIELD_LIST})
					{
						/* Test encoding, or not encoding a totalCountHint. */
						for (boolean encodeTotalCountHint : new boolean[]{false, true})
						{
							for (int sizeIndex = 0; sizeIndex < ((innerContainerType == DataTypes.OPAQUE) ? opaqueDataSizes.length : containerOverrunTest_fieldIds.length); ++sizeIndex)
							{
								int payloadEncodedSize = 0;
								boolean rollbackEntireContainer = false;
								int innerEntryEncodeCount = 0;

								/* FilterList */
								final int filterListHeaderEncodedSize = 1 /* flags */ + 1 /* Container type */ + 1 /* Entry count */ + 1 /* Possible totalCountHint */;
								final int filterEntryHeaderEncodedSize = 1 /* Action & flags */ + 1 /* id */; 

								switch(innerContainerType)
								{
									case DataTypes.FIELD_LIST: 
										payloadEncodedSize = containerOverrunTest_fieldListEncodedSizes[sizeIndex];
										break;
									case DataTypes.OPAQUE: 
										payloadEncodedSize = opaqueDataSizes[sizeIndex];
										break;

									default: fail();
								}

								/* Keep increasing the buffer size until we successfully encode all intended data
								 * (three entries, plus set definitions and/or summary data if they are part of the test) */
								boolean encodeTestPassed = false;
								for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
								{                
									int bufferSizeNeeded = 0;

									rollbackEntireContainer = false;

									mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
									assertNotNull(mainEncodeBuffer);

									/* Encode */
									eIter.clear();
									eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
									
									filterList.clear();
									
									filterList.containerType(innerContainerType);

									if (encodeTotalCountHint)
									{
										filterList.applyHasTotalCountHint();
										filterList.totalCountHint(3);
									}

									/* Determine size needed for filterList.encodeInit to succeed. */
									bufferSizeNeeded += filterListHeaderEncodedSize;

									/* Start encoding filterList. */
									if (bufferSizeNeeded <= encodeBufSize)
										assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeInit(eIter));
									else
									{
										assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, filterList.encodeInit(eIter));
										rollbackEntireContainer = true;
									}


									if (!encodeTotalCountHint) /* Didn't actually encode totalCountHint, so the byte reserved for it is now available. */
										bufferSizeNeeded -= 1;

									boolean innerEntryEncodeFailed = false;
									innerEntryEncodeCount = 0;
									if (!rollbackEntireContainer)
									{
										while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
										{
											int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

											/* Now try to encode FilterListEntries, using inline encoding or pre-encoded data. 
											 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
											filterEntry.clear();
											filterEntry.id(15);
											filterEntry.action(FilterEntryActions.SET);

											if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
											{
												bufferSizeNeeded += filterEntryHeaderEncodedSize + 3 /* reserved size bytes */;

												if (bufferSizeNeeded <= encodeBufSize)
													assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeInit(eIter, 0));
												else
												{
													assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, filterEntry.encodeInit(eIter, 0));
													assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeComplete(eIter, false));
													innerEntryEncodeFailed = true;
													break;
												}

												bufferSizeNeeded += payloadEncodedSize;

												switch(innerContainerType)
												{
													case DataTypes.FIELD_LIST:
														if (bufferSizeNeeded <= encodeBufSize)
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
															innerEntryEncodeFailed = true;
														}

														assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
														break;

													case DataTypes.OPAQUE:
														if (bufferSizeNeeded <= encodeBufSize)
														{
															assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeComplete(eIter, true));
														}
														else
														{
															assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
															assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encodeComplete(eIter, false));
															innerEntryEncodeFailed = true;
														}
														break;

													default: fail("No testing implemented for this containerType"); break;
												}

											}
											else
											{
												/* Pre-encode entry payload. */
												preEncEntryByteBuf.clear();
												preEncodeBuffer.clear();
												switch(innerContainerType)
												{   
													case DataTypes.FIELD_LIST:
														preEncodeBuffer.data(preEncEntryByteBuf);
														preEncodeIter.clear();
														preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, null));
														break;
													case DataTypes.OPAQUE:
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
														preEncEntryByteBuf.flip();
														preEncodeBuffer.data(preEncEntryByteBuf);
														break;
													default: fail("No testing implemented for this containerType"); break;
												}

												bufferSizeNeeded += filterEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

												filterEntry.encodedData(preEncodeBuffer);

												if (bufferSizeNeeded <= encodeBufSize)
													assertEquals(CodecReturnCodes.SUCCESS, filterEntry.encode(eIter));
												else
												{
													assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, filterEntry.encode(eIter));
													innerEntryEncodeFailed = true;
												}
											}

											if (innerEntryEncodeFailed)
											{
												assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
												assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
											}
											else
												++innerEntryEncodeCount;
										}
									}

									/* If filterList header,set defs, or summary data failed to encode, roll it back.
									 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
									 * Otherwise move onto the decode anyway to verify the partial data. */
									if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
									{
										assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeComplete(eIter, false));

										/* Iterator should be at start of buffer. */
										assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
										assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
										assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
										continue;
									}

									assertEquals(CodecReturnCodes.SUCCESS, filterList.encodeComplete(eIter, true));

									/* Send buffer over connection. */
									mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

									/* Decode filterList. */
									dIter.clear();
									dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

									assertEquals(CodecReturnCodes.SUCCESS, filterList.decode(dIter));
									assertEquals(innerContainerType, filterList.containerType());

									if (encodeTotalCountHint)
									{
										assertTrue(filterList.checkHasTotalCountHint());
										assertEquals(3, filterList.totalCountHint());
									}

									/* Decode all entries present. */
									for (int i = 0; i < innerEntryEncodeCount; ++i)
									{
										assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(dIter));
										assertEquals(15, filterEntry.id());
										assertEquals(FilterEntryActions.SET, filterEntry.action());

										switch(innerContainerType)
										{
											case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, null); break;
											case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(filterEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
											default: fail("No testing implemented for this containerType"); break;
										}
									}

									assertEquals(CodecReturnCodes.END_OF_CONTAINER, filterEntry.decode(dIter));

									/* Stop on successful encode & decode of three filterList entries (and summaryData/setDefs if present). */
									if (innerEntryEncodeCount == maxInnerEntryCount)
									{
										encodeTestPassed = true;
										break;
									}
								}
								assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
								/* End FilterList */
							} /* End sizeIndex loop */
						} /* End totalCountHint loop */
					} /* End containerType loop */
				} /* End entry encode loop */
			} /* End rollback on entry-encode failure loop */

			/* Tests for FieldList and ElementList (they do not encode set definitions summaryData, or totalCountHint).*/

			/* Test rolling back everything upon failing to encode an entry, or just rolling back the failed entry and validating the remaining data. */
			for (boolean rollbackContainerOnInnerEntryEncodeFail : new boolean[]{false, true})
			{
				/* Test pre-encoding entries, and encoding them inline. */
				for (ContainerOverrunTest_EncodeMethod entryEncodeMethod : new ContainerOverrunTest_EncodeMethod[] { ContainerOverrunTest_EncodeMethod.INLINE_ENCODE, ContainerOverrunTest_EncodeMethod.PRE_ENCODE })
				{
					/* Test encoding OPAQUE and FIELD_LIST data as summary data and as entry payloads. */
					for (int innerContainerType : new int[]{DataTypes.OPAQUE, DataTypes.FIELD_LIST})
					{
						for (int sizeIndex = 0; sizeIndex < ((innerContainerType == DataTypes.OPAQUE) ? opaqueDataSizes.length : containerOverrunTest_fieldIds.length); ++sizeIndex)
						{
							int payloadEncodedSize = 0;
							boolean rollbackEntireContainer = false;
							int innerEntryEncodeCount = 0;

							switch(innerContainerType)
							{
								case DataTypes.FIELD_LIST: 
									payloadEncodedSize = containerOverrunTest_fieldListEncodedSizes[sizeIndex];
									break;
								case DataTypes.OPAQUE: 
									payloadEncodedSize = opaqueDataSizes[sizeIndex];
									break;

								default: fail();
							}

							/* FieldList */
							final int fieldListHeaderEncodedSize = 1 /* flags */ + 2 /* Entry count */;
							final int fieldEntryHeaderEncodedSize = 1 /* flags */ + 1 /* fieldId */; 

							/* Keep increasing the buffer size until we successfully encode all intended data
							 * (three entries, plus set definitions and/or summary data if they are part of the test) */
							boolean encodeTestPassed = false;
							for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
							{                
								int bufferSizeNeeded = 0;

								rollbackEntireContainer = false;

								mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
								assertNotNull(mainEncodeBuffer);

								/* Encode */
								eIter.clear();
								eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

								fieldList.clear();
								
								fieldList.applyHasStandardData();

								/* Determine size needed for fieldList.encodeInit to succeed. */
								bufferSizeNeeded += fieldListHeaderEncodedSize;

								/* Start encoding fieldList. */
								if (bufferSizeNeeded <= encodeBufSize)
									assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(eIter, null, 0));
								else
								{
									assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldList.encodeInit(eIter, null, 0));
									rollbackEntireContainer = true;
								}

								boolean innerEntryEncodeFailed = false;
								innerEntryEncodeCount = 0;
								if (!rollbackEntireContainer)
								{
									while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
									{
										int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

										/* Now try to encode FieldEntries, using inline encoding or pre-encoded data. 
										 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
										fieldEntry.clear();
										fieldEntry.fieldId(15);
										fieldEntry.dataType(innerContainerType);

										if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
										{
											bufferSizeNeeded += fieldEntryHeaderEncodedSize + 3 /* reserved size bytes */;

											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeInit(eIter, 0));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encodeInit(eIter, 0));
												assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(eIter, false));
												innerEntryEncodeFailed = true;
												break;
											}

											bufferSizeNeeded += payloadEncodedSize;

											switch(innerContainerType)
											{
												case DataTypes.FIELD_LIST:
													if (bufferSizeNeeded <= encodeBufSize)
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
													else
													{
														assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
														innerEntryEncodeFailed = true;
													}

													assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
													break;

												case DataTypes.OPAQUE:
													if (bufferSizeNeeded <= encodeBufSize)
													{
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
														assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(eIter, true));
													}
													else
													{
														assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
														assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encodeComplete(eIter, false));
														innerEntryEncodeFailed = true;
													}
													break;

												default: fail("No testing implemented for this containerType"); break;
											}

										}
										else
										{
											/* Pre-encode entry payload. */
											preEncEntryByteBuf.clear();
											preEncodeBuffer.clear();
											switch(innerContainerType)
											{   
												case DataTypes.FIELD_LIST:
													preEncodeBuffer.data(preEncEntryByteBuf);
													preEncodeIter.clear();
													preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
													assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, null));
													break;
												case DataTypes.OPAQUE:
													assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
													preEncEntryByteBuf.flip();
													preEncodeBuffer.data(preEncEntryByteBuf);
													break;
												default: fail("No testing implemented for this containerType"); break;
											}

											bufferSizeNeeded += fieldEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

											fieldEntry.encodedData(preEncodeBuffer);

											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter));
												innerEntryEncodeFailed = true;
											}
										}

										if (innerEntryEncodeFailed)
										{
											assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
											assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
										}
										else
											++innerEntryEncodeCount;
									}
								}

								/* If fieldList header,set defs, or summary data failed to encode, roll it back.
								 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
								 * Otherwise move onto the decode anyway to verify the partial data. */
								if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
								{
									assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, false));

									/* Iterator should be at start of buffer. */
									assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
									assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
									assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
									continue;
								}

								assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, true));

								/* Send buffer over connection. */
								mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

								/* Decode fieldList. */
								dIter.clear();
								dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

								assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
								assertTrue(fieldList.checkHasStandardData());

								/* Decode all entries present. */
								for (int i = 0; i < innerEntryEncodeCount; ++i)
								{
									assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
									assertEquals(15, fieldEntry.fieldId());
									assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());

									switch(innerContainerType)
									{
										case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, null); break;
										case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(fieldEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
										default: fail("No testing implemented for this containerType"); break;
									}
								}

								assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));

								/* Stop on successful encode & decode of three fieldList entries (and summaryData/setDefs if present). */
								if (innerEntryEncodeCount == maxInnerEntryCount)
								{
									encodeTestPassed = true;
									break;
								}
							}
							assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
							/* End FieldList */

							/* ElementList */
							final String elementName = new String("STUFF");
							final int elementListHeaderEncodedSize = 1 /* flags */ + 2 /* Entry count */;
							final int elementEntryHeaderEncodedSize = 1 /* flags */ + 1 /* name length */ + elementName.length() /* name */; 

							/* Keep increasing the buffer size until we successfully encode all intended data
							 * (three entries, plus set definitions and/or summary data if they are part of the test) */
							encodeTestPassed = false;
							for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
							{                
								int bufferSizeNeeded = 0;

								rollbackEntireContainer = false;

								mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
								assertNotNull(mainEncodeBuffer);

								/* Encode */
								eIter.clear();
								eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

								
								elementList.clear();
								elementList.applyHasStandardData();

								/* Determine size needed for elementList.encodeInit to succeed. */
								bufferSizeNeeded += elementListHeaderEncodedSize;

								/* Start encoding elementList. */
								if (bufferSizeNeeded <= encodeBufSize)
									assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(eIter, null, 0));
								else
								{
									assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementList.encodeInit(eIter, null, 0));
									rollbackEntireContainer = true;
								}

								boolean innerEntryEncodeFailed = false;
								innerEntryEncodeCount = 0;
								if (!rollbackEntireContainer)
								{
									while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
									{
										int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

										/* Now try to encode ElementEntries, using inline encoding or pre-encoded data. 
										 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
										elementEntry.clear();
										elementEntry.name().data(elementName);
										elementEntry.dataType(innerContainerType);

										if (entryEncodeMethod == ContainerOverrunTest_EncodeMethod.INLINE_ENCODE)
										{
											bufferSizeNeeded += elementEntryHeaderEncodedSize + 3 /* reserved size bytes */;

											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(eIter, 0));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encodeInit(eIter, 0));
												assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(eIter, false));
												innerEntryEncodeFailed = true;
												break;
											}

											bufferSizeNeeded += payloadEncodedSize;

											switch(innerContainerType)
											{
												case DataTypes.FIELD_LIST:
													if (bufferSizeNeeded <= encodeBufSize)
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
													else
													{
														assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeFieldList(eIter, sizeIndex, null));
														innerEntryEncodeFailed = true;
													}

													assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(eIter, !innerEntryEncodeFailed));
													break;

												case DataTypes.OPAQUE:
													if (bufferSizeNeeded <= encodeBufSize)
													{
														assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
														assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(eIter, true));
													}
													else
													{
														assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, containerOverrunTest_encodeOpaque(eIter, expectedOpaqueData, payloadEncodedSize));
														assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(eIter, false));
														innerEntryEncodeFailed = true;
													}
													break;

												default: fail("No testing implemented for this containerType"); break;
											}

										}
										else
										{
											/* Pre-encode entry payload. */
											preEncEntryByteBuf.clear();
											preEncodeBuffer.clear();
											switch(innerContainerType)
											{   
												case DataTypes.FIELD_LIST:
													preEncodeBuffer.data(preEncEntryByteBuf);
													preEncodeIter.clear();
													preEncodeIter.setBufferAndRWFVersion(preEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());
													assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_encodeFieldList(preEncodeIter, sizeIndex, null));
													break;
												case DataTypes.OPAQUE:
													assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
													preEncEntryByteBuf.flip();
													preEncodeBuffer.data(preEncEntryByteBuf);
													break;
												default: fail("No testing implemented for this containerType"); break;
											}

											bufferSizeNeeded += elementEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

											elementEntry.encodedData(preEncodeBuffer);

											if (bufferSizeNeeded <= encodeBufSize)
												assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter));
											else
											{
												assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter));
												innerEntryEncodeFailed = true;
											}
										}

										if (innerEntryEncodeFailed)
										{
											assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
											assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
										}
										else
											++innerEntryEncodeCount;
									}
								}

								/* If elementList header,set defs, or summary data failed to encode, roll it back.
								 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
								 * Otherwise move onto the decode anyway to verify the partial data. */
								if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
								{
									assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(eIter, false));

									/* Iterator should be at start of buffer. */
									assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
									assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
									assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
									continue;
								}

								assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(eIter, true));

								/* Send buffer over connection. */
								mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

								/* Decode elementList. */
								dIter.clear();
								dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

								assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
								assertTrue(elementList.checkHasStandardData());

								/* Decode all entries present. */
								for (int i = 0; i < innerEntryEncodeCount; ++i)
								{
									assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
									assertEquals(elementName, elementEntry.name().toString());
									assertEquals(innerContainerType, elementEntry.dataType());

									switch(innerContainerType)
									{
										case DataTypes.FIELD_LIST: containerOverrunTest_decodeFieldList(dIter, sizeIndex, null); break;
										case DataTypes.OPAQUE: containerOverrunTest_decodeOpaque(elementEntry.encodedData(), expectedOpaqueData, payloadEncodedSize); break;
										default: fail("No testing implemented for this containerType"); break;
									}
								}

								assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));

								/* Stop on successful encode & decode of three elementList entries (and summaryData/setDefs if present). */
								if (innerEntryEncodeCount == maxInnerEntryCount)
								{
									encodeTestPassed = true;
									break;
								}
							}
							assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
							/* End ElementList */
						} /* End sizeIndex loop */
					} /* End containerType loop */
				} /* End entry encode loop */
			} /* End rollback on entry-encode failure loop */

			/* Tests for Array */

			for (boolean rollbackContainerOnInnerEntryEncodeFail : new boolean[]{false, true})
			{
				/* Array */
				final int arrayHeaderEncodedSize = 1 /* primitiveType */ + 1 /* itemLength */ + 2 /* entry count */;
				final int arrayEntryHeaderEncodedSize = 0 /* Nothing */;

				boolean rollbackEntireContainer = false;
				int innerEntryEncodeCount = 0;

				for (int sizeIndex = 0; sizeIndex < opaqueDataSizes.length; ++sizeIndex)
				{
					int payloadEncodedSize = opaqueDataSizes[sizeIndex];

					/* Keep increasing the buffer size until we successfully encode all intended data
					 * (three entries, plus set definitions and/or summary data if they are part of the test) */
					boolean encodeTestPassed = false;
					for (int encodeBufSize = 0; encodeBufSize <= byteBufferSize; ++encodeBufSize)
					{                
						int bufferSizeNeeded = 0;

						rollbackEntireContainer = false;

						mainEncodeBuffer = clientChannel.getBuffer(encodeBufSize, false, error);
						assertNotNull(mainEncodeBuffer);

						/* Encode */
						eIter.clear();
						eIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

						array.clear();
						
						/* Determine size needed for array.encodeInit to succeed. */
						bufferSizeNeeded += arrayHeaderEncodedSize;

						array.primitiveType(DataTypes.BUFFER);

						/* Start encoding array. */
						if (bufferSizeNeeded <= encodeBufSize)
							assertEquals(CodecReturnCodes.SUCCESS, array.encodeInit(eIter));
						else
						{
							assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, array.encodeInit(eIter));
							rollbackEntireContainer = true;
						}

						boolean innerEntryEncodeFailed = false;
						innerEntryEncodeCount = 0;
						if (!rollbackEntireContainer)
						{
							while (!innerEntryEncodeFailed && innerEntryEncodeCount < maxInnerEntryCount)
							{
								int entryEncodeStartPos = ((EncodeIteratorImpl)eIter)._curBufPos;

								/* Now try to encode ArrayEntries (array can do pre-encoded only).
								 * If we run out of space, either rollback the entry or rollback everything (both are tested). */ 
								arrayEntry.clear();

								/* Pre-encode entry payload. */
								preEncEntryByteBuf.clear();
								preEncodeBuffer.clear();

								assertEquals(CodecReturnCodes.SUCCESS, containerOverrunTest_writeOpaqueByteBuffer(payloadEncodedSize, expectedOpaqueData, preEncEntryByteBuf));
								preEncEntryByteBuf.flip();
								preEncodeBuffer.data(preEncEntryByteBuf);

								bufferSizeNeeded += arrayEntryHeaderEncodedSize + (payloadEncodedSize >= 0xFE ? 3 : 1) /* size byte(s) */ + payloadEncodedSize;

								arrayEntry.encodedData(preEncodeBuffer);

								if (bufferSizeNeeded <= encodeBufSize)
									assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.encode(eIter));
								else
								{
									assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, arrayEntry.encode(eIter));
									innerEntryEncodeFailed = true;
								}

								if (innerEntryEncodeFailed)
								{
									assertEquals(entryEncodeStartPos, ((EncodeIteratorImpl)eIter)._curBufPos);
									assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
								}
								else
									++innerEntryEncodeCount;
							}
						}

						/* If array header,set defs, or summary data failed to encode, roll it back.
						 * If any entries failed to encode, rollback if we are currently testing rollback in that case.
						 * Otherwise move onto the decode anyway to verify the partial data. */
						if (rollbackEntireContainer || innerEntryEncodeFailed && rollbackContainerOnInnerEntryEncodeFail)
						{
							assertEquals(CodecReturnCodes.SUCCESS, array.encodeComplete(eIter, false));

							/* Iterator should be at start of buffer. */
							assertEquals(mainEncodeBuffer.dataStartPosition(), ((EncodeIteratorImpl)eIter)._curBufPos);
							assertEquals(-1, ((EncodeIteratorImpl)eIter)._encodingLevel);
							assertEquals(TransportReturnCodes.SUCCESS, clientChannel.releaseBuffer(mainEncodeBuffer, error));
							continue;
						}

						assertEquals(CodecReturnCodes.SUCCESS, array.encodeComplete(eIter, true));

						/* Send buffer over connection. */
						mainEncodeBuffer = containerOverrunTest_sendMessage(clientChannel, mainEncodeBuffer, serverChannel);

						/* Decode array. */
						dIter.clear();
						dIter.setBufferAndRWFVersion(mainEncodeBuffer, Codec.majorVersion(), Codec.minorVersion());

						assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
						assertEquals(DataTypes.BUFFER, array.primitiveType());

						/* Decode all entries present. */
						for (int i = 0; i < innerEntryEncodeCount; ++i)
						{
							assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.decode(dIter));
							containerOverrunTest_decodeOpaque(arrayEntry.encodedData(), expectedOpaqueData, payloadEncodedSize);
						}

						assertEquals(CodecReturnCodes.END_OF_CONTAINER, arrayEntry.decode(dIter));

						/* Stop on successful encode & decode of three array entries (and summaryData/setDefs if present). */
						if (innerEntryEncodeCount == maxInnerEntryCount)
						{
							encodeTestPassed = true;
							break;
						}
					}
					assertTrue(encodeTestPassed); /* Ensure a successful encode/decode for each dataSize. */
					/* End Array */
				} /* End sizeIndex loop */
			} /* End rollback on entry-encode failure loop */
		}
		catch(IOException e)
		{
			e.printStackTrace();
			fail("Caught IOException");
			return;
		}
		finally
		{
			if(server != null)
				assertEquals(TransportReturnCodes.SUCCESS, server.close(error));
			assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
		}
    }

    Buffer primitiveEntryOverrunTest_encodeBuffer = CodecFactory.createBuffer();
    ByteBuffer primitiveEntryOverrunTest_byteBuffer = ByteBuffer.allocateDirect(1024);

    /* Realign to a bytebuffer that has the specified space. */
    public void primitiveEntryOverrunTest_realignBuffer(EncodeIterator eIter, int newSize)
    {
        primitiveEntryOverrunTest_byteBuffer  = ByteBuffer.allocateDirect(newSize);
    	primitiveEntryOverrunTest_encodeBuffer.data(primitiveEntryOverrunTest_byteBuffer);
        assertEquals(CodecReturnCodes.SUCCESS, eIter.realignBuffer(primitiveEntryOverrunTest_encodeBuffer));
    }

    @Test
    public void setDefinedAndStandardDataOverrunTest()
    {
        /* Test overrun & rollback of encoding a FieldList & ElementList with the different possible primitives, both as set-defined and 
         * standard data.
         * Wrap the FieldList or ElementList in a series that also encodes the relevant Field/Element SetDef, to test overrun & rollback 
         * of encoding them.
         * When encoding fails, increase buffer size by 1, realign and try again. 
         * See ETA-2340. */

        /* FieldList */

        /* Setup field set def db, with entries. */
        LocalFieldSetDefDb fsetDb = CodecFactory.createLocalFieldSetDefDb();
        FieldSetDefEntry[] entries = new FieldSetDefEntry[34];

        entries[0] = CodecFactory.createFieldSetDefEntry();
        entries[0].fieldId(0);
        entries[0].dataType(DataTypes.INT_1);

        entries[1] = CodecFactory.createFieldSetDefEntry();
        entries[1].fieldId(1);
        entries[1].dataType(DataTypes.INT_2);

        entries[2] = CodecFactory.createFieldSetDefEntry();
        entries[2].fieldId(2);
        entries[2].dataType(DataTypes.INT_4);

        entries[3] = CodecFactory.createFieldSetDefEntry();
        entries[3].fieldId(3);
        entries[3].dataType(DataTypes.INT_8);

        entries[4] = CodecFactory.createFieldSetDefEntry();
        entries[4].fieldId(4);
        entries[4].dataType(DataTypes.UINT_1);

        entries[5] = CodecFactory.createFieldSetDefEntry();
        entries[5].fieldId(5);
        entries[5].dataType(DataTypes.UINT_2);

        entries[6] = CodecFactory.createFieldSetDefEntry();
        entries[6].fieldId(6);
        entries[6].dataType(DataTypes.UINT_4);

        entries[7] = CodecFactory.createFieldSetDefEntry();
        entries[7].fieldId(7);
        entries[7].dataType(DataTypes.UINT_8);

        entries[8] = CodecFactory.createFieldSetDefEntry();
        entries[8].fieldId(8);
        entries[8].dataType(DataTypes.FLOAT_4);

        entries[9] = CodecFactory.createFieldSetDefEntry();
        entries[9].fieldId(9);
        entries[9].dataType(DataTypes.DOUBLE_8);

        entries[10] = CodecFactory.createFieldSetDefEntry();
        entries[10].fieldId(10);
        entries[10].dataType(DataTypes.REAL_4RB);

        entries[11] = CodecFactory.createFieldSetDefEntry();
        entries[11].fieldId(11);
        entries[11].dataType(DataTypes.REAL_8RB);

        entries[12] = CodecFactory.createFieldSetDefEntry();
        entries[12].fieldId(12);
        entries[12].dataType(DataTypes.DATE_4);

        entries[13] = CodecFactory.createFieldSetDefEntry();
        entries[13].fieldId(13);
        entries[13].dataType(DataTypes.TIME_3);

        entries[14] = CodecFactory.createFieldSetDefEntry();
        entries[14].fieldId(14);
        entries[14].dataType(DataTypes.TIME_5);

        entries[15] = CodecFactory.createFieldSetDefEntry();
        entries[15].fieldId(15);
        entries[15].dataType(DataTypes.DATETIME_7);

        entries[16] = CodecFactory.createFieldSetDefEntry();
        entries[16].fieldId(16);
        entries[16].dataType(DataTypes.DATETIME_11);

        entries[17] = CodecFactory.createFieldSetDefEntry();
        entries[17].fieldId(17);
        entries[17].dataType(DataTypes.DATETIME_12);

        entries[18] = CodecFactory.createFieldSetDefEntry();
        entries[18].fieldId(18);
        entries[18].dataType(DataTypes.TIME_7);

        entries[19] = CodecFactory.createFieldSetDefEntry();
        entries[19].fieldId(19);
        entries[19].dataType(DataTypes.TIME_8);

        entries[20] = CodecFactory.createFieldSetDefEntry();
        entries[20].fieldId(20);
        entries[20].dataType(DataTypes.INT);

        entries[21] = CodecFactory.createFieldSetDefEntry();
        entries[21].fieldId(21);
        entries[21].dataType(DataTypes.UINT);

        entries[22] = CodecFactory.createFieldSetDefEntry();
        entries[22].fieldId(22);
        entries[22].dataType(DataTypes.FLOAT);

        entries[23] = CodecFactory.createFieldSetDefEntry();
        entries[23].fieldId(23);
        entries[23].dataType(DataTypes.DOUBLE);

        entries[24] = CodecFactory.createFieldSetDefEntry();
        entries[24].fieldId(24);
        entries[24].dataType(DataTypes.REAL);

        entries[25] = CodecFactory.createFieldSetDefEntry();
        entries[25].fieldId(25);
        entries[25].dataType(DataTypes.DATE);

        entries[26] = CodecFactory.createFieldSetDefEntry();
        entries[26].fieldId(26);
        entries[26].dataType(DataTypes.TIME);

        entries[27] = CodecFactory.createFieldSetDefEntry();
        entries[27].fieldId(27);
        entries[27].dataType(DataTypes.DATETIME);

        entries[28] = CodecFactory.createFieldSetDefEntry();
        entries[28].fieldId(28);
        entries[28].dataType(DataTypes.QOS);

        entries[29] = CodecFactory.createFieldSetDefEntry();
        entries[29].fieldId(29);
        entries[29].dataType(DataTypes.ENUM);

        entries[30] = CodecFactory.createFieldSetDefEntry();
        entries[30].fieldId(30);
        entries[30].dataType(DataTypes.BUFFER);

        entries[31] = CodecFactory.createFieldSetDefEntry();
        entries[31].fieldId(31);
        entries[31].dataType(DataTypes.ASCII_STRING);

        entries[32] = CodecFactory.createFieldSetDefEntry();
        entries[32].fieldId(32);
        entries[32].dataType(DataTypes.UTF8_STRING);

        entries[33] = CodecFactory.createFieldSetDefEntry();
        entries[33].fieldId(33);
        entries[33].dataType(DataTypes.RMTES_STRING);

        fsetDb.definitions()[0].setId(0);
        fsetDb.definitions()[0].count(34);
        fsetDb.definitions()[0].entries(entries);

        EncodeIterator eIter = CodecFactory.createEncodeIterator();
        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Int intVal = CodecFactory.createInt();
        UInt uintVal = CodecFactory.createUInt();
        Float floatVal = CodecFactory.createFloat();
        Double doubleVal = CodecFactory.createDouble();
        Real realVal = CodecFactory.createReal();
        Date dateVal = CodecFactory.createDate();
        Time timeVal = CodecFactory.createTime();
        DateTime dateTimeVal = CodecFactory.createDateTime();
        Qos qosVal = CodecFactory.createQos();
        Enum enumVal = CodecFactory.createEnum();
        Buffer bufferVal = CodecFactory.createBuffer();
        
        /* Buffer size needed to encode (starting at series header) */
        final int seriesHeaderEncodedSize = 1 /* Flags */ + 1 /* Container type */ + 2 /* SetDef size bytes */ + 2 /* Entry count */;
        int bufSizeNeeded = seriesHeaderEncodedSize;
        int encodeBufSize = bufSizeNeeded;

        primitiveEntryOverrunTest_byteBuffer = ByteBuffer.allocateDirect(encodeBufSize);
        primitiveEntryOverrunTest_encodeBuffer.data(primitiveEntryOverrunTest_byteBuffer);
        eIter.setBufferAndRWFVersion(primitiveEntryOverrunTest_encodeBuffer, Codec.majorVersion(), Codec.minorVersion());

        /* Encode Series. */
        series.clear();
        series.containerType(DataTypes.FIELD_LIST);
        series.applyHasSetDefs();
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeInit(eIter, 0, 0));

        /* Encode set definition. */
        bufSizeNeeded += 1 /* Flags */ + 1 /* SetDef Count */ + 1 /* Set ID */ + 1 /* Entry count */ 
            + 34 * 3 /* entries (fieldId, dataType) */;
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fsetDb.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fsetDb.encode(eIter));
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeSetDefsComplete(eIter, true));

        /* Encode SeriesEntry. */
        bufSizeNeeded += 3 /* Size bytes */;
        encodeBufSize = bufSizeNeeded;
        primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        seriesEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeInit(eIter, 0));

        /* Encode field list. */
        bufSizeNeeded += 1 /* FieldList flags */ + 2 /* Set data length */ + 2 /* Standard data length */;
        encodeBufSize = bufSizeNeeded;
        primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);

        fieldList.clear();
        fieldList.applyHasSetData();
        fieldList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeInit(eIter, fsetDb, 0));

        /* INT_1 needs 1 byte. */
        bufSizeNeeded += 1;
        fieldEntry.clear();
        fieldEntry.fieldId(0);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* INT_2 needs 2 bytes. */
        bufSizeNeeded += 2;
        fieldEntry.clear();
        fieldEntry.fieldId(1);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* INT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        fieldEntry.clear();
        fieldEntry.fieldId(2);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* INT_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(3);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* UINT_1 needs 1 byte. */
        bufSizeNeeded += 1;
        fieldEntry.clear();
        fieldEntry.fieldId(4);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* UINT_2 needs 2 bytes. */
        bufSizeNeeded += 2;
        fieldEntry.clear();
        fieldEntry.fieldId(5);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* UINT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        fieldEntry.clear();
        fieldEntry.fieldId(6);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* UINT_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(7);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* FLOAT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        fieldEntry.clear();
        fieldEntry.fieldId(8);
        fieldEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, floatVal));

        /* DOUBLE_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(9);
        fieldEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, doubleVal));

        /* REAL_4RB needs 5 bytes for this value. */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(10);
        fieldEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, realVal));
                
        /* REAL_8RB needs 9 bytes for this value. */
        bufSizeNeeded += 9;
        fieldEntry.clear();
        fieldEntry.fieldId(11);
        fieldEntry.dataType(DataTypes.REAL);
        realVal.value(9223372036854775807L, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, realVal));

        /* DATE_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        fieldEntry.clear();
        fieldEntry.fieldId(12);
        fieldEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateVal));

        /* TIME_3 needs 3 bytes. */
        bufSizeNeeded += 3;
        fieldEntry.clear();
        fieldEntry.fieldId(13);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* TIME_5 needs 5 bytes. */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(14);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* DATETIME_7 needs 7 bytes. */
        bufSizeNeeded += 7;
        fieldEntry.clear();
        fieldEntry.fieldId(15);
        fieldEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateTimeVal));

        /* DATETIME_11 needs 11 bytes. */
        bufSizeNeeded += 11;
        fieldEntry.clear();
        fieldEntry.fieldId(16);
        fieldEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateTimeVal));

        /* DATETIME_12 needs 12 bytes. */
        bufSizeNeeded += 12;
        fieldEntry.clear();
        fieldEntry.fieldId(17);
        fieldEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateTimeVal));

        /* TIME_7 needs 7 bytes. */
        bufSizeNeeded += 7;
        fieldEntry.clear();
        fieldEntry.fieldId(18);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* TIME_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(19);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* This INT needs 3 bytes. */
        bufSizeNeeded += 3;
        fieldEntry.clear();
        fieldEntry.fieldId(20);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(255);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* This UINT needs 3 bytes. */
        bufSizeNeeded += 3;
        fieldEntry.clear();
        fieldEntry.fieldId(21);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* This FLOAT needs 5 bytes. */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, floatVal));

        /* This DOUBLE needs 9 bytes. */
        bufSizeNeeded += 9;
        fieldEntry.clear();
        fieldEntry.fieldId(23);
        fieldEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, doubleVal));

        /* This REAL needs 6 bytes. */
        bufSizeNeeded += 6;
        fieldEntry.clear();
        fieldEntry.fieldId(24);
        fieldEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, realVal));

        /* This DATE needs 5 bytes. */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(25);
        fieldEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }

        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateVal));

        /* This TIME needs 9 bytes. */
        bufSizeNeeded += 9;
        fieldEntry.clear();
        fieldEntry.fieldId(26);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* This DATETIME needs 13 bytes. */
        bufSizeNeeded += 13;
        fieldEntry.clear();
        fieldEntry.fieldId(27);
        fieldEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateTimeVal));

        /* This QOS needs 6 bytes. */
        bufSizeNeeded += 6;
        fieldEntry.clear();
        fieldEntry.fieldId(28);
        fieldEntry.dataType(DataTypes.QOS);
        qosVal.timeliness(QosTimeliness.DELAYED);
        qosVal.timeInfo(3000);
        qosVal.rate(QosRates.TIME_CONFLATED);
        qosVal.rateInfo(5000);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, qosVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, qosVal));

        /* This ENUM needs 3 bytes. */
        bufSizeNeeded += 3;
        fieldEntry.clear();
        fieldEntry.fieldId(29);
        fieldEntry.dataType(DataTypes.ENUM);
        enumVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, enumVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, enumVal));

        /* This BUFFER needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(30);
        fieldEntry.dataType(DataTypes.BUFFER);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This ASCII_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(31);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This UTF8_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UTF8_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(33);
        fieldEntry.dataType(DataTypes.RMTES_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SET_COMPLETE, fieldEntry.encode(eIter, bufferVal));

        /* Standard data */

        /* This INT needs 5 bytes (2 for fieldId, 3 for value). */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(20);
        fieldEntry.dataType(DataTypes.INT);
        intVal.value(255);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, intVal));

        /* This UINT needs 5 bytes (2 for fieldId, 3 for value). */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(21);
        fieldEntry.dataType(DataTypes.UINT);
        uintVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, uintVal));

        /* This UINT needs 7 bytes (2 for fieldId, 5 for value). */
        bufSizeNeeded += 7;
        fieldEntry.clear();
        fieldEntry.fieldId(22);
        fieldEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, floatVal));

        /* This DOUBLE needs 11 bytes (2 for fieldId, 9 for value). */
        bufSizeNeeded += 11;
        fieldEntry.clear();
        fieldEntry.fieldId(23);
        fieldEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, doubleVal));

        /* This REAL needs 8 bytes (2 for fieldId, 6 for value). */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(24);
        fieldEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, realVal));

        /* This DATE needs 7 bytes (2 for fieldId, 5 for value). */
        bufSizeNeeded += 7;
        fieldEntry.clear();
        fieldEntry.fieldId(25);
        fieldEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }

        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateVal));

        /* This TIME needs 11 bytes (2 for fieldId, 9 for value). */
        bufSizeNeeded += 11;
        fieldEntry.clear();
        fieldEntry.fieldId(26);
        fieldEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, timeVal));

        /* This DATETIME needs 13 bytes (2 for fieldId, 13 for value). */
        bufSizeNeeded += 15;
        fieldEntry.clear();
        fieldEntry.fieldId(27);
        fieldEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, dateTimeVal));

        /* This QOS needs 8 bytes (2 for fieldId, 6 for value). */
        bufSizeNeeded += 8;
        fieldEntry.clear();
        fieldEntry.fieldId(28);
        fieldEntry.dataType(DataTypes.QOS);
        qosVal.timeliness(QosTimeliness.DELAYED);
        qosVal.timeInfo(3000);
        qosVal.rate(QosRates.TIME_CONFLATED);
        qosVal.rateInfo(5000);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, qosVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, qosVal));

        /* This ENUM needs 5 bytes (2 for fieldId, 3 for value). */
        bufSizeNeeded += 5;
        fieldEntry.clear();
        fieldEntry.fieldId(29);
        fieldEntry.dataType(DataTypes.ENUM);
        enumVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, enumVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, enumVal));

        /* This BUFFER needs 10 bytes (2 for fieldId, 8 for value). */
        bufSizeNeeded += 10;
        fieldEntry.clear();
        fieldEntry.fieldId(30);
        fieldEntry.dataType(DataTypes.BUFFER);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This ASCII_STRING needs 10 bytes (2 for fieldId, 8 for value). */
        bufSizeNeeded += 10;
        fieldEntry.clear();
        fieldEntry.fieldId(31);
        fieldEntry.dataType(DataTypes.ASCII_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This UTF8_STRING needs 10 bytes (2 for fieldId, 8 for value). */
        bufSizeNeeded += 10;
        fieldEntry.clear();
        fieldEntry.fieldId(32);
        fieldEntry.dataType(DataTypes.UTF8_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 10 bytes (2 for fieldId, 8 for value). */
        bufSizeNeeded += 10;
        fieldEntry.clear();
        fieldEntry.fieldId(33);
        fieldEntry.dataType(DataTypes.RMTES_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 10 bytes (2 for fieldId, 8 for value). */
        /* Encode this one via encodedData. */
        bufSizeNeeded += 10;
        fieldEntry.clear();
        fieldEntry.fieldId(34);
        fieldEntry.dataType(DataTypes.RMTES_STRING);
        fieldEntry.encodedData().data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter));

        /* This RMTES_STRING will be blank and needs 3 bytes (2 for fieldId, 1 for value). */
        bufSizeNeeded += 3;
        fieldEntry.clear();
        fieldEntry.fieldId(35);
        fieldEntry.dataType(DataTypes.RMTES_STRING);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, fieldEntry.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.encode(eIter));

        assertEquals(CodecReturnCodes.SUCCESS, fieldList.encodeComplete(eIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeComplete(eIter, true));
        
        
        /* Decode the series, set definition, and fields, to validate data. */
        primitiveEntryOverrunTest_byteBuffer.flip();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Buffer decodeBuffer = CodecFactory.createBuffer();
        decodeBuffer.data(primitiveEntryOverrunTest_byteBuffer);
        dIter.setBufferAndRWFVersion(decodeBuffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(DataTypes.FIELD_LIST, series.containerType());
        assertTrue(series.checkHasSetDefs());
        
        fsetDb.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fsetDb.decode(dIter));
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, fsetDb));
        
        /* INT_1 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(0, fieldEntry.fieldId());
        assertEquals(DataTypes.INT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_2 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(1, fieldEntry.fieldId());
        assertEquals(DataTypes.INT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(2, fieldEntry.fieldId());
        assertEquals(DataTypes.INT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_8 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(3, fieldEntry.fieldId());
        assertEquals(DataTypes.INT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* UINT_1 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(4, fieldEntry.fieldId());
        assertEquals(DataTypes.UINT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_2 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(5, fieldEntry.fieldId());
        assertEquals(DataTypes.UINT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(6, fieldEntry.fieldId());
        assertEquals(DataTypes.UINT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_8 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(7, fieldEntry.fieldId());
        assertEquals(DataTypes.UINT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* FLOAT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(8, fieldEntry.fieldId());
        assertEquals(DataTypes.FLOAT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE_8 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(9, fieldEntry.fieldId());
        assertEquals(DataTypes.DOUBLE, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL_4RB */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        assertEquals(DataTypes.REAL, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* REAL_8RB */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(11, fieldEntry.fieldId());
        assertEquals(DataTypes.REAL, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(9223372036854775807L, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE_4 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(12, fieldEntry.fieldId());
        assertEquals(DataTypes.DATE, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME_3 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(13, fieldEntry.fieldId());
        assertEquals(DataTypes.TIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(0, timeVal.millisecond());
        assertEquals(0, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* TIME_5 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(14, fieldEntry.fieldId());
        assertEquals(DataTypes.TIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(0, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* DATETIME_7 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(15, fieldEntry.fieldId());
        assertEquals(DataTypes.DATETIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(0, dateTimeVal.millisecond());
        assertEquals(0, dateTimeVal.microsecond());
        assertEquals(0, dateTimeVal.nanosecond());

        /* DATETIME_11 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(16, fieldEntry.fieldId());
        assertEquals(DataTypes.DATETIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(0, dateTimeVal.nanosecond());

        /* DATETIME_12 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(17, fieldEntry.fieldId());
        assertEquals(DataTypes.DATETIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* TIME_7 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(18, fieldEntry.fieldId());
        assertEquals(DataTypes.TIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* TIME_8 */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(19, fieldEntry.fieldId());
        assertEquals(DataTypes.TIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* INT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(20, fieldEntry.fieldId());
        assertEquals(DataTypes.INT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(255, intVal.toLong());

        /* UINT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(21, fieldEntry.fieldId());
        assertEquals(DataTypes.UINT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(256, uintVal.toLong());
        
        /* FLOAT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(22, fieldEntry.fieldId());
        assertEquals(DataTypes.FLOAT, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(23, fieldEntry.fieldId());
        assertEquals(DataTypes.DOUBLE, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(24, fieldEntry.fieldId());
        assertEquals(DataTypes.REAL, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(25, fieldEntry.fieldId());
        assertEquals(DataTypes.DATE, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(26, fieldEntry.fieldId());
        assertEquals(DataTypes.TIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* DATETIME */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(27, fieldEntry.fieldId());
        assertEquals(DataTypes.DATETIME, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* QOS */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(28, fieldEntry.fieldId());
        assertEquals(DataTypes.QOS, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, qosVal.decode(dIter));
        assertEquals(QosTimeliness.DELAYED, qosVal.timeliness());
        assertEquals(3000, qosVal.timeInfo());
        assertEquals(QosRates.TIME_CONFLATED, qosVal.rate());
        assertEquals(5000, qosVal.rateInfo());

        /* ENUM */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(29, fieldEntry.fieldId());
        assertEquals(DataTypes.ENUM, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, enumVal.decode(dIter));
        assertEquals(256, enumVal.toInt());

        /* BUFFER */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(30, fieldEntry.fieldId());
        assertEquals(DataTypes.BUFFER, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* ASCII_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(31, fieldEntry.fieldId());
        assertEquals(DataTypes.ASCII_STRING, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* UTF8_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UTF8_STRING, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(33, fieldEntry.fieldId());
        assertEquals(DataTypes.RMTES_STRING, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* Standard data */

        /* INT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(20, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(255, intVal.toLong());

        /* UINT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(21, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(256, uintVal.toLong());
        
        /* FLOAT */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(22, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(23, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(24, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(25, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(26, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* DATETIME */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(27, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* QOS */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(28, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, qosVal.decode(dIter));
        assertEquals(QosTimeliness.DELAYED, qosVal.timeliness());
        assertEquals(3000, qosVal.timeInfo());
        assertEquals(QosRates.TIME_CONFLATED, qosVal.rate());
        assertEquals(5000, qosVal.rateInfo());

        /* ENUM */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(29, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, enumVal.decode(dIter));
        assertEquals(256, enumVal.toInt());

        /* BUFFER */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(30, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* ASCII_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(31, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* UTF8_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(33, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING (encoded via encodedData) */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(34, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING (encoded as blank) */
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(35, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, bufferVal.decode(dIter));

        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, seriesEntry.decode(dIter));

        /* ElementList */

        /* Setup element set def db, with entries. */
        LocalElementSetDefDb esetDb = CodecFactory.createLocalElementSetDefDb();
        ElementSetDefEntry[] elementSetEntries = new ElementSetDefEntry[34];

        elementSetEntries[0] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[0].name().data("0");
        elementSetEntries[0].dataType(DataTypes.INT_1);

        elementSetEntries[1] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[1].name().data("1");
        elementSetEntries[1].dataType(DataTypes.INT_2);

        elementSetEntries[2] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[2].name().data("2");
        elementSetEntries[2].dataType(DataTypes.INT_4);

        elementSetEntries[3] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[3].name().data("3");
        elementSetEntries[3].dataType(DataTypes.INT_8);

        elementSetEntries[4] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[4].name().data("4");
        elementSetEntries[4].dataType(DataTypes.UINT_1);

        elementSetEntries[5] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[5].name().data("5");
        elementSetEntries[5].dataType(DataTypes.UINT_2);

        elementSetEntries[6] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[6].name().data("6");
        elementSetEntries[6].dataType(DataTypes.UINT_4);

        elementSetEntries[7] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[7].name().data("7");
        elementSetEntries[7].dataType(DataTypes.UINT_8);

        elementSetEntries[8] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[8].name().data("8");
        elementSetEntries[8].dataType(DataTypes.FLOAT_4);

        elementSetEntries[9] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[9].name().data("9");
        elementSetEntries[9].dataType(DataTypes.DOUBLE_8);

        elementSetEntries[10] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[10].name().data("10");
        elementSetEntries[10].dataType(DataTypes.REAL_4RB);

        elementSetEntries[11] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[11].name().data("11");
        elementSetEntries[11].dataType(DataTypes.REAL_8RB);

        elementSetEntries[12] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[12].name().data("12");
        elementSetEntries[12].dataType(DataTypes.DATE_4);

        elementSetEntries[13] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[13].name().data("13");
        elementSetEntries[13].dataType(DataTypes.TIME_3);

        elementSetEntries[14] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[14].name().data("14");
        elementSetEntries[14].dataType(DataTypes.TIME_5);

        elementSetEntries[15] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[15].name().data("15");
        elementSetEntries[15].dataType(DataTypes.DATETIME_7);

        elementSetEntries[16] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[16].name().data("16");
        elementSetEntries[16].dataType(DataTypes.DATETIME_11);

        elementSetEntries[17] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[17].name().data("17");
        elementSetEntries[17].dataType(DataTypes.DATETIME_12);

        elementSetEntries[18] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[18].name().data("18");
        elementSetEntries[18].dataType(DataTypes.TIME_7);

        elementSetEntries[19] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[19].name().data("19");
        elementSetEntries[19].dataType(DataTypes.TIME_8);

        elementSetEntries[20] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[20].name().data("20");
        elementSetEntries[20].dataType(DataTypes.INT);

        elementSetEntries[21] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[21].name().data("21");
        elementSetEntries[21].dataType(DataTypes.UINT);

        elementSetEntries[22] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[22].name().data("22");
        elementSetEntries[22].dataType(DataTypes.FLOAT);

        elementSetEntries[23] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[23].name().data("23");
        elementSetEntries[23].dataType(DataTypes.DOUBLE);

        elementSetEntries[24] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[24].name().data("24");
        elementSetEntries[24].dataType(DataTypes.REAL);

        elementSetEntries[25] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[25].name().data("25");
        elementSetEntries[25].dataType(DataTypes.DATE);

        elementSetEntries[26] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[26].name().data("26");
        elementSetEntries[26].dataType(DataTypes.TIME);

        elementSetEntries[27] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[27].name().data("27");
        elementSetEntries[27].dataType(DataTypes.DATETIME);

        elementSetEntries[28] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[28].name().data("28");
        elementSetEntries[28].dataType(DataTypes.QOS);

        elementSetEntries[29] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[29].name().data("29");
        elementSetEntries[29].dataType(DataTypes.ENUM);

        elementSetEntries[30] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[30].name().data("30");
        elementSetEntries[30].dataType(DataTypes.BUFFER);

        elementSetEntries[31] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[31].name().data("31");
        elementSetEntries[31].dataType(DataTypes.ASCII_STRING);

        elementSetEntries[32] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[32].name().data("32");
        elementSetEntries[32].dataType(DataTypes.UTF8_STRING);

        elementSetEntries[33] = CodecFactory.createElementSetDefEntry();
        elementSetEntries[33].name().data("33");
        elementSetEntries[33].dataType(DataTypes.RMTES_STRING);

        esetDb.definitions()[0].setId(0);
        esetDb.definitions()[0].count(34);
        esetDb.definitions()[0].entries(elementSetEntries);

        /* Buffer size needed to encode (starting at series header) */
        bufSizeNeeded = seriesHeaderEncodedSize;
        encodeBufSize = bufSizeNeeded;

        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        
        primitiveEntryOverrunTest_byteBuffer = ByteBuffer.allocateDirect(encodeBufSize);
        primitiveEntryOverrunTest_encodeBuffer.data(primitiveEntryOverrunTest_byteBuffer);
        eIter.clear();
        eIter.setBufferAndRWFVersion(primitiveEntryOverrunTest_encodeBuffer, Codec.majorVersion(), Codec.minorVersion());

        /* Encode Series. */
        series.clear();
        series.containerType(DataTypes.ELEMENT_LIST);
        series.applyHasSetDefs();
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeInit(eIter, 0, 0));

        /* Encode set definition. */
        bufSizeNeeded += 1 /* Flags */ + 1 /* SetDef Count */ + 1 /* Set ID */ + 1 /* Entry count */ 
            + 10 * 3 /* The first ten entries which have one-character names, "0" to "9" (name-length, name, dataType) */
            + 24 * 4 /* The remaining entries, have two-character names, "10" to "33" (name-length, name, dataType) */
            + 1 /* The check for the length of each entry's name-length assumes the worst-case (2 bytes), even
                 * though the entries only actually use one. So we will need one more byte available to completely encode it. */
            ;
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, esetDb.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(0, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, esetDb.encode(eIter));
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeSetDefsComplete(eIter, true));
        
        bufSizeNeeded -= 1 /* Back off the one byte the encoder didn't use for the set definition. */;

        /* Encode SeriesEntry. */
        bufSizeNeeded += 3 /* Size bytes */;
        encodeBufSize = bufSizeNeeded;
        primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        seriesEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeInit(eIter, 0));
        
        bufSizeNeeded += 1 /* ElementList flags */ + 2 /* Set data length */ + 2 /* Standard data length */;
        encodeBufSize = bufSizeNeeded;
        primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);

        /* Encode element list. */
        elementList.clear();
        elementList.applyHasSetData();
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(eIter, esetDb, 0));

        /* INT_1 needs 1 byte. */
        bufSizeNeeded += 1;
        elementEntry.clear();
        elementEntry.name().data("0");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* INT_2 needs 2 bytes. */
        bufSizeNeeded += 2;
        elementEntry.clear();
        elementEntry.name().data("1");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* INT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        elementEntry.clear();
        elementEntry.name().data("2");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* INT_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("3");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* UINT_1 needs 1 byte. */
        bufSizeNeeded += 1;
        elementEntry.clear();
        elementEntry.name().data("4");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* UINT_2 needs 2 bytes. */
        bufSizeNeeded += 2;
        elementEntry.clear();
        elementEntry.name().data("5");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* UINT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        elementEntry.clear();
        elementEntry.name().data("6");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* UINT_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("7");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(64);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* FLOAT_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        elementEntry.clear();
        elementEntry.name().data("8");
        elementEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, floatVal));

        /* DOUBLE_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("9");
        elementEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, doubleVal));

        /* REAL_4RB needs 5 bytes for this value. */
        bufSizeNeeded += 5;
        elementEntry.clear();
        elementEntry.name().data("10");
        elementEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, realVal));
                
        /* REAL_8RB needs 9 bytes for this value. */
        bufSizeNeeded += 9;
        elementEntry.clear();
        elementEntry.name().data("11");
        elementEntry.dataType(DataTypes.REAL);
        realVal.value(9223372036854775807L, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, realVal));

        /* DATE_4 needs 4 bytes. */
        bufSizeNeeded += 4;
        elementEntry.clear();
        elementEntry.name().data("12");
        elementEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateVal));

        /* TIME_3 needs 3 bytes. */
        bufSizeNeeded += 3;
        elementEntry.clear();
        elementEntry.name().data("13");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* TIME_5 needs 5 bytes. */
        bufSizeNeeded += 5;
        elementEntry.clear();
        elementEntry.name().data("14");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* DATETIME_7 needs 7 bytes. */
        bufSizeNeeded += 7;
        elementEntry.clear();
        elementEntry.name().data("15");
        elementEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateTimeVal));

        /* DATETIME_11 needs 11 bytes. */
        bufSizeNeeded += 11;
        elementEntry.clear();
        elementEntry.name().data("16");
        elementEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateTimeVal));

        /* DATETIME_12 needs 12 bytes. */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("17");
        elementEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateTimeVal));

        /* TIME_7 needs 7 bytes. */
        bufSizeNeeded += 7;
        elementEntry.clear();
        elementEntry.name().data("18");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* TIME_8 needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("19");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* This INT needs 3 bytes. */
        bufSizeNeeded += 3;
        elementEntry.clear();
        elementEntry.name().data("20");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(255);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* This UINT needs 3 bytes. */
        bufSizeNeeded += 3;
        elementEntry.clear();
        elementEntry.name().data("21");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* This FLOAT needs 5 bytes. */
        bufSizeNeeded += 5;
        elementEntry.clear();
        elementEntry.name().data("22");
        elementEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, floatVal));

        /* This DOUBLE needs 9 bytes. */
        bufSizeNeeded += 9;
        elementEntry.clear();
        elementEntry.name().data("23");
        elementEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, doubleVal));

        /* This REAL needs 6 bytes. */
        bufSizeNeeded += 6;
        elementEntry.clear();
        elementEntry.name().data("24");
        elementEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, realVal));

        /* This DATE needs 5 bytes. */
        bufSizeNeeded += 5;
        elementEntry.clear();
        elementEntry.name().data("25");
        elementEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }

        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateVal));

        /* This TIME needs 9 bytes. */
        bufSizeNeeded += 9;
        elementEntry.clear();
        elementEntry.name().data("26");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* This DATETIME needs 13 bytes. */
        bufSizeNeeded += 13;
        elementEntry.clear();
        elementEntry.name().data("27");
        elementEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateTimeVal));

        /* This QOS needs 6 bytes. */
        bufSizeNeeded += 6;
        elementEntry.clear();
        elementEntry.name().data("28");
        elementEntry.dataType(DataTypes.QOS);
        qosVal.timeliness(QosTimeliness.DELAYED);
        qosVal.timeInfo(3000);
        qosVal.rate(QosRates.TIME_CONFLATED);
        qosVal.rateInfo(5000);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, qosVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, qosVal));

        /* This ENUM needs 3 bytes. */
        bufSizeNeeded += 3;
        elementEntry.clear();
        elementEntry.name().data("29");
        elementEntry.dataType(DataTypes.ENUM);
        enumVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, enumVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, enumVal));

        /* This BUFFER needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("30");
        elementEntry.dataType(DataTypes.BUFFER);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This ASCII_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("31");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This UTF8_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("32");
        elementEntry.dataType(DataTypes.UTF8_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 8 bytes. */
        bufSizeNeeded += 8;
        elementEntry.clear();
        elementEntry.name().data("33");
        elementEntry.dataType(DataTypes.RMTES_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SET_COMPLETE, elementEntry.encode(eIter, bufferVal));

        /* Standard data */

        /* This INT needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
        bufSizeNeeded += 7;
        elementEntry.clear();
        elementEntry.name().data("20");
        elementEntry.dataType(DataTypes.INT);
        intVal.value(255);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, intVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, intVal));

        /* This UINT needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
        bufSizeNeeded += 7;
        elementEntry.clear();
        elementEntry.name().data("21");
        elementEntry.dataType(DataTypes.UINT);
        uintVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, uintVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, uintVal));

        /* This FLOAT needs 7 bytes (1 for dataType, 3 for name, 5 for value) */
        bufSizeNeeded += 9;
        elementEntry.clear();
        elementEntry.name().data("22");
        elementEntry.dataType(DataTypes.FLOAT);
        floatVal.value(1.0f);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, floatVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, floatVal));

        /* This DOUBLE needs 13 bytes (1 for dataType, 3 for name, 9 for value) */
        bufSizeNeeded += 13;
        elementEntry.clear();
        elementEntry.name().data("23");
        elementEntry.dataType(DataTypes.DOUBLE);
        doubleVal.value(1.0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, doubleVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, doubleVal));

        /* This REAL needs 10 bytes (1 for dataType, 3 for name, 6 for value) */
        bufSizeNeeded += 10;
        elementEntry.clear();
        elementEntry.name().data("24");
        elementEntry.dataType(DataTypes.REAL);
        realVal.value(2147483647, RealHints.EXPONENT0);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, realVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, realVal));

        /* This DATE needs 9 bytes (1 for dataType, 3 for name, 5 for value) */
        bufSizeNeeded += 9;
        elementEntry.clear();
        elementEntry.name().data("25");
        elementEntry.dataType(DataTypes.DATE);
        dateVal.day(12);
        dateVal.month(11);
        dateVal.year(1955);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }

        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateVal));

        /* This TIME needs 13 bytes (1 for dataType, 3 for name, 9 for value) */
        bufSizeNeeded += 13;
        elementEntry.clear();
        elementEntry.name().data("26");
        elementEntry.dataType(DataTypes.TIME);
        timeVal.hour(22);
        timeVal.minute(4);
        timeVal.second(0);
        timeVal.millisecond(1);
        timeVal.microsecond(2);
        timeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, timeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, timeVal));

        /* This DATETIME needs 17 bytes (1 for dataType, 3 for name, 13 for value) */
        bufSizeNeeded += 17;
        elementEntry.clear();
        elementEntry.name().data("27");
        elementEntry.dataType(DataTypes.DATETIME);
        dateTimeVal.day(12);
        dateTimeVal.month(11);
        dateTimeVal.year(1955);
        dateTimeVal.hour(22);
        dateTimeVal.minute(4);
        dateTimeVal.second(0);
        dateTimeVal.millisecond(1);
        dateTimeVal.microsecond(2);
        dateTimeVal.nanosecond(3);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, dateTimeVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, dateTimeVal));

        /* This QOS needs 10 bytes (1 for dataType, 3 for name, 6 for value) */
        bufSizeNeeded += 10;
        elementEntry.clear();
        elementEntry.name().data("28");
        elementEntry.dataType(DataTypes.QOS);
        qosVal.timeliness(QosTimeliness.DELAYED);
        qosVal.timeInfo(3000);
        qosVal.rate(QosRates.TIME_CONFLATED);
        qosVal.rateInfo(5000);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, qosVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, qosVal));

        /* This ENUM needs 7 bytes (1 for dataType, 3 for name, 3 for value) */
        bufSizeNeeded += 7;
        elementEntry.clear();
        elementEntry.name().data("29");
        elementEntry.dataType(DataTypes.ENUM);
        enumVal.value(256);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, enumVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, enumVal));

        /* This BUFFER needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("30");
        elementEntry.dataType(DataTypes.BUFFER);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This ASCII_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("31");
        elementEntry.dataType(DataTypes.ASCII_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This UTF8_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("32");
        elementEntry.dataType(DataTypes.UTF8_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("33");
        elementEntry.dataType(DataTypes.RMTES_STRING);
        bufferVal.data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter, bufferVal));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter, bufferVal));

        /* This RMTES_STRING needs 12 bytes (1 for dataType, 3 for name, 8 for value) */
        /* Encode this one via encodedData. */
        bufSizeNeeded += 12;
        elementEntry.clear();
        elementEntry.name().data("34");
        elementEntry.dataType(DataTypes.RMTES_STRING);
        elementEntry.encodedData().data("OVERRUN");
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter));

        /* This RMTES_STRING will be blank and needs 5 bytes (1 for dataType, 3 for name, 1 for value) */
        bufSizeNeeded += 5;
        elementEntry.clear();
        elementEntry.name().data("35");
        elementEntry.dataType(DataTypes.RMTES_STRING);
        while (encodeBufSize < bufSizeNeeded)
        {
            int startPos = ((EncodeIteratorImpl)eIter)._curBufPos;
            assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, elementEntry.encode(eIter));
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._curBufPos);
            assertEquals(startPos, ((EncodeIteratorImpl)eIter)._writer.position());
            assertEquals(1, ((EncodeIteratorImpl)eIter)._encodingLevel);
            ++encodeBufSize;
            primitiveEntryOverrunTest_realignBuffer(eIter, encodeBufSize);
        }
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(eIter));
        

        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(eIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, series.encodeComplete(eIter, true));
        
        
        /* Decode the series, set definition, and elements, to validate data. */
        primitiveEntryOverrunTest_byteBuffer.flip();
        decodeBuffer.data(primitiveEntryOverrunTest_byteBuffer);
        dIter.setBufferAndRWFVersion(decodeBuffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(DataTypes.ELEMENT_LIST, series.containerType());
        assertTrue(series.checkHasSetDefs());

        esetDb.clear();
        assertEquals(CodecReturnCodes.SUCCESS, esetDb.decode(dIter));
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, esetDb));
        
        /* INT_1 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("0"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_2 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("1"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("2"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* INT_8 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("3"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(64, intVal.toLong());

        /* UINT_1 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("4"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_2 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("5"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("6"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* UINT_8 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("7"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(64, uintVal.toLong());

        /* FLOAT_4 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("8"));
        assertEquals(DataTypes.FLOAT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE_8 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("9"));
        assertEquals(DataTypes.DOUBLE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL_4RB */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("10"));
        assertEquals(DataTypes.REAL, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* REAL_8RB */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("11"));
        assertEquals(DataTypes.REAL, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(9223372036854775807L, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE_4 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("12"));
        assertEquals(DataTypes.DATE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME_3 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("13"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(0, timeVal.millisecond());
        assertEquals(0, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* TIME_5 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("14"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(0, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* DATETIME_7 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("15"));
        assertEquals(DataTypes.DATETIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(0, dateTimeVal.millisecond());
        assertEquals(0, dateTimeVal.microsecond());
        assertEquals(0, dateTimeVal.nanosecond());

        /* DATETIME_11 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("16"));
        assertEquals(DataTypes.DATETIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(0, dateTimeVal.nanosecond());

        /* DATETIME_12 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("17"));
        assertEquals(DataTypes.DATETIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* TIME_7 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("18"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(0, timeVal.nanosecond());

        /* TIME_8 */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("19"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* INT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("20"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(255, intVal.toLong());

        /* UINT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("21"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(256, uintVal.toLong());
        
        /* FLOAT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("22"));
        assertEquals(DataTypes.FLOAT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("23"));
        assertEquals(DataTypes.DOUBLE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("24"));
        assertEquals(DataTypes.REAL, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("25"));
        assertEquals(DataTypes.DATE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("26"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* DATETIME */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("27"));
        assertEquals(DataTypes.DATETIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* QOS */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("28"));
        assertEquals(DataTypes.QOS, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, qosVal.decode(dIter));
        assertEquals(QosTimeliness.DELAYED, qosVal.timeliness());
        assertEquals(3000, qosVal.timeInfo());
        assertEquals(QosRates.TIME_CONFLATED, qosVal.rate());
        assertEquals(5000, qosVal.rateInfo());

        /* ENUM */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("29"));
        assertEquals(DataTypes.ENUM, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, enumVal.decode(dIter));
        assertEquals(256, enumVal.toInt());

        /* BUFFER */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("30"));
        assertEquals(DataTypes.BUFFER, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* ASCII_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("31"));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* UTF8_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("32"));
        assertEquals(DataTypes.UTF8_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("33"));
        assertEquals(DataTypes.RMTES_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* Standard data */

        /* INT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("20"));
        assertEquals(DataTypes.INT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, intVal.decode(dIter));
        assertEquals(255, intVal.toLong());

        /* UINT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("21"));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uintVal.decode(dIter));
        assertEquals(256, uintVal.toLong());
        
        /* FLOAT */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("22"));
        assertEquals(DataTypes.FLOAT, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, floatVal.decode(dIter));
        assertEquals(1.0f, floatVal.toFloat(), 0);

        /* DOUBLE */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("23"));
        assertEquals(DataTypes.DOUBLE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, doubleVal.decode(dIter));
        assertEquals(1.0, doubleVal.toDouble(), 0);

        /* REAL */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("24"));
        assertEquals(DataTypes.REAL, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, realVal.decode(dIter));
        assertEquals(2147483647, realVal.toLong(), 0);
        assertEquals(RealHints.EXPONENT0, realVal.hint());

        /* DATE */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("25"));
        assertEquals(DataTypes.DATE, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateVal.decode(dIter));
        assertEquals(12, dateVal.day());
        assertEquals(11, dateVal.month());
        assertEquals(1955, dateVal.year());

        /* TIME */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("26"));
        assertEquals(DataTypes.TIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, timeVal.decode(dIter));
        assertEquals(22, timeVal.hour());
        assertEquals(4, timeVal.minute());
        assertEquals(0, timeVal.second());
        assertEquals(1, timeVal.millisecond());
        assertEquals(2, timeVal.microsecond());
        assertEquals(3, timeVal.nanosecond());

        /* DATETIME */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("27"));
        assertEquals(DataTypes.DATETIME, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, dateTimeVal.decode(dIter));
        assertEquals(12, dateTimeVal.day());
        assertEquals(11, dateTimeVal.month());
        assertEquals(1955, dateTimeVal.year());
        assertEquals(22, dateTimeVal.hour());
        assertEquals(4, dateTimeVal.minute());
        assertEquals(0, dateTimeVal.second());
        assertEquals(1, dateTimeVal.millisecond());
        assertEquals(2, dateTimeVal.microsecond());
        assertEquals(3, dateTimeVal.nanosecond());

        /* QOS */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("28"));
        assertEquals(DataTypes.QOS, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, qosVal.decode(dIter));
        assertEquals(QosTimeliness.DELAYED, qosVal.timeliness());
        assertEquals(3000, qosVal.timeInfo());
        assertEquals(QosRates.TIME_CONFLATED, qosVal.rate());
        assertEquals(5000, qosVal.rateInfo());

        /* ENUM */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("29"));
        assertEquals(DataTypes.ENUM, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, enumVal.decode(dIter));
        assertEquals(256, enumVal.toInt());

        /* BUFFER */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("30"));
        assertEquals(DataTypes.BUFFER, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* ASCII_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("31"));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* UTF8_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("32"));
        assertEquals(DataTypes.UTF8_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("33"));
        assertEquals(DataTypes.RMTES_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING (encoded via encodedData) */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("34"));
        assertEquals(DataTypes.RMTES_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, bufferVal.decode(dIter));
        assertTrue(bufferVal.toString().equals("OVERRUN"));

        /* RMTES_STRING (encoded as blank) */
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertTrue(elementEntry.name().toString().equals("35"));
        assertEquals(DataTypes.RMTES_STRING, elementEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, bufferVal.decode(dIter));

        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, seriesEntry.decode(dIter));
    }


	/* Used by containerEntryEncodeDecodeTest.
	 * Entries are encoded according to the specified method
	 * (where applicable; some containers tested do not have actions or cannot encode inline) */
    private enum EncodeEntryTestMethod
    {
        INLINE_ENCODE, /* Encode the entry using encodeInit() & encodeComplete(). */
        PRE_ENCODE, /* Encode the entry using encode() with pre-encoded data. */
        DELETE /* Encode the entry with the appropriate clear/delete action for that container. */
    }
    
    @Test
    public void containerEntryEncodeDecodeTest()
    {
        /* Test encoding and decoding of the different containers (and array) with varying numbers
         * of entries. 
         * Tests encoding data inline, encoding pre-encoded, and encoding delete actions (or 
         * whichever of these the container can encode).  See ETA-2118, ETA-2207. */

        Buffer buffer = CodecFactory.createBuffer();
        Buffer inlineBuffer = CodecFactory.createBuffer();
        EncodeIterator eIter = CodecFactory.createEncodeIterator();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        ByteBuffer byteBuf = ByteBuffer.allocateDirect(512);

        final int maxEntries = 5;

        for (EncodeEntryTestMethod encodeEntryTestMethod : EncodeEntryTestMethod.values())
        {
            /* Vector */
            for (int i = 0; i <= maxEntries; ++i)
            {
                Vector vec = CodecFactory.createVector();
                VectorEntry vecEntry = CodecFactory.createVectorEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                vec.containerType(DataTypes.OPAQUE);
                assertEquals(CodecReturnCodes.SUCCESS, vec.encodeInit(eIter, 0, 0));

                for (int j = 0; j < i; ++j)
                {
                    vecEntry.clear();
                    vecEntry.index(j);

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            vecEntry.action(VectorEntryActions.INSERT);
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.encodeInit(eIter, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            vecEntry.action(VectorEntryActions.INSERT);
                            vecEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.encode(eIter));
                            break;
                        case DELETE:
                            vecEntry.action(VectorEntryActions.CLEAR);
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.encode(eIter));
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.SUCCESS, vec.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);
                
                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, vec.decode(dIter));
                assertEquals(DataTypes.OPAQUE, vec.containerType());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.decode(dIter));
                            assertEquals(VectorEntryActions.INSERT, vecEntry.action());
                            assertEquals(j, vecEntry.index());
                            assertTrue(vecEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            assertEquals(CodecReturnCodes.SUCCESS, vecEntry.decode(dIter));
                            assertEquals(VectorEntryActions.CLEAR, vecEntry.action());
                            assertEquals(j, vecEntry.index());
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, vecEntry.decode(dIter));
            }

            /* Map */
            for (int i = 0; i <= maxEntries; ++i)
            {
                Map map = CodecFactory.createMap();
                MapEntry mapEntry = CodecFactory.createMapEntry();
                Int key = CodecFactory.createInt();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                map.containerType(DataTypes.OPAQUE);
                map.keyPrimitiveType(DataTypes.INT);
                assertEquals(CodecReturnCodes.SUCCESS, map.encodeInit(eIter, 0, 0));

                for (int j = 0; j < i; ++j)
                {
                    mapEntry.clear();
                    key.value(j);

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            mapEntry.action(MapEntryActions.ADD);
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeInit(eIter, key, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            mapEntry.action(MapEntryActions.ADD);
                            mapEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encode(eIter, key));
                            break;
                        case DELETE:
                            mapEntry.action(MapEntryActions.DELETE);
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.encode(eIter, key));
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.SUCCESS, map.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);

                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
                assertEquals(DataTypes.OPAQUE, map.containerType());
                assertEquals(DataTypes.INT, map.keyPrimitiveType());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, key));
                            assertEquals(MapEntryActions.ADD, mapEntry.action());
                            assertEquals(j, key.toLong());
                            assertTrue(mapEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, key));
                            assertEquals(MapEntryActions.DELETE, mapEntry.action());
                            assertEquals(j, key.toLong());
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, mapEntry.decode(dIter, key));  
            }

            /* Series */
            for (int i = 0; i <= maxEntries; ++i)
            {
                Series series = CodecFactory.createSeries();
                SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                series.containerType(DataTypes.OPAQUE);
                assertEquals(CodecReturnCodes.SUCCESS, series.encodeInit(eIter, 0, 0));

                for (int j = 0; j < i; ++j)
                {
                    seriesEntry.clear();

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeInit(eIter, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            seriesEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.encode(eIter));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                    }

                }

                assertEquals(CodecReturnCodes.SUCCESS, series.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);
                
                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
                assertEquals(DataTypes.OPAQUE, series.containerType());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
                            assertTrue(seriesEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                   }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, seriesEntry.decode(dIter));
            }

            /* FilterList */
            for (int i = 0; i <= maxEntries; ++i)
            {
                FilterList fl = CodecFactory.createFilterList();
                FilterEntry flEntry = CodecFactory.createFilterEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                fl.containerType(DataTypes.OPAQUE);
                assertEquals(CodecReturnCodes.SUCCESS, fl.encodeInit(eIter));

                for (int j = 0; j < i; ++j)
                {
                    flEntry.clear();
                    flEntry.id(j);

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            flEntry.action(FilterEntryActions.SET);
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encodeInit(eIter, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            flEntry.action(FilterEntryActions.SET);
                            flEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encode(eIter));
                            break;
                        case DELETE:
                            flEntry.action(FilterEntryActions.CLEAR);
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encode(eIter));
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.SUCCESS, fl.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);

                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, fl.decode(dIter));
                assertEquals(DataTypes.OPAQUE, fl.containerType());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.decode(dIter));
                            assertEquals(FilterEntryActions.SET, flEntry.action());
                            assertEquals(j, flEntry.id());
                            assertTrue(flEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.decode(dIter));
                            assertEquals(FilterEntryActions.CLEAR, flEntry.action());
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, flEntry.decode(dIter));
            }

            /* FieldList */
            for (int i = 0; i <= maxEntries; ++i)
            {
                FieldList fl = CodecFactory.createFieldList();
                FieldEntry flEntry = CodecFactory.createFieldEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                fl.applyHasStandardData();
                assertEquals(CodecReturnCodes.SUCCESS, fl.encodeInit(eIter, null, 0));

                for (int j = 0; j < i; ++j)
                {
                    flEntry.clear();
                    flEntry.fieldId(j);
                    flEntry.dataType(DataTypes.OPAQUE);

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encodeInit(eIter, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            flEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.encode(eIter));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.SUCCESS, fl.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);

                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, fl.decode(dIter, null));
                assertTrue(fl.checkHasStandardData());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, flEntry.decode(dIter));
                            assertEquals(DataTypes.UNKNOWN, flEntry.dataType());
                            assertEquals(j, flEntry.fieldId());
                            assertTrue(flEntry.encodedData().toString().equals("OPAQUE"));
                             assertTrue(flEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, flEntry.decode(dIter));
            }

            /* ElementList */
            for (int i = 0; i <= maxEntries; ++i)
            {
                ElementList el = CodecFactory.createElementList();
                ElementEntry elEntry = CodecFactory.createElementEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                el.applyHasStandardData();
                assertEquals(CodecReturnCodes.SUCCESS, el.encodeInit(eIter, null, 0));

                for (int j = 0; j < i; ++j)
                {
                    elEntry.clear();
                    elEntry.name().data(Integer.toString(j));
                    elEntry.dataType(DataTypes.OPAQUE);

                    switch (encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, elEntry.encodeInit(eIter, 0));
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFInit(inlineBuffer));
                            inlineBuffer.data().put("OPAQUE".getBytes());
                            assertEquals(CodecReturnCodes.SUCCESS, eIter.encodeNonRWFComplete(inlineBuffer, true));
                            assertEquals(CodecReturnCodes.SUCCESS, elEntry.encodeComplete(eIter, true));
                            break;
                        case PRE_ENCODE:
                            elEntry.encodedData().data("OPAQUE");
                            assertEquals(CodecReturnCodes.SUCCESS, elEntry.encode(eIter));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.SUCCESS, el.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);

                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, el.decode(dIter, null));
                assertTrue(el.checkHasStandardData());

                for (int j = 0; j < i; ++j)
                {
                    switch(encodeEntryTestMethod)
                    {
                        case INLINE_ENCODE:
                        case PRE_ENCODE:
                            assertEquals(CodecReturnCodes.SUCCESS, elEntry.decode(dIter));
                            assertEquals(DataTypes.OPAQUE, elEntry.dataType());
                            assertTrue(elEntry.name().toString().equals(Integer.toString(j)));
                            assertTrue(elEntry.encodedData().toString().equals("OPAQUE"));
                            break;
                        case DELETE:
                            /* No entry encoded (not applicable). */
                            break;
                        default:
                            break;
                    }
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, elEntry.decode(dIter));
            }

            /* Array */
            for (int i = 0; i <= maxEntries; ++i)
            {
                Array array = CodecFactory.createArray();
                ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

                byteBuf.clear();
                buffer.data(byteBuf);

                /* Encode */
                eIter.clear();
                eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, array.encodeInit(eIter));
                array.primitiveType(DataTypes.BUFFER);

                for (int j = 0; j < i; ++j)
                {
                    arrayEntry.clear();

                    /* Array Entries are encoded as pre-encoded only. */
                    arrayEntry.encodedData().data("OPAQUE");
                    assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.encode(eIter));
                }

                assertEquals(CodecReturnCodes.SUCCESS, array.encodeComplete(eIter, true));

                byteBuf.limit(byteBuf.position());
                byteBuf.position(0);
                buffer.data(byteBuf);

                /* Decode */
                dIter.clear();
                dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

                assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));

                for (int j = 0; j < i; ++j)
                {
                    assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.decode(dIter));
                    assertTrue(arrayEntry.encodedData().toString().equals("OPAQUE"));
                }

                assertEquals(CodecReturnCodes.END_OF_CONTAINER, arrayEntry.decode(dIter));
            }
        }
    }
}
