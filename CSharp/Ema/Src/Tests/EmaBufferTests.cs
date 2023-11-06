/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System.Linq;
using System.Text;

namespace LSEG.Ema.Access.Tests;

public class EmaBufferTests
{
    [Fact]
    public void EmaBuffer1_Test()
    {
        EmaBuffer buff = new EmaBuffer();
        Assert.Equal(0, buff.Length);

        buff.Append(1);
        Assert.Equal(1, buff.Length);

        int len = 10;

        for (byte i = 0; i < len; i++)
        {
            buff.Append(i);
        }

        Assert.Equal(len + 1, buff.Length);

        buff.Clear();
        Assert.Equal(0, buff.Length);
    }

    [Fact]
    public void EmaBuffer2_Test()
    {
        byte[] arr = { 0, 1, 2, 3 };
        EmaBuffer buff = new EmaBuffer(arr);

        Assert.Equal(arr.Length, buff.Length);

        buff.Append((byte)4);
        Assert.Equal(arr.Length + 1, buff.Length);

        buff.Append(arr);
        Assert.Equal(arr.Length + arr.Length + 1, buff.Length);

        // copy constructor
        EmaBuffer buff2 = new EmaBuffer(buff);
        Assert.Equal(buff2.Length, buff.Length);

        Assert.True(buff2.Equals(buff));

        // equality
        buff.Clear();
        buff2.Clear();

        // a buffer equals itself
        Assert.True(buff.Equals(buff));

        // empty buffers are equal too
        Assert.True(buff2.Equals(buff));

        buff.Append(arr);
        buff2.CopyFrom(buff.Buffer);
        Assert.True(buff2.Equals(buff));
        Assert.True(buff.Equals(buff2));

        buff[0] = 100;
        Assert.False(buff.Equals(arr));
        Assert.False(buff2.Equals(buff));
        Assert.False(buff.Equals(buff2));

        int sum = buff.Buffer.ToArray().Sum(x => x);
        int sum2 = buff2.Buffer.ToArray().Sum(x => x);

        Assert.Equal(sum, sum2 + 100);
    }

    [Fact]
    public void EmaBuffer3_Test()
    {
        EmaBuffer emaBuff = new EmaBuffer(Encoding.ASCII.GetBytes("ABC"));

        ByteBuffer emaByteBuffer = new ByteBuffer(emaBuff.AsByteArray());
        emaByteBuffer.Flip();
        Assert.Equal(3, emaByteBuffer.Limit);

        byte[] rawBytes = emaByteBuffer.Contents;
        Assert.Equal((byte)'A', rawBytes[0]);
        Assert.Equal((byte)'B', rawBytes[1]);
        Assert.Equal((byte)'C', rawBytes[2]);

        Eta.Codec.Buffer etaBuff = new();

        Assert.Equal(CodecReturnCode.SUCCESS, etaBuff.Data(emaByteBuffer));

        Assert.Equal(3, etaBuff.GetLength());

        Assert.Equal("ABC", etaBuff.ToString());
    }

    [Fact]
    public void EmaBufferAssign_Test()
    {
        byte[] arr = { 1, 2, 3, 4, 5 };
        EmaBuffer emaBuf = new EmaBuffer();

        int offset = 1;
        int len = 3;

        // EmaBuffer is turned into a span-like window into an internal buffer
        // at the specified offset and of the specified length
        emaBuf.AssignFrom(arr, offset, len);

        Assert.Equal(len, emaBuf.Length);
        Assert.Equal(arr[offset], emaBuf[0]);
        Assert.Equal(arr[offset + len - 1], emaBuf[^1]);

        // this buffer will copy bytes into internal array
        EmaBuffer emaBufSpan = new EmaBuffer(new System.Span<byte>(arr, offset, len));

        Assert.Equal(emaBufSpan, emaBuf);

        // modify internal buffer
        arr[offset]++;

        // internal buffer backing emaBuf was modified,
        // buffers are no longer Equal
        Assert.NotEqual(emaBufSpan, emaBuf);

        byte[] copy = emaBuf.AsByteArray();

        Assert.Equal(emaBuf.Length, copy.Length);
        Assert.Equal(emaBuf[^1], copy[^1]);

        emaBuf[0]--;

        // returned byte array is indeed a copy of the internal buffer
        Assert.NotEqual(emaBuf[0], copy[0]);
    }

    [Fact]
    public void EmaBufferArray_Test()
    {
        byte[] arr = { 1, 2, 3, 4, 5 };
        EmaBuffer emaBuf = new EmaBuffer(arr, 0, arr.Length);
        Assert.Equal(arr.Length, emaBuf.Length);

        Assert.Equal(arr[0], emaBuf[0]);
        Assert.Equal(arr[1], emaBuf[1]);
        Assert.Equal(arr[arr.Length - 1], emaBuf[emaBuf.Length - 1]);

        emaBuf.CopyFrom(arr, 2, arr.Length - 2);

        Assert.Equal(arr.Length - 2, emaBuf.Length);
        Assert.Equal(arr[2], emaBuf[0]);
    }

    [Fact]
    public void EmaBufferU16Array_Test()
    {
        char[] arr = { '1', '2', '3', '4', '5' };
        EmaBufferU16 emaBuf = new EmaBufferU16(arr, 0, arr.Length);
        Assert.Equal(arr.Length, emaBuf.Length);

        Assert.Equal(arr[0], emaBuf[0]);
        Assert.Equal(arr[1], emaBuf[1]);
        Assert.Equal(arr[arr.Length - 1], emaBuf[emaBuf.Length - 1]);

        emaBuf.CopyFrom(arr, 2, arr.Length - 2);

        Assert.Equal(arr.Length - 2, emaBuf.Length);
        Assert.Equal(arr[2], emaBuf[0]);
    }

    [Fact]
    public void EmaBufferAppend_Test()
    {
        EmaBuffer emaBuf = new();

        Assert.Equal(0, emaBuf.Length);

        // append single byte
        emaBuf.Append(0);

        Assert.Equal(1, emaBuf.Length);
        Assert.Equal(new byte[] { 0 }, emaBuf.AsByteArray());

        // apend several bytes as a span
        emaBuf.Append(new byte[] { 1, 2, 3 });
        Assert.Equal(4, emaBuf.Length);
        Assert.Equal(new byte[] { 0, 1, 2, 3 }, emaBuf.AsByteArray());

        EmaBuffer emaBuf2 = new(emaBuf);
        Assert.Equal(emaBuf, emaBuf2);

        // append another buffer
        emaBuf2.Append(emaBuf);
        Assert.NotEqual(emaBuf, emaBuf2);
        Assert.Equal(8, emaBuf2.Length);
        Assert.Equal(new byte[] { 0, 1, 2, 3, 0, 1, 2, 3 }, emaBuf2.AsByteArray());
    }


    [Fact]
    public void EmaBufferU16Append_Test()
    {
        EmaBufferU16 emaBuf = new();

        Assert.Equal(0, emaBuf.Length);

        // append single byte
        emaBuf.Append(' ');

        Assert.Equal(1, emaBuf.Length);
        Assert.Equal(new char[] { ' ' }, emaBuf.Buffer.ToArray());

        // apend several bytes as a span
        emaBuf.Append(new char[] { 'a', 'b', 'c' });
        Assert.Equal(4, emaBuf.Length);
        Assert.Equal(new char[] { ' ', 'a', 'b', 'c' }, emaBuf.Buffer.ToArray());

        EmaBufferU16 emaBuf2 = new(emaBuf);
        Assert.Equal(emaBuf, emaBuf2);

        // append another buffer
        emaBuf2.Append(emaBuf);
        Assert.NotEqual(emaBuf, emaBuf2);
        Assert.Equal(8, emaBuf2.Length);
        Assert.Equal(new char[] { ' ', 'a', 'b', 'c', ' ', 'a', 'b', 'c' }, emaBuf2.Buffer.ToArray());
    }

    #region HexString tests

    [Fact]
    public void TestAsRawHexString_fullBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        for (int i = 1; i <= 32; i++)
        {
            buffer.Append((byte)i);
        }

        string expectedOutput =
            "0102 0304 0506 0708 090A 0B0C 0D0E 0F10\n" +
            "1112 1314 1516 1718 191A 1B1C 1D1E 1F20";

        string hexString = buffer.AsRawHexString();

        Assert.Equal(expectedOutput, hexString);
    }

    [Fact]
    public void TestAsRawHexString_partialBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        for (int i = 1; i <= 34; i++)
        {
            buffer.Append((byte)i);
        }

        string expectedOutput =
            "0102 0304 0506 0708 090A 0B0C 0D0E 0F10\n" +
            "1112 1314 1516 1718 191A 1B1C 1D1E 1F20\n" +
            "2122";

        string hexString = buffer.AsRawHexString();

        Assert.Equal(expectedOutput, hexString);
    }

    [Fact]
    public void TestAsRawHexString_noBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        string hexString = buffer.AsRawHexString();

        Assert.Empty(hexString);
    }

    [Fact]
    public void TestAsHexString_fullBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        for (int i = 1; i <= 32; i++)
        {
            buffer.Append((byte)i);
        }

        string expectedOutput =
            "0102 0304 0506 0708  090A 0B0C 0D0E 0F10   ................\n" +
            "1112 1314 1516 1718  191A 1B1C 1D1E 1F20   ............... ";

        string hexString = buffer.AsHexString();

        Assert.Equal(expectedOutput, hexString);
    }

    [Fact]
    public void TestAsHexString_partialBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        for (int i = 1; i <= 34; i++)
        {
            buffer.Append((byte)i);
        }

        string expectedOutput =
            "0102 0304 0506 0708  090A 0B0C 0D0E 0F10   ................\n" +
            "1112 1314 1516 1718  191A 1B1C 1D1E 1F20   ............... \n" +
            "2122                                       !\"";

        string hexString = buffer.AsHexString();

        Assert.Equal(expectedOutput, hexString);
    }

    [Fact]
    public void TestAsHexString_noBytes()
    {
        EmaBuffer buffer = new EmaBuffer();

        string hexString = buffer.AsHexString();

        Assert.Empty(hexString);
    }

    #endregion
}
