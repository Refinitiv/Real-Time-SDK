///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.codec;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.BufferImpl;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;

public class BufferJunit
{
    @BeforeClass
    public static void setRunningInJunits()
    {
        BufferImpl._runningInJunits = true;
    }

    @AfterClass
    public static void clearRunningInJunits()
    {
        BufferImpl._runningInJunits = false;
    }
    
    @Test
    public void equalsTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        Buffer buf2 = CodecFactory.createBuffer();

        String s1 = "abc";
        // equal to s1
        byte[] bArray1 = new byte[] { 0x61, 0x62, 0x63 };
        // not equal to s1, since length is different.
        byte[] bArray2 = new byte[] { 0x61, 0x62, 0x63, 0x00 };
        // lexically greater than s1
        byte[] bArray3 = new byte[] { 0x61, 0x62, 0x63, 0x64, 0x00 };
        // lexically greater than s1
        byte[] bArray4 = new byte[] { 0x70, 0x61, 0x74, 0x00 };
        // lexically greater than s1
        byte[] bArray5 = new byte[] { 0x70, 0x61, 0x74, 0x72, 0x00 };
        // lexically less than s1
        byte[] bArray6 = new byte[] { 0x61, 0x61, 0x61, 0x00 };
        // lexically less than s1
        byte[] bArray7 = new byte[] { 0x61, 0x61, 0x61, 0x61, 0x00 };
        // not equal to s1
        byte[] bArray8 = new byte[] { 0x61, 0x62, 0x63, 0x00, 0x64 };
        // less than s1
        byte[] bArray9 = new byte[] { 0x41, 0x42, 0x43 };
        // lexically less that s1
        byte[] bArray11 = new byte[] { 0x41, 0x42, 0x43, 0x44, 0x00 };
        // lexically less that s1
        byte[] bArray12 = new byte[] { 0x50, 0x41, 0x54, 0x00 };
        // not equal to s1, case is different.
        byte[] bArray20 = new byte[] { 0x61, 0x62, 0x63, 0x00, 0x00 };
        // not equal to bArray9, case is different
        byte[] bArray21 = new byte[] { 0x61, 0x62, 0x63, 0x00, 0x00, 0x00 };

        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(s1));
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray1), 0, bArray1.length));
        assertTrue(buf2.equals(buf1));
        assertTrue(buf2.equals(buf2));
        assertTrue(buf1.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray2), 0, bArray2.length));
        // buf2 length is different than buf1 length.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray3), 0, bArray3.length));
        // buf2 is lexically greater than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray4), 0, bArray4.length));
        // buf2 is lexically greater than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray5), 0, bArray5.length));
        // buf2 is lexically greater than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray6), 0, bArray6.length));
        // buf2 is lexically less than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray7), 0, bArray7.length));
        // buf2 is lexically less than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray8), 0, bArray8.length));
        // buf2 is not equal to buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray9), 0, bArray9.length));
        // buf2 is lexically less than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray11), 0, bArray11.length));
        // buf2 is lexically less than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray12), 0, bArray12.length));
        // buf2 is lexically less than buf1.
        assertFalse(buf2.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray20), 0, bArray20.length));
        // buf2 is not equal to buf1, case is different.
        assertFalse(buf2.equals(buf1));

        // changing buf1
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf1.data(ByteBuffer.wrap(bArray20), 0, bArray20.length));
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray21), 0, bArray21.length));
        // buf2 is not equals to buf1, case is different.
        assertFalse(buf2.equals(buf1));
    }

    @Test
    public void bigBufferTest()
    {
        int len = 1024 * 1000; // 1M
        byte[] bArray = new byte[len];
        byte[] bArrayL = new byte[len];

        // load a byte array with a large amount of data.
        byte b = 0x20; // space char
        for (int i = 0; i < len; i++)
        {
            bArray[i] = b;
            bArrayL[i] = (byte)((b > 0x40 && b < 0x5b) ? b + 0x20 : b); // lowercase
                                                                        // only
            if (++b > 0x7e)
                b = 0x20;
        }

        Buffer buf1 = CodecFactory.createBuffer();
        Buffer buf2 = CodecFactory.createBuffer();

        // compare same buffer
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf1.data(ByteBuffer.wrap(bArray), 0, bArray.length));
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray), 0, bArray.length));
        assertTrue(buf2.equals(buf1));

        // compare lower case buffer (b2) to mixed case buffer (b1).
        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArrayL), 0, bArrayL.length));
        // b2 is lexically greater than b1
        assertFalse(buf2.equals(buf1));
    }

    @Test
    public void equalsWithStringsTest()
    {
        // test RsslBuffers Backed by Strings.

        Buffer buf1 = CodecFactory.createBuffer();
        buf1.data("John Carter");
        Buffer buf2 = CodecFactory.createBuffer();

        assertEquals(CodecReturnCodes.SUCCESS, buf2.data("Tars Tarkas"));
        assertFalse(buf2.equals(buf1));
        assertFalse(buf1.equals(buf2));

        assertEquals(CodecReturnCodes.SUCCESS, buf2.data("Tars A. Tarkas"));
        assertFalse(buf2.equals(buf1));
        assertFalse(buf1.equals(buf2));

        assertEquals(CodecReturnCodes.SUCCESS, buf2.data("John Carter"));
        assertTrue(buf2.equals(buf1));
        assertTrue(buf1.equals(buf2));

        assertTrue(buf2.equals(buf2));
        assertTrue(buf1.equals(buf1));
    }

    @Test
    public void equalsWithDifferentBackingTest()
    {
        // test RsslBuffer backed by ByteBuffer against an RsslBuffer backed by
        // String,
        // both equals and not equals cases.
        Buffer buf1 = CodecFactory.createBuffer();
        buf1.data("Dejah Thoris");
        Buffer buf2 = CodecFactory.createBuffer();

        // "Dejah Thoris" equal to buf1
        byte[] bArray1 = new byte[] { 0x44, 0x65, 0x6a, 0x61, 0x68, 0x20, 0x54, 0x68, 0x6f, 0x72,
                0x69, 0x73 };
        // "DejXh Thoris" not equal to buf1
        byte[] bArray2 = new byte[] { 0x44, 0x65, 0x6a, 0x58, 0x68, 0x20, 0x54, 0x68, 0x6f, 0x72,
                0x69, 0x73 };
        // "Dejah A. Thoris" not equal to buf1
        byte[] bArray3 = new byte[] { 0x44, 0x65, 0x6a, 0x61, 0x68, 0x20, 0x51, 0x2E, 0x20, 0x54,
                0x68, 0x6f, 0x72, 0x69, 0x73 };

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray1), 0, bArray1.length));
        assertTrue(buf2.equals(buf1));
        assertTrue(buf1.equals(buf2));
        assertTrue(buf2.equals(buf2));
        assertTrue(buf1.equals(buf1));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray2), 0, bArray2.length));
        assertFalse(buf2.equals(buf1));
        assertFalse(buf1.equals(buf2));

        assertEquals(CodecReturnCodes.SUCCESS,
                     buf2.data(ByteBuffer.wrap(bArray3), 0, bArray3.length));
        assertFalse(buf2.equals(buf1));
        assertFalse(buf1.equals(buf2));
    }

    @Test
    public void copyByteBufferTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(10);
        ByteBuffer bb2 = ByteBuffer.allocate(10);

        // test with no backing data in RsslBuffer.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(bb2));

        // test copy with ByteBuffer as backing.
        bb1.putLong(0x0123456798765432L);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(bb2));
        assertEquals(0x0123456798765432L, bb2.getLong());

        // test with null destBuffer
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy((ByteBuffer)null));

        // test with a dest ByteBuffer that is too small.
        int origLimit = bb2.limit();
        bb2.limit(5);
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, buf1.copy(bb2));
        bb2.limit(origLimit);

        // test with backing length = 0.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 0));

        // test copy with String as backing.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data("Abcdefghij"));

        bb2.position(0);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(bb2));
        assertEquals(0x4162636465666768L, bb2.getLong());
        assertEquals(0x696a, bb2.getShort());
    }

    @Test
    public void copyByteArrayTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(10);
        byte[] ba = new byte[10];

        // test with no backing data in RsslBuffer.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba));

        // test copy with ByteBuffer as backing.
        bb1.putLong(0x0123456798765432L);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba));
        assertEquals((byte)0x01, ba[0]);
        assertEquals((byte)0x23, ba[1]);
        assertEquals((byte)0x45, ba[2]);
        assertEquals((byte)0x67, ba[3]);
        assertEquals((byte)0x98, ba[4]);
        assertEquals((byte)0x76, ba[5]);
        assertEquals((byte)0x54, ba[6]);
        assertEquals((byte)0x32, ba[7]);

        // test with null byte[] destBuffer
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy((byte[])null));

        // test with a dest byte[] that is too small.
        byte[] baSmall = new byte[5];
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, buf1.copy(baSmall));

        // test with backing length = 0.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 0));

        // test copy with String as backing.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data("Abcdefgh"));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba));
        assertEquals(0x41, ba[0]);
        assertEquals(0x62, ba[1]);
        assertEquals(0x63, ba[2]);
        assertEquals(0x64, ba[3]);
        assertEquals(0x65, ba[4]);
        assertEquals(0x66, ba[5]);
        assertEquals(0x67, ba[6]);
        assertEquals(0x68, ba[7]);
    }

    @Test
    public void copyByteArrayWithOffsetTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(10);
        byte[] ba = new byte[10];

        // test with no backing data in RsslBuffer.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba));

        // test copy with ByteBuffer as backing.
        bb1.putLong(0x0123456798765432L);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba, 2));
        assertEquals((byte)0x01, ba[2]);
        assertEquals((byte)0x23, ba[3]);
        assertEquals((byte)0x45, ba[4]);
        assertEquals((byte)0x67, ba[5]);
        assertEquals((byte)0x98, ba[6]);
        assertEquals((byte)0x76, ba[7]);
        assertEquals((byte)0x54, ba[8]);
        assertEquals((byte)0x32, ba[9]);

        // test with null byte[] destBuffer
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy((byte[])null));

        // test with a dest byte[] that is too small.
        byte[] baSmall = new byte[5];
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, buf1.copy(baSmall));

        // test with backing length = 0.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 0));

        // test copy with String as backing.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data("Abcdefgh"));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(ba, 2));
        assertEquals(0x41, ba[2]);
        assertEquals(0x62, ba[3]);
        assertEquals(0x63, ba[4]);
        assertEquals(0x64, ba[5]);
        assertEquals(0x65, ba[6]);
        assertEquals(0x66, ba[7]);
        assertEquals(0x67, ba[8]);
        assertEquals(0x68, ba[9]);
    }

    @Test
    public void copyRsslBufferTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        Buffer buf2 = CodecFactory.createBuffer();
        Buffer buf3 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(10);
        ByteBuffer bb2 = ByteBuffer.allocate(10);
        ByteBuffer bb3 = ByteBuffer.allocate(5);

        // test with no backing data in source RsslBuffer.
        assertEquals(CodecReturnCodes.SUCCESS, buf2.data(bb2));
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(buf2));

        // test copy with RsslBuffer as backing.
        bb1.putLong(0x0123456798765432L);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));

        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(buf2));
        assertEquals(0x0123456798765432L, bb2.getLong());

        // test with no backing data in destination RsslBuffer.
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy(buf3));

        // test with null destBuffer
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy((Buffer)null));

        // test with a destination RsslBuffer that is too small.
        assertEquals(CodecReturnCodes.SUCCESS, buf3.data(bb3));
        assertEquals(CodecReturnCodes.BUFFER_TOO_SMALL, buf1.copy(buf3));

        // test with source backing length = 0.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 0));

        // test with destination backing length = 0;
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));
        assertEquals(CodecReturnCodes.SUCCESS, buf2.data(bb2, 0, 0));
        
        // test copy with source with String as backing.
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data("Abcdefghij"));

        // reset it
        bb2.clear();
        assertEquals(CodecReturnCodes.SUCCESS, buf2.data(bb2, 0, 10));
        assertEquals(CodecReturnCodes.SUCCESS, buf1.copy(buf2));
        // need to use position and length from RsslBuffer
        ByteBuffer bTmp = buf2.data().asReadOnlyBuffer();
        bTmp.position(buf2.position());
        bTmp.limit(buf2.length() + buf2.position());
        assertEquals(0x4162636465666768L, bTmp.getLong());
        assertEquals(0x696a, bTmp.getShort());
        
        // test with destination with String as backing.
        assertEquals(CodecReturnCodes.SUCCESS, buf2.data("defghijk"));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.copy(buf2));
        
        Buffer buf4 = CodecFactory.createBuffer();
        Buffer buf5 = CodecFactory.createBuffer();
        ByteBuffer bb4 = ByteBuffer.allocate(10);
        ByteBuffer bb5 = ByteBuffer.allocate(10);
        assertEquals(CodecReturnCodes.SUCCESS, buf4.data(bb4));
        assertEquals(CodecReturnCodes.SUCCESS, buf5.data(bb5));
        bb4.putLong(0x0123456798765432L);
        assertEquals(CodecReturnCodes.SUCCESS, buf4.copy(buf5));
        assertEquals(0x0123456798765432L, bb5.getLong());
                
    }
    
    @Test
    public void setDataTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.data((ByteBuffer)null));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.data((ByteBuffer)null, 0, 0));
        assertEquals(CodecReturnCodes.INVALID_ARGUMENT, buf1.data((String)null));
    }
    
    @Test
    public void getDataBackedByString()
    {
        ByteBuffer expected = ByteBuffer.allocate(13);
        // ApplicationID
        expected.put(0, (byte)0x41);
        expected.put(1, (byte)0x70);
        expected.put(2, (byte)0x70);
        expected.put(3, (byte)0x6c);
        expected.put(4, (byte)0x69);
        expected.put(5, (byte)0x63);
        expected.put(6, (byte)0x61);
        expected.put(7, (byte)0x74);
        expected.put(8, (byte)0x69);
        expected.put(9, (byte)0x6f);
        expected.put(10, (byte)0x6e);
        expected.put(11, (byte)0x49);
        expected.put(12, (byte)0x44);
                
        Buffer buf1 = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data("ApplicationID"));
        ByteBuffer actual = buf1.data();
        
        assertEquals(expected, actual);
    }
    
    @Test
    public void toHexStringTest()
    {
        String expectedString = "Abcdefgh";
        String expectedHexString = "0000: 41 62 63 64 65 66 67 68                            Abcdefgh";
        String expectedLong =
                  "0000: 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F   ................\n"
                + "0001: 10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F   ................\n"
                + "0002: 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F    !\"#$%&'()*+,-./\n"
                + "0003: 30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F   0123456789:;<=>?\n"
                + "0004: 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F   @ABCDEFGHIJKLMNO\n"
                + "0005: 50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F   PQRSTUVWXYZ[\\]^_\n"
                + "0006: 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F   `abcdefghijklmno\n"
                + "0007: 70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F   pqrstuvwxyz{|}~.\n"
                + "0008: 80 81 82 83 84 85 86 87  88 89 8A 8B 8C 8D 8E 8F   ................\n"
                + "0009: 90 91 92 93 94 95 96 97  98 99 9A 9B 9C 9D 9E 9F   ................\n"
                + "000A: A0 A1 A2 A3 A4 A5 A6 A7  A8 A9 AA AB AC AD AE AF   ................\n"
                + "000B: B0 B1 B2 B3 B4 B5 B6 B7  B8 B9 BA BB BC BD BE BF   ................\n"
                + "000C: C0 C1 C2 C3 C4 C5 C6 C7  C8 C9 CA CB CC CD CE CF   ................\n"
                + "000D: D0 D1 D2 D3 D4 D5 D6 D7  D8 D9 DA DB DC DD DE DF   ................\n"
                + "000E: E0 E1 E2 E3 E4 E5 E6 E7  E8 E9 EA EB EC ED EE EF   ................\n"
                + "000F: F0 F1 F2 F3 F4 F5 F6 F7  F8 F9 FA FB FC FD FE FF   ................\n"
                + "0010: 00                                                 .";

        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(1024);
        bb1.putLong(0x4162636465666768L);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));
        String hs = buf1.toHexString();
        //System.out.println(hs);
        System.out.println(hs);
        assertEquals(expectedHexString, hs);        
        
        int max = 257;
        for(int i = 0; i < max; i++)
            bb1.put(i, (byte)(i%256));
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, max));
        String hsLong = buf1.toHexString();
        System.out.println(hsLong);
        assertEquals(expectedLong, hsLong);
        
        // test with backing buffer as a string
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(expectedString));
        String actual = buf1.toHexString();
        assertEquals(expectedHexString, actual);
    }
    
    @Test
    public void toStringTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        String expected = "username";
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(expected));
        String actual = buf1.toString();
        assertEquals(expected, actual);
        
        ByteBuffer bb1 = ByteBuffer.allocate(8);
        // username
        bb1.put(0, (byte)0x75);
        bb1.put(1, (byte)0x73);
        bb1.put(2, (byte)0x65);
        bb1.put(3, (byte)0x72);
        bb1.put(4, (byte)0x6e);
        bb1.put(5, (byte)0x61);
        bb1.put(6, (byte)0x6d);
        bb1.put(7, (byte)0x65);
        
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1));
        actual = buf1.toString();
        assertEquals(expected, actual);
    }
    
    @Test
    public void copyReferenceTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(8);
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(bb1, 0, 8));
        
        Buffer buf2 = CodecFactory.createBuffer();
        ((BufferImpl)buf2).copyReferences(buf1);
        
        assertEquals(bb1, buf2.data());
        assertEquals(0, buf2.position());
        assertEquals(8, buf2.length());
        assertEquals((String)null, ((BufferImpl)buf2).dataString());
        
        // test backed with string
        String expectedString = "Test String";
        ByteBuffer bbExpected = ByteBuffer.allocate(11);
        bbExpected.put(0, (byte)0x54);
        bbExpected.put(1, (byte)0x65);
        bbExpected.put(2, (byte)0x73);
        bbExpected.put(3, (byte)0x74);
        bbExpected.put(4, (byte)0x20);
        bbExpected.put(5, (byte)0x53);
        bbExpected.put(6, (byte)0x74);
        bbExpected.put(7, (byte)0x72);
        bbExpected.put(8, (byte)0x69);
        bbExpected.put(9, (byte)0x6e);
        bbExpected.put(10, (byte)0x67);
        
        assertEquals(CodecReturnCodes.SUCCESS, buf1.data(expectedString));
        ((BufferImpl)buf2).copyReferences(buf1);
        
        assertEquals(bbExpected, buf2.data());
        assertEquals(0, buf2.position());
        assertEquals(11, buf2.length());
        assertEquals(expectedString, ((BufferImpl)buf2).dataString());
    }
    /**
     * This tests the capacity() method.
     */
    @Test
    public void getCapacityTest()
    {
        Buffer buf1 = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(8);
        buf1.data(bb1);
        
        //capacity = byte buffer size
        assertEquals(buf1.capacity(), 8);
       
        ByteBuffer backingByteBuffer = buf1.data();
        for (int i = 0; i < 5; i++)
        {
            backingByteBuffer.put((byte)(i % 256)); // 0-255
        }
      
        //capacity remains the same
        assertEquals(buf1.capacity(), 8); 
        
        //capacity after new limit
        backingByteBuffer.limit(6);
        assertEquals(buf1.capacity(), 6);
    }
}
