/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import org.junit.Before;
import org.junit.Test;

import java.lang.Double;
import java.lang.Float;
import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;

public class BasicPrimitiveConverterTest {

    JsonConverterError error;

    @Before
    public void init() {
        error = ConverterFactory.createJsonConverterError();
    }

    @Test
    public void testLengthDivide() {

        assertEquals(6, BasicPrimitiveConverter.getIntLengthDivide(234654));
        assertEquals(1, BasicPrimitiveConverter.getIntLengthDivide(0));
        assertEquals(9, BasicPrimitiveConverter.getIntLengthDivide(-23542354));
        assertEquals(10, BasicPrimitiveConverter.getIntLengthDivide(Integer.MAX_VALUE));
        assertEquals(11, BasicPrimitiveConverter.getIntLengthDivide(Integer.MIN_VALUE));
        assertEquals(7, BasicPrimitiveConverter.getIntLengthDivide(-123456));
    }

    @Test
    public void testLengthCompare() {

        assertEquals(6, BasicPrimitiveConverter.getIntLengthCompare(234654));
        assertEquals(1, BasicPrimitiveConverter.getIntLengthCompare(0));
        assertEquals(9, BasicPrimitiveConverter.getIntLengthCompare(-23542354));
        assertEquals(10, BasicPrimitiveConverter.getIntLengthCompare(Integer.MAX_VALUE));
        assertEquals(11, BasicPrimitiveConverter.getIntLengthCompare(Integer.MIN_VALUE));
        assertEquals(7, BasicPrimitiveConverter.getIntLengthCompare(-123456));

    }

    @Test
    public void testWriteInt() {
        byte[] buf = new byte[25];
        BasicPrimitiveConverter.writeInt(123456778, 9, buf, 7 );
        for (int i = 7; i < 16; i++)
            assertEquals("123456778".charAt(i - 7), (char)buf[i]);

        BasicPrimitiveConverter.writeInt(123000078, 9, buf, 7 );
        for (int i = 7; i < 16; i++)
            assertEquals("123000078".charAt(i - 7), (char)buf[i]);

        BasicPrimitiveConverter.writeInt(123450000, 9, buf, 7 );
        for (int i = 7; i < 16; i++)
            assertEquals("123450000".charAt(i - 7), (char)buf[i]);
    }

    @Test
    public void testWriteLong() {
    	 byte[] buf = new byte[35];
    	 int i;
         BasicPrimitiveConverter.writeLong(12345677812345L, 14, 7, buf);
         for (i = 7; i < 21; i++) {
             assertEquals("12345677812345".charAt(i - 7), (char)buf[i]);
         }
         assertEquals(buf[i+1], 0); 
         buf = new byte[35];
        BasicPrimitiveConverter.writeLong(-12345677812345L, 15, 7, buf);
        for (i = 7; i < 22; i++) {
            assertEquals("-12345677812345".charAt(i - 7), (char)buf[i]);
        }
        assertEquals(buf[i+1], 0); 
    }

    @Test
    public void testRealLength() {
        Real r = CodecFactory.createReal();
        r.value(12345678625L, RealHints.FRACTION_16); //771604914.0625
        assertEquals(14, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(356789242556346L, RealHints.FRACTION_256); //1393707978735.7265625
        assertEquals(21, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(23, RealHints.FRACTION_256);
        assertEquals(10, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(23, RealHints.EXPONENT_4);
        assertEquals(6, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(234678, RealHints.EXPONENT_4);
        assertEquals(7, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(234678, RealHints.EXPONENT0);
        assertEquals(6, BasicPrimitiveConverter.getRealLength(r, false));
        r.value(234678, RealHints.EXPONENT4);
        assertEquals(10, BasicPrimitiveConverter.getRealLength(r, false));
    }

    @Test
    public void testWriteReal() {
        Real r = CodecFactory.createReal();
        byte[] buf = new byte[40];
        r.value(12345678625L, RealHints.FRACTION_16); //771604914.0625
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 14; i++) {
            assertEquals("771604914.0625".charAt(i), (char) buf[i]);
        }
        buf = new byte[40];
        r.value(356789242556346L, RealHints.FRACTION_256); //1393707978735.7265625
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 21; i++) {
            assertEquals("1393707978735.7265625".charAt(i), (char) buf[i]);
        }
        buf = new byte[40];
        r.value(23, RealHints.FRACTION_256); //0.08984375
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 10; i++) {
            assertEquals("0.08984375".charAt(i), (char) buf[i]);
        }
        buf = new byte[40];
        r.value(23, RealHints.EXPONENT_4); //0.0023
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 6; i++) {
            assertEquals("0.0023".charAt(i), (char) buf[i]);
        }
        buf = new byte[40];
        r.value(234678, RealHints.EXPONENT_4); //23.4678
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 7; i++) {
            assertEquals("23.4678".charAt(i), (char) buf[i]);
        }
        buf = new byte[40];
        r.value(234678, RealHints.EXPONENT0); //234678
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 6; i++) {
            assertEquals("234678".charAt(i), (char) buf[i]);
        }
        buf = new byte[40]; //2346780000
        r.value(234678, RealHints.EXPONENT4);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 10; i++) {
            assertEquals("2346780000".charAt(i), (char) buf[i]);
        }
        buf = new byte[40]; //234678.0000
        r.value(2346780000L, RealHints.EXPONENT_4);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("234678.0000".charAt(i), (char) buf[i]);
        }
        buf = new byte[40]; //234678.0050
        r.value(2346780050L, RealHints.EXPONENT_4);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("234678.0050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //23467.80050
        r.value(2346780050L, RealHints.EXPONENT_5);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("23467.80050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //2346.780050
        r.value(2346780050L, RealHints.EXPONENT_6);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("2346.780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234.6780050
        r.value(2346780050L, RealHints.EXPONENT_7);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("234.6780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234.6780050
        r.value(2346780050L, RealHints.EXPONENT_8);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("23.46780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234678.0050
        r.value(2346780050L, RealHints.EXPONENT_9);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("2.346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234678.0050
        r.value(2346780050L, RealHints.EXPONENT_10);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 12; i++) {
            assertEquals("0.2346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234678.0050
        r.value(2346780050L, RealHints.EXPONENT_11);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 13; i++) {
            assertEquals("0.02346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //234678.0050
        r.value(2346780050L, RealHints.EXPONENT_12);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 14; i++) {
            assertEquals("0.002346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0.0002346780050
        r.value(2346780050L, RealHints.EXPONENT_13);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 15; i++) {
            assertEquals("0.0002346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0.00002346780050
        r.value(2346780050L, RealHints.EXPONENT_14);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 16; i++) {
            assertEquals("0.00002346780050".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(0, RealHints.EXPONENT_14);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 1; i++) {
            assertEquals("0".charAt(i), (char) buf[i]);
        }
        assertEquals(0, buf[1]);

        buf = new byte[40]; //0
        r.value(0, RealHints.FRACTION_16);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 1; i++) {
            assertEquals("0".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_2);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 4; i++) {
            assertEquals("12.5".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_4);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 4; i++) {
            assertEquals("6.25".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_8);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 5; i++) {
            assertEquals("3.125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_16);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 6; i++) {
            assertEquals("1.5625".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_32);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 7; i++) {
            assertEquals("0.78125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_64);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 8; i++) {
            assertEquals("0.390625".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_128);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 9; i++) {
            assertEquals("0.1953125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(25, RealHints.FRACTION_256);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 10; i++) {
            assertEquals("0.09765625".charAt(i), (char) buf[i]);
        }
        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_2);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 5; i++) {
            assertEquals("-12.5".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_4);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 5; i++) {
            assertEquals("-6.25".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_8);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 6; i++) {
            assertEquals("-3.125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_16);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 7; i++) {
            assertEquals("-1.5625".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_32);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 8; i++) {
            assertEquals("-0.78125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_64);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 9; i++) {
            assertEquals("-0.390625".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_128);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 10; i++) {
            assertEquals("-0.1953125".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_256);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 11; i++) {
            assertEquals("-0.09765625".charAt(i), (char) buf[i]);
        }

        buf = new byte[40]; //0
        r.value(-25, RealHints.FRACTION_1);
        BasicPrimitiveConverter.writeReal(r, buf, 0, false);
        for (int i = 0; i < 3; i++) {
            assertEquals("-25".charAt(i), (char) buf[i]);
        }

    }

    @Test
    public void testWriteDouble() {

        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[50];
        String string;
        com.refinitiv.eta.codec.Double dv = CodecFactory.createDouble();

        dv.value(1.7897777777773334);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < Double.toString(1.7897777777773334).length(); j++) {
            assertEquals(Double.toString(1.7897777777773334).charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(-1.7897777777773334);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < Double.toString(-1.7897777777773334).length(); j++) {
            assertEquals(Double.toString(-1.7897777777773334).charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(254254.7897777777);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        string = Double.toString(254254.7897777777);
        for (int j = 0; j < string.length(); j++) {
            assertEquals(string.charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(-1.125);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.125".charAt(j), (char)jb.data[j]);
        }
        assertEquals(0, jb.data[6]);

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(-1.789);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < Double.toString(-1.789).length(); j++) {
            assertEquals(Double.toString(-1.789).charAt(j), (char)jb.data[j]);
        }
        assertEquals(0, jb.data[6]);

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(-0.000125);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < Double.toString(-0.000125).length(); j++) {
            assertEquals(Double.toString(-0.000125).charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(123456789123456789123.0);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < Double.toString(123456789123456789123.0).length(); j++) {
            assertEquals(Double.toString(123456789123456789123.0).charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(Double.MIN_VALUE);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        string = Double.toString(Double.MIN_VALUE);
        for (int j = 0; j < string.length(); j++) {
            assertEquals(string.charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(Double.MAX_VALUE);
        string = Double.toString(Double.MAX_VALUE);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        for (int j = 0; j < string.length(); j++) {
            assertEquals(string.charAt(j), (char)jb.data[j]);
        }

        jb.data = new byte[50];
        jb.position = 0;
        dv.value(0);
        BasicPrimitiveConverter.writeDouble(dv, jb, false, error);
        string = Double.toString(0);
        for (int j = 0; j < string.length(); j++) {
            assertEquals(string.charAt(j), (char)jb.data[j]);
        }
    }

    @Test
    public void testWriteDouble0() {

        byte[] buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(1.7897777777773334566, 0, buf);
        for (int j = 0; j < 16; j++) {
            assertEquals("1.7897777777773334566".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(-1.7897777777773334566, 0, buf);
        for (int j = 0; j < 17; j++) {
            assertEquals("-1.7897777777773334566".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(254254.7897777777, 0, buf);
        for (int j = 0; j < 16; j++) {
            assertEquals("254254.7897777777".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(-1.125, 0, buf);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.125".charAt(j), (char)buf[j]);
        }
        assertEquals(0, buf[6]);

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(-1.789, 0, buf);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.789".charAt(j), (char)buf[j]);
        }
        assertEquals(0, buf[6]);

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(-0.000125, 0, buf);
        for (int j = 0; j < 8; j++) {
            assertEquals("-1.25e-4".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(123456789123456789123.0, 0, buf);
        for (int j = 0; j < 16; j++) {
            assertEquals("1.23456789123456".charAt(j), (char)buf[j]);
        }
        for (int j = 16; j < 20; j++) {
            if (buf[j] == 'e'){
                assertEquals('0' + 2, buf[j+1]);
                assertEquals('0' + 0, buf[j+2]);
                assertEquals( 0, buf[j+3]);
                break;
            }
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(Double.MIN_VALUE, 0, buf);
        for (int j = 0; j < 4; j++) {
            assertEquals("4.94".charAt(j), (char)buf[j]);
        }
        for (int j = 0; j < 20; j++) {
            if (buf[j] == 'e'){
                assertEquals('-' + 0, buf[j+1]);
                assertEquals('0' + 3, buf[j+2]);
                assertEquals( '0' + 2, buf[j+3]);
                assertEquals( '0' + 4, buf[j+4]);
                assertEquals( 0, buf[j+5]);
                break;
            }
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(Double.MAX_VALUE, 0, buf);
        for (int j = 0; j < 16; j++) {
            assertEquals("1.79769313486231".charAt(j), (char)buf[j]);
        }
        for (int j = 16; j < 20; j++) {
            if (buf[j] == 'e'){
                assertEquals('0' + 3, buf[j+1]);
                assertEquals('0' + 0, buf[j+2]);
                assertEquals( '0' + 8, buf[j+3]);
                assertEquals( 0, buf[j+4]);
                break;
            }
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeDouble0(0, 0, buf);
        assertEquals('0', (char)buf[0]);
        for (int j = 1; j < 50; j++) {
            assertEquals(buf[j], 0);
        }
    }

    @Test
    public void testWriteFloat0() {

        byte[] buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(1.789f, 0, buf);
        for (int j = 0; j < 5; j++) {
            assertEquals("1.789".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(-1.78977f, 0, buf);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.78977".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(254254.78f, 0, buf);
        for (int j = 0; j < 7; j++) {
            assertEquals("254254.78".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(-1.125f, 0, buf);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.125".charAt(j), (char)buf[j]);
        }
        assertEquals(0, buf[6]);

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(-1.789f, 0, buf);
        for (int j = 0; j < 6; j++) {
            assertEquals("-1.789".charAt(j), (char)buf[j]);
        }
        assertEquals(0, buf[6]);

       buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(-0.000125f, 0, buf);
        for (int j = 0; j < 8; j++) {
            assertEquals("-1.25e-4".charAt(j), (char)buf[j]);
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(1234567891.0f, 0, buf);
        for (int j = 0; j < 8; j++) {
            assertEquals("1.234567891".charAt(j), (char)buf[j]);
        }
        for (int j = 7; j < 20; j++) {
            if (buf[j] == 'e'){
                assertEquals('0' + 9, buf[j+1]);
                assertEquals( 0, buf[j+2]);
                break;
            }
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(Float.MIN_VALUE, 0, buf);
        for (int j = 0; j < 3; j++) {
            assertEquals("1.4".charAt(j), (char)buf[j]);
        }
        for (int j = 0; j < 20; j++) {
            if (buf[j] == 'e'){
                assertEquals('-' + 0, buf[j+1]);
                assertEquals('0' + 4, buf[j+2]);
                assertEquals( '0' + 5, buf[j+3]);
                assertEquals( 0, buf[j+4]);
                break;
            }
        }

        buf = new byte[50];
        BasicPrimitiveConverter.writeFloat0(0, 0, buf);
        assertEquals('0', (char)buf[0]);
        for (int j = 1; j < 50; j++) {
            assertEquals(buf[j], 0);
        }
    }

    @Test
    public void testWriteDate() {
        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[50];
        Date date = CodecFactory.createDate();
        boolean res;

        jb.position = 0;
        jb.data = new byte[50];
        date.year(2020);
        date.month(1);
        date.day(2);
        res = BasicPrimitiveConverter.writeDate(date, jb, error);
        for (int i = 0; i < 12; i++) {
            assertEquals("\"2020-01-02\"".charAt(i), (char)jb.data[i]);
        }
        assertEquals(12, jb.position);
        date.clear();

        jb.position = 0;
        jb.data = new byte[50];
        date.year(2020);
        date.month(11);
        date.day(2);
        res = BasicPrimitiveConverter.writeDate(date, jb, error);
        for (int i = 0; i < 12; i++) {
            assertEquals("\"2020-11-02\"".charAt(i), (char)jb.data[i]);
        }
        assertEquals(12, jb.position);
        date.clear();

        jb.position = 0;
        jb.data = new byte[50];
        date.year(2020);
        date.month(1);
        date.day(21);
        res = BasicPrimitiveConverter.writeDate(date, jb, error);
        for (int i = 0; i < 12; i++) {
            assertEquals("\"2020-01-21\"".charAt(i), (char)jb.data[i]);
        }
        assertEquals(12, jb.position);
        date.clear();

        jb.position = 0;
        jb.data = new byte[50];
        date.clear();
        res = BasicPrimitiveConverter.writeDate(date, jb, error);
        for (int i = 0; i < 4; i++) {
            assertEquals("null".charAt(i), (char)jb.data[i]);
        }
        assertEquals(4, jb.position);
    }

    @Test
    public void testWriteTime() {

        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[5];
        Time time = CodecFactory.createTime();
        boolean res;

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(23);
        time.minute(255);
        time.second(255);
        time.millisecond(65535);
        time.microsecond(2047);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(4, jb.position);
        for (int i = 0; i < 4; i++) {
            assertEquals("\"23\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[10];
        time.clear();
        time.hour(23);
        time.minute(59);
        time.second(255);
        time.millisecond(65535);
        time.microsecond(2047);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(7, jb.position);
        for (int i = 0; i < 7; i++) {
            assertEquals("\"23:59\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(23);
        time.minute(59);
        time.second(2);
        time.millisecond(65535);
        time.microsecond(2047);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(10, jb.position);
        for (int i = 0; i < 10; i++) {
            assertEquals("\"23:59:02\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(0);
        time.minute(0);
        time.second(2);
        time.millisecond(65535);
        time.microsecond(2047);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(10, jb.position);
        for (int i = 0; i < 10; i++) {
            assertEquals("\"00:00:02\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(0);
        time.minute(0);
        time.second(2);
        time.millisecond(0);
        time.microsecond(2);
        time.nanosecond(500);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(18, jb.position);
        for (int i = 0; i < 18; i++) {
            assertEquals("\"00:00:02.0000025\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(1);
        time.minute(0);
        time.second(2);
        time.millisecond(10);
        time.microsecond(25);
        time.nanosecond(50);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(19, jb.position);
        for (int i = 0; i < 19; i++) {
            assertEquals("\"01:00:02.01002505\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(1);
        time.minute(0);
        time.second(2);
        time.millisecond(0);
        time.microsecond(0);
        time.nanosecond(0);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(10, jb.position);
        for (int i = 0; i < 10; i++) {
            assertEquals("\"01:00:02\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(1);
        time.minute(0);
        time.second(2);
        time.millisecond(0);
        time.microsecond(123);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(17, jb.position);
        for (int i = 0; i < 17; i++) {
            assertEquals("\"01:00:02.000123\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        time.clear();
        time.hour(255);
        time.minute(255);
        time.second(255);
        time.millisecond(65535);
        time.microsecond(2047);
        time.nanosecond(2047);
        res = BasicPrimitiveConverter.writeTime(time, jb, error);
        assertEquals(4, jb.position);
        for (int i = 0; i < 4; i++) {
            assertEquals("null".charAt(i), (char)jb.data[i]);
        }
    }

    @Test
    public void testDateTimeWrite() {

        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[50];
        DateTime dateTime = CodecFactory.createDateTime();
        boolean res;

        jb.position = 0;
        jb.data = new byte[7];
        dateTime.clear();
        dateTime.hour(255);
        dateTime.minute(255);
        dateTime.second(255);
        dateTime.millisecond(65535);
        dateTime.microsecond(2047);
        dateTime.nanosecond(2047);
        dateTime.year(2020);
        dateTime.month(11);
        dateTime.day(2);
        res = BasicPrimitiveConverter.writeDateTime(dateTime, jb, error);
        assertEquals(12, jb.position);
        for (int i = 0; i < 12; i++) {
            assertEquals("\"2020-11-02\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        dateTime.clear();
        dateTime.hour(12);
        dateTime.minute(255);
        dateTime.second(255);
        dateTime.millisecond(65535);
        dateTime.microsecond(2047);
        dateTime.nanosecond(2047);
        dateTime.year(2020);
        dateTime.month(11);
        dateTime.day(2);
        res = BasicPrimitiveConverter.writeDateTime(dateTime, jb, error);
        assertEquals(15, jb.position);
        for (int i = 0; i < 15; i++) {
            assertEquals("\"2020-11-02T12\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[3];
        dateTime.clear();
        dateTime.hour(12);
        dateTime.minute(50);
        dateTime.second(13);
        dateTime.millisecond(0);
        dateTime.microsecond(15);
        dateTime.nanosecond(200);
        dateTime.year(2020);
        dateTime.month(11);
        dateTime.day(2);
        res = BasicPrimitiveConverter.writeDateTime(dateTime, jb, error);
        assertEquals(29, jb.position);
        for (int i = 0; i < 29; i++) {
            assertEquals("\"2020-11-02T12:50:13.0000152\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        dateTime.clear();
        dateTime.hour(12);
        dateTime.minute(50);
        dateTime.second(13);
        dateTime.millisecond(0);
        dateTime.microsecond(15);
        dateTime.nanosecond(200);
        res = BasicPrimitiveConverter.writeDateTime(dateTime, jb, error);
        assertEquals(19, jb.position);
        for (int i = 0; i < 19; i++) {
            assertEquals("\"T12:50:13.0000152\"".charAt(i), (char)jb.data[i]);
        }

        jb.position = 0;
        jb.data = new byte[50];
        dateTime.blank();
        res = BasicPrimitiveConverter.writeDateTime(dateTime, jb, error);
        assertEquals(4, jb.position);
        for (int i = 0; i < 4; i++) {
            assertEquals("null".charAt(i), (char)jb.data[i]);
        }
    }

    @Test
    public void testWriteSafeAndAsciiString() {

        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[4];

        Buffer b = CodecFactory.createBuffer();
        b.data("test\u0003[]\"abcd\\ fsg");
        boolean res = BasicPrimitiveConverter.writeSafeString(b, jb, error);
        assertEquals('\\', jb.data[5]);
        assertEquals('u', jb.data[6]);
        assertEquals('0', jb.data[7]);
        assertEquals('0', jb.data[8]);
        assertEquals('0', jb.data[9]);
        assertEquals('3', jb.data[10]);
        assertEquals('[', jb.data[11]);
        assertEquals(']', jb.data[12]);
        assertEquals('\\', jb.data[13]);
        assertEquals('\"', jb.data[14]);
        assertEquals(jb.position, 26);

        ByteBuffer bb = ByteBuffer.allocate(0);
        jb.position = 0;
        jb.data = new byte[50];
        b.clear();
        b.data(bb);
        res = BasicPrimitiveConverter.writeAsciiString(b, jb, error);
        for (int i = 0; i < 4; i++) {
            assertEquals("null".charAt(i), jb.data[i] & 0xFF);
        }
        assertEquals(4, jb.position);
    }

    @Test
    public void testWriteRTMESString() {

        JsonBuffer jb = new JsonBuffer();
        jb.data = new byte[2];
        Buffer b = CodecFactory.createBuffer();
        b.data("test string [] \" \\ \u0012");
        boolean res = BasicPrimitiveConverter.writeRMTESString(b, jb, error);
        assertEquals('\"', jb.data[0]);
        assertEquals('t', jb.data[1]);
        assertEquals('e', jb.data[2]);
        assertEquals('s', jb.data[3]);
        assertEquals('t', jb.data[4]);
        assertEquals('\\', jb.data[16]);
        assertEquals('\"', jb.data[17]);
        assertEquals('u', jb.data[23]);
        assertEquals(jb.position, 29);

        Buffer b1 = CodecFactory.createBuffer();
        b1.data("");
        jb.position = 0;
        jb.data = new byte[50];
        res = BasicPrimitiveConverter.writeRMTESString(b1, jb, error);
        for (int i = 0; i < 4; i++) {
            assertEquals("null".charAt(i), jb.data[i] & 0xFF);
        }
        assertEquals(4, jb.position);
    }
}
