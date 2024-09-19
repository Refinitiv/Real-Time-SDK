/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

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
        Int integer = CodecFactory.createInt();
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
        
      //Test with -15
        buf.data(ByteBuffer.allocate(30));
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        integer.value(-15);
        integer.encode(enc);
        jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[100];
        jsonBuf.position = 5;

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.INT).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[4]);
        assertEquals('-', jsonBuf.data[5]);
        assertEquals('1', (char)jsonBuf.data[6]);
        assertEquals('5', (char)jsonBuf.data[7]);
        assertEquals(0, jsonBuf.data[8]);

        //Test with a long number that doesn't fit the initial byte array
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;
        uInt.value(12345678091234567L);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        uInt.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.UINT).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[2]);
        int length = "12345678091234567".length();
        int i;
        for (i = 0; i < length; i++) {
            assertEquals("12345678091234567".charAt(i), jsonBuf.data[i + 3]);
        }
        
      //Test with a long negative number that doesn't fit the initial byte array
        buf.clear();
        buf.data(ByteBuffer.allocate(30));
        enc.clear();
        dec.clear();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 3;
        uInt.value(-12345678091234567L);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        uInt.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.INT).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[2]);
        length = "-12345678091234567".length();
        for (i = 0; i < length; i++) {
            assertEquals("-12345678091234567".charAt(i), jsonBuf.data[i + 3]);
        }
        assertEquals((i+3), jsonBuf.position);
    }

    @Test
    public void testRealConversion() {
        EncodeIterator enc = CodecFactory.createEncodeIterator();
        DecodeIterator dec = CodecFactory.createDecodeIterator();
        Real real = CodecFactory.createReal();
        Buffer buf = CodecFactory.createBuffer();
        int i;
        
        /* Preserve trailing zeroes with > 32-bit value before decimal */
        buf.data(ByteBuffer.allocate(50));
        JsonBuffer jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(6543210123000L, 11);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        int length = "6543210123.000".length();
        for (i = 0; i < length; i++) {
            assertEquals("6543210123.000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Preserve trailing zeroes with > 32-bit value before decimal, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-6543210123000L, 11);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-6543210123.000".length();
        for (i = 0; i < length; i++) {
            assertEquals("-6543210123.000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Preserve trailing zeroes with < 32-bit value before decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(65432101000L, 11);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "65432101.000".length();
        for (i = 0; i < length; i++) {
            assertEquals("65432101.000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Preserve trailing zeroes with < 32-bit value before decimal, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf = new JsonBuffer();
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-65432101000L, 11);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-65432101.000".length();
        for (i = 0; i < length; i++) {
            assertEquals("-65432101.000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, no decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(6543210123L, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "6543210123".length();
        for (i = 0; i < length; i++) {
            assertEquals("6543210123".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, no decimal, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-6543210123L, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-6543210123".length();
        for (i = 0; i < length; i++) {
            assertEquals("-6543210123".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, no decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(654321012, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, no decimal, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-654321012, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("-654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, no decimal, add trailing zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(6543210123L, 18);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "65432101230000".length();
        for (i = 0; i < length; i++) {
            assertEquals("65432101230000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, no decimal, add trailing zeroes, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-6543210123L, 18);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-65432101230000".length();
        for (i = 0; i < length; i++) {
            assertEquals("-65432101230000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, no decimal, add trailing zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(654321012, 18);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "6543210120000".length();
        for (i = 0; i < length; i++) {
            assertEquals("6543210120000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, no decimal, add trailing zeroes, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-654321012, 18);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-6543210120000".length();
        for (i = 0; i < length; i++) {
            assertEquals("-6543210120000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(9876543210123L, 1);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "0.9876543210123".length();
        for (i = 0; i < length; i++) {
            assertEquals("0.9876543210123".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, decimal, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9876543210123L, 1);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-0.9876543210123".length();
        for (i = 0; i < length; i++) {
            assertEquals("-0.9876543210123".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(654321012, 5);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "0.654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("0.654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-654321012, 5);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-0.654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("-0.654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, decimal, leading zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(98765432101L, 0);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "0.00098765432101".length();
        for (i = 0; i < length; i++) {
            assertEquals("0.00098765432101".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* > 32-bit value, decimal, leading zeroes, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-98765432101L, 0);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-0.00098765432101".length();
        for (i = 0; i < length; i++) {
            assertEquals("-0.00098765432101".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, decimal, leading zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(654321012, 0);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "0.00000654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("0.00000654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* < 32-bit value, decimal, leading zeroes, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-654321012, 0);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-0.00000654321012".length();
        for (i = 0; i < length; i++) {
            assertEquals("-0.00000654321012".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max 64-bit value, no decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(9223372036854775807L, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "9223372036854775807".length();
        for (i = 0; i < length; i++) {
            assertEquals("9223372036854775807".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, no decimal */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 14);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-9223372036854775808".length();
        for (i = 0; i < length; i++) {
            assertEquals("-9223372036854775808".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max 64-bit value, decimal, > 32-bit, then < 32-bit */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(9223372036854775807L, 5);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "9223372036.854775807".length();
        for (i = 0; i < length; i++) {
            assertEquals("9223372036.854775807".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, > 32-bit, then < 32-bit */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 5);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-9223372036.854775808".length();
        for (i = 0; i < length; i++) {
            assertEquals("-9223372036.854775808".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max 64-bit value, decimal, < 32-bit, then > 32-bit */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(9223372036854775807L, 4);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "922337203.6854775807".length();
        for (i = 0; i < length; i++) {
            assertEquals("922337203.6854775807".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, < 32-bit, then > 32-bit */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 4);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-922337203.6854775808".length();
        for (i = 0; i < length; i++) {
            assertEquals("-922337203.6854775808".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max 64-bit value, trailing zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(9223372036854775807L, 19);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "922337203685477580700000".length();
        for (i = 0; i < length; i++) {
            assertEquals("922337203685477580700000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, trailing zeroes */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 19);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-922337203685477580800000".length();
        for (i = 0; i < length; i++) {
            assertEquals("-922337203685477580800000".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* blank */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.blank();
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "null".length();
        for (i = 0; i < length; i++) {
            assertEquals("null".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* blank */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(55, 3);
        real.blank();
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "null".length();
        for (i = 0; i < length; i++) {
            assertEquals("null".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* positive infinity */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(55, 33);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "Inf".length();
        for (i = 0; i < length; i++) {
            assertEquals("Inf".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* negative infinity */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(55, 34);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-Inf".length();
        for (i = 0; i < length; i++) {
            assertEquals("-Inf".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* not a number */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(55, 35);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "NaN".length();
        for (i = 0; i < length; i++) {
            assertEquals("NaN".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* Simple fraction 50/2 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(50, 23);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "25".length();
        for (i = 0; i < length; i++) {
            assertEquals("25".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* Simple fraction 50/2, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-50, 23);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-25".length();
        for (i = 0; i < length; i++) {
            assertEquals("-25".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 50/128 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(50, 29);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "0.390625".length();
        for (i = 0; i < length; i++) {
            assertEquals("0.390625".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 50/128, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-50, 29);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-0.390625".length();
        for (i = 0; i < length; i++) {
            assertEquals("-0.390625".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 50/32 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(50, 27);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "1.5625".length();
        for (i = 0; i < length; i++) {
            assertEquals("1.5625".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 50/32, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-50, 27);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-1.5625".length();
        for (i = 0; i < length; i++) {
            assertEquals("-1.5625".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 98765432101234/2 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(98765432101234L, 23);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "49382716050617".length();
        for (i = 0; i < length; i++) {
            assertEquals("49382716050617".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 98765432101234/2, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-98765432101234L, 23);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-49382716050617".length();
        for (i = 0; i < length; i++) {
            assertEquals("-49382716050617".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 98765432101234/128 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(98765432101234L, 29);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "771604938290.8906".length();
        for (i = 0; i < length; i++) {
            assertEquals("771604938290.8906".charAt(i), jsonBuf.data[i + 1]);
        }
        
        /* More complex fraction 98765432101234/128, negative */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-98765432101234L, 29);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-771604938290.8906".length();
        for (i = 0; i < length; i++) {
            assertEquals("-771604938290.8906".charAt(i), jsonBuf.data[i + 1]);
        }
       
        /* Max negative 64-bit value, fraction 2 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 22);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-9223372036854775808".length();
        for (i = 0; i < length; i++) {
            assertEquals("-9223372036854775808".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
       
        
        /* Max negative 64-bit value, fraction 1 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 22);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-9223372036854775808".length();
        for (i = 0; i < length; i++) {
            assertEquals("-9223372036854775808".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, fraction 1/2 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 23);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-4611686018427387904".length();
        for (i = 0; i < length; i++) {
            assertEquals("-4611686018427387904".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
        
        /* Max negative 64-bit value, fraction 1/128 */
        buf.data(ByteBuffer.allocate(50));
        jsonBuf.data = new byte[10];
        jsonBuf.position = 1;

        real.clear();
        real.value(-9223372036854775808L, 29);
        enc.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        real.encode(enc);

        dec.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(true, converter.getPrimitiveHandler(DataTypes.REAL).encodeJson(dec, jsonBuf, convError));
        assertEquals(0, jsonBuf.data[0]);
        length = "-72057594037927936".length();
        for (i = 0; i < length; i++) {
            assertEquals("-72057594037927936".charAt(i), jsonBuf.data[i + 1]);
        }
        assertEquals((i+1), jsonBuf.position);
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
