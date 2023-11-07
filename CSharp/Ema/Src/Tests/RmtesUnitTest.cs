/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System.Text;

using LSEG.Eta.Common;

using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests;


public class RmtesUnitTest
{
    ITestOutputHelper output;

    private const byte ESC = 0x1B;

    /// <summary>
    /// Partial update. Moves cursor to index 0 and replaces first two characters with '1'
    /// and '2' (0x31, 0x32).
    /// </summary>
    private readonly byte[] IN_PARTIAL_BUF1 = { ESC, 0x5B, (byte)'0', 0x60, (byte)'1', (byte)'2' };

    /// <summary>
    /// Partial update. Replace space(0x20) starting index at 9(0x39), also repeat(0x62)
    /// 2 (0x32) times.
    /// </summary>
    private readonly byte[] IN_PARTIAL_BUF2 = { ESC, 0x5B, (byte)'9', 0x60, (byte)' ',
                                                ESC, 0x5B, (byte)'2', 0x62 };

    private RmtesBuffer inputRmtesBuf = new();
    private ByteBuffer inputByteBuf = new ByteBuffer(30);

    private readonly byte[] INPUT_BYTE =
        { 0x57, 0x61, 0x69, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x4C, 0x42,
          0x4D, 0x2E, 0x2E, 0x2E };

    private const string TARGET_STRING = "Waiting for LBM...";
    private EmaBufferU16 targetCharBuf = new(30);

    private RmtesBuffer inputRmtesBuf1 = new();
    private ByteBuffer inputByteBuf1 = new ByteBuffer(30);

    private readonly byte[] INPUT_BYTE1 = { 0x43, 0x4F, 0x46, 0x2F, 0x4E, 0x4A };

    private const string TARGET_STRING1 = "COF/NJ";
    private EmaBufferU16 targetCharBuf1 = new(30);

    public RmtesUnitTest(ITestOutputHelper output)
    {
        this.output = output;
    }

    [Fact]
    public void TestRmtesBuffer_asUTF16()
    {
        output.WriteLine("TestRmtesBuffer_asUTF16: test RmtesBuffer function call");

        SetupFirstBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        EmaBufferU16 outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_toString()
    {
        output.WriteLine("TestRmtesBuffer_toString: test RmtesBuffer function call");

        SetupFirstBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        string outputString = outputRmtesBuf.Apply(inputRmtesBuf).ToString();

        Assert.Equal(TARGET_STRING, outputString);

        // test recall again
        Assert.Equal(TARGET_STRING, outputRmtesBuf.ToString());

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_toStringAsUTF16()
    {
        output.WriteLine("TestRmtesBuffer_toStringAsUTF16: test RmtesBuffer function call");

        SetupFirstBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        string outputString = outputRmtesBuf.Apply(inputRmtesBuf).ToString();

        Assert.Equal(TARGET_STRING, outputString);

        output.WriteLine(outputString);

        output.WriteLine(TARGET_STRING);

        EmaBufferU16 outputCharBuffer = outputRmtesBuf.GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_clear()
    {
        output.WriteLine("TestRmtesBuffer_clear: test RmtesBuffer function call");

        SetupFirstBuffer();
        SetupSecondBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        EmaBufferU16 outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        EmaBuffer outputByteBuffer = outputRmtesBuf.Clear().Apply(inputRmtesBuf1).GetAsUTF8();
        outputRmtesBuf.GetAsUTF8();

        Assert.Equal(6, outputByteBuffer.Length);
        Assert.Equal(INPUT_BYTE1, outputByteBuffer.Buffer.ToArray());

        outputByteBuffer = outputRmtesBuf.GetAsUTF8();

        Assert.Equal(6, outputByteBuffer.Length);
        Assert.Equal(INPUT_BYTE1, outputByteBuffer.Buffer.ToArray());

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_selfApply()
    {
        output.WriteLine("TestRmtesBuffer_selfApply: test RmtesBuffer function call");

        SetupFirstBuffer();

        EmaBufferU16 outputCharBuffer1 = inputRmtesBuf.GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer1.ToString());
        Assert.Equal(0, outputCharBuffer1.CompareTo(targetCharBuf));

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        outputRmtesBuf.Apply(inputRmtesBuf);
        EmaBufferU16 outputCharBuffer = outputRmtesBuf.GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_selfApplyApplyAgain()
    {
        output.WriteLine("TestRmtesBuffer_selfApplyApplyAgain: test RmtesBuffer function call");

        SetupFirstBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        string outputString = outputRmtesBuf.Apply(inputRmtesBuf).ToString();

        Assert.Equal(TARGET_STRING, outputString);

        EmaBufferU16 outputCharBuffer = outputRmtesBuf.GetAsUTF16();

        Assert.Equal(TARGET_STRING, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_emptyBuffer()
    {
        output.WriteLine("TestRmtesBuffer_emptyBuffer: test RmtesBuffer function call");

        SetupEmptyBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();

        // in EMA.NET applying empty buffers won't throw an exception
        Assert.NotNull(outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16());

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_partialUpdate()
    {
        output.WriteLine("TestRmtesBuffer_partialUpdate: test RmtesBuffer function call");

        string cacheStr = "abcdefghijkl";
        inputRmtesBuf = new RmtesBuffer(Encoding.ASCII.GetBytes(cacheStr));

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(cacheStr.ToCharArray());

        // case1
        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        EmaBufferU16 outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();
        Assert.Equal(cacheStr.Length, outputCharBuffer.Length);
        Assert.Equal(cacheStr, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        // apply partial
        outputCharBuffer = outputRmtesBuf.Apply(IN_PARTIAL_BUF1).GetAsUTF16();

        string targetString = "12cdefghijkl";

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(targetString.ToCharArray());

        Assert.Equal(targetString.Length, outputCharBuffer.Length);
        Assert.Equal(targetString, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();

        // case2
        inputRmtesBuf = new RmtesBuffer(Encoding.ASCII.GetBytes("abcdefghijkl"));
        outputRmtesBuf.Clear();
        outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();

        // apply partial
        inputRmtesBuf1 = new RmtesBuffer();

        outputCharBuffer = outputRmtesBuf.Apply(IN_PARTIAL_BUF2).GetAsUTF16();

        targetString = "abcdefghi   ";

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(targetString.ToCharArray());

        Assert.Equal(targetString, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_callToStringBeforeApplyPartialUpdates()
    {
        output.WriteLine("TestRmtesBuffer_callToStringBeforeApplyPartialUpdates: test running toString RmtesBuffer function call before apply");

        string cacheStr = "abcdefghijkl";
        inputByteBuf.Put(Encoding.ASCII.GetBytes(cacheStr));
        inputByteBuf.Flip();
        inputRmtesBuf = new RmtesBuffer(Encoding.ASCII.GetBytes(cacheStr));

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(cacheStr.ToCharArray());

        // case1
        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        EmaBufferU16 outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();
        Assert.Equal(cacheStr, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        // apply partial
        StringBuilder str = new StringBuilder();

        outputCharBuffer = outputRmtesBuf.Apply(IN_PARTIAL_BUF1).GetAsUTF16();

        string targetString = "12cdefghijkl";

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(targetString.ToCharArray());

        // Assert.Equal(12, outputCharBuffer.Length);
        Assert.Equal(targetString, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();

        // case2
        inputRmtesBuf = new RmtesBuffer(Encoding.ASCII.GetBytes(cacheStr));

        outputRmtesBuf.Clear();
        outputCharBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF16();

        // apply partial
        inputRmtesBuf1 = new RmtesBuffer(); // IN_PARTIAL_BUF2);

        // Run toString() on RmtesBuffer before applying it to another RmtesBuffer
        inputRmtesBuf1.ToString();

        outputCharBuffer = outputRmtesBuf.Apply(IN_PARTIAL_BUF2 /*inputRmtesBuf1*/).GetAsUTF16();

        targetString = "abcdefghi   ";

        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(targetString.ToCharArray());

        // Assert.Equal(12, outputCharBuffer.Length);
        Assert.Equal(targetString, outputCharBuffer.ToString());
        Assert.Equal(0, outputCharBuffer.CompareTo(targetCharBuf));

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_asUTF8Test()
    {
        output.WriteLine("TestRmtesBuffer_asUTF8Test: test RmtesBuffer function call");

        SetupFirstBuffer();

        RmtesBuffer outputRmtesBuf = new RmtesBuffer();
        EmaBuffer outputByteBuffer = outputRmtesBuf.Apply(inputRmtesBuf).GetAsUTF8();

        Assert.Equal(18, outputByteBuffer.Length);
        Assert.Equal(INPUT_BYTE, outputByteBuffer.AsByteArray());

        ClearBuffer();
    }

    [Fact]
    public void TestRmtesBuffer_constructor()
    {
        //case1 intial with default contructor
        RmtesBuffer rmtesBuf = new();

        byte[] cacheBuf1 = Encoding.BigEndianUnicode.GetBytes("abcdefghijklabcdefghijklabcdefghijkl");
        string outBuf = "abcdefghijklabcdefghijklabcdefghijkl";

        EmaBuffer utf8Buf = rmtesBuf.Apply(cacheBuf1).GetAsUTF8();

        Assert.Equal(36, utf8Buf.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf);

        //case2 initial with length
        RmtesBuffer rmtesBuf1 = new(5);

        EmaBuffer utf8Buf1 = rmtesBuf1.Apply(cacheBuf1).GetAsUTF8();

        Assert.Equal(36, utf8Buf1.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf1);

        //case3
        RmtesBuffer rmtesBuf6 = new RmtesBuffer(rmtesBuf1);

        EmaBuffer utf8Buf4 = rmtesBuf6.GetAsUTF8();

        Assert.Equal(36, utf8Buf4.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf4);

        //case4 test initial cached buffer with partial updates
        try
        {
            RmtesBuffer rmtesBuf3 = new RmtesBuffer(IN_PARTIAL_BUF1);
            Assert.False(true);
        }
        catch (OmmException)
        {
            Assert.True(true, "Exception is expected");
        }

        //case4 intial with RmtesBuffer
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        outBuf = "abcdefghijkl";
        RmtesBuffer rmtesBuf4 = new RmtesBuffer(cacheBuf1);
        RmtesBuffer rmtesBuf5 = new RmtesBuffer(rmtesBuf4);

        EmaBuffer utf8Buf2 = rmtesBuf4.GetAsUTF8();

        Assert.Equal(12, utf8Buf2.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf2);

        string toString = rmtesBuf5.ToString();

        Assert.Equal(12, toString.Length);
        Assert.Equal(outBuf, toString);
    }

    [Fact]
    public void TestRmtesBuffer_applyFullUpdates()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        byte[] cacheBuf2 = Encoding.ASCII.GetBytes("1234567890");
        string outBuf = "1234567890";
        RmtesBuffer rmtesBuf1 = new(cacheBuf1);
        RmtesBuffer rmtesBuf2 = new(rmtesBuf1);

        EmaBuffer utf8Buf = rmtesBuf2.Apply(cacheBuf2).GetAsUTF8();

        Assert.Equal(10, utf8Buf.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf);

        string toString = rmtesBuf2.ToString();

        Assert.Equal(10, toString.Length);
        Assert.Equal(outBuf, toString);

        //case2  Reallocate bigger mem
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        cacheBuf2 = Encoding.ASCII.GetBytes("1234567890abcdefghijkl");
        outBuf = "1234567890abcdefghijkl";
        RmtesBuffer rmtesBuf3 = new RmtesBuffer(cacheBuf1);
        RmtesBuffer rmtesBuf4 = new RmtesBuffer(cacheBuf2);
        RmtesBuffer rmtesBuf5 = new RmtesBuffer(cacheBuf1);

        EmaBuffer utf8Buf1 = rmtesBuf3.Apply(rmtesBuf4).GetAsUTF8();

        Assert.Equal(22, utf8Buf1.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf1);

        string toString1 = rmtesBuf3.Apply(rmtesBuf5).ToString();

        outBuf = "abcdefghijkl";
        Assert.Equal(12, toString1.Length);
        Assert.Equal(outBuf, toString1);
    }

    [Fact]
    public void TestRmtesBuffer_applyPartialUpdates()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        string outBuf = "12cdefghijkl";
        RmtesBuffer rmtesBuf1 = new RmtesBuffer(cacheBuf1);

        EmaBuffer utf8Buf = rmtesBuf1.Apply(IN_PARTIAL_BUF1).GetAsUTF8();

        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf);

        //case2  Reallocate bigger mem
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");

        outBuf = "abcdefghi   ";
        RmtesBuffer rmtesBuf2 = new RmtesBuffer(cacheBuf1); // "abcdefghijkl"

        rmtesBuf2.Apply(IN_PARTIAL_BUF2);

        Assert.Equal(12, rmtesBuf2.GetAsUTF8().Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)),
            rmtesBuf2.GetAsUTF8());

        outBuf = "12cdefghi   ";
        string toString = rmtesBuf2.Apply(IN_PARTIAL_BUF1).ToString();

        Assert.Equal(outBuf.Length, toString.Length);
        Assert.Equal(outBuf, toString);
    }

    [Fact]
    public void TestRmtesBuffer_callToStringBeforeApplyPartialUpdates_bytes()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        string outBuf = "12cdefghijkl";
        RmtesBuffer rmtesBuf1 = new(cacheBuf1);

        EmaBuffer utf8Buf = rmtesBuf1.Apply(IN_PARTIAL_BUF1).GetAsUTF8();

        Assert.Equal(12, utf8Buf.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf);

        //case2  Reallocate bigger mem
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");

        outBuf = "abcdefghi   ";
        RmtesBuffer rmtesBuf2 = new RmtesBuffer(cacheBuf1);

        EmaBuffer utf8Buf1 = rmtesBuf2.Apply(IN_PARTIAL_BUF2).GetAsUTF8();

        Assert.Equal(outBuf.Length, utf8Buf1.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf1);

        RmtesBuffer rmtesBuf3 = new RmtesBuffer();// IN_PARTIAL_BUF1);

        // Run toString() on RmtesBuffer before applying it to another RmtesBuffer
        rmtesBuf3.ToString();

        outBuf = "12cdefghi   ";
        string toString = rmtesBuf2.Apply(IN_PARTIAL_BUF1 /*rmtesBuf3*/).ToString();

        // Assert.Equal(outBuf.Length, toString.Length);
        Assert.Equal(outBuf, toString);
    }

    [Fact]
    public void TestRmtesBuffer_applyUpdatesAsUTF16()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        byte[] cacheBuf2 = Encoding.ASCII.GetBytes("1234567890abc");
        string outBuf = "1234567890abc";
        EmaBufferU16 outShortBuf = new EmaBufferU16(new char[] { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c' });
        RmtesBuffer rmtesBuf1 = new(cacheBuf1);
        RmtesBuffer rmtesBuf2 = new(rmtesBuf1);

        EmaBufferU16 utf16Buf = rmtesBuf2.Apply(cacheBuf2).GetAsUTF16();

        Assert.Equal(13, utf16Buf.Length);
        Assert.Equal(outShortBuf, utf16Buf);

        string toString = rmtesBuf2.ToString();

        Assert.Equal(13, toString.Length);
        Assert.Equal(outBuf, toString);

        //case2
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        outBuf = "abcdefghij   l";
        EmaBufferU16 outShortBuf1 = new EmaBufferU16(new char[] { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' ' });
        RmtesBuffer rmtesBuf3 = new(cacheBuf1);

        EmaBufferU16 utf16Buf1 = rmtesBuf3.Apply(IN_PARTIAL_BUF2).GetAsUTF16();

        Assert.Equal(12, utf16Buf1.Length);
        Assert.Equal(outShortBuf1, utf16Buf1);

        EmaBufferU16 outShortBuf2 = new(new char[] { '1', '2', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' ' });
        EmaBufferU16 utf16Buf2 = rmtesBuf3.Apply(IN_PARTIAL_BUF1).GetAsUTF16();

        Assert.Equal(12, utf16Buf2.Length);
        Assert.Equal(outShortBuf2, utf16Buf2);
    }

    [Fact]
    public void TestRmtesBuffer_getAsUTF16EmaBuff16Copy()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        byte[] cacheBuf2 = Encoding.ASCII.GetBytes("1234567890abc");
        string outBuf = "1234567890abc";
        EmaBufferU16 outUInt16Buf = new(new char[] { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c' });
        RmtesBuffer rmtesBuf1 = new(cacheBuf1);
        RmtesBuffer rmtesBuf2 = new(rmtesBuf1);

        EmaBufferU16 utf16Buf = rmtesBuf2.Apply(cacheBuf2).GetAsUTF16();

        Assert.Equal(13, utf16Buf.Length);
        Assert.Equal(outUInt16Buf, utf16Buf);

        string toString = rmtesBuf2.ToString();

        Assert.Equal(13, toString.Length);
        Assert.Equal(outBuf, toString);

        //case2
        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        outBuf = "abcdefghij   l";
        EmaBufferU16 outUInt16Buf1 = new EmaBufferU16(new char[] { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' ' });
        RmtesBuffer rmtesBuf3 = new(cacheBuf1);

        EmaBufferU16 utf16Buf1 = rmtesBuf3.Apply(IN_PARTIAL_BUF2).GetAsUTF16();

        Assert.Equal(12, utf16Buf1.Length);
        Assert.Equal(outUInt16Buf1, utf16Buf1);

        EmaBufferU16 outUInt16Buf2 = new EmaBufferU16(new char[] { '1', '2', 'c', 'd', 'e', 'f', 'g', 'h', 'i', ' ', ' ', ' ' });
        EmaBufferU16 utf16Buf2 = rmtesBuf3.Apply(IN_PARTIAL_BUF1).GetAsUTF16();

        Assert.Equal(12, utf16Buf2.Length);
        Assert.Equal(outUInt16Buf2, utf16Buf2);
    }

    [Fact]
    public void TestRmtesBuffer_clearBytes()
    {
        //case1
        byte[] cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        byte[] cacheBuf2 = Encoding.ASCII.GetBytes("1234567890");

        RmtesBuffer rmtesBuf1 = new RmtesBuffer(cacheBuf1);
        RmtesBuffer rmtesBuf2 = new RmtesBuffer(rmtesBuf1);

        EmaBufferU16 utf16Buf = rmtesBuf2.Apply(cacheBuf2).GetAsUTF16();

        rmtesBuf2.Clear();

        string outBuf = "1234567890abcdefghijkl";

        cacheBuf1 = Encoding.ASCII.GetBytes("abcdefghijkl");
        cacheBuf2 = Encoding.ASCII.GetBytes(outBuf);

        rmtesBuf2.Apply(cacheBuf1);
        rmtesBuf2.Apply(cacheBuf2);

        EmaBuffer utf8Buf = rmtesBuf2.GetAsUTF8();

        Assert.Equal(22, utf8Buf.Length);
        Assert.Equal(new EmaBuffer(Encoding.ASCII.GetBytes(outBuf)), utf8Buf);

        rmtesBuf2.Clear();

        Assert.Equal(0, rmtesBuf2.GetAsUTF8().Length);
        Assert.Equal(0, rmtesBuf2.GetAsUTF16().Length);
        Assert.Equal(0, rmtesBuf2.ToString().Length);

        rmtesBuf2.Clear();
    }

    #region Helper methods

    private void SetupFirstBuffer()
    {
        targetCharBuf.Clear();
        targetCharBuf.CopyFrom(TARGET_STRING.ToCharArray());

        inputByteBuf.Clear();
        inputByteBuf.Put(INPUT_BYTE);
        inputByteBuf.Flip();

        inputRmtesBuf = new RmtesBuffer(INPUT_BYTE);
    }

    private void SetupSecondBuffer()
    {
        targetCharBuf1.Clear();
        targetCharBuf1.CopyFrom(TARGET_STRING1.ToCharArray());

        inputByteBuf1.Clear();
        inputByteBuf1.Put(INPUT_BYTE1);
        inputByteBuf1.Flip();
        inputRmtesBuf1 = new RmtesBuffer(INPUT_BYTE1);
    }

    private void ClearBuffer()
    {
        inputRmtesBuf.Clear();
        inputRmtesBuf1.Clear();
    }

    private void SetupEmptyBuffer()
    {
        targetCharBuf.Clear();
        targetCharBuf.CopyFrom("".ToCharArray());

        inputRmtesBuf.Clear();
    }

    #endregion
}
