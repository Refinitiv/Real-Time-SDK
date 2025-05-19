/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Tests;

namespace LSEG.Ema.Access.Tests
{
    public class ElementListTest
    {
        [Fact]
        public void ElementListDecodingTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleElementList(encodeIterator, new int[] { DataTypes.INT, DataTypes.UINT });

            ElementList elementList = manager.GetOmmElementList();
            elementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null);
            Assert.Equal(Access.DataType.DataTypes.ELEMENT_LIST, elementList.DataType);

            var enumerator = elementList.GetEnumerator();
            
            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.Equal(CodecTestUtil.intName.ToString(), element.Name);
            var load1 = element.OmmIntValue();
            Assert.Equal(CodecTestUtil.iv.ToLong(), load1.Value);
            
            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.UINT, element.LoadType);
            Assert.Equal(CodecTestUtil.uintName.ToString(), element.Name);
            var load2 = element.OmmUIntValue();
            Assert.Equal((ulong)CodecTestUtil.uintv.ToLong(), load2.Value);
            Assert.False(enumerator.MoveNext());
        }

        [Fact]
        public void ElementListDecodingNestedElementListTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleElementList(encodeIterator, new int[] { DataTypes.ELEMENT_LIST, DataTypes.ELEMENT_LIST });

            ElementList elementList = manager.GetOmmElementList();
            elementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null);

            var enumerator = elementList.GetEnumerator();
            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            
            var load1 = element.ElementList();
            var enumerator1 = load1.GetEnumerator();
            Assert.True(enumerator1.MoveNext());
            var element1 = enumerator1.Current;
            Assert.Equal(DataType.DataTypes.INT, element1.LoadType);

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            enumerator1 = load1.GetEnumerator();
            Assert.True(enumerator1.MoveNext());
            element1 = enumerator1.Current;
            Assert.Equal(DataType.DataTypes.INT, element1.LoadType);
            Assert.False(enumerator.MoveNext());
        }
        
        [Fact]
        public void ElementListDecodingEnumeratorResetTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleElementList(encodeIterator, new int[] { DataTypes.INT, DataTypes.UINT });

            ElementList elementList = manager.GetOmmElementList();
            elementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null);
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, elementList.DataType);

            var enumerator = elementList.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.Equal(CodecTestUtil.intName.ToString(), element.Name);
            var load1 = element.OmmIntValue();
            Assert.Equal(CodecTestUtil.iv.ToLong(), load1.Value);

            enumerator.Reset();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.Equal(CodecTestUtil.intName.ToString(), element.Name);
            load1 = element.OmmIntValue();
            Assert.Equal(CodecTestUtil.iv.ToLong(), load1.Value);

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.UINT, element.LoadType);
            Assert.Equal(CodecTestUtil.uintName.ToString(), element.Name);
            var load2 = element.OmmUIntValue();
            Assert.Equal((ulong)CodecTestUtil.uintv.ToLong(), load2.Value);
            Assert.False(enumerator.MoveNext());
        }

        [Fact]
        public void EncodeAndDecodeElementListTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            ElementList elementList = manager.GetOmmElementList();
            elementList.AddInt("first", 25);
            elementList.AddInt("second", 0);
            elementList.MarkForClear().Complete();

            var buffer = elementList!.Encoder!.m_encodeIterator!.Buffer();
            elementList.DecodeElementList(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null);

            Assert.Equal(Access.DataType.DataTypes.ELEMENT_LIST, elementList.DataType);

            var enumerator = elementList.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.Equal("first", element.Name);
            var load1 = element.OmmIntValue();
            Assert.Equal(25, load1.Value);

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.INT, element.LoadType);
            Assert.Equal("second", element.Name);
            var load2 = element.OmmIntValue();
            Assert.Equal(0, load2.Value);
            Assert.False(enumerator.MoveNext());

            elementList.ClearAndReturnToPool_All();
        }

    }
}
