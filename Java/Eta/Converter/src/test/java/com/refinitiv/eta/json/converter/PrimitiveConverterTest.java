package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.codec.Double;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static org.junit.Assert.assertEquals;

public class PrimitiveConverterTest {

    private JsonAbstractConverter converter;
    private JsonConverterError convError;

    @Before
    public void init() {
        convError = ConverterFactory.createJsonConverterError();

        converter = (JsonAbstractConverter) ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, true)
                .build(convError);
    }

    @Test
    public void testLongConvertion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        UInt uInt = CodecFactory.createUInt();
        Buffer buf = CodecFactory.createBuffer();

        //Test with 15
        buf.data(ByteBuffer.allocate(30));
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        uInt.value(15);
        uInt.encode(enc);
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[100];
        jsonBuf.position = 5;

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.UINT).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[4]);
        assertEquals('1', (char)jsonBuf.data[5]);
        assertEquals('5', (char)jsonBuf.data[6]);
        assertEquals(0, jsonBuf.data[7]);

        //Test with a long number that doesn't fit the initial byte array
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        uInt.value(123456789123456L);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        uInt.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.UINT).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[2]);
        assertEquals('1', (char)jsonBuf.data[3]);
        assertEquals('2', (char)jsonBuf.data[4]);
        assertEquals('3', (char)jsonBuf.data[5]);
        assertEquals('6', (char)jsonBuf.data[17]);
        assertEquals(0, jsonBuf.data[18]);
    }

    @Test
    public void testRealConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        Real real = CodecFactory.createReal();
        Buffer buf = CodecFactory.createBuffer();

        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.value(9321654987123L, 25);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        int length = "1165206873390.375".length();
        for (int i = 0; i < length; i++) {
            assertEquals("1165206873390.375".charAt(i), jsonBuf.data[i + 1]);
        }
    }

    @Test
    public void testDoubleConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        Double dv = CodecFactory.createDouble();
        Buffer buf = CodecFactory.createBuffer();

        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 2;

        dv.value(123456789.123);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        dv.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.DOUBLE).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[1]);
        int length = "1.23456789123E8".length();
        for (int i = 0; i < length; i++) {
            assertEquals("1.23456789123E8".charAt(i), jsonBuf.data[i + 2]);
        }
    }

    @Test
    public void testDateConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        Date date = CodecFactory.createDate();
        Buffer buf = CodecFactory.createBuffer();

        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        date.year(1975);
        date.month(12);
        date.day(25);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        date.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.DATE).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[2]);
        int length = "\"1975-12-25\"".length();
        for (int i = 0; i < length; i++) {
            assertEquals("\"1975-12-25\"".charAt(i), jsonBuf.data[i + 3]);
        }
    }

    @Test
    public void testTimeConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        Time time = CodecFactory.createTime();
        Buffer buf = CodecFactory.createBuffer();

        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 6;

        time.hour(12);
        time.minute(55);
        time.second(43);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        time.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.TIME).encodeJson(dec, jsonBuf, convError));
        for (int i = 0; i < "\"12:55:43\"".length(); i++) {
            assertEquals("\"12:55:43\"".charAt(i), jsonBuf.data[i + 6]);
        }

        time.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        time.hour(11);
        time.minute(5);
        time.second(13);
        time.millisecond(0);
        time.microsecond(120);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        time.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.TIME).encodeJson(dec, jsonBuf, convError));
        for (int i = 0; i < "\"11:05:13.00012\"".length(); i++) {
            assertEquals("\"11:05:13.00012\"".charAt(i), jsonBuf.data[i + 3]);
        }
        assertEquals(0, jsonBuf.data[19]);
    }

    @Test
    public void testDateTimeConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        DateTime dateTime = CodecFactory.createDateTime();
        Buffer buf = CodecFactory.createBuffer();

        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 6;

        dateTime.hour(12);
        dateTime.minute(55);
        dateTime.second(43);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        dateTime.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.DATETIME).encodeJson(dec, jsonBuf, convError));
        for (int i = 0; i < "\"T12:55:43\"".length(); i++) {
            assertEquals(("\"T12:55:43\"").charAt(i), jsonBuf.data[i + 6]);
        }

        dateTime.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        dateTime.hour(11);
        dateTime.minute(5);
        dateTime.second(13);
        dateTime.millisecond(0);
        dateTime.microsecond(120);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        dateTime.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.DATETIME).encodeJson(dec, jsonBuf, convError));
        for (int i = 0; i < "\"T11:05:13.00012\"".length(); i++) {
            assertEquals("\"T11:05:13.00012\"".charAt(i), jsonBuf.data[i + 3]);
        }
        assertEquals(0, jsonBuf.data[20]);

        dateTime.clear();
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        dateTime.hour(11);
        dateTime.minute(5);
        dateTime.second(13);
        dateTime.millisecond(0);
        dateTime.microsecond(120);
        dateTime.year(1999);
        dateTime.month(12);
        dateTime.day(1);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        dateTime.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.DATETIME).encodeJson(dec, jsonBuf, convError));
        for (int i = 0; i < "\"1999-12-01T11:05:13.00012\"".length(); i++) {
            assertEquals("\"1999-12-01T11:05:13.00012\"".charAt(i), jsonBuf.data[i + 3]);
        }
        assertEquals(0, jsonBuf.data[31]);
    }

    @Test
    public void testAsciiStringConversion() {
        ByteBuffer buf;
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        Buffer inBuffer = CodecFactory.createBuffer();
        inBuffer.data(ByteBuffer.allocate(200));

        buf = putString("test string [] \" 12");
        buffer.data(buf);
        encIter.setBufferAndRWFVersion(inBuffer, Codec.majorVersion(), Codec.minorVersion());
        buffer.encode(encIter);

        decIter.setBufferAndRWFVersion(inBuffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 4;

        assertEquals(true, converter.getPrimitiveHandler(DataTypes.ASCII_STRING).encodeJson(decIter, jsonBuf, convError));
        assertEquals('\"', (char)jsonBuf.data[4]);
        assertEquals('t', (char)jsonBuf.data[5]);
        assertEquals('e', (char)jsonBuf.data[6]);
        assertEquals('s', (char)jsonBuf.data[7]);
        assertEquals('t', (char)jsonBuf.data[8]);
        assertEquals(' ', (char)jsonBuf.data[9]);
        assertEquals('s', (char)jsonBuf.data[10]);
        assertEquals('\\', (char)jsonBuf.data[20]);
        assertEquals('\"', (char)jsonBuf.data[21]);
    }

    @Test
    public void testRMTESStringConversion() {
        ByteBuffer buf;
        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        DecodeIterator decIter = CodecFactory.createDecodeIterator();
        Buffer buffer = CodecFactory.createBuffer();
        Buffer inBuffer = CodecFactory.createBuffer();
        inBuffer.data(ByteBuffer.allocate(200));

        buf = putString("01 [] () abcd");
        buffer.data(buf);
        encIter.setBufferAndRWFVersion(inBuffer, Codec.majorVersion(), Codec.minorVersion());
        buffer.encode(encIter);

        decIter.setBufferAndRWFVersion(inBuffer, Codec.majorVersion(), Codec.minorVersion());
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;

        assertEquals(true, converter.getPrimitiveHandler(DataTypes.RMTES_STRING).encodeJson(decIter, jsonBuf, convError));
        assertEquals('\"', (char)jsonBuf.data[3]);
        assertEquals('0', (char)jsonBuf.data[4]);
        assertEquals('1', (char)jsonBuf.data[5]);
        assertEquals(' ', (char)jsonBuf.data[6]);
        assertEquals('[', (char)jsonBuf.data[7]);
        assertEquals(']', (char)jsonBuf.data[8]);
        assertEquals(' ', (char)jsonBuf.data[9]);
        assertEquals('(', (char)jsonBuf.data[10]);
        assertEquals('\"', (char)jsonBuf.data[17]);
    }

    //borrowed from PrimitiveDataJunit.java
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
}
