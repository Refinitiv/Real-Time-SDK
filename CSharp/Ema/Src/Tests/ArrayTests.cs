/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;

using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests;

public class ArrayTests
{
    private const int DEFAULT_BUFFER_SIZE = 1024;

    ITestOutputHelper output;

    public ArrayTests(ITestOutputHelper output)
    {
        this.output = output;
    }

    [Fact]
    public void TestArrayAscii_Decode()
    {
        output.WriteLine("TestArrayAscii_Decode: Decoding OmmArray of Ascii");

        LSEG.Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        LSEG.Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.ASCII_STRING;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        LSEG.Eta.Codec.Buffer bufText = new();
        bufText.Data("ABC");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("DEFGH");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("KLMNOPQRS");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        // Now do EMA decoding of OmmArray of Ascii
        OmmArray ar = new();

        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();
        Assert.True(arIter.MoveNext());

        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae1.Load.DataType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", ae1.OmmAsciiValue().Value);

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae2.LoadType);
        Assert.Equal("DEFGH", ae2.OmmAsciiValue().Value);

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae3.LoadType);
        Assert.Equal("KLMNOPQRS", ae3.OmmAsciiValue().Value);

        Assert.False(arIter.MoveNext(), "OmmArray with three Ascii - fourth next()");

        arIter.Dispose();

        // second pass, enumerator must start from the beginning
        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", ae1.OmmAsciiValue().Value);

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae2.LoadType);
            Assert.Equal("DEFGH", ae2.OmmAsciiValue().Value);

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmAsciiValue().Value);

            Assert.False(arIter.MoveNext(), "OmmArray with three Ascii - fourth next()");
        }
    }

    [Fact]
    public void TestArrayAsciiOneBlankEntry_Decode()
    {
        output.WriteLine("TestArrayAsciiOneBlankEntry_Decode: OmmArray Decode Ascii One Blank Entry");

        LSEG.Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        LSEG.Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.Clear();
        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.ASCII_STRING;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        LSEG.Eta.Codec.Buffer bufText = new();
        bufText.Data("ABC");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.EncodeBlank(iter));

        bufText.Clear();
        bufText.Data("KLMNOPQRS");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Ascii On Blank
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae1.LoadType);
        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", ae1.OmmAsciiValue().Value);

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae2.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae2.Code);

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ASCII, ae3.LoadType);
        Assert.Equal("KLMNOPQRS", ae3.OmmAsciiValue().Value);

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae1.LoadType);
            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmAsciiValue().Value.Equals("ABC"), "OmmArrayEntry.ascii()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae2.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2.Code);

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii (one blank) - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ASCII, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmAsciiValue().Value);

            Assert.False(arIter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayBlank_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("testArrayBlank_Decode: Decoding Blank OmmArray with " + appendText);

        LSEG.Eta.Codec.Array array = new();

        LSEG.Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        array.ItemLength = fixedSize ? 4 : 0; //varying size only
        array.PrimitiveType = DataTypes.UINT;

        //Now do EMA decoding of OmmArray
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.False(arIter.MoveNext());
    }

    [Fact]
    void TestArrayDate_Decode()
    {
        output.WriteLine("testArrayDate_Decode: Decoding OmmArray of Date");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.DATE;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.Date date = new();
        date.Year(1111);
        date.Month(11);
        date.Day(1);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, date));

        date.Year(2222);
        date.Month(2);
        date.Day(2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, date));

        date.Year(3333);
        date.Month(3);
        date.Day(3);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, date));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Dates
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Date - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmDateValue().Year == 1111, "OmmArrayEntry.OmmDateValue().Year");
        Assert.True(ae1.OmmDateValue().Month == 11, "OmmArrayEntry.OmmDateValue().Month");
        Assert.True(ae1.OmmDateValue().Day == 1, "OmmArrayEntry.OmmDateValue().Day");

        Assert.True(arIter.MoveNext(), "OmmArray with three Date - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.True(ae2.LoadType == DataType.DataTypes.DATE, "OmmArrayEntry.LoadType == DataType.DataTypes.DATE");
        Assert.True(ae2.OmmDateValue().Year == 2222, "OmmArrayEntry.OmmDateValue().Year");
        Assert.True(ae2.OmmDateValue().Month == 2, "OmmArrayEntry.OmmDateValue().Month");
        Assert.True(ae2.OmmDateValue().Day == 2, "OmmArrayEntry.OmmDateValue().Day");

        Assert.True(arIter.MoveNext(), "OmmArray with three Date - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.True(ae3.LoadType == DataType.DataTypes.DATE, "OmmArrayEntry.LoadType == DataType.DataTypes.DATE");
        Assert.True(ae3.OmmDateValue().Year == 3333, "OmmArrayEntry.OmmDateValue().Year");
        Assert.True(ae3.OmmDateValue().Month == 3, "OmmArrayEntry.OmmDateValue().Month");
        Assert.True(ae3.OmmDateValue().Day == 3, "OmmArrayEntry.OmmDateValue().Day");

        Assert.True(!arIter.MoveNext(), "OmmArray with three Date - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Date - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.DATE, "OmmArrayEntry.LoadType == DataType.DataTypes.DATE");

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmDateValue().Year == 1111, "OmmArrayEntry.OmmDateValue().Year");
            Assert.True(ae1.OmmDateValue().Month == 11, "OmmArrayEntry.OmmDateValue().Month");
            Assert.True(ae1.OmmDateValue().Day == 1, "OmmArrayEntry.OmmDateValue().Day");

            Assert.True(arIter.MoveNext(), "OmmArray with three Date - second next()");
            ae2 = arIter.Current;
            Assert.True(ae2.LoadType == DataType.DataTypes.DATE, "OmmArrayEntry.LoadType == DataType.DataTypes.DATE");
            Assert.True(ae2.OmmDateValue().Year == 2222, "OmmArrayEntry.OmmDateValue().Year");
            Assert.True(ae2.OmmDateValue().Month == 2, "OmmArrayEntry.OmmDateValue().Month");
            Assert.True(ae2.OmmDateValue().Day == 2, "OmmArrayEntry.OmmDateValue().Day");

            Assert.True(arIter.MoveNext(), "OmmArray with three Date - third next()");
            ae3 = arIter.Current;
            Assert.True(ae3.LoadType == DataType.DataTypes.DATE, "OmmArrayEntry.LoadType == DataType.DataTypes.DATE");
            Assert.True(ae3.OmmDateValue().Year == 3333, "OmmArrayEntry.OmmDateValue().Year");
            Assert.True(ae3.OmmDateValue().Month == 3, "OmmArrayEntry.OmmDateValue().Month");
            Assert.True(ae3.OmmDateValue().Day == 3, "OmmArrayEntry.OmmDateValue().Day");

            Assert.True(!arIter.MoveNext(), "OmmArray with three Date - fourth next()");
        }
    }

    void ArrayEncodeDouble(Eta.Codec.Buffer buf, bool fixedSize, double[] values, int numValues)
    {
        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 8 : 0;
        array.PrimitiveType = DataTypes.DOUBLE;
        Eta.Codec.Double val = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        for (int i = 0; i < numValues; i++)
        {
            val.Value(values[i]);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, val));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayDouble_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("testArrayDouble_Decode: Decoding OmmArray of Double with " + appendText);

        //encode 3 Doubles
        Eta.Codec.Buffer buf = new();
        double[] values = { -11.1111, 22.2222, -33.3333 };
        ArrayEncodeDouble(buf, fixedSize, values, 3);

        //Now do EMA decoding of OmmArray of Doubles
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.Equal(fixedSize, ar.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Double - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);
        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(-11.1111, ae1.OmmDoubleValue().Value);

        Assert.True(arIter.MoveNext(), "OmmArray with three Double - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
        Assert.Equal(22.2222, ae2.OmmDoubleValue().Value);

        Assert.True(arIter.MoveNext(), "OmmArray with three Double - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
        Assert.Equal(-33.3333, ae3.OmmDoubleValue().Value);

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.Equal(fixedSize, ar.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Double - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);
            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmDoubleValue().Value == -11.1111, "OmmArrayEntry.doubleValue()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Double - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
            Assert.Equal(22.2222, ae2.OmmDoubleValue().Value);

            Assert.True(arIter.MoveNext(), "OmmArray with three Double - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
            Assert.Equal(-33.3333, ae3.OmmDoubleValue().Value);

            Assert.False(arIter.MoveNext());
        }
    }

    void ArrayEncodeFloat(Eta.Codec.Buffer buf, bool fixedSize, float[] values, int numValues)
    {
        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 4 : 0;
        array.PrimitiveType = DataTypes.FLOAT;
        Eta.Codec.Float val = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        for (int i = 0; i < numValues; i++)
        {
            val.Value(values[i]);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, val));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    void TestArrayFloat_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("testArrayFloat_Decode: Decoding OmmArray of Float with " + appendText);

        //encode 3 Floats
        Eta.Codec.Buffer buf = new();
        float[] values = { -11.11f, 22.22f, -33.33f };
        ArrayEncodeFloat(buf, fixedSize, values, 3);

        //Now do EMA decoding of OmmArray of Floats
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Float - hasFixedWidth()");
        Assert.True(ar.FixedWidth == (fixedSize ? 4 : 0), "OmmArray with three Float - getFixedWidth()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Float - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmFloatValue().Value == -11.11f, "OmmArrayEntry.floatValue()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Float - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.True(ae2.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");
        Assert.True(ae2.OmmFloatValue().Value == 22.22f, "OmmArrayEntry.floatValue()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Float - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.True(ae3.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");
        Assert.True(ae3.OmmFloatValue().Value == -33.33f, "OmmArrayEntry.floatValue()");

        Assert.True(!arIter.MoveNext(), "OmmArray with three Float - fourth next()");


        arIter = ar.GetEnumerator();
        {
            Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Float - hasFixedWidth()");
            Assert.True(ar.FixedWidth == (fixedSize ? 4 : 0), "OmmArray with three Float - getFixedWidth()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Float - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.True(ae1.OmmFloatValue().Value == -11.11f, "OmmArrayEntry.floatValue()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Float - second next()");
            ae2 = arIter.Current;
            Assert.True(ae2.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");
            Assert.True(ae2.OmmFloatValue().Value == 22.22f, "OmmArrayEntry.floatValue()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Float - third next()");
            ae3 = arIter.Current;
            Assert.True(ae3.LoadType == DataType.DataTypes.FLOAT, "OmmArrayEntry.LoadType == DataType.DataTypes.FLOAT");
            Assert.True(ae3.OmmFloatValue().Value == -33.33f, "OmmArrayEntry.floatValue()");

            Assert.True(!arIter.MoveNext(), "OmmArray with three Float - fourth next()");
        }
    }

    private void ArrayEncodeInt(Eta.Codec.Buffer buf, bool fixedSize, long[] values, int numValues)
    {
        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 4 : 0;
        array.PrimitiveType = DataTypes.INT;
        Eta.Codec.Int val = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        for (int i = 0; i < numValues; i++)
        {
            val.Value(values[i]);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, val));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayInt_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayInt_Decode: Decoding OmmArray of Int with " + appendText);
        //encode 3 Ints
        Eta.Codec.Buffer buf = new();
        long[] values = { -11, 22, -33 };
        ArrayEncodeInt(buf, fixedSize, values, 3);

        //Now do EMA decoding of OmmArray of Ints
        OmmArray ommArray = new();
        ommArray.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        Assert.Equal(fixedSize, ommArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 4 : 0), ommArray.FixedWidth);

        IEnumerator<OmmArrayEntry> arIter = ommArray.GetEnumerator();

        Assert.True(arIter.MoveNext());
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(values[0], ae1.OmmIntValue().Value);

        Assert.True(arIter.MoveNext());
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
        Assert.Equal(values[1], ae2.OmmIntValue().Value);

        Assert.True(arIter.MoveNext());
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
        Assert.Equal(values[2], ae3.OmmIntValue().Value);

        Assert.False(arIter.MoveNext());

        arIter = ommArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, ommArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 4 : 0), ommArray.FixedWidth);

            Assert.True(arIter.MoveNext());
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(values[0], ae1.OmmIntValue().Value);

            Assert.True(arIter.MoveNext());
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
            Assert.Equal(values[1], ae2.OmmIntValue().Value);

            Assert.True(arIter.MoveNext());
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
            Assert.Equal(values[2], ae3.OmmIntValue().Value);

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayReal_Decode()
    {
        output.WriteLine("testArrayReal_Decode: Decoding OmmArray of Real");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();
        Eta.Codec.Buffer buf = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0;
        array.PrimitiveType = DataTypes.REAL;

        Eta.Codec.Real real = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        real.Value(11, RealHints.EXPONENT_2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        real.Value(22, RealHints.FRACTION_2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        real.Value(-33, RealHints.FRACTION_2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        real.Value(22801, 31);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));
        iter.Buffer().Data().WriteAt(iter.Buffer().Data().Position - 2, (byte)0x1F);

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Real - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmRealValue().Mantissa == 11, "OmmArrayEntry.real().mantissa()");
        Assert.True(ae1.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.EXPONENT_NEG_2, "OmmArrayEntry.real().magnitudeType()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Real - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
        Assert.True(ae2.OmmRealValue().Mantissa == 22, "OmmArrayEntry.real().mantissa()");
        Assert.True(ae2.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Real - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
        Assert.True(ae3.OmmRealValue().Mantissa == -33, "OmmArrayEntry.real().mantissa()");
        Assert.True(ae3.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Real - fourth next()");
        OmmArrayEntry ae4 = arIter.Current;
        // todo: for some reason writing arbitrary byte into the buffer
        // does not corrupt it enough to cause an error
        //Assert.Equal(DataType.DataTypes.ERROR, ae4.LoadType);
        //Assert.True(ae4.Code == Data.DataCode.NO_CODE, "OmmArrayEntry.code()");
        //Assert.True(ae4.OmmErrorValue().ErrorCode == OmmError.ErrorCodes.INCOMPLETE_DATA, "OmmArrayEntry.error().errorCode()");

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Real - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmRealValue().Mantissa == 11, "OmmArrayEntry.real().mantissa()");
            Assert.True(ae1.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.EXPONENT_NEG_2, "OmmArrayEntry.real().magnitudeType()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Real - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
            Assert.True(ae2.OmmRealValue().Mantissa == 22, "OmmArrayEntry.real().mantissa()");
            Assert.True(ae2.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Real - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
            Assert.True(ae3.OmmRealValue().Mantissa == -33, "OmmArrayEntry.real().mantissa()");
            Assert.True(ae3.OmmRealValue().MagnitudeType == OmmReal.MagnitudeTypes.DIVISOR_2, "OmmArrayEntry.real().magnitudeType()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Real - fourth next()");
            ae4 = arIter.Current;
            // todo: for some reason writing arbitrary byte into the buffer
            // does not corrupt it enough to cause an error
            //Assert.Equal(DataType.DataTypes.ERROR, ae4.LoadType);
            //Assert.True(ae4.Code == Data.DataCode.NO_CODE, "OmmArrayEntry.code()");
            //Assert.True(ae4.OmmErrorValue().ErrorCode == OmmError.ErrorCodes.INCOMPLETE_DATA, "OmmArrayEntry.error().errorCode()");

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayRealOneBlankEntry_Decode()
    {
        output.WriteLine("TestArrayRealOneBlankEntry_Decode: Decoding OmmArray of Real (one blank)");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();
        Eta.Codec.Buffer buf = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0;
        array.PrimitiveType = DataTypes.REAL;

        Eta.Codec.Real real = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        real.Blank();
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        real.Value(22, RealHints.FRACTION_2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        real.Value(-33, RealHints.FRACTION_2);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, real));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Reals
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.LoadType == DataType.DataTypes.REAL, "OmmArrayEntry.LoadType == DataType.DataTypes.REAL");
        Assert.Equal(Data.DataCode.BLANK, ae1.Code);

        Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
        Assert.True(ae2.OmmRealValue().Mantissa == 22, "OmmArrayEntry.real().mantissa()");
        Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae2.OmmRealValue().MagnitudeType);

        Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
        Assert.True(ae3.OmmRealValue().Mantissa == -33, "OmmArrayEntry.real().mantissa()");
        Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae3.OmmRealValue().MagnitudeType);

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.REAL, "OmmArrayEntry.LoadType == DataType.DataTypes.REAL");

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1.Code);

            Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
            Assert.True(ae2.OmmRealValue().Mantissa == 22, "OmmArrayEntry.real().mantissa()");
            Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae2.OmmRealValue().MagnitudeType);

            Assert.True(arIter.MoveNext(), "OmmArray with three Real (one blank) - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
            Assert.True(ae3.OmmRealValue().Mantissa == -33, "OmmArrayEntry.real().mantissa()");
            Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae3.OmmRealValue().MagnitudeType);

            Assert.False(arIter.MoveNext());
        }
    }

    void ArrayEncodeUInt(Eta.Codec.Buffer buf, bool fixedSize, long[] values, int numValues)
    {
        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 4 : 0;
        array.PrimitiveType = DataTypes.UINT;
        Eta.Codec.UInt val = new();

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        for (int i = 0; i < numValues; i++)
        {
            val.Value(values[i]);
            Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, val));
        }

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayUInt_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayUInt_Decode: Decoding OmmArray of UInt with " + appendText);

        //encode 3 UInts
        Eta.Codec.Buffer buf = new();
        long[] values = { 11, 22, 33 };
        ArrayEncodeUInt(buf, fixedSize, values, 3);

        //Now do EMA decoding of OmmArray of UInts
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three UInt - hasFixedWidth()");
        Assert.Equal((fixedSize ? 4 : 0), ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three UInt - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.True(ae1.OmmUIntValue().Value == 11, "OmmArrayEntry.getUInt()");

        Assert.True(arIter.MoveNext(), "OmmArray with three UInt - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.True(ae2.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");
        Assert.True(ae2.OmmUIntValue().Value == 22, "OmmArrayEntry.getUInt()");

        Assert.True(arIter.MoveNext(), "OmmArray with three UInt - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.True(ae3.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");
        Assert.True(ae3.OmmUIntValue().Value == 33, "OmmArrayEntry.getUInt()");

        Assert.True(!arIter.MoveNext(), "OmmArray with three UInt - fourth next()");


        arIter = ar.GetEnumerator();
        {
            Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three UInt - hasFixedWidth()");
            Assert.Equal((fixedSize ? 4 : 0), ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three UInt - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");
            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.True(ae1.OmmUIntValue().Value == 11, "OmmArrayEntry.getUInt()");

            Assert.True(arIter.MoveNext(), "OmmArray with three UInt - second next()");
            ae2 = arIter.Current;
            Assert.True(ae2.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");
            Assert.True(ae2.OmmUIntValue().Value == 22, "OmmArrayEntry.getUInt()");

            Assert.True(arIter.MoveNext(), "OmmArray with three UInt - third next()");
            ae3 = arIter.Current;
            Assert.True(ae3.LoadType == DataType.DataTypes.UINT, "OmmArrayEntry.LoadType == DataType.DataTypes.UINT");
            Assert.True(ae3.OmmUIntValue().Value == 33, "OmmArrayEntry.getUInt()");

            Assert.True(!arIter.MoveNext(), "OmmArray with three UInt - fourth next()");
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayTime_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayTime_Decode: Decoding OmmArray of Time with " + appendText);

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 5 : 0; //varying size only
        array.PrimitiveType = DataTypes.TIME;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.Time time = new();
        time.Hour(02);
        time.Minute(03);
        time.Second(04);
        time.Millisecond(05);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, time));

        time.Hour(04);
        time.Minute(05);
        time.Second(06);
        time.Millisecond(07);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, time));

        time.Hour(14);
        time.Minute(15);
        time.Second(16);
        time.Millisecond(17);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, time));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));


        //Now do EMA decoding of OmmArray of Times
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Time - hasFixedWidth()");
        Assert.Equal((fixedSize ? 5 : 0), ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three Time - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmTimeValue().Hour == 02, "OmmArrayEntry.time().Hour");
        Assert.True(ae1.OmmTimeValue().Minute == 03, "OmmArrayEntry.time().Minute");
        Assert.True(ae1.OmmTimeValue().Second == 04, "OmmArrayEntry.time().Second");
        Assert.True(ae1.OmmTimeValue().Millisecond == 05, "OmmArrayEntry.time().Millisecond");

        Assert.True(arIter.MoveNext(), "OmmArray with three Time - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.True(ae2.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");
        Assert.True(ae2.OmmTimeValue().Hour == 04, "OmmArrayEntry.time().Hour");
        Assert.True(ae2.OmmTimeValue().Minute == 05, "OmmArrayEntry.time().Minute");
        Assert.True(ae2.OmmTimeValue().Second == 06, "OmmArrayEntry.time().Second");
        Assert.True(ae2.OmmTimeValue().Millisecond == 07, "OmmArrayEntry.time().Millisecond");

        Assert.True(arIter.MoveNext(), "OmmArray with three Time - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.True(ae3.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");
        Assert.True(ae3.OmmTimeValue().Hour == 14, "OmmArrayEntry.time().Hour");
        Assert.True(ae3.OmmTimeValue().Minute == 15, "OmmArrayEntry.time().Minute");
        Assert.True(ae3.OmmTimeValue().Second == 16, "OmmArrayEntry.time().Second");
        Assert.True(ae3.OmmTimeValue().Millisecond == 17, "OmmArrayEntry.time().Millisecond");

        Assert.True(!arIter.MoveNext(), "OmmArray with three Time - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Time - hasFixedWidth()");
            Assert.True(ar.FixedWidth == (fixedSize ? 5 : 0), "OmmArray with three Time - getFixedWidth()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Time - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmTimeValue().Hour == 02, "OmmArrayEntry.time().Hour");
            Assert.True(ae1.OmmTimeValue().Minute == 03, "OmmArrayEntry.time().Minute");
            Assert.True(ae1.OmmTimeValue().Second == 04, "OmmArrayEntry.time().Second");
            Assert.True(ae1.OmmTimeValue().Millisecond == 05, "OmmArrayEntry.time().Millisecond");

            Assert.True(arIter.MoveNext(), "OmmArray with three Time - second next()");
            ae2 = arIter.Current;
            Assert.True(ae2.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");
            Assert.True(ae2.OmmTimeValue().Hour == 04, "OmmArrayEntry.time().Hour");
            Assert.True(ae2.OmmTimeValue().Minute == 05, "OmmArrayEntry.time().Minute");
            Assert.True(ae2.OmmTimeValue().Second == 06, "OmmArrayEntry.time().Second");
            Assert.True(ae2.OmmTimeValue().Millisecond == 07, "OmmArrayEntry.time().Millisecond");

            Assert.True(arIter.MoveNext(), "OmmArray with three Time - third next()");
            ae3 = arIter.Current;
            Assert.True(ae3.LoadType == DataType.DataTypes.TIME, "OmmArrayEntry.LoadType == DataType.DataTypes.ENUM");
            Assert.True(ae3.OmmTimeValue().Hour == 14, "OmmArrayEntry.time().Hour");
            Assert.True(ae3.OmmTimeValue().Minute == 15, "OmmArrayEntry.time().Minute");
            Assert.True(ae3.OmmTimeValue().Second == 16, "OmmArrayEntry.time().Second");
            Assert.True(ae3.OmmTimeValue().Millisecond == 17, "OmmArrayEntry.time().Millisecond");

            Assert.False(arIter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayDateTime_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayDateTime_Decode: Decoding OmmArray of DateTime with " + appendText);

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();
        Eta.Codec.Buffer buf = new();

        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 9 : 0; //varying size only
        array.PrimitiveType = DataTypes.DATETIME;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.DateTime dateTime = new();
        dateTime.Year(1111);
        dateTime.Month(11);
        dateTime.Day(1);
        dateTime.Hour(14);
        dateTime.Minute(15);
        dateTime.Second(16);
        dateTime.Millisecond(17);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, dateTime));

        dateTime.Year(2222);
        dateTime.Month(2);
        dateTime.Day(2);
        dateTime.Hour(14);
        dateTime.Minute(15);
        dateTime.Second(16);
        dateTime.Millisecond(17);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, dateTime));

        dateTime.Year(3333);
        dateTime.Month(3);
        dateTime.Day(3);
        dateTime.Hour(14);
        dateTime.Minute(15);
        dateTime.Second(16);
        dateTime.Millisecond(17);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, dateTime));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of DateTimes
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.Equal(ar.HasFixedWidth, fixedSize);
        Assert.Equal(ar.FixedWidth, (fixedSize ? 9 : 0));

        Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.DATETIME, "OmmArrayEntry.LoadType == DataType.DataTypes.DATETIME");

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);
        Assert.Equal(ae1.LoadType, DataType.DataTypes.DATETIME);
        Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
        Assert.Equal(11, ae1.OmmDateTimeValue().Month);
        Assert.Equal(1, ae1.OmmDateTimeValue().Day);
        Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae1.OmmDateTimeValue().Second);
        Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

        Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(ae2.LoadType, DataType.DataTypes.DATETIME);
        Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
        Assert.Equal(2, ae2.OmmDateTimeValue().Month);
        Assert.Equal(2, ae2.OmmDateTimeValue().Day);
        Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae2.OmmDateTimeValue().Second);
        Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

        Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(ae3.LoadType, DataType.DataTypes.DATETIME);
        Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
        Assert.Equal(3, ae3.OmmDateTimeValue().Month);
        Assert.Equal(3, ae3.OmmDateTimeValue().Day);
        Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae3.OmmDateTimeValue().Second);
        Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

        Assert.False(arIter.MoveNext(), "OmmArray with three DateTime - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.Equal(ar.HasFixedWidth, fixedSize);
            Assert.Equal(ar.FixedWidth, (fixedSize ? 9 : 0));

            Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - first next()");
            ae1 = arIter.Current;
            Assert.Equal(ae1.LoadType, DataType.DataTypes.DATETIME);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
            Assert.Equal(11, ae1.OmmDateTimeValue().Month);
            Assert.Equal(1, ae1.OmmDateTimeValue().Day);
            Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae1.OmmDateTimeValue().Second);
            Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

            Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - second next()");
            ae2 = arIter.Current;
            Assert.Equal(ae2.LoadType, DataType.DataTypes.DATETIME);
            Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
            Assert.Equal(2, ae2.OmmDateTimeValue().Month);
            Assert.Equal(2, ae2.OmmDateTimeValue().Day);
            Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae2.OmmDateTimeValue().Second);
            Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

            Assert.True(arIter.MoveNext(), "OmmArray with three DateTime - third next()");
            ae3 = arIter.Current;

            Assert.Equal(ae3.LoadType, DataType.DataTypes.DATETIME);
            Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
            Assert.Equal(3, ae3.OmmDateTimeValue().Month);
            Assert.Equal(3, ae3.OmmDateTimeValue().Day);
            Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae3.OmmDateTimeValue().Second);
            Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayBuffer_Decode()
    {
        output.WriteLine("TestArrayBuffer_Decode: Decoding OmmArray of Buffer with varying size");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.BUFFER;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.Buffer bufText = new();
        bufText.Data("ABC");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("DEFGH");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("KLMNOPQRS");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Buffers
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        // todo: implement ToString method -> output.WriteLine(ar.ToString());

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.True(ar.FixedWidth == 0, "OmmArray with three Buffer - getFixedWidth()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.BUFFER, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));

        Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.BUFFER, ae2.LoadType);
        Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));

        Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.BUFFER, ae3.LoadType);
        Assert.Equal("KLMNOPQRS", Encoding.ASCII.GetString(ae3.OmmBufferValue().Value.Buffer));

        Assert.True(!arIter.MoveNext(), "OmmArray with three Buffer - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.True(ar.FixedWidth == 0, "OmmArray with three Buffer - getFixedWidth()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));

            Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae2.LoadType);
            Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));

            Assert.True(arIter.MoveNext(), "OmmArray with three Buffer - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", Encoding.ASCII.GetString(ae3.OmmBufferValue().Value.Buffer));

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayQos_Decode()
    {
        output.WriteLine("testArrayQos_Decode: Decoding OmmArray of Qos");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.QOS;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Qos qos = new();
        qos.Timeliness(Eta.Codec.QosTimeliness.REALTIME);
        qos.Rate(Eta.Codec.QosRates.TICK_BY_TICK);
        qos.IsDynamic = true;
        qos.RateInfo(0);
        qos.TimeInfo(0);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, qos));

        qos.Timeliness(Eta.Codec.QosTimeliness.REALTIME);
        qos.Rate(Eta.Codec.QosRates.TIME_CONFLATED);
        qos.IsDynamic = true;
        qos.RateInfo(9);
        qos.TimeInfo(0);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, qos));

        qos.Timeliness(Eta.Codec.QosTimeliness.DELAYED);
        qos.Rate(Eta.Codec.QosRates.JIT_CONFLATED);
        qos.IsDynamic = true;
        qos.RateInfo(0);
        qos.TimeInfo(15);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, qos));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Qos
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.True(ar.FixedWidth == 0, "OmmArray with three Qos - getFixedWidth()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Qos - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmQosValue().Timeliness == OmmQos.Timelinesses.REALTIME, "OmmArrayEntry.OmmQosValue().timeliness()");
        Assert.True(ae1.OmmQosValue().Rate == OmmQos.Rates.TICK_BY_TICK, "OmmArrayEntry.OmmQosValue().rate()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Qos - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.True(ae2.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");
        Assert.True(ae2.OmmQosValue().Timeliness == OmmQos.Timelinesses.REALTIME, "OmmArrayEntry.OmmQosValue().timeliness()");
        Assert.True(ae2.OmmQosValue().Rate == 9, "OmmArrayEntry.OmmQosValue().rate()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Qos - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.True(ae3.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");
        Assert.True(ae3.OmmQosValue().Timeliness == 15, "OmmArrayEntry.OmmQosValue().timeliness()");
        Assert.True(ae3.OmmQosValue().Rate == OmmQos.Rates.JUST_IN_TIME_CONFLATED);
        Assert.Equal("Timeliness: 15/JustInTimeConflated", ae3.OmmQosValue().ToString());

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.True(ar.FixedWidth == 0, "OmmArray with three Qos - getFixedWidth()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Qos - first next()");
            ae1 = arIter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmQosValue().Timeliness == OmmQos.Timelinesses.REALTIME, "OmmArrayEntry.OmmQosValue().timeliness()");
            Assert.True(ae1.OmmQosValue().Rate == OmmQos.Rates.TICK_BY_TICK, "OmmArrayEntry.OmmQosValue().rate()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Qos - second next()");
            ae2 = arIter.Current;
            Assert.True(ae2.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");
            Assert.True(ae2.OmmQosValue().Timeliness == OmmQos.Timelinesses.REALTIME, "OmmArrayEntry.OmmQosValue().timeliness()");
            Assert.True(ae2.OmmQosValue().Rate == 9, "OmmArrayEntry.OmmQosValue().rate()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Qos - third next()");
            ae3 = arIter.Current;
            Assert.True(ae3.LoadType == DataType.DataTypes.QOS, "OmmArrayEntry.LoadType == DataType.DataTypes.QOS");
            Assert.True(ae3.OmmQosValue().Timeliness == 15, "OmmArrayEntry.OmmQosValue().timeliness()");
            Assert.True(ae3.OmmQosValue().Rate == OmmQos.Rates.JUST_IN_TIME_CONFLATED, "OmmArrayEntry.OmmQosValue().rate()");

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayState_Decode()
    {
        output.WriteLine("testArrayState_Decode: Decoding OmmArray of State");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.STATE;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.State state = new();
        state.StreamState(Eta.Codec.StreamStates.OPEN);
        state.DataState(Eta.Codec.DataStates.OK);
        state.Code(Eta.Codec.StateCodes.NONE);
        state.Text().Data("Succeeded");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, state));

        state.StreamState(Eta.Codec.StreamStates.CLOSED_RECOVER);
        state.DataState(Eta.Codec.DataStates.SUSPECT);
        state.Code(Eta.Codec.StateCodes.TIMEOUT);
        state.Text().Data("Suspect Data");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, state));

        state.StreamState(Eta.Codec.StreamStates.CLOSED);
        state.DataState(Eta.Codec.DataStates.SUSPECT);
        state.Code(Eta.Codec.StateCodes.USAGE_ERROR);
        state.Text().Data("Usage Error");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, state));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of States
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        Assert.True(arIter.MoveNext(), "OmmArray with three State - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmStateValue().StreamState == OmmState.StreamStates.OPEN, "OmmArrayEntry.OmmStateValue().streamState()");
        Assert.True(ae1.OmmStateValue().DataState == OmmState.DataStates.OK, "OmmArrayEntry.OmmStateValue().dataState()");
        Assert.True(ae1.OmmStateValue().StatusCode == OmmState.StatusCodes.NONE, "OmmArrayEntry.OmmStateValue().statusCode()");
        Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
        Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

        Assert.True(arIter.MoveNext(), "OmmArray with three State - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
        Assert.True(ae2.OmmStateValue().StreamState == OmmState.StreamStates.CLOSED_RECOVER, "OmmArrayEntry.OmmStateValue().streamState()");
        Assert.True(ae2.OmmStateValue().DataState == OmmState.DataStates.SUSPECT, "OmmArrayEntry.OmmStateValue().dataState()");
        Assert.True(ae2.OmmStateValue().StatusCode == OmmState.StatusCodes.TIMEOUT, "OmmArrayEntry.OmmStateValue().statusCode()");
        Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
        Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

        Assert.True(arIter.MoveNext(), "OmmArray with three State - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
        Assert.True(ae3.OmmStateValue().StreamState == OmmState.StreamStates.CLOSED, "OmmArrayEntry.OmmStateValue().streamState()");
        Assert.True(ae3.OmmStateValue().DataState == OmmState.DataStates.SUSPECT, "OmmArrayEntry.OmmStateValue().dataState()");
        Assert.True(ae3.OmmStateValue().StatusCode == OmmState.StatusCodes.USAGE_ERROR, "OmmArrayEntry.OmmStateValue().statusCode()");
        Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
        Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

        Assert.False(arIter.MoveNext());

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext(), "OmmArray with three State - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmStateValue().StreamState == OmmState.StreamStates.OPEN, "OmmArrayEntry.OmmStateValue().streamState()");
            Assert.True(ae1.OmmStateValue().DataState == OmmState.DataStates.OK, "OmmArrayEntry.OmmStateValue().dataState()");
            Assert.True(ae1.OmmStateValue().StatusCode == OmmState.StatusCodes.NONE, "OmmArrayEntry.OmmStateValue().statusCode()");
            Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
            Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

            Assert.True(arIter.MoveNext(), "OmmArray with three State - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
            Assert.True(ae2.OmmStateValue().StreamState == OmmState.StreamStates.CLOSED_RECOVER, "OmmArrayEntry.OmmStateValue().streamState()");
            Assert.True(ae2.OmmStateValue().DataState == OmmState.DataStates.SUSPECT, "OmmArrayEntry.OmmStateValue().dataState()");
            Assert.True(ae2.OmmStateValue().StatusCode == OmmState.StatusCodes.TIMEOUT, "OmmArrayEntry.OmmStateValue().statusCode()");
            Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
            Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

            Assert.True(arIter.MoveNext(), "OmmArray with three State - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
            Assert.True(ae3.OmmStateValue().StreamState == OmmState.StreamStates.CLOSED, "OmmArrayEntry.OmmStateValue().streamState()");
            Assert.True(ae3.OmmStateValue().DataState == OmmState.DataStates.SUSPECT, "OmmArrayEntry.OmmStateValue().dataState()");
            Assert.True(ae3.OmmStateValue().StatusCode == OmmState.StatusCodes.USAGE_ERROR, "OmmArrayEntry.OmmStateValue().statusCode()");
            Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
            Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

            Assert.True(!arIter.MoveNext(), "OmmArray with three State - fourth next()");
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayEnum_Decode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("testArrayEnum_Decode: Decoding OmmArray of Enum with " + appendText);

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = fixedSize ? 2 : 0; //varying size only
        array.PrimitiveType = DataTypes.ENUM;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));
        Eta.Codec.Enum testEnum = new();
        testEnum.Value(29);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, testEnum));

        testEnum.Value(5300);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, testEnum));

        testEnum.Value(8100);
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, testEnum));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Enums
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Enum - hasFixedWidth()");
        Assert.True(ar.FixedWidth == (fixedSize ? 2 : 0), "OmmArray with three Enum - getFixedWidth()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Enum - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.True(ae1.OmmEnumValue().Value == 29, "OmmArrayEntry.enumValue()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Enum - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
        Assert.True(ae2.OmmEnumValue().Value == 5300, "OmmArrayEntry.enumValue()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Enum - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
        Assert.True(ae3.OmmEnumValue().Value == 8100, "OmmArrayEntry.enumValue()");

        Assert.True(!arIter.MoveNext(), "OmmArray with three Enum - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.True(ar.HasFixedWidth == fixedSize, "OmmArray with three Enum - hasFixedWidth()");
            Assert.True(ar.FixedWidth == (fixedSize ? 2 : 0), "OmmArray with three Enum - getFixedWidth()");

            arIter = ar.GetEnumerator();

            Assert.True(arIter.MoveNext(), "OmmArray with three Enum - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.True(ae1.OmmEnumValue().Value == 29, "OmmArrayEntry.enumValue()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Enum - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
            Assert.True(ae2.OmmEnumValue().Value == 5300, "OmmArrayEntry.enumValue()");

            Assert.True(arIter.MoveNext(), "OmmArray with three Enum - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
            Assert.True(ae3.OmmEnumValue().Value == 8100, "OmmArrayEntry.enumValue()");

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayUtf8_Decode()
    {
        output.WriteLine("TestArrayUtf8_Decode: Decoding OmmArray of Utf8");

        Eta.Codec.Array array = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        array.ItemLength = 0; //varying size only
        array.PrimitiveType = DataTypes.UTF8_STRING;

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeInit(iter));

        Eta.Codec.Buffer bufText = new();
        bufText.Data("ABC");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("DEFGH");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Clear();
        bufText.Data("KLMNOPQRS");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, array.EncodeComplete(iter, true));

        //Now do EMA decoding of OmmArray of Utf8
        OmmArray ar = new();
        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), buf, null);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.False(ar.HasFixedWidth);
        Assert.True(ar.FixedWidth == 0, "OmmArray with three Utf8 - getFixedWidth()");

        Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - first next()");
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", Encoding.UTF8.GetString(ae1.OmmUtf8Value().Value.Buffer));

        Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
        Assert.Equal("DEFGH", Encoding.UTF8.GetString(ae2.OmmUtf8Value().Value.Buffer));

        Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
        Assert.Equal("KLMNOPQRS", Encoding.UTF8.GetString(ae3.OmmUtf8Value().Value.Buffer));

        Assert.True(!arIter.MoveNext(), "OmmArray with three Utf8 - fourth next()");

        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            arIter = ar.GetEnumerator();

            Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - first next()");
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", Encoding.UTF8.GetString(ae1.OmmUtf8Value().Value.Buffer));
            Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
            Assert.Equal("DEFGH", Encoding.UTF8.GetString(ae2.OmmUtf8Value().Value.Buffer));

            Assert.True(arIter.MoveNext(), "OmmArray with three Utf8 - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", Encoding.UTF8.GetString(ae3.OmmUtf8Value().Value.Buffer));

            Assert.False(arIter.MoveNext());
        }
    }

    [Fact]
    public void TestArrayRmtes_Decode()
    {
        Eta.Codec.Array rsslArray = new();
        EncodeIterator iter = new();
        ArrayEntry arrayEntry = new();

        Eta.Codec.Buffer rsslBuf = new();
        rsslBuf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        iter.SetBufferAndRWFVersion(rsslBuf, Codec.MajorVersion(), Codec.MinorVersion());

        rsslArray.ItemLength = 0; // varying size only
        rsslArray.PrimitiveType = DataTypes.RMTES_STRING;

        Assert.Equal(CodecReturnCode.SUCCESS, rsslArray.EncodeInit(iter));

        LSEG.Eta.Codec.Buffer bufText = new();
        bufText.Data("ABC");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Data("DEFGH");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        bufText.Data("KLMNOPQRS");
        Assert.Equal(CodecReturnCode.SUCCESS, arrayEntry.Encode(iter, bufText));

        Assert.Equal(CodecReturnCode.SUCCESS, rsslArray.EncodeComplete(iter, true));

        // Now do EMA decoding of OmmArray of Rmtes
        OmmArray ar = new();

        ar.SetRsslData(Codec.MajorVersion(), Codec.MinorVersion(), rsslBuf, null);

        Assert.False(ar.HasFixedWidth);
        Assert.Equal(0, ar.FixedWidth);

        IEnumerator<OmmArrayEntry> arIter = ar.GetEnumerator();

        Assert.True(arIter.MoveNext());
        OmmArrayEntry ae1 = arIter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae1.Load.DataType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", ae1.OmmRmtesValue().Value.ToString());

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - second next()");
        OmmArrayEntry ae2 = arIter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae2.Load.DataType);
        Assert.Equal("DEFGH", ae2.OmmRmtesValue().Value.ToString());

        Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - third next()");
        OmmArrayEntry ae3 = arIter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae3.Load.DataType);
        Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().Value.ToString());

        Assert.False(arIter.MoveNext(), "OmmArray with three Ascii - fourth next()");

        // second pass, enumerator must start from the beginning
        arIter = ar.GetEnumerator();
        {
            Assert.False(ar.HasFixedWidth);
            Assert.Equal(0, ar.FixedWidth);

            Assert.True(arIter.MoveNext());
            ae1 = arIter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae1.Load.DataType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", ae1.OmmRmtesValue().Value.ToString());

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - second next()");
            ae2 = arIter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae2.Load.DataType);
            Assert.Equal("DEFGH", ae2.OmmRmtesValue().Value.ToString());

            Assert.True(arIter.MoveNext(), "OmmArray with three Ascii - third next()");
            ae3 = arIter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae3.Load.DataType);
            Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().Value.ToString());

            Assert.False(arIter.MoveNext(), "OmmArray with three Ascii - fourth next()");
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayInt_Encode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayInt_Encode: Encoding Int OmmArray with " + appendText);

        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 4;

        encArray.AddInt(-11).AddInt(22).AddInt(-33).Complete();

        // Now do ETA decoding of OmmArray of Ints
        Eta.Codec.Array array = new();
        ArrayEntry arEntry = new();
        Eta.Codec.Int intValue = new();

        DecodeIterator decodeIter = new();
        decodeIter.SetBufferAndRWFVersion(encArray.Encoder!.m_encodeIterator!.Buffer(), Codec.MajorVersion(), Codec.MinorVersion());
        array.Decode(decodeIter);

        Assert.Equal(DataTypes.INT, array.PrimitiveType);
        Assert.Equal((fixedSize ? 4 : 0), array.ItemLength);

        arEntry.Decode(decodeIter);
        intValue.Decode(decodeIter);
        Assert.Equal(-11, intValue.ToLong());

        arEntry.Decode(decodeIter);
        intValue.Decode(decodeIter);
        Assert.Equal(22, intValue.ToLong());

        arEntry.Decode(decodeIter);
        intValue.Decode(decodeIter);
        Assert.Equal(-33, intValue.ToLong());
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayBuffer_EncodeDecode(bool fixedSize)
    {
        string appendText = fixedSize ? "fixed size" : "varying size";
        output.WriteLine("TestArrayBuffer_EncodeDecode: Encode and Decode OmmArray Buffer  with " + appendText);

        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 8;

        EmaBuffer buff = new EmaBuffer(Encoding.ASCII.GetBytes("ABC"));
        encArray.AddBuffer(buff);

        buff.CopyFrom(Encoding.ASCII.GetBytes("DEFGH"));
        encArray.AddBuffer(buff);

        buff.CopyFrom(Encoding.ASCII.GetBytes("KLMNOPQRS"));
        if (fixedSize)
            Assert.Throws<OmmInvalidUsageException>(() => encArray.AddBuffer(buff));
        else
            encArray.AddBuffer(buff);

        encArray.Complete();

        // decode

        OmmArray decArray = new();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        decArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, decArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), decArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = decArray.GetEnumerator();

        Assert.True(iter.MoveNext(), "OmmArray with three Buffer - first next()");
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.BUFFER, ae1.LoadType);
        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        EmaBuffer emaBuffer = ae1.OmmBufferValue().Value;

        if (fixedSize)
        {
            Assert.Equal(8, emaBuffer.Length);
            Assert.StartsWith("ABC", Encoding.ASCII.GetString(emaBuffer.Buffer));
        }
        else
            Assert.Equal("ABC", Encoding.ASCII.GetString(emaBuffer.Buffer));

        Assert.True(iter.MoveNext(), "OmmArray with three Buffer - second next()");
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(ae2.LoadType, DataType.DataTypes.BUFFER);

        emaBuffer = ae2.OmmBufferValue().Value;

        if (fixedSize)
        {
            Assert.Equal(8, emaBuffer.Length);
            Assert.StartsWith("DEFGH", Encoding.ASCII.GetString(emaBuffer.Buffer));
        }
        else
        {
            Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));
        }

        OmmArrayEntry ae3;

        if (!fixedSize)
        {
            Assert.True(iter.MoveNext(), "OmmArray with three Buffer - third next()");
            ae3 = iter.Current;
            Assert.True(new EmaBuffer(Encoding.ASCII.GetBytes("KLMNOPQRS")).Equals(ae3.OmmBufferValue().Value));
        }

        Assert.False(iter.MoveNext());

        iter = decArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, decArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), decArray.FixedWidth);

            Assert.True(iter.MoveNext(), "OmmArray with three Buffer - first next()");
            ae1 = iter.Current;

            Assert.Equal(DataType.DataTypes.BUFFER, ae1.LoadType);
            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            if (fixedSize)
            {
                Assert.StartsWith("DEFGH", Encoding.ASCII.GetString(emaBuffer.Buffer));
            }
            else
            {
                Assert.Equal("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));
            }

            Assert.True(iter.MoveNext(), "OmmArray with three Buffer - second next()");
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae2.LoadType);

            if (fixedSize)
            {
                Assert.StartsWith("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));
            }
            else
            {
                Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));
            }

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext(), "OmmArray with three Buffer - third next()");
                ae3 = iter.Current;
                Assert.Equal("KLMNOPQRS", Encoding.ASCII.GetString(ae3.OmmBufferValue().Value.Buffer));
            }

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        //Encoding (including blanks)
        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("ABC")));

        if (fixedSize)
        {
           var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeBuffer()); // Blank buffer
           Assert.Equal("Failed to encode (BUFFER) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeBuffer(); // Blank buffer
        }

        encArray.AddBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("DEFGH")));

        if(!fixedSize)
            encArray.AddCodeBuffer(); // Blank buffer

        if (fixedSize)
            Assert.Throws<OmmInvalidUsageException>(() => encArray.AddBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("KLMNOPQRS"))));
        else
            encArray.AddBuffer(new EmaBuffer(Encoding.ASCII.GetBytes("KLMNOPQRS")));

        encArray.Complete();

        decArray = new();

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        decArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        iter = decArray.GetEnumerator();

        Assert.Equal(fixedSize, decArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), decArray.FixedWidth);

        Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - first next()");

        ae1 = iter.Current;
        Assert.True(ae1.LoadType == DataType.DataTypes.BUFFER, "OmmArrayEntry.LoadType == DataType.DataTypes.BUFFER");
        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        if(fixedSize)
            Assert.StartsWith("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));
        else
            Assert.Equal("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));

        OmmArrayEntry ae1b;

        if (!fixedSize)
        {
            Assert.True(iter.MoveNext(), "OmmArray with three Buffer  (with 2 blanks)- second next()");
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
        }

        Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - third next()");
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.BUFFER, ae2.LoadType);

        if(fixedSize)
            Assert.StartsWith("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));
        else
            Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - fourth next()");
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae2b.LoadType);

            Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - fifth next()");
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", Encoding.ASCII.GetString(ae3.OmmBufferValue().Value.Buffer));
        }

        Assert.False(iter.MoveNext());

        iter = decArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, decArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), decArray.FixedWidth);

            Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - first next()");

            ae1 = iter.Current;
            Assert.True(ae1.LoadType == DataType.DataTypes.BUFFER, "OmmArrayEntry.LoadType == DataType.DataTypes.BUFFER");
            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            if (fixedSize)
            {
                Assert.StartsWith("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));
            }
            else
            {
                Assert.Equal("ABC", Encoding.ASCII.GetString(ae1.OmmBufferValue().Value.Buffer));
            }

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - second next()");
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.BUFFER, ae1b.LoadType);
            }

            Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - third next()");
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.BUFFER, ae2.LoadType);

            if(fixedSize)
                Assert.StartsWith("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));
            else
                Assert.Equal("DEFGH", Encoding.ASCII.GetString(ae2.OmmBufferValue().Value.Buffer));

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - fourth next()");
                ae2b = iter.Current;
                Assert.True(ae2b.LoadType == DataType.DataTypes.BUFFER, "OmmArrayEntry.LoadType == DataType.DataTypes.BUFFER");

                Assert.True(iter.MoveNext(), "OmmArray with three Buffer (with 2 blanks) - fifth next()");
                ae3 = iter.Current;
                Assert.Equal(DataType.DataTypes.BUFFER, ae3.LoadType);
                Assert.Equal("KLMNOPQRS", Encoding.ASCII.GetString(ae3.OmmBufferValue().Value.Buffer));
            }

            Assert.False(iter.MoveNext());
        }
    }

#if false
    [Fact]
    public void TestArray_EncodeETA_DecodeEMA_EncodeEMA_DecodeETA()
    {
        output.WriteLine("testArray_EncodeETA_DecodeEMA_EncodeEMA_DecodeETA: Encode Array with ETA, Decode Array with EMA, Encode Array with EMA, Decode Array with ETA");

        // Create a ETA Buffer to encode into
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(1024));

        // Encode Array with ETA.
        CodecReturnCode retVal;
        if ((retVal = TestUtilities.eta_EncodeArrayAll(buf)) < CodecReturnCode.SUCCESS)
        {
            output.WriteLine("Error encoding array.");
            output.WriteLine("Error " + retVal.GetAsString() + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeArrayAll.  " + "Error Text: "
                    + retVal.GetAsInfo());
            return;
        }

        // Decode Array with EMA.
        OmmArray array = JUnitTestConnect.createOmmArray();
        JUnitTestConnect.setRsslData(array, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);

        TestUtilities.EmaDecode_ETAArrayAll(array);

        // Copy decoded entries into a different Array with EMA
        OmmArray arrayCopy = new();
        if (array.HasFixedWidth)
        {
            arrayCopy.FixedWidth = array.FixedWidth;
        }
        IEnumerator<OmmArrayEntry> iterator = array.GetEnumerator();
        while (iterator.MoveNext())
        {
            OmmArrayEntry arrayEntry = iterator.Current;
            arrayCopy.add(arrayEntry);
        }

        // decode array copy
        OmmArray arrayDecCopy = JUnitTestConnect.createOmmArray();
        JUnitTestConnect.setRsslData(arrayDecCopy, arrayCopy, Codec.MajorVersion(), Codec.MinorVersion(), null, null);

        // compare with original
        TestUtilities.EmaDecode_ETAArrayAll(arrayDecCopy);

        output.WriteLine("\ntestArray_EncodeETA_DecodeEMA_EncodeEMA_DecodeETA passed");
    }
#endif

    [Fact]
    public void TestArray_Error()
    {
        {
            OmmArray arr = new();
            Assert.Throws<OmmInvalidUsageException>(() => arr.Complete());
        }

        {
            OmmArray arr = new();
            arr.AddAscii("entry 1");
            Assert.Throws<OmmInvalidUsageException>(() => arr.AddUInt(123));
            arr.Complete();
        }

        {
            OmmArray arr = new();
            arr.AddAscii("entry 1");
            Assert.Throws<OmmInvalidUsageException>(() => arr.AddCodeUInt());
            arr.Complete();
        }

        {
            OmmArray arr = new();
            double d1 = 1.0;
            arr.AddDouble(d1);
            Assert.Throws<OmmInvalidUsageException>(() => arr.AddRealFromDouble(d1));
            arr.Complete();
        }

        {
            OmmArray arr = new();
            arr.AddCodeUInt();
            Assert.Throws<OmmInvalidUsageException>(() => arr.AddAscii("entry 1"));
            arr.Complete();
        }

        {
            OmmArray arr = new OmmArray();
            arr.AddAscii("entry 1");
            arr.AddAscii("entry 2");
            arr.Complete();

            Assert.Throws<OmmInvalidUsageException>(() => arr.AddAscii("entry 3"));
        }

        {
            OmmArray arr = new();
            arr.AddAscii("entry 1");
            arr.AddAscii("entry 2");
            arr.Complete();

            arr.Clear();

            arr.AddAscii("entry 3");
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayInt_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddInt(-11).AddInt(22).AddInt(-33).Complete();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(-11, ae1.OmmIntValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
        Assert.Equal(22, ae1.OmmIntValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
        Assert.Equal(-33, ae1.OmmIntValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(-11, ae1.OmmIntValue().Value);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
            Assert.Equal(22, ae1.OmmIntValue().Value);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
            Assert.Equal(-33, ae1.OmmIntValue().Value);

            Assert.False(iter.MoveNext());
        }

        // second round
        encArray.Clear();

        if (fixedSize)
        {
            encArray.FixedWidth = 8;
        }

        encArray.AddInt(-11);

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeInt()); // Blank buffer
            Assert.Equal("Failed to encode (INT) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeInt();
        }

        encArray.AddInt(22);

        if (!fixedSize)
            encArray.AddCodeInt();

        encArray.AddInt(-33);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(-11, ae1.OmmIntValue().Value);

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            Assert.Equal(DataType.DataTypes.INT, ae1b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
        Assert.Equal(22, ae2.OmmIntValue().Value);

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
            Assert.Equal(DataType.DataTypes.INT, ae2b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
        Assert.Equal(-33, ae3.OmmIntValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            iter = encArray.GetEnumerator();

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(-11, ae1.OmmIntValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.INT, ae1b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae2.LoadType);
            Assert.Equal(22, ae1.OmmIntValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.INT, ae2b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.INT, ae3.LoadType);
            Assert.Equal(-33, ae1.OmmIntValue().Value);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void TestArrayUInt_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddUInt(11).AddUInt(22).AddUInt(33).Complete();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.UINT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(11ul, ae1.OmmUIntValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.UINT, ae2.LoadType);
        Assert.Equal(22ul, ae1.OmmUIntValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.UINT, ae3.LoadType);
        Assert.Equal(33ul, ae1.OmmUIntValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(11ul, ae1.OmmUIntValue().Value);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae2.LoadType);
            Assert.Equal(22ul, ae1.OmmUIntValue().Value);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae3.LoadType);
            Assert.Equal(33ul, ae1.OmmUIntValue().Value);

            Assert.False(iter.MoveNext());
        }

        // second round
        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddUInt(11);

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeUInt()); // Blank buffer
            Assert.Equal("Failed to encode (UINT) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeUInt();
        }

        encArray.AddUInt(22);

        if (!fixedSize)
            encArray.AddCodeUInt();

        encArray.AddUInt(33);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(Data.DataCode.NO_CODE, ae1.Code);
        Assert.Equal(DataType.DataTypes.UINT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(11ul, ae1.OmmUIntValue().Value);

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            Assert.Equal(DataType.DataTypes.UINT, ae1b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.UINT, ae2.LoadType);
        Assert.Equal(22ul, ae2.OmmUIntValue().Value);

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
            Assert.Equal(DataType.DataTypes.UINT, ae2b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.UINT, ae3.LoadType);
        Assert.Equal(33ul, ae3.OmmUIntValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            iter = encArray.GetEnumerator();

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(11ul, ae1.OmmUIntValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
                Assert.Equal(DataType.DataTypes.UINT, ae1b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae2.LoadType);
            Assert.Equal(22ul, ae1.OmmUIntValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
                Assert.Equal(DataType.DataTypes.UINT, ae2b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.UINT, ae3.LoadType);
            Assert.Equal(33ul, ae1.OmmUIntValue().Value);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayFloat_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 4;

        encArray.AddFloat(-11.11f).AddFloat(22.22f).AddFloat(-33.33f).Complete();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.FLOAT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(-11.11f, ae1.OmmFloatValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.FLOAT, ae2.LoadType);
        Assert.Equal(22.22f, ae1.OmmFloatValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.FLOAT, ae3.LoadType);
        Assert.Equal(-33.33f, ae1.OmmFloatValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(-11.11f, ae1.OmmFloatValue().Value);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae2.LoadType);
            Assert.Equal(22.22f, ae1.OmmFloatValue().Value);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae3.LoadType);
            Assert.Equal(-33.33f, ae1.OmmFloatValue().Value);

            Assert.False(iter.MoveNext());
        }

        // second round
        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 4;

        encArray.AddFloat(-11.11f);

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeFloat()); // Blank buffer
            Assert.Equal("Failed to encode (FLOAT) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeFloat();
        }

        encArray.AddFloat(22.22f);

        if (!fixedSize)
            encArray.AddCodeFloat();

        encArray.AddFloat(-33.33f);
        encArray.Complete();

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        ae1 = iter.Current;

        Assert.Equal(DataType.DataTypes.FLOAT, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(-11.11f, ae1.OmmFloatValue().Value);

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.FLOAT, ae2.LoadType);
        Assert.Equal(22.22f, ae2.OmmFloatValue().Value);

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
        }

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.FLOAT, ae3.LoadType);
        Assert.Equal(-33.33f, ae3.OmmFloatValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

            iter = encArray.GetEnumerator();

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(-11.11f, ae1.OmmFloatValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.FLOAT, ae1b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae2.LoadType);
            Assert.Equal(22.22f, ae1.OmmFloatValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.FLOAT, ae2b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
            }

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.FLOAT, ae3.LoadType);
            Assert.Equal(-33.33f, ae1.OmmFloatValue().Value);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayDouble_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddDouble(-11.1111).AddDouble(22.2222).AddDouble(-33.3333).Complete();

        // Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(-11.1111, ae1.OmmDoubleValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
        Assert.Equal(22.2222, ae1.OmmDoubleValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
        Assert.Equal(-33.3333, ae1.OmmDoubleValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmTimeValue().Hour);

            Assert.Equal(-11.1111, ae1.OmmDoubleValue().Value);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
            Assert.Equal(22.2222, ae1.OmmDoubleValue().Value);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
            Assert.Equal(-33.3333, ae1.OmmDoubleValue().Value);

            Assert.False(iter.MoveNext());
        }

        // second round
        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddDouble(-11.1111);

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeDouble()); // Blank buffer
            Assert.Equal("Failed to encode (DOUBLE) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeDouble();
        }

        encArray.AddDouble(22.2222);

        if (!fixedSize)
            encArray.AddCodeDouble();

        encArray.AddDouble(-33.3333);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        ae1 = iter.Current;

        Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmEnumValue().Value);

        Assert.Equal(-11.1111, ae1.OmmDoubleValue().Value);

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae1b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
        Assert.Equal(22.2222, ae2.OmmDoubleValue().Value);

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae2b.LoadType);
        }

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
        Assert.Equal(-33.3333, ae3.OmmDoubleValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            iter = encArray.GetEnumerator();

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(-11.1111, ae1.OmmDoubleValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.DOUBLE, ae1b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae2.LoadType);
            Assert.Equal(22.2222, ae1.OmmDoubleValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.DOUBLE, ae2b.LoadType);
            }

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DOUBLE, ae3.LoadType);
            Assert.Equal(-33.3333, ae1.OmmDoubleValue().Value);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayReal_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();
        EncodeIterator encIter = new();
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        encIter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        encArray.StartEncoding(encIter);

        if (fixedSize)
            encArray.FixedWidth = 8;

        try
        {
            encArray
                .AddReal(11, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                .AddReal(22, OmmReal.MagnitudeTypes.DIVISOR_2)
                .AddReal(-33, OmmReal.MagnitudeTypes.DIVISOR_2)
                .Complete();
        }
        catch(OmmInvalidUsageException exp)
        {
            if(fixedSize)
            {
                Assert.Equal($"Unsupported FixedWidth encoding in AddReal(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
                return;
            }

            Assert.False(true);
        }

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(11, ae1.OmmRealValue().Mantissa);
        Assert.Equal(OmmReal.MagnitudeTypes.EXPONENT_NEG_2, ae1.OmmRealValue().MagnitudeType);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
        Assert.Equal(22, ae2.OmmRealValue().Mantissa);
        Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae2.OmmRealValue().MagnitudeType);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
        Assert.Equal(-33, ae3.OmmRealValue().Mantissa);
        Assert.Equal(OmmReal.MagnitudeTypes.DIVISOR_2, ae3.OmmRealValue().MagnitudeType);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmTimeValue().Hour);

            Assert.Equal(11, ae1.OmmRealValue().Mantissa);
            Assert.Equal(OmmReal.MagnitudeTypes.EXPONENT_NEG_2, ae1.OmmRealValue().MagnitudeType);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
            Assert.Equal(22, ae1.OmmRealValue().Mantissa);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
            Assert.Equal(-33, ae1.OmmRealValue().Mantissa);

            Assert.False(iter.MoveNext());
        }

        // second round
        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddReal(11, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        encArray.AddCodeReal();
        encArray.AddReal(22, OmmReal.MagnitudeTypes.DIVISOR_2);
        encArray.AddCodeReal();
        encArray.AddReal(-33, OmmReal.MagnitudeTypes.DIVISOR_2);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        ae1 = iter.Current;

        Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmEnumValue().Value);

        Assert.Equal(11, ae1.OmmRealValue().Mantissa);
        Assert.Equal(OmmReal.MagnitudeTypes.EXPONENT_NEG_2, ae1.OmmRealValue().MagnitudeType);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae1b.LoadType);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
        Assert.Equal(22, ae2.OmmRealValue().Mantissa);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae2b.LoadType);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
        Assert.Equal(-33, ae3.OmmRealValue().Mantissa);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(11, ae1.OmmRealValue().Mantissa);

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae1b.LoadType);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae2.LoadType);
            Assert.Equal(22, ae2.OmmRealValue().Mantissa);

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae2b.LoadType);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.REAL, ae3.LoadType);
            Assert.Equal(-33, ae3.OmmRealValue().Mantissa);

            Assert.False(iter.MoveNext());
        }
    }

    // todo: this test uses FilterList and Map containers for testing, implement it once
    // this branch is merged with the containers branch

    // public void TestArrayAscii_EncodeDecode( bool fixedSize )

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayDate_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        //Encoding
        if (fixedSize)
            encArray.FixedWidth = 4;

        encArray.AddDate(1111, 11, 1);
        encArray.AddDate(2222, 2, 2);
        encArray.AddDate(3333, 3, 3);
        encArray.Complete();

        //Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(1111, ae1.OmmDateValue().Year);
        Assert.Equal(11, ae1.OmmDateValue().Month);
        Assert.Equal(1, ae1.OmmDateValue().Day);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae2.LoadType);
        Assert.Equal(2222, ae2.OmmDateValue().Year);
        Assert.Equal(2, ae2.OmmDateValue().Month);
        Assert.Equal(2, ae2.OmmDateValue().Day);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae3.LoadType);
        Assert.Equal(3333, ae3.OmmDateValue().Year);
        Assert.Equal(3, ae3.OmmDateValue().Month);
        Assert.Equal(3, ae3.OmmDateValue().Day);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(1111, ae1.OmmDateValue().Year);
            Assert.Equal(11, ae1.OmmDateValue().Month);
            Assert.Equal(1, ae1.OmmDateValue().Day);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae2.LoadType);
            Assert.Equal(2222, ae2.OmmDateValue().Year);
            Assert.Equal(2, ae2.OmmDateValue().Month);
            Assert.Equal(2, ae2.OmmDateValue().Day);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae3.LoadType);
            Assert.Equal(3333, ae3.OmmDateValue().Year);
            Assert.Equal(3, ae3.OmmDateValue().Month);
            Assert.Equal(3, ae3.OmmDateValue().Day);

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        //Encoding (including blanks)
        if (fixedSize)
            encArray.FixedWidth = 4;

        encArray.AddDate(1111, 11, 1);
        encArray.AddCodeDate();
        encArray.AddDate(2222, 2, 2);
        encArray.AddCodeDate();
        encArray.AddDate(3333, 3, 3);
        encArray.Complete();

        //Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(1111, ae1.OmmDateValue().Year);
        Assert.Equal(11, ae1.OmmDateValue().Month);
        Assert.Equal(1, ae1.OmmDateValue().Day);

        OmmArrayEntry ae1b;
        Assert.True(iter.MoveNext());
        ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae1b.LoadType);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae2.LoadType);
        Assert.Equal(2222, ae2.OmmDateValue().Year);
        Assert.Equal(2, ae2.OmmDateValue().Month);
        Assert.Equal(2, ae2.OmmDateValue().Day);

        OmmArrayEntry ae2b;
        Assert.True(iter.MoveNext());
        ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae2b.LoadType);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATE, ae3.LoadType);
        Assert.Equal(3333, ae3.OmmDateValue().Year);
        Assert.Equal(3, ae3.OmmDateValue().Month);
        Assert.Equal(3, ae3.OmmDateValue().Day);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 4 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(1111, ae1.OmmDateValue().Year);
            Assert.Equal(11, ae1.OmmDateValue().Month);
            Assert.Equal(1, ae1.OmmDateValue().Day);

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae1b.LoadType);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae2.LoadType);
            Assert.Equal(2222, ae2.OmmDateValue().Year);
            Assert.Equal(2, ae2.OmmDateValue().Month);
            Assert.Equal(2, ae2.OmmDateValue().Day);

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae2b.LoadType);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATE, ae3.LoadType);
            Assert.Equal(3333, ae3.OmmDateValue().Year);
            Assert.Equal(3, ae3.OmmDateValue().Month);
            Assert.Equal(3, ae3.OmmDateValue().Day);

            Assert.False(iter.MoveNext());
        }
    }


    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayTime_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        EncodeIterator encIter = new();
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        encIter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        encArray.StartEncoding(encIter);

        //Encoding
        if (fixedSize)
            encArray.FixedWidth = 8;

        try
        {
            encArray.AddTime(02, 03, 04, 05);
        }
        catch(OmmInvalidUsageException exp)
        {
            if(fixedSize)
            {
                Assert.Equal($"Unsupported FixedWidth encoding in AddTime(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
                return;
            }

            Assert.False(true);
        }

        encArray.AddTime(04, 05, 06, 07);
        encArray.AddTime(14, 15, 16, 17);
        encArray.Complete();

        // Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(02, ae1.OmmTimeValue().Hour);
        Assert.Equal(03, ae1.OmmTimeValue().Minute);
        Assert.Equal(04, ae1.OmmTimeValue().Second);
        Assert.Equal(05, ae1.OmmTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae2.LoadType);
        Assert.Equal(04, ae2.OmmTimeValue().Hour);
        Assert.Equal(05, ae2.OmmTimeValue().Minute);
        Assert.Equal(06, ae2.OmmTimeValue().Second);
        Assert.Equal(07, ae2.OmmTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae3.LoadType);
        Assert.Equal(14, ae3.OmmTimeValue().Hour);
        Assert.Equal(15, ae3.OmmTimeValue().Minute);
        Assert.Equal(16, ae3.OmmTimeValue().Second);
        Assert.Equal(17, ae3.OmmTimeValue().Millisecond);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(02, ae1.OmmTimeValue().Hour);
            Assert.Equal(03, ae1.OmmTimeValue().Minute);
            Assert.Equal(04, ae1.OmmTimeValue().Second);
            Assert.Equal(05, ae1.OmmTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae2.LoadType);
            Assert.Equal(04, ae2.OmmTimeValue().Hour);
            Assert.Equal(05, ae2.OmmTimeValue().Minute);
            Assert.Equal(06, ae2.OmmTimeValue().Second);
            Assert.Equal(07, ae2.OmmTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae3.LoadType);
            Assert.Equal(14, ae3.OmmTimeValue().Hour);
            Assert.Equal(15, ae3.OmmTimeValue().Minute);
            Assert.Equal(16, ae3.OmmTimeValue().Second);
            Assert.Equal(17, ae3.OmmTimeValue().Millisecond);

            Assert.False(iter.MoveNext());

        }

        encArray.Clear();

        //Encoding
        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddTime(02, 03, 04, 05);
        encArray.AddCodeTime();
        encArray.AddTime(04, 05, 06, 07);
        encArray.AddCodeTime();
        encArray.AddTime(14, 15, 16, 17);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(02, ae1.OmmTimeValue().Hour);
        Assert.Equal(03, ae1.OmmTimeValue().Minute);
        Assert.Equal(04, ae1.OmmTimeValue().Second);
        Assert.Equal(05, ae1.OmmTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae1b.LoadType);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae2.LoadType);
        Assert.Equal(04, ae2.OmmTimeValue().Hour);
        Assert.Equal(05, ae2.OmmTimeValue().Minute);
        Assert.Equal(06, ae2.OmmTimeValue().Second);
        Assert.Equal(07, ae2.OmmTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae2b.LoadType);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.TIME, ae3.LoadType);
        Assert.Equal(14, ae3.OmmTimeValue().Hour);
        Assert.Equal(15, ae3.OmmTimeValue().Minute);
        Assert.Equal(16, ae3.OmmTimeValue().Second);
        Assert.Equal(17, ae3.OmmTimeValue().Millisecond);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(02, ae1.OmmTimeValue().Hour);
            Assert.Equal(03, ae1.OmmTimeValue().Minute);
            Assert.Equal(04, ae1.OmmTimeValue().Second);
            Assert.Equal(05, ae1.OmmTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae1b.LoadType);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae2.LoadType);
            Assert.Equal(04, ae2.OmmTimeValue().Hour);
            Assert.Equal(05, ae2.OmmTimeValue().Minute);
            Assert.Equal(06, ae2.OmmTimeValue().Second);
            Assert.Equal(07, ae2.OmmTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae2b.LoadType);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.TIME, ae3.LoadType);
            Assert.Equal(14, ae3.OmmTimeValue().Hour);
            Assert.Equal(15, ae3.OmmTimeValue().Minute);
            Assert.Equal(16, ae3.OmmTimeValue().Second);
            Assert.Equal(17, ae3.OmmTimeValue().Millisecond);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayDateTime_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        EncodeIterator encIter = new();
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        encIter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        encArray.StartEncoding(encIter);

        //Encoding
        if (fixedSize)
            encArray.FixedWidth = 9;

        encArray.AddDateTime(1111, 11, 1, 14, 15, 16, 17);
        encArray.AddDateTime(2222, 2, 2, 14, 15, 16, 17);
        encArray.AddDateTime(3333, 3, 3, 14, 15, 16, 17);

        encArray.Complete();

        //Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 9 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
        Assert.Equal(11, ae1.OmmDateTimeValue().Month);
        Assert.Equal(1, ae1.OmmDateTimeValue().Day);
        Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae1.OmmDateTimeValue().Second);
        Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae2.LoadType);
        Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
        Assert.Equal(2, ae2.OmmDateTimeValue().Month);
        Assert.Equal(2, ae2.OmmDateTimeValue().Day);
        Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae2.OmmDateTimeValue().Second);
        Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae3.LoadType);
        Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
        Assert.Equal(3, ae3.OmmDateTimeValue().Month);
        Assert.Equal(3, ae3.OmmDateTimeValue().Day);
        Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae3.OmmDateTimeValue().Second);
        Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 9 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
            Assert.Equal(11, ae1.OmmDateTimeValue().Month);
            Assert.Equal(1, ae1.OmmDateTimeValue().Day);
            Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae1.OmmDateTimeValue().Second);
            Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae2.LoadType);
            Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
            Assert.Equal(2, ae2.OmmDateTimeValue().Month);
            Assert.Equal(2, ae2.OmmDateTimeValue().Day);
            Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae2.OmmDateTimeValue().Second);
            Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae3.LoadType);
            Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
            Assert.Equal(3, ae3.OmmDateTimeValue().Month);
            Assert.Equal(3, ae3.OmmDateTimeValue().Day);
            Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae3.OmmDateTimeValue().Second);
            Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        //Encoding
        if (fixedSize)
            encArray.FixedWidth = 9;

        encArray.AddDateTime(1111, 11, 1, 14, 15, 16, 17);
        
        encArray.AddCodeDateTime();

        encArray.AddDateTime(2222, 2, 2, 14, 15, 16, 17);

        encArray.AddCodeDateTime();

        encArray.AddDateTime(3333, 3, 3, 14, 15, 16, 17);

        encArray.Complete();

        //Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 9 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

        Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
        Assert.Equal(11, ae1.OmmDateTimeValue().Month);
        Assert.Equal(1, ae1.OmmDateTimeValue().Day);
        Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae1.OmmDateTimeValue().Second);
        Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

        OmmArrayEntry ae1b;

        Assert.True(iter.MoveNext());
        ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae1b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae1b.Code);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae2.LoadType);
        Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
        Assert.Equal(2, ae2.OmmDateTimeValue().Month);
        Assert.Equal(2, ae2.OmmDateTimeValue().Day);
        Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae2.OmmDateTimeValue().Second);
        Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

        OmmArrayEntry ae2b;

        Assert.True(iter.MoveNext());
        ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae2b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.DATETIME, ae3.LoadType);
        Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
        Assert.Equal(3, ae3.OmmDateTimeValue().Month);
        Assert.Equal(3, ae3.OmmDateTimeValue().Day);
        Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
        Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
        Assert.Equal(16, ae3.OmmDateTimeValue().Second);
        Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 9 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmIntValue().Value);

            Assert.Equal(1111, ae1.OmmDateTimeValue().Year);
            Assert.Equal(11, ae1.OmmDateTimeValue().Month);
            Assert.Equal(1, ae1.OmmDateTimeValue().Day);
            Assert.Equal(14, ae1.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae1.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae1.OmmDateTimeValue().Second);
            Assert.Equal(17, ae1.OmmDateTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae1b.LoadType);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae2.LoadType);
            Assert.Equal(2222, ae2.OmmDateTimeValue().Year);
            Assert.Equal(2, ae2.OmmDateTimeValue().Month);
            Assert.Equal(2, ae2.OmmDateTimeValue().Day);
            Assert.Equal(14, ae2.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae2.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae2.OmmDateTimeValue().Second);
            Assert.Equal(17, ae2.OmmDateTimeValue().Millisecond);

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae2b.LoadType);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.DATETIME, ae3.LoadType);
            Assert.Equal(3333, ae3.OmmDateTimeValue().Year);
            Assert.Equal(3, ae3.OmmDateTimeValue().Month);
            Assert.Equal(3, ae3.OmmDateTimeValue().Day);
            Assert.Equal(14, ae3.OmmDateTimeValue().Hour);
            Assert.Equal(15, ae3.OmmDateTimeValue().Minute);
            Assert.Equal(16, ae3.OmmDateTimeValue().Second);
            Assert.Equal(17, ae3.OmmDateTimeValue().Millisecond);

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayQos_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        // Encoding
        if (fixedSize)
            encArray.FixedWidth = 16;

        try
        {
            encArray.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
        }
        catch(OmmInvalidUsageException exp)
        {
            /* QoS doesn't support fixed length */
            if(fixedSize)
            {
                Assert.Equal($"Unsupported FixedWidth encoding in AddQos(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
                return;
            }
            else
            {
                Assert.False(true);
            }
        }

        encArray.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED);
        encArray.AddQos(500u, 900u);
        encArray.AddQos(OmmQos.Timelinesses.INEXACT_DELAYED, 659);
        encArray.AddQos(938u, OmmQos.Rates.JUST_IN_TIME_CONFLATED);
        encArray.AddQos(70000u, 80000u);
        encArray.Complete();

        // Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 16 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmStateValue().DataType);

        Assert.Equal(OmmQos.Timelinesses.REALTIME, ae1.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.TICK_BY_TICK, ae1.OmmQosValue().Rate);
        Assert.Equal("RealTime", ae1.OmmQosValue().TimelinessAsString());
        Assert.Equal("TickByTick", ae1.OmmQosValue().RateAsString());
        Assert.Equal("RealTime/TickByTick", ae1.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae2.LoadType);
        Assert.Equal(OmmQos.Timelinesses.REALTIME, ae2.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae2.OmmQosValue().Rate);
        Assert.Equal("RealTime", ae2.OmmQosValue().TimelinessAsString());
        Assert.Equal("JustInTimeConflated", ae2.OmmQosValue().RateAsString());
        Assert.Equal("RealTime/JustInTimeConflated", ae2.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae3.LoadType);
        Assert.Equal(500u, ae3.OmmQosValue().Timeliness);
        Assert.Equal(900u, ae3.OmmQosValue().Rate);
        Assert.Equal("Timeliness: 500", ae3.OmmQosValue().TimelinessAsString());
        Assert.Equal("Rate: 900", ae3.OmmQosValue().RateAsString());
        Assert.Equal("Timeliness: 500/Rate: 900", ae3.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae4 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae4.LoadType);
        Assert.Equal(OmmQos.Timelinesses.INEXACT_DELAYED, ae4.OmmQosValue().Timeliness);
        Assert.Equal(659u, ae4.OmmQosValue().Rate);
        Assert.Equal("InexactDelayed", ae4.OmmQosValue().TimelinessAsString());
        Assert.Equal("Rate: 659", ae4.OmmQosValue().RateAsString());
        Assert.Equal("InexactDelayed/Rate: 659", ae4.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae5 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae5.LoadType);
        Assert.Equal(938u, ae5.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae5.OmmQosValue().Rate);
        Assert.Equal("Timeliness: 938", ae5.OmmQosValue().TimelinessAsString());
        Assert.Equal("JustInTimeConflated", ae5.OmmQosValue().RateAsString());
        Assert.Equal("Timeliness: 938/JustInTimeConflated", ae5.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae6 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae6.LoadType);
        Assert.Equal(OmmQos.Timelinesses.INEXACT_DELAYED, ae6.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae6.OmmQosValue().Rate);
        Assert.Equal("InexactDelayed", ae6.OmmQosValue().TimelinessAsString());
        Assert.Equal("JustInTimeConflated", ae6.OmmQosValue().RateAsString());
        Assert.Equal("InexactDelayed/JustInTimeConflated", ae6.OmmQosValue().ToString());

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();

        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 16 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmStateValue().DataType);

            Assert.Equal(OmmQos.Timelinesses.REALTIME, ae1.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.TICK_BY_TICK, ae1.OmmQosValue().Rate);
            Assert.Equal("RealTime", ae1.OmmQosValue().TimelinessAsString());
            Assert.Equal("TickByTick", ae1.OmmQosValue().RateAsString());
            Assert.Equal("RealTime/TickByTick", ae1.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae2.LoadType);
            Assert.Equal(OmmQos.Timelinesses.REALTIME, ae2.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae2.OmmQosValue().Rate);
            Assert.Equal("RealTime", ae2.OmmQosValue().TimelinessAsString());
            Assert.Equal("JustInTimeConflated", ae2.OmmQosValue().RateAsString());
            Assert.Equal("RealTime/JustInTimeConflated", ae2.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae3.LoadType);
            Assert.Equal(500u, ae3.OmmQosValue().Timeliness);
            Assert.Equal(900u, ae3.OmmQosValue().Rate);
            Assert.Equal("Timeliness: 500", ae3.OmmQosValue().TimelinessAsString());
            Assert.Equal("Rate: 900", ae3.OmmQosValue().RateAsString());
            Assert.Equal("Timeliness: 500/Rate: 900", ae3.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae4 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae4.LoadType);
            Assert.Equal(OmmQos.Timelinesses.INEXACT_DELAYED, ae4.OmmQosValue().Timeliness);
            Assert.Equal(659u, ae4.OmmQosValue().Rate);
            Assert.Equal("InexactDelayed", ae4.OmmQosValue().TimelinessAsString());
            Assert.Equal("Rate: 659", ae4.OmmQosValue().RateAsString());
            Assert.Equal("InexactDelayed/Rate: 659", ae4.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae5 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae5.LoadType);
            Assert.Equal(938u, ae5.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae5.OmmQosValue().Rate);
            Assert.Equal("Timeliness: 938", ae5.OmmQosValue().TimelinessAsString());
            Assert.Equal("JustInTimeConflated", ae5.OmmQosValue().RateAsString());
            Assert.Equal("Timeliness: 938/JustInTimeConflated", ae5.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae6 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae6.LoadType);
            Assert.Equal(OmmQos.Timelinesses.INEXACT_DELAYED, ae6.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae6.OmmQosValue().Rate);
            Assert.Equal("InexactDelayed", ae6.OmmQosValue().TimelinessAsString());
            Assert.Equal("JustInTimeConflated", ae6.OmmQosValue().RateAsString());
            Assert.Equal("InexactDelayed/JustInTimeConflated", ae6.OmmQosValue().ToString());

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        // Encoding (including blanks)
        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
        encArray.AddCodeQos();
        encArray.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.JUST_IN_TIME_CONFLATED);
        encArray.AddCodeQos();
        encArray.AddQos(555u, 7777u);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmStateValue().DataType);

        Assert.Equal(OmmQos.Timelinesses.REALTIME, ae1.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.TICK_BY_TICK, ae1.OmmQosValue().Rate);
        Assert.Equal("RealTime", ae1.OmmQosValue().TimelinessAsString());
        Assert.Equal("TickByTick", ae1.OmmQosValue().RateAsString());
        Assert.Equal("RealTime/TickByTick", ae1.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae1b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae1b.Code);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae2.LoadType);
        Assert.Equal(OmmQos.Timelinesses.REALTIME, ae2.OmmQosValue().Timeliness);
        Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae2.OmmQosValue().Rate);
        Assert.Equal("RealTime", ae2.OmmQosValue().TimelinessAsString());
        Assert.Equal("JustInTimeConflated", ae2.OmmQosValue().RateAsString());
        Assert.Equal("RealTime/JustInTimeConflated", ae2.OmmQosValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae2b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.QOS, ae3.LoadType);
        Assert.Equal(555u, ae3.OmmQosValue().Timeliness);
        Assert.Equal(7777u, ae3.OmmQosValue().Rate);
        Assert.Equal("Timeliness: 555", ae3.OmmQosValue().TimelinessAsString());
        Assert.Equal("Rate: 7777", ae3.OmmQosValue().RateAsString());
        Assert.Equal("Timeliness: 555/Rate: 7777", ae3.OmmQosValue().ToString());

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            iter = encArray.GetEnumerator();

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmStateValue().DataType);

            Assert.Equal(OmmQos.Timelinesses.REALTIME, ae1.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.TICK_BY_TICK, ae1.OmmQosValue().Rate);
            Assert.Equal("RealTime", ae1.OmmQosValue().TimelinessAsString());
            Assert.Equal("TickByTick", ae1.OmmQosValue().RateAsString());
            Assert.Equal("RealTime/TickByTick", ae1.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae2.LoadType);
            Assert.Equal(OmmQos.Timelinesses.REALTIME, ae2.OmmQosValue().Timeliness);
            Assert.Equal(OmmQos.Rates.JUST_IN_TIME_CONFLATED, ae2.OmmQosValue().Rate);
            Assert.Equal("RealTime", ae2.OmmQosValue().TimelinessAsString());
            Assert.Equal("JustInTimeConflated", ae2.OmmQosValue().RateAsString());
            Assert.Equal("RealTime/JustInTimeConflated", ae2.OmmQosValue().ToString());

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.QOS, ae3.LoadType);
            Assert.Equal(555u, ae3.OmmQosValue().Timeliness);
            Assert.Equal(7777u, ae3.OmmQosValue().Rate);
            Assert.Equal("Timeliness: 555", ae3.OmmQosValue().TimelinessAsString());
            Assert.Equal("Rate: 7777", ae3.OmmQosValue().RateAsString());
            Assert.Equal("Timeliness: 555/Rate: 7777", ae3.OmmQosValue().ToString());

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayState_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        // Encoding

        if (fixedSize)
            encArray.FixedWidth = 8;

        try
        {
            encArray.AddState(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Succeeded");
        }
        catch(OmmInvalidUsageException exp)
        {
            if(fixedSize)
            {
                Assert.Equal($"Unsupported FixedWidth encoding in AddState(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
                return;
            }

            Assert.False(true);
        }

        encArray.AddState(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.TIMEOUT, "Suspect Data");
        encArray.AddState(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.USAGE_ERROR, "Usage Error");
        encArray.Complete();

        // Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(OmmState.StreamStates.OPEN, ae1.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.OK, ae1.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, ae1.OmmStateValue().StatusCode);
        Assert.Equal("None", ae1.OmmStateValue().StatusCodeAsString());
        Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
        Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
        Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, ae2.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.SUSPECT, ae2.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.TIMEOUT, ae2.OmmStateValue().StatusCode);
        Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
        Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
        Assert.Equal(OmmState.StreamStates.CLOSED, ae3.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.SUSPECT, ae3.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, ae3.OmmStateValue().StatusCode);
        Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
        Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());

            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(OmmState.StreamStates.OPEN, ae1.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.OK, ae1.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.NONE, ae1.OmmStateValue().StatusCode);
            Assert.Equal("None", ae1.OmmStateValue().StatusCodeAsString());
            Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
            Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
            Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, ae2.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.SUSPECT, ae2.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.TIMEOUT, ae2.OmmStateValue().StatusCode);
            Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
            Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
            Assert.Equal(OmmState.StreamStates.CLOSED, ae3.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.SUSPECT, ae3.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, ae3.OmmStateValue().StatusCode);
            Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
            Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        // Encoding (including blanks)
        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddState(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Succeeded");
        encArray.AddCodeState();
        encArray.AddState(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.TIMEOUT, "Suspect Data");
        encArray.AddCodeState();
        encArray.AddState(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.USAGE_ERROR, "Usage Error");
        encArray.Complete();

        //Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(OmmState.StreamStates.OPEN, ae1.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.OK, ae1.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.NONE, ae1.OmmStateValue().StatusCode);
        Assert.Equal("None", ae1.OmmStateValue().StatusCodeAsString());
        Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
        Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1b = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae1b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae1b.Code);

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
        Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, ae2.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.SUSPECT, ae2.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.TIMEOUT, ae2.OmmStateValue().StatusCode);
        Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
        Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2b = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae2b.LoadType);
        Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
        Assert.Equal(OmmState.StreamStates.CLOSED, ae3.OmmStateValue().StreamState);
        Assert.Equal(OmmState.DataStates.SUSPECT, ae3.OmmStateValue().DataState);
        Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, ae3.OmmStateValue().StatusCode);
        Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
        Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(OmmState.StreamStates.OPEN, ae1.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.OK, ae1.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.NONE, ae1.OmmStateValue().StatusCode);
            Assert.Equal("None", ae1.OmmStateValue().StatusCodeAsString());
            Assert.Equal("Succeeded", ae1.OmmStateValue().StatusText);
            Assert.Equal("Open / Ok / None / 'Succeeded'", ae1.OmmStateValue().ToString());

            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae2.LoadType);
            Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, ae2.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.SUSPECT, ae2.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.TIMEOUT, ae2.OmmStateValue().StatusCode);
            Assert.Equal("Suspect Data", ae2.OmmStateValue().StatusText);
            Assert.Equal("Closed, Recoverable / Suspect / Timeout / 'Suspect Data'", ae2.OmmStateValue().ToString());

            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.STATE, ae3.LoadType);
            Assert.Equal(OmmState.StreamStates.CLOSED, ae3.OmmStateValue().StreamState);
            Assert.Equal(OmmState.DataStates.SUSPECT, ae3.OmmStateValue().DataState);
            Assert.Equal(OmmState.StatusCodes.USAGE_ERROR, ae3.OmmStateValue().StatusCode);
            Assert.Equal("Usage Error", ae3.OmmStateValue().StatusText);
            Assert.Equal("Closed / Suspect / Usage error / 'Usage Error'", ae3.OmmStateValue().ToString());

            Assert.False(iter.MoveNext());
        }

        if (fixedSize)
            Assert.False(true, "Encode OmmArray State with blanks - exception expected");
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayEnum_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        // Encoding
        if (fixedSize)
            encArray.FixedWidth = 2;

        encArray.AddEnum(29);
        encArray.AddEnum(5300);
        encArray.AddEnum(8100);
        encArray.Complete();

        // Decoding

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 2 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(29, ae1.OmmEnumValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
        Assert.Equal(5300, ae2.OmmEnumValue().Value);

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
        Assert.Equal(8100, ae3.OmmEnumValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 2 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());

            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(29, ae1.OmmEnumValue().Value);

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
            Assert.Equal(5300, ae2.OmmEnumValue().Value);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
            Assert.Equal(8100, ae3.OmmEnumValue().Value);

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        //Encoding (including blanks)
        if (fixedSize)
            encArray.FixedWidth = 2;

        encArray.AddEnum(29);

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeEnum()); // Blank buffer
            Assert.Equal("Failed to encode (ENUM) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeEnum();
        }

        encArray.AddEnum(5300);

        if(!fixedSize)
            encArray.AddCodeEnum();
        encArray.AddEnum(8100);
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 2 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());

        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal(29, ae1.OmmEnumValue().Value);

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
        Assert.Equal(5300, ae2.OmmEnumValue().Value);

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
        }

        Assert.True(iter.MoveNext());
        ae3 = iter.Current;
        Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
        Assert.Equal(8100, ae3.OmmEnumValue().Value);

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 2 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());

            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal(29, ae1.OmmEnumValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.ENUM, ae1b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae2.LoadType);
            Assert.Equal(5300, ae2.OmmEnumValue().Value);

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.ENUM, ae2b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae2b.Code);
            }

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.ENUM, ae3.LoadType);
            Assert.Equal(8100, ae3.OmmEnumValue().Value);

            Assert.False(iter.MoveNext());

            iter = encArray.GetEnumerator();
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayUtf8_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("ABC")));
        encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("DEFGH")));

        try
        {
            encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("KLMNOPQRS")));
        }
        catch(OmmInvalidUsageException exp)
        {
            if (fixedSize)
            {
                Assert.Equal($"Passed in value is longer than fixed width in AddUtf8(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
            }
            else
            {
                Assert.False(true);
            }
        }

        encArray.Complete();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.StartsWith("ABC", ae1.OmmUtf8Value().AsString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
        Assert.StartsWith("DEFGH", ae2.OmmUtf8Value().AsString());

        OmmArrayEntry ae3;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmUtf8Value().AsString());
        }

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();

        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.StartsWith("ABC", ae1.OmmUtf8Value().AsString());

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
            Assert.StartsWith("DEFGH", ae2.OmmUtf8Value().AsString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae3 = iter.Current;
                Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
                Assert.Equal("KLMNOPQRS", ae3.OmmUtf8Value().AsString());
            }

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("ABC")));

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeUtf8()); // Blank buffer
            Assert.Equal("Failed to encode (UTF8_STRING) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeUtf8();
        }
         
        encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("DEFGH")));
        if (!fixedSize)
        {
            encArray.AddCodeUtf8();
            encArray.AddUtf8(new EmaBuffer(Encoding.UTF8.GetBytes("KLMNOPQRS")));
        }
        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.StartsWith("ABC", ae1.OmmUtf8Value().AsString());

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
        Assert.StartsWith("DEFGH", ae2.OmmUtf8Value().AsString());

        OmmArrayEntry ae2b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmUtf8Value().AsString());
        }

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.StartsWith("ABC", ae1.OmmUtf8Value().AsString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.UTF8, ae2.LoadType);
            Assert.StartsWith("DEFGH", ae2.OmmUtf8Value().AsString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.UTF8, ae2b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

                Assert.True(iter.MoveNext());
                ae3 = iter.Current;
                Assert.Equal(DataType.DataTypes.UTF8, ae3.LoadType);
                Assert.Equal("KLMNOPQRS", ae3.OmmUtf8Value().AsString());
            }

            Assert.False(iter.MoveNext());
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void TestArrayRmtes_EncodeDecode(bool fixedSize)
    {
        OmmArray encArray = new();

        EncodeIterator encIter = new();
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(DEFAULT_BUFFER_SIZE));

        encIter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        encArray.StartEncoding(encIter);

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("ABC")));
        encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("DEFGH")));

        try
        {
            encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("KLMNOPQRS")));
        }
        catch(OmmInvalidUsageException exp)
        {
            if(fixedSize)
            {
                Assert.Equal($"Passed in value is longer than fixed width in AddRmtes(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
            }
            else
            {
                Assert.False(true);
            }
        }

        encArray.Complete();

        DecodeIterator decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

        IEnumerator<OmmArrayEntry> iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", ae1.OmmRmtesValue().ToString());

        Assert.True(iter.MoveNext());
        OmmArrayEntry ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae2.LoadType);
        Assert.Equal("DEFGH", ae2.OmmRmtesValue().ToString());

        OmmArrayEntry ae3;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().ToString());
        }

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal((fixedSize ? 8 : 0), encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", ae1.OmmRmtesValue().ToString());

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae2.LoadType);
            Assert.Equal("DEFGH", ae2.OmmRmtesValue().ToString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae3 = iter.Current;
                Assert.Equal(DataType.DataTypes.RMTES, ae3.LoadType);
                Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().ToString());
            }

            Assert.False(iter.MoveNext());
        }

        encArray.Clear();

        if (fixedSize)
            encArray.FixedWidth = 8;

        encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("ABC")));

        if (fixedSize)
        {
            var caughtException = Assert.Throws<OmmInvalidUsageException>(() => encArray.AddCodeRmtes()); // Blank buffer
            Assert.Equal("Failed to encode (RMTES_STRING) while encoding OmmArray. Reason='INVALID_ARGUMENT'", caughtException.Message);
        }
        else
        {
            encArray.AddCodeRmtes();
        }

        encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("DEFGH")));

        if (!fixedSize)
            encArray.AddCodeRmtes();

        try
        {
            encArray.AddRmtes(new EmaBuffer(Encoding.ASCII.GetBytes("KLMNOPQRS")));
        }
        catch (OmmInvalidUsageException exp)
        {
            if (fixedSize)
            {
                Assert.Equal($"Passed in value is longer than fixed width in AddRmtes(). Fixed width='{encArray.FixedWidth}'.", exp.Message);
            }
            else
            {
                Assert.False(true);
            }
        }

        encArray.Complete();

        // Decoding

        decodeIter = new DecodeIterator();
        decodeIter.SetBufferAndRWFVersion(encArray!.Encoder!.m_encodeIterator!.Buffer(),
            Codec.MajorVersion(), Codec.MinorVersion());
        encArray.SetRsslData(decodeIter, encArray!.Encoder!.m_encodeIterator!.Buffer());

        Assert.Equal(fixedSize, encArray.HasFixedWidth);
        Assert.Equal(fixedSize ? 8 : 0, encArray.FixedWidth);

        iter = encArray.GetEnumerator();

        Assert.True(iter.MoveNext());
        ae1 = iter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae1.LoadType);

        Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

        Assert.Equal("ABC", ae1.OmmRmtesValue().ToString());

        OmmArrayEntry ae1b;
        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae1b = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae1b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
        }

        Assert.True(iter.MoveNext());
        ae2 = iter.Current;
        Assert.Equal(DataType.DataTypes.RMTES, ae2.LoadType);
        Assert.Equal("DEFGH", ae2.OmmRmtesValue().ToString());

        OmmArrayEntry ae2b;

        if (!fixedSize)
        {
            Assert.True(iter.MoveNext());
            ae2b = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae2b.LoadType);
            Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

            Assert.True(iter.MoveNext());
            ae3 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae3.LoadType);
            Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().ToString());
        }

        Assert.False(iter.MoveNext());

        iter = encArray.GetEnumerator();
        {
            Assert.Equal(fixedSize, encArray.HasFixedWidth);
            Assert.Equal(fixedSize ? 8 : 0, encArray.FixedWidth);

            Assert.True(iter.MoveNext());
            ae1 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae1.LoadType);

            Assert.Throws<OmmInvalidUsageException>(() => ae1.OmmUIntValue().Value);

            Assert.Equal("ABC", ae1.OmmRmtesValue().ToString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae1b = iter.Current;
                Assert.Equal(DataType.DataTypes.RMTES, ae1b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae1b.Code);
            }

            Assert.True(iter.MoveNext());
            ae2 = iter.Current;
            Assert.Equal(DataType.DataTypes.RMTES, ae2.LoadType);
            Assert.Equal("DEFGH", ae2.OmmRmtesValue().ToString());

            if (!fixedSize)
            {
                Assert.True(iter.MoveNext());
                ae2b = iter.Current;
                Assert.Equal(DataType.DataTypes.RMTES, ae2b.LoadType);
                Assert.Equal(Data.DataCode.BLANK, ae2b.Code);

                Assert.True(iter.MoveNext());
                ae3 = iter.Current;
                Assert.Equal(DataType.DataTypes.RMTES, ae3.LoadType);
                Assert.Equal("KLMNOPQRS", ae3.OmmRmtesValue().ToString());
            }

            Assert.False(iter.MoveNext());
        }
    }

    /// <summary>
    /// Check that OmmArrayEntry can recover from error codes.
    /// </summary>
    [Fact]
    public void OmmArrayEntryErrorHandling_Test()
    {
        OmmInt ommInt = new OmmInt();
        OmmArrayEntry entry = new OmmArrayEntry(ommInt);

        Assert.NotNull(entry.Load);

        entry.Error(OmmError.ErrorCodes.NO_SET_DEFINITION);
        Assert.NotEqual(Data.DataCode.BLANK, entry.Code);
        Assert.Equal(OmmError.ErrorCodes.NO_SET_DEFINITION, entry.OmmErrorValue().ErrorCode);
        Assert.Throws<OmmInvalidUsageException>(() => entry.OmmIntValue().Value);

        entry.LoadType = DataTypes.INT;
        Assert.Equal(DataTypes.INT, entry.LoadType);
    }
}
