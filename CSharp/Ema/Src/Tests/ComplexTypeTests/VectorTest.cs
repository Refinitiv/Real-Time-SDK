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
    public class VectorTest
    {
        [Fact]
        public void VectorDecodingTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleVector(encodeIterator, DataTypes.ELEMENT_LIST,
                true,
                true,
                true, new VectorEntryActions[] { VectorEntryActions.INSERT, VectorEntryActions.UPDATE },
                new bool[] { false, false }); 

            Vector vector = manager.GetOmmVector();
            Assert.Equal(CodecReturnCode.SUCCESS, vector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.VECTOR, vector.DataType);
            Assert.Equal(DataTypes.ELEMENT_LIST, vector.SummaryData().DataType);

            var enumerator = vector.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.INSERT, element.Action);
            Assert.False(element.HasPermissionData);

            var load1 = element.ElementList();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.UPDATE, element.Action);
            Assert.False(element.HasPermissionData);
            var load2 = element.ElementList();

            Assert.False(enumerator.MoveNext());
        }

        [Fact]
        public void VectorDecodingEnumeratorResetTest()
        {
            EmaObjectManager manager = new EmaObjectManager();

            Buffer buffer = new Buffer();
            buffer.Data(new ByteBuffer(1024));

            EncodeIterator encodeIterator = new EncodeIterator();
            encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());

            CodecTestUtil.EncodeSimpleVector(encodeIterator, DataTypes.ELEMENT_LIST,
                true,
                true,
                true, new VectorEntryActions[] { VectorEntryActions.INSERT, VectorEntryActions.UPDATE },
                new bool[] { false, false });

            Vector vector = manager.GetOmmVector();
            Assert.Equal(CodecReturnCode.SUCCESS, vector.DecodeVector(Codec.MajorVersion(), Codec.MinorVersion(), buffer, null, null));
            Assert.Equal(Access.DataType.DataTypes.VECTOR, vector.DataType);
            Assert.Equal(DataTypes.ELEMENT_LIST, vector.SummaryData().DataType);

            var enumerator = vector.GetEnumerator();

            Assert.True(enumerator.MoveNext());
            var element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.INSERT, element.Action);
            Assert.False(element.HasPermissionData);

            var load1 = element.ElementList();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.UPDATE, element.Action);
            Assert.False(element.HasPermissionData);
            var load2 = element.ElementList();

            Assert.False(enumerator.MoveNext());
            
            enumerator.Reset();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.INSERT, element.Action);
            Assert.False(element.HasPermissionData);

            load1 = element.ElementList();

            Assert.True(enumerator.MoveNext());
            element = enumerator.Current;
            Assert.Equal(DataType.DataTypes.ELEMENT_LIST, element.LoadType);
            Assert.Equal(VectorAction.UPDATE, element.Action);
            Assert.False(element.HasPermissionData);
            load2 = element.ElementList();
        }
    }
}
