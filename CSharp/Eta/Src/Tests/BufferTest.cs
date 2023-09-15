/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using Xunit.Categories;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace CodecTestProject
{
    [Category("ByteBuffer")]
    public class BufferTest
    {
        [Fact]
        [Category("Unit")]
        public void EqualsTest()
        {
            LSEG.Eta.Codec.Buffer buf1 = new LSEG.Eta.Codec.Buffer();
            LSEG.Eta.Codec.Buffer buf2 = new LSEG.Eta.Codec.Buffer();

            System.String s1 = "abc";
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

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(s1));
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray1), 0, bArray1.Length));


            Assert.True(buf2.Equals(buf1));
            Assert.True(buf2.Equals(buf2));
            Assert.True(buf1.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray2), 0, bArray2.Length));
            // buf2 length is different than buf1 length.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray3), 0, bArray3.Length));
            // buf2 is lexically greater than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray4), 0, bArray4.Length));
            // buf2 is lexically greater than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray5), 0, bArray5.Length));
            // buf2 is lexically greater than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray6), 0, bArray6.Length));
            // buf2 is lexically less than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray7), 0, bArray7.Length));
            // buf2 is lexically less than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray8), 0, bArray8.Length));
            // buf2 is not equal to buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray9), 0, bArray9.Length));
            // buf2 is lexically less than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray11), 0, bArray11.Length));
            // buf2 is lexically less than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray12), 0, bArray12.Length));
            // buf2 is lexically less than buf1.
            Assert.False(buf2.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray20), 0, bArray20.Length));
            // buf2 is not equal to buf1, case is different.
            Assert.False(buf2.Equals(buf1));

            // changing buf1
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf1.Data(ByteBuffer.Wrap(bArray20), 0, bArray20.Length));
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray21), 0, bArray21.Length));
            // buf2 is not equals to buf1, case is different.
            Assert.False(buf2.Equals(buf1));
        }

        [Fact]
        [Category("Unit")]
        public void BigBufferTest()
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

            LSEG.Eta.Codec.Buffer buf1 = new LSEG.Eta.Codec.Buffer();
            LSEG.Eta.Codec.Buffer buf2 = new LSEG.Eta.Codec.Buffer();

            // compare same buffer
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf1.Data(ByteBuffer.Wrap(bArray), 0, bArray.Length));
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray), 0, bArray.Length));
            Assert.True(buf2.Equals(buf1));

            // compare lower case buffer (b2) to mixed case buffer (b1).
            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArrayL), 0, bArrayL.Length));
            // b2 is lexically greater than b1
            Assert.False(buf2.Equals(buf1));
        }

        [Fact]
        [Category("Unit")]
        public void EqualsWithStringsTest()
        {
            // test RsslBuffers Backed by Strings.

            Buffer buf1 = new Buffer();
            buf1.Data("John Carter");
            Buffer buf2 = new Buffer();

            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data("Tars Tarkas"));
            Assert.False(buf2.Equals(buf1));
            Assert.False(buf1.Equals(buf2));

            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data("Tars A. Tarkas"));
            Assert.False(buf2.Equals(buf1));
            Assert.False(buf1.Equals(buf2));

            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data("John Carter"));
            Assert.True(buf2.Equals(buf1));
            Assert.True(buf1.Equals(buf2));

            Assert.True(buf2.Equals(buf2));
            Assert.True(buf1.Equals(buf1));
        }

        [Fact]
        [Category("Unit")]
        public void EqualsWithDifferentBackingTest()
        {
            // test RsslBuffer backed by ByteBuffer against an RsslBuffer backed by
            // String,
            // both equals and not equals cases.
            Buffer buf1 = new Buffer();
            buf1.Data("Dejah Thoris");
            Buffer buf2 = new Buffer();

            // "Dejah Thoris" equal to buf1
            byte[] bArray1 = new byte[] { 0x44, 0x65, 0x6a, 0x61, 0x68, 0x20, 0x54, 0x68, 0x6f, 0x72,
                0x69, 0x73 };
            // "DejXh Thoris" not equal to buf1
            byte[] bArray2 = new byte[] { 0x44, 0x65, 0x6a, 0x58, 0x68, 0x20, 0x54, 0x68, 0x6f, 0x72,
                0x69, 0x73 };
            // "Dejah A. Thoris" not equal to buf1
            byte[] bArray3 = new byte[] { 0x44, 0x65, 0x6a, 0x61, 0x68, 0x20, 0x51, 0x2E, 0x20, 0x54,
                0x68, 0x6f, 0x72, 0x69, 0x73 };

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray1), 0, bArray1.Length));
            Assert.True(buf2.Equals(buf1));
            Assert.True(buf1.Equals(buf2));
            Assert.True(buf2.Equals(buf2));
            Assert.True(buf1.Equals(buf1));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray2), 0, bArray2.Length));
            Assert.False(buf2.Equals(buf1));
            Assert.False(buf1.Equals(buf2));

            Assert.Equal(CodecReturnCode.SUCCESS,
                         buf2.Data(ByteBuffer.Wrap(bArray3), 0, bArray3.Length));
            Assert.False(buf2.Equals(buf1));
            Assert.False(buf1.Equals(buf2));
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferIntoNullByteBufferFails()
        {
            Buffer buffer = new Buffer();

            CodecReturnCode actual = buffer.Copy((ByteBuffer)null);

            // test with null destBuffer
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferEmptyIntoByteBufferOk()
        {
            Buffer buffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(10);

            CodecReturnCode actual = buffer.Copy(byteBuffer);

            // test with null destBuffer
            Assert.Equal(CodecReturnCode.SUCCESS, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferNoLengthIntoByteBufferOk()
        {
            Buffer buffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(10);

            CodecReturnCode actual = buffer.Data(byteBuffer, 0, 0);

            // test with null destBuffer
            Assert.Equal(CodecReturnCode.SUCCESS, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferIntoByteBufferOk()
        {
            long expectedValue = 123456798765432;
            Buffer buffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(10);
            byteBuffer.Write(expectedValue);

            buffer.Data(byteBuffer, 0, 8);

            ByteBuffer byteBufferOut = new ByteBuffer(10);
            buffer.Copy(byteBufferOut);

            long actualValue = byteBufferOut.ReadLong();
            Assert.Equal(expectedValue, actualValue);
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferIntoByteBufferTooSmall()
        {
            Buffer buffer = new Buffer();
            ByteBuffer byteBuffer = new ByteBuffer(10);
            buffer.Data(byteBuffer);

            byteBuffer.Write(123456798765432L);

            ByteBuffer byteBufferOut = new ByteBuffer(3);

            var actual = buffer.Copy(byteBufferOut);
            Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyBufferFromString()
        {
            Buffer buffer = new Buffer();
            var actual = buffer.Data("Abcdefghij");
            Assert.Equal(CodecReturnCode.SUCCESS, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyToByteBufferOk()
        {
            ByteBuffer byteBuffer = new ByteBuffer(10);

            // Attach new Buffer to the ByteBuffer
            Buffer buffer = new Buffer();
            buffer.Data(byteBuffer);

            long expectedLong = 123456798765432L;
            short expectedShort = -127;

            byteBuffer.Write(expectedLong);
            byteBuffer.Write(expectedShort);

            // Copy the Buffer into the new ByteBuffer
            ByteBuffer byteBufferOut = new ByteBuffer(10);
            Assert.Equal(CodecReturnCode.SUCCESS, buffer.Copy(byteBufferOut));

            byteBufferOut.Flip();
            Assert.Equal(expectedLong, byteBufferOut.ReadLong());
            Assert.Equal(expectedShort, byteBufferOut.ReadShort());
        }

        [Fact]
        [Category("Unit")]
        public void CopyByteArrayTest()
        {
            Buffer buf1 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(10);
            byte[] ba = new byte[10];

            // test with no backing data in RsslBuffer.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba));

            // test copy with ByteBuffer as backing.
            bb1.Write(0x0123456798765432L);
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba));
            Assert.Equal((byte)0x01, ba[0]);
            Assert.Equal((byte)0x23, ba[1]);
            Assert.Equal((byte)0x45, ba[2]);
            Assert.Equal((byte)0x67, ba[3]);
            Assert.Equal((byte)0x98, ba[4]);
            Assert.Equal((byte)0x76, ba[5]);
            Assert.Equal((byte)0x54, ba[6]);
            Assert.Equal((byte)0x32, ba[7]);

            // test with null byte[] destBuffer
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Copy((byte[])null));

            // test with a dest byte[] that is too small.
            byte[] baSmall = new byte[5];
            Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, buf1.Copy(baSmall));

            // test with backing length = 0.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 0));

            // test copy with String as backing.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data("Abcdefgh"));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba));
            Assert.Equal(0x41, ba[0]);
            Assert.Equal(0x62, ba[1]);
            Assert.Equal(0x63, ba[2]);
            Assert.Equal(0x64, ba[3]);
            Assert.Equal(0x65, ba[4]);
            Assert.Equal(0x66, ba[5]);
            Assert.Equal(0x67, ba[6]);
            Assert.Equal(0x68, ba[7]);
        }

        [Fact]
        [Category("Unit")]
        public void CopyByteArrayWithOffsetTest()
        {
            Buffer buf1 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(10);
            byte[] ba = new byte[10];

            // test with no backing data in RsslBuffer.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba));

            // test copy with ByteBuffer as backing.
            bb1.Write(0x0123456798765432L);
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba, 2));
            Assert.Equal((byte)0x01, ba[2]);
            Assert.Equal((byte)0x23, ba[3]);
            Assert.Equal((byte)0x45, ba[4]);
            Assert.Equal((byte)0x67, ba[5]);
            Assert.Equal((byte)0x98, ba[6]);
            Assert.Equal((byte)0x76, ba[7]);
            Assert.Equal((byte)0x54, ba[8]);
            Assert.Equal((byte)0x32, ba[9]);

            // test with null byte[] destBuffer
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Copy((byte[])null));

            // test with a dest byte[] that is too small.
            byte[] baSmall = new byte[5];
            Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, buf1.Copy(baSmall));

            // test with backing length = 0.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 0));

            // test copy with String as backing.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data("Abcdefgh"));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(ba, 2));
            Assert.Equal(0x41, ba[2]);
            Assert.Equal(0x62, ba[3]);
            Assert.Equal(0x63, ba[4]);
            Assert.Equal(0x64, ba[5]);
            Assert.Equal(0x65, ba[6]);
            Assert.Equal(0x66, ba[7]);
            Assert.Equal(0x67, ba[8]);
            Assert.Equal(0x68, ba[9]);
        }

        [Fact]
        [Category("Unit")]
        public void CopyRsslBufferTest()
        {
            Buffer buf1 = new Buffer();
            Buffer buf2 = new Buffer();
            Buffer buf3 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(10);
            ByteBuffer bb2 = new ByteBuffer(10);
            ByteBuffer bb3 = new ByteBuffer(5);

            // test with no backing data in source RsslBuffer.
            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data(bb2));
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(buf2));

            // test copy with RsslBuffer as backing.
            bb1.Write(0123456798765432);
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(buf2));
            Assert.Equal(0123456798765432, bb2.ReadLong());

            // test with no backing data in destination RsslBuffer.
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Copy(buf3));

            // test with null destBuffer
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Copy((Buffer)null));

            // test with a destination RsslBuffer that is too small.
            Assert.Equal(CodecReturnCode.SUCCESS, buf3.Data(bb3));
            Assert.Equal(CodecReturnCode.BUFFER_TOO_SMALL, buf1.Copy(buf3));

            // test with source backing length = 0.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 0));

            // test with destination backing length = 0;
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));
            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data(bb2, 0, 0));

            // test copy with source with String as backing.
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data("Abcdefghij"));

            // reset it
            bb2.Clear();
            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data(bb2, 0, 10));
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Copy(buf2));
            // need to use position and length from RsslBuffer
            ByteBuffer bTmp = buf2.Data();
            bTmp.ReadPosition = buf2.Position;
            bTmp.Reserve(buf2.Length + buf2.Position);
            Assert.Equal<long>(System.Net.IPAddress.NetworkToHostOrder(System.BitConverter.ToInt64(bTmp.Contents, 0)), bTmp.ReadLong());
            Assert.Equal<short>(System.Net.IPAddress.NetworkToHostOrder(System.BitConverter.ToInt16(bTmp.Contents, 8)), bTmp.ReadShort());

            // test with destination with String as backing.
            Assert.Equal(CodecReturnCode.SUCCESS, buf2.Data("defghijk"));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Copy(buf2));

            Buffer buf4 = new Buffer();
            Buffer buf5 = new Buffer();
            ByteBuffer bb4 = new ByteBuffer(10);
            ByteBuffer bb5 = new ByteBuffer(10);
            Assert.Equal(CodecReturnCode.SUCCESS, buf4.Data(bb4));
            Assert.Equal(CodecReturnCode.SUCCESS, buf5.Data(bb5));
            bb4.Write(0x0123456798765432L);
            Assert.Equal(CodecReturnCode.SUCCESS, buf4.Copy(buf5));
            Assert.Equal(0x0123456798765432L, bb5.ReadLong());

        }

        [Fact]
        [Category("Unit")]
        public void SetDataTest()
        {
            Buffer buf1 = new Buffer();
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Data((ByteBuffer)null));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Data((ByteBuffer)null, 0, 0));
            Assert.Equal(CodecReturnCode.INVALID_ARGUMENT, buf1.Data((string)null));
        }

        [Fact]
        [Category("Unit")]
        public void GetDataBackedByString()
        {
            ByteBuffer expected = new ByteBuffer(13);


            // ApplicationID          
            expected.Write((byte)0x41);
            expected.Write((byte)0x70);
            expected.Write((byte)0x70);
            expected.Write((byte)0x6c);
            expected.Write((byte)0x69);
            expected.Write((byte)0x63);
            expected.Write((byte)0x61);
            expected.Write((byte)0x74);
            expected.Write((byte)0x69);
            expected.Write((byte)0x6f);
            expected.Write((byte)0x6e);
            expected.Write((byte)0x49);
            expected.Write((byte)0x44);
            expected.Flip();

            Buffer buf1 = new Buffer();
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data("ApplicationID"));
            ByteBuffer actual = buf1.Data();


            Assert.True(expected.Equals(actual));
        }

        [Fact]
        [Category("Unit")]
        public void ToHexStringTest()
        {
            string expectedString = "Abcdefgh";
            string expectedHexString = "0000: 41 62 63 64 65 66 67 68                            Abcdefgh";
            string expectedLong =
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

            Buffer buf1 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(1024);
            bb1.Write(System.Net.IPAddress.HostToNetworkOrder(7523094288207667777));
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));
            string hs = buf1.ToHexString();
            Assert.Equal(expectedHexString, hs);

            int max = 257;
            for (int i = 0; i < max; i++)
                bb1.WriteAt(i, (byte)(i % 256));

            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, max));
            string hsLong = buf1.ToHexString();
            Assert.Equal(expectedLong, hsLong);

            //// test with backing buffer as a string
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(expectedString));
            string actual = buf1.ToHexString();
            Assert.Equal(expectedHexString, actual);
        }

        [Fact]
        [Category("Unit")]
        public void CopyReferenceTest()
        {
            Buffer buf1 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(8);
            Assert.Equal(CodecReturnCode.SUCCESS, buf1.Data(bb1, 0, 8));

            Buffer buf2 = new Buffer();
            buf2.CopyReferences(buf1);

            Assert.Equal<ByteBuffer>(bb1, buf2.Data());
            Assert.Equal<int>(0, buf2.Position);
            Assert.Equal<int>(8, buf2.Length);
            Assert.Equal((string)null, buf2.DataString());

            // test backed with string
            string expectedString = "Test String";
            var encoder = new System.Text.ASCIIEncoding();
            ByteBuffer bbExpected = new ByteBuffer(encoder.GetBytes(expectedString));

            buf1.Data(expectedString);
            buf2.CopyReferences(buf1);

            Assert.True(bbExpected.Equals(buf2.Data()));
            Assert.Equal(0, buf2.Position);
            Assert.Equal(11, buf2.Length);
            Assert.Equal(expectedString, buf2.DataString());
        }
        /**
         * This tests the capacity() method.
         */
        [Fact]
        [Category("Unit")]
        public void GetCapacityTest()
        {
            Buffer buf1 = new Buffer();
            ByteBuffer bb1 = new ByteBuffer(8);
            buf1.Data(bb1);

            //capacity = byte buffer size
            Assert.Equal(8, buf1.Capacity);

            ByteBuffer backingByteBuffer = buf1.Data();
            for (int i = 0; i < 5; i++)
            {
                backingByteBuffer.Write((byte)(i % 256)); // 0-255
            }

            //capacity remains the same
            Assert.Equal(8, buf1.Capacity);
        }
    }
}
