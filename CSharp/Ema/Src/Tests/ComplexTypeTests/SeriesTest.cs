/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Tests;

namespace LSEG.Ema.Access.Tests
{
    public class SeriesTest
    {
        [Fact]
        public void SeriesDecodingTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleSeries(encodeIterator, DataTypes.MAP, true, false, 2, 2);

            Series series = manager.GetOmmSeries();
            Assert.Equal(CodecReturnCode.SUCCESS, series.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.SERIES, series.DataType);
            Assert.True(series.HasTotalCountHint);

            var enumerator = series.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            var load1 = element.Map();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            var load2 = element.Map();

            Assert.False(enumerator.MoveNext());
        }

        [Fact]
        public void SeriesDecodingEnumeratorResetTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleSeries(encodeIterator, DataTypes.MAP, true, false, 2, 2);

            Series series = manager.GetOmmSeries();
            Assert.Equal(CodecReturnCode.SUCCESS, series.DecodeSeries(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.SERIES, series.DataType);
            Assert.True(series.HasTotalCountHint);

            var enumerator = series.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            var load1 = element.Map();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            var load2 = element.Map();

            Assert.False(enumerator.MoveNext());

            enumerator.Reset();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            load1 = element.Map();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.MAP, element.LoadType);
            load2 = element.Map();

            Assert.False(enumerator.MoveNext());
        }
    }
}
