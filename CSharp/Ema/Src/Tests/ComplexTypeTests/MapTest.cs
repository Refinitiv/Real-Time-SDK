/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Tests;

namespace LSEG.Ema.Access.Tests
{
    public class MapTest : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
        }

        [Fact]
        public void MapDecodingTest()
        {
            EmaObjectManager objectManager = new EmaObjectManager();
            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleMap(encodeIterator,
                DataTypes.ELEMENT_LIST,
                new MapEntryActions[] { MapEntryActions.ADD, MapEntryActions.DELETE },
                new bool[] { false, false },
                DataTypes.INT,
                true,
                true,
                true);

            Map map = objectManager.GetOmmMap();
            Assert.Equal(CodecReturnCode.SUCCESS, map.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.MAP, map.DataType);
            Assert.Equal(DataTypes.ELEMENT_LIST, map.SummaryData().DataType);

            var enumerator = map.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(MapAction.ADD, element.Action);
            Assert.False(element.HasPermissionData);
            
            var load1 = element.ElementList();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.NO_DATA, element.LoadType);
            Assert.Equal(MapAction.DELETE, element.Action);
            Assert.False(element.HasPermissionData);
            
            Assert.False(enumerator.MoveNext());
        }

        [Fact]
        public void MapDecodingEnumeratorResetTest()
        {
            EmaObjectManager objectManager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleMap(encodeIterator,
                DataTypes.ELEMENT_LIST,
                new MapEntryActions[] { MapEntryActions.ADD, MapEntryActions.DELETE },
                new bool[] { false, false },
                DataTypes.INT,
                true,
                true,
                true);

            Map map = objectManager.GetOmmMap();
            Assert.Equal(CodecReturnCode.SUCCESS, map.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.MAP, map.DataType);
            Assert.Equal(DataTypes.ELEMENT_LIST, map.SummaryData().DataType);

            var enumerator = map.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(MapAction.ADD, element.Action);
            Assert.False(element.HasPermissionData);

            var load1 = element.ElementList();

            enumerator.Reset();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(MapAction.ADD, element.Action);
            Assert.False(element.HasPermissionData);

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.NO_DATA, element.LoadType);
            Assert.Equal(MapAction.DELETE, element.Action);
            Assert.False(element.HasPermissionData);
        }

        [Fact]
        public void MapEncodeAndDecodeTest()
        {
            EmaObjectManager objectManager = new EmaObjectManager();
            Map map = objectManager.GetOmmMap();

            map.KeyType(DataTypes.INT);
            
            ElementList elementList = objectManager.GetOmmElementList();
            elementList.AddInt("first", 1);
            elementList.MarkForClear().Complete();
            map.SummaryData(elementList);

            map.AddKeyInt(2, MapAction.ADD, elementList);
            map.AddKeyInt(3, MapAction.DELETE, elementList);
            map.MarkForClear().Complete();

            var buffer = map!.Encoder!.m_encodeIterator!.Buffer();

            Assert.Equal(CodecReturnCode.SUCCESS, map.DecodeMap(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.MAP, map.DataType);
            Assert.Equal(DataTypes.ELEMENT_LIST, map.SummaryData().DataType);

            var enumerator = map.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(MapAction.ADD, element.Action);
            Assert.False(element.HasPermissionData);

            var load1 = element.ElementList();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.NO_DATA, element.LoadType);
            Assert.Equal(MapAction.DELETE, element.Action);
            Assert.False(element.HasPermissionData);

            Assert.False(enumerator.MoveNext());

            elementList.ClearAndReturnToPool_All();
            map.ClearAndReturnToPool_All();
            enumerator.Dispose();
        }
    }
}
