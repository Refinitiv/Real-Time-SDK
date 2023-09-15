/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Ema.Access.Tests;


public class AccessTests
{
    /// <summary>
    /// do the very basic test
    /// </summary>
    [Fact]
    public void SimpleIntWrite_Test()
    {
        OmmArray arr = new();
        Assert.Equal(DataType.DataTypes.ARRAY, arr.DataType);

        arr.AddInt(1);
        arr.Complete();
        arr.Clear();
        Assert.True(true);
    }

    [Fact(Skip = "Incomplete test")]
    public void IntWriteRead_Test()
    {
        OmmArray arr = new();
        for (int i = 0; i < 10; i++)
        {
            arr.AddInt(i);
        }

        arr.Complete();

        foreach (OmmArrayEntry entry in arr)
        {
            Assert.NotEqual(Data.DataCode.BLANK, entry.Code);
        }
    }

    /// <summary>
    /// Test automatic underlying buffer enlargement by the OmmArray implementation.
    /// </summary>
    /// <remarks>When the underlying ETA buffer runs out of space for encoding OmmArray
    /// contents, it should be automatically enlarged.</remarks>
    /// <param name="initialBufferSize"></param>
    [Theory(Skip = "Incomplete test")]
    [InlineData(0)]
    [InlineData(2)]
    [InlineData(3)]
    [InlineData(11)]
    [InlineData(4096)]
    public void RealignArrayBuffer_Test(int initialBufferSize)
    {
        // test that the OmmArray automatically adjusts underlying buffer size
        OmmArray encArray = new();
        EncodeIterator encIter = new();
        Eta.Codec.Buffer buf = new();
        buf.Data(new ByteBuffer(initialBufferSize));

        encIter.SetBufferAndRWFVersion(buf, Codec.MajorVersion(), Codec.MinorVersion());

        encArray.StartEncoding(encIter);

        int itemsCount = 10_000;

        for (int i = 0; i < itemsCount; i++)
        {
            encArray.AddInt(i);
        }

        encArray.Complete();

        int count = 0;

        foreach (var entry in encArray)
        {
            Assert.NotEqual(Data.DataCode.BLANK, entry.Code);
            Assert.Equal(count, entry.OmmIntValue().Value);
            count++;
        }

        Assert.Equal(itemsCount, count);
    }
}
