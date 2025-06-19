/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.IO;
using System.Net;

using Xunit;
using Xunit.Categories;

using LSEG.Eta.Common;

namespace CodecTestProject
{
    [Category("ByteBuffer")]
    public class ByteBufferTest
    {
        [Fact]
        [Category("Unit")]
        public void ConstructorTest()
        {
            int capacity = 500;
            ByteBuffer byteBuffer = new ByteBuffer(capacity);
            Assert.Equal(capacity, byteBuffer.Capacity);
            Assert.Equal(0, byteBuffer.Position);
            Assert.Equal(capacity, byteBuffer.Limit);
            Assert.Equal(0, byteBuffer.Begin);
            Assert.Equal(capacity, byteBuffer.Remaining);
            Assert.True(byteBuffer.HasRemaining);
        }

        [Fact]
        [Category("Unit")]
        public void FlipTest()
        {
            byte[] bArrayData = new byte[] { 65, 66, 67 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            Assert.Equal(5, byteBuffer1.Capacity);
            Assert.Equal(5, byteBuffer1.Limit);
            Assert.Equal(bArrayData.Length, byteBuffer1.Position);

            // Flip position to read data from the buffer
            byteBuffer1.Flip();

            Assert.Equal(5, byteBuffer1.Capacity);
            Assert.Equal(bArrayData.Length, byteBuffer1.Limit);
            Assert.Equal(0, byteBuffer1.Position);

            for (int idx = 0; idx < byteBuffer1.Limit; idx++)
            {
                Assert.Equal<byte>(bArrayData[idx], byteBuffer1.Contents[idx]);
            }
        }


        [Fact]
        [Category("Unit")]
        public void PositionTest()
        {
            byte[] bArrayData = new byte[] { 55, 56, 57 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            Assert.Equal<long>(3, byteBuffer1.Position);

            byteBuffer1.Flip();

            Assert.Equal<long>(0, byteBuffer1.Position);
        }

        [Fact]
        [Category("Unit")]
        public void RewindTest()
        {
            byte[] bArrayData = new byte[] { 55, 56, 57 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer1.Flip();

            for (int i = 0; i < bArrayData.Length; i++)
            {
                Assert.Equal<int>(bArrayData[i], byteBuffer1.ReadByte());
            }

            byteBuffer1.Rewind();

            for (int i = 0; i < bArrayData.Length; i++)
            {
                Assert.Equal<int>(bArrayData[i], byteBuffer1.ReadByte());
            }
        }


        [Fact]
        [Category("Unit")]
        public void WrapTest()
        {
            ByteBuffer byteBuffer = new ByteBuffer(3);
            byte[] bArray1 = new byte[] { 61, 62, 63 };
            byteBuffer.Put(bArray1);

            ByteBuffer wrapBuffer = ByteBuffer.Wrap(bArray1);

            Assert.True(byteBuffer.Equals(wrapBuffer));
        }

        [Fact]
        [Category("Unit")]
        public void EqualsTest()
        {
            byte[] bArrayData = new byte[] { 61, 62, 63, 64, 65, 66 };

            int capacity = 30;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);
            ByteBuffer byteBuffer2 = new ByteBuffer(capacity).Put(bArrayData);
            byteBuffer1.Flip();
            byteBuffer2.Flip();

            Assert.True(byteBuffer1.Equals(byteBuffer2));
        }

        [Fact]
        [Category("Unit")]
        public void ClearTest()
        {
            byte[] bArrayData = new byte[] { 70, 71, 72 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            Assert.Equal(5, byteBuffer1.Capacity);
            Assert.Equal(5, byteBuffer1.Limit);
            Assert.Equal(bArrayData.Length, byteBuffer1.Position);

            byteBuffer1.Clear();

            Assert.Equal(5, byteBuffer1.Capacity);
            Assert.Equal(5, byteBuffer1.Limit);
            Assert.Equal(0, byteBuffer1.Position);
            Assert.Equal(0, byteBuffer1.Begin);

            byte[] bArrayData2 = new byte[] { 70, 71, 72, 73, 74 };

            byteBuffer1.Put(bArrayData2);

            Assert.Equal(5, byteBuffer1.Capacity);
            Assert.Equal(5, byteBuffer1.Limit);
            Assert.Equal(bArrayData2.Length, byteBuffer1.Position);

            byteBuffer1.Flip();

            Assert.Equal(0, byteBuffer1.Position);

            for (int idx = 0; idx < byteBuffer1.Limit; idx++)
            {
                Assert.Equal<byte>(bArrayData2[idx], byteBuffer1.Contents[idx]);
            }
        }

        [Fact]
        [Category("Unit")]
        public void GetTest()
        {
            byte[] bArrayData = new byte[] { 73, 74, 75, 76, 77 };

            int capacity = 5;
            ByteBuffer byteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer.Flip();

            for (int i = 0; i < byteBuffer.Limit; i++)
            {
                Assert.Equal<int>(bArrayData[i], byteBuffer.ReadByte());
            }
        }


        [Fact]
        [Category("Unit")]
        public void CompactTest()
        {
            byte[] bArrayData = new byte[] { 78, 79, 80, 81, 82 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer1.Flip();

            for (int i = 0; i < 3; i++)
            {
                byteBuffer1.ReadByte(); // Read 3 bytes from the stream
            }

            byteBuffer1.Compact();

            // Only 2 bytes left after the compact method.
            byteBuffer1.Flip();

            Assert.Equal<int>(5, byteBuffer1.Capacity);
            Assert.Equal<long>(2, byteBuffer1.Limit);
            Assert.Equal<long>(0, byteBuffer1.Position);

            for (int i = 3; i < bArrayData.Length; i++)
            {
                Assert.Equal<int>(bArrayData[i], byteBuffer1.ReadByte());
            }
        }

        [Fact]
        [Category("Unit")]
        public void RemainingAndHasRemainingTest()
        {
            byte[] bArrayData = new byte[] { 78, 79, 80, 81, 82 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer1.Flip();

            for (int i = 0; i < bArrayData.Length; i++)
            {
                byteBuffer1.ReadByte();

                Assert.Equal<long>(byteBuffer1.Limit - byteBuffer1.Position, byteBuffer1.Remaining);

                if (i < bArrayData.Length - 1) // Make sure that it still has remaining
                    Assert.True(byteBuffer1.HasRemaining);
            }

            Assert.False(byteBuffer1.HasRemaining);
        }

        [Fact]
        [Category("Unit")]
        public void GetSpecificOffSetAndLengthTest()
        {
            byte[] bArrayData = new byte[] { 83, 84, 85, 86, 87 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer1.Flip();

            byte[] getBufferData1 = new byte[5];

            byteBuffer1.ReadBytesInto(getBufferData1, 0, (int)byteBuffer1.Limit);

            Assert.Equal<byte>(bArrayData[0], getBufferData1[0]);
            Assert.Equal<byte>(bArrayData[1], getBufferData1[1]);
            Assert.Equal<byte>(bArrayData[2], getBufferData1[2]);
            Assert.Equal<byte>(bArrayData[3], getBufferData1[3]);
            Assert.Equal<byte>(bArrayData[4], getBufferData1[4]);

            byte[] bExpectedData = new byte[] { 0, 0, 83, 84, 85 };

            byte[] getBufferData2 = new byte[5];

            byteBuffer1.Flip();

            byteBuffer1.ReadBytesInto(getBufferData2, 2, 3);

            Assert.Equal<byte>(bExpectedData[0], getBufferData2[0]);
            Assert.Equal<byte>(bExpectedData[1], getBufferData2[1]);
            Assert.Equal<byte>(bExpectedData[2], getBufferData2[2]);
            Assert.Equal<byte>(bExpectedData[3], getBufferData2[3]);
            Assert.Equal<byte>(bExpectedData[4], getBufferData2[4]);
        }

        [Fact]
        [Category("Unit")]
        public void WriteByteTest()
        {
            int capacity = 5;
            ByteBuffer byteBuffer = new ByteBuffer(capacity);

            byteBuffer.Write((byte)90);
            byteBuffer.Write((byte)91);
            byteBuffer.Write((byte)92);
            byteBuffer.Write((byte)93);

            Assert.Equal<long>(4, byteBuffer.Position);
            Assert.Equal<long>(5, byteBuffer.Limit);

            byteBuffer.Flip();

            Assert.Equal<int>(90, byteBuffer.ReadByte());
            Assert.Equal<int>(91, byteBuffer.ReadByte());
            Assert.Equal<int>(92, byteBuffer.ReadByte());
            Assert.Equal<int>(93, byteBuffer.ReadByte());
        }

        [Fact]
        [Category("Unit")]
        public void GetBytesTest()
        {
            byte[] bArrayData = new byte[] { 88, 89, 90, 91, 92 };

            int capacity = 10;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);
            testByteBuffer.Flip();

            byte[] outputArray = testByteBuffer.Contents;

            Assert.Equal<int>(capacity, outputArray.Length);

            int index = 0;

            for (; index < bArrayData.Length; index++)
            {
                Assert.Equal<byte>(bArrayData[index], outputArray[index]);
            }

            for (; index < outputArray.Length; index++)
            {
                Assert.Equal<byte>(0, outputArray[index]);
            }

        }

        [Fact]
        [Category("Unit")]
        public void GetByeTestOutOfRange()
        {
            byte[] bArrayData = new byte[] { 73, 74, 75, 76, 77 };

            int capacity = 5;
            ByteBuffer byteBuffer1 = new ByteBuffer(capacity).Put(bArrayData);

            byteBuffer1.Flip();

            Exception ex = Assert.Throws<IndexOutOfRangeException>(() => byteBuffer1.Contents[10]);


            Assert.Equal("Index was outside the bounds of the array.", ex.Message);
        }

        [Fact]
        [Category("Unit")]
        public void PutByteBufferTest()
        {
            byte[] bArrayData = new byte[] { 88, 89, 90, 91, 92 };

            int capacity = 5;
            ByteBuffer srcByteBuffer = new ByteBuffer(capacity).Put(bArrayData);
            srcByteBuffer.Flip();

            ByteBuffer testByteBuffer = new ByteBuffer(capacity + 5);

            testByteBuffer.Put(srcByteBuffer);

            Assert.Equal<long>(srcByteBuffer.Limit, testByteBuffer.Position);
            Assert.Equal<long>(10, testByteBuffer.Limit);

            testByteBuffer.Flip();

            Assert.Equal<long>(0, testByteBuffer.Position);
            Assert.Equal<long>(5, testByteBuffer.Limit);
            Assert.Equal<int>(testByteBuffer.Capacity, testByteBuffer.Contents.Length);

            Assert.Equal<int>(88, testByteBuffer.Contents[0]);
            Assert.Equal<int>(89, testByteBuffer.Contents[1]);
            Assert.Equal<int>(90, testByteBuffer.Contents[2]);
            Assert.Equal<int>(91, testByteBuffer.Contents[3]);
            Assert.Equal<int>(92, testByteBuffer.Contents[4]);
            Assert.Equal<int>(0, testByteBuffer.Contents[5]);
            Assert.Equal<int>(0, testByteBuffer.Contents[6]);
            Assert.Equal<int>(0, testByteBuffer.Contents[7]);
            Assert.Equal<int>(0, testByteBuffer.Contents[8]);
            Assert.Equal<int>(0, testByteBuffer.Contents[9]);
        }

        [Fact]
        [Category("Unit")]
        public void PutByteArrayOffsetLengthTest()
        {
            byte[] bArrayData = new byte[] { 88, 89, 90, 91, 92 };

            int capacity = 5;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            testByteBuffer.Put(bArrayData, 2, 3);

            testByteBuffer.Flip();

            Assert.Equal<long>(3, testByteBuffer.Limit);

            Assert.Equal<int>(90, testByteBuffer.ReadByte());
            Assert.Equal<int>(91, testByteBuffer.ReadByte());
            Assert.Equal<int>(92, testByteBuffer.ReadByte());
        }

        [Fact]
        [Category("Unit")]
        public void GetByteWithIndexTest()
        {
            int capacity = 5;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            testByteBuffer.Write((byte)90);
            testByteBuffer.Write((byte)91);
            testByteBuffer.Write((byte)92);
            testByteBuffer.Write((byte)93);
            testByteBuffer.Write((byte)94);

            testByteBuffer.Flip();

            Assert.Equal<byte>(90, testByteBuffer.Contents[0]);
            Assert.Equal<byte>(91, testByteBuffer.Contents[1]);
            Assert.Equal<byte>(92, testByteBuffer.Contents[2]);
            Assert.Equal<byte>(93, testByteBuffer.Contents[3]);
            Assert.Equal<byte>(94, testByteBuffer.Contents[4]);
        }

        [Fact]
        [Category("Unit")]
        public void PutSrcPosTest()
        {
            int capacity = 5;
            ByteBuffer byteBuffer = new ByteBuffer(capacity);
            byteBuffer.WritePosition = capacity;
            int value = 99;
            for (int index = 4; index >= 0; index--)
            {
                byteBuffer.WriteAt(index, (byte)(value - index));
            }
            byteBuffer.Flip();

            Assert.Equal<long>(5, byteBuffer.Limit);
            Assert.Equal<long>(0, byteBuffer.Position);

            Assert.Equal<byte>(99, byteBuffer.Contents[0]);
            Assert.Equal<byte>(98, byteBuffer.Contents[1]);
            Assert.Equal<byte>(97, byteBuffer.Contents[2]);
            Assert.Equal<byte>(96, byteBuffer.Contents[3]);
            Assert.Equal<byte>(95, byteBuffer.Contents[4]);
        }

        [Fact]
        [Category("Unit")]
        public void GetDoubleTest()
        {
            double expected = double.MaxValue;
            byte[] bArrayData = BitConverter.GetBytes(expected);

            System.Array.Reverse(bArrayData);

            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            testByteBuffer.Flip();

            Assert.Equal<double>(expected, testByteBuffer.ReadDouble());
        }

        [Fact]
        [Category("Unit")]
        public void GetDoubleByIndexTest()
        {
            double expected = double.MaxValue;
            double expected2 = 2.2250738585072014E-308;

            byte[] bArrayData = BitConverter.GetBytes(expected);
            System.Array.Reverse(bArrayData);

            byte[] bArrayData2 = BitConverter.GetBytes(expected2);
            System.Array.Reverse(bArrayData2);

            int capacity = 16;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                                                .Put(bArrayData)
                                                .Put(bArrayData2);

            testByteBuffer.Flip();

            Assert.Equal<double>(expected2, testByteBuffer.ReadDoubleAt(8));
            Assert.Equal<double>(expected, testByteBuffer.ReadDoubleAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void GetFloatTest()
        {
            float expected = float.MaxValue;
            byte[] bArrayData = BitConverter.GetBytes(expected);
            System.Array.Reverse(bArrayData);

            int capacity = 4;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            testByteBuffer.Flip();

            Assert.Equal<double>(expected, testByteBuffer.ReadFloat());
        }

        [Fact]
        [Category("Unit")]
        public void GetFloatByIndexTest()
        {
            float expected = float.MaxValue;
            float expected2 = float.MinValue;
            byte[] bArrayData = BitConverter.GetBytes(expected);
            System.Array.Reverse(bArrayData);

            byte[] bArrayData2 = BitConverter.GetBytes(expected2);
            System.Array.Reverse(bArrayData2);

            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData).Put(bArrayData2);

            testByteBuffer.Flip();

            Assert.Equal<double>(expected2, testByteBuffer.ReadFloatAt(4));
            Assert.Equal<double>(expected, testByteBuffer.ReadFloatAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void GetIntTest()
        {
            int expected = int.MaxValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));

            int capacity = 4;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            testByteBuffer.Flip();

            Assert.Equal<int>(expected, testByteBuffer.ReadInt());
        }
        [Fact]
        [Category("Unit")]
        public void GetIntByIndexTest()
        {
            int expected = int.MaxValue;
            int expected2 = int.MinValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));
            byte[] bArrayData2 = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected2));

            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData).Put(bArrayData2);

            testByteBuffer.Flip();

            Assert.Equal<int>(expected2, testByteBuffer.ReadIntAt(4));
            Assert.Equal<int>(expected, testByteBuffer.ReadIntAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void GetLongTest()
        {
            long expected = long.MaxValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));

            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            testByteBuffer.Flip();

            Assert.Equal<long>(expected, testByteBuffer.ReadLong());
        }

        [Fact]
        [Category("Unit")]
        public void GetLongByIndexTest()
        {
            long expected = long.MaxValue;
            long expected2 = long.MinValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));
            byte[] bArrayData2 = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected2));

            int capacity = 16;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData).Put(bArrayData2);

            testByteBuffer.Flip();

            Assert.Equal<long>(expected2, testByteBuffer.ReadLongAt(8));
            Assert.Equal<long>(expected, testByteBuffer.ReadLongAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void GetShortTest()
        {
            short expected = short.MaxValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));

            int capacity = 2;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData);

            testByteBuffer.Flip();

            Assert.Equal<short>(expected, testByteBuffer.ReadShort());
        }
        [Fact]
        [Category("Unit")]
        public void GetShortByIndexTest()
        {
            short expected = short.MaxValue;
            short expected2 = short.MinValue;
            byte[] bArrayData = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected));
            byte[] bArrayData2 = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(expected2));

            int capacity = 16;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity).Put(bArrayData).Put(bArrayData2);

            testByteBuffer.Flip();

            Assert.Equal<short>(expected2, testByteBuffer.ReadShortAt(2));
            Assert.Equal<short>(expected, testByteBuffer.ReadShortAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void PutDoubleTest()
        {
            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            double expected = double.MaxValue;

            testByteBuffer.Write(expected);

            testByteBuffer.Flip();

            Assert.Equal<double>(expected, testByteBuffer.ReadDouble());
        }

        [Fact]
        [Category("Unit")]
        public void PutDoubleByIndexTest()
        {
            int capacity = 16;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((double)0))
                .Put(BitConverter.GetBytes((double)0));

            testByteBuffer.Flip();

            double expected = double.MaxValue;
            double expected2 = double.MinValue;

            testByteBuffer.WriteAt(0, expected);
            testByteBuffer.WriteAt(8, expected2);

            Assert.Equal<double>(expected2, testByteBuffer.ReadDoubleAt(8));
            Assert.Equal<double>(expected, testByteBuffer.ReadDoubleAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void PutFloatTest()
        {
            int capacity = 4;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            float expected = float.MaxValue;

            testByteBuffer.Write(expected);

            testByteBuffer.Flip();

            Assert.Equal<float>(expected, testByteBuffer.ReadFloat());
        }

        [Fact]
        [Category("Unit")]
        public void PutFlaotByIndexTest()
        {
            int capacity = 8;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((float)0))
                .Put(BitConverter.GetBytes((float)0));

            testByteBuffer.Flip();

            float expected = float.MaxValue;
            float expected2 = float.MinValue;

            testByteBuffer.WriteAt(0, expected);
            testByteBuffer.WriteAt(4, expected2);

            Assert.Equal<float>(expected2, testByteBuffer.ReadFloatAt(4));
            Assert.Equal<float>(expected, testByteBuffer.ReadFloatAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void PutIntTest()
        {
            int capacity = 4;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            int expected = int.MaxValue;

            testByteBuffer.Write(expected);

            testByteBuffer.Flip();

            Assert.Equal<int>(expected, testByteBuffer.ReadInt());
        }

        [Fact]
        [Category("Unit")]
        public void PutIntByIndexTest()
        {
            int capacity = 8;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((int)0))
                .Put(BitConverter.GetBytes((int)0));

            testByteBuffer.Flip();

            int expected = int.MaxValue;
            int expected2 = int.MinValue;

            testByteBuffer.WriteAt(0, expected);
            testByteBuffer.WriteAt(4, expected2);

            Assert.Equal<int>(expected2, testByteBuffer.ReadIntAt(4));
            Assert.Equal<int>(expected, testByteBuffer.ReadIntAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void PutLongTest()
        {
            int capacity = 8;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            long expected = long.MaxValue;

            testByteBuffer.Write(expected);

            testByteBuffer.Flip();

            Assert.Equal<long>(expected, testByteBuffer.ReadLong());
        }

        [Fact]
        [Category("Unit")]
        public void PutLongByIndexTest()
        {
            int capacity = 16;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((long)0))
                .Put(BitConverter.GetBytes((long)0));

            testByteBuffer.Flip();

            long expected = long.MaxValue;
            long expected2 = long.MinValue;

            testByteBuffer.WriteAt(0, expected);
            testByteBuffer.WriteAt(8, expected2);

            Assert.Equal<long>(expected2, testByteBuffer.ReadLongAt(8));
            Assert.Equal<long>(expected, testByteBuffer.ReadLongAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void PutShortTest()
        {
            int capacity = 2;
            ByteBuffer testByteBuffer = new ByteBuffer(capacity);

            short expected = short.MaxValue;

            testByteBuffer.Write(expected);

            testByteBuffer.Flip();

            Assert.Equal<short>(expected, testByteBuffer.ReadShort());
        }

        [Fact]
        [Category("Unit")]
        public void PutShortByIndexTest()
        {
            int capacity = 4;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((short)0))
                .Put(BitConverter.GetBytes((short)0));

            testByteBuffer.Flip();

            short expected = short.MaxValue;
            short expected2 = short.MinValue;

            testByteBuffer.WriteAt(0, expected);
            testByteBuffer.WriteAt(2, expected2);

            Assert.Equal<short>(expected2, testByteBuffer.ReadShortAt(2));
            Assert.Equal<short>(expected, testByteBuffer.ReadShortAt(0));
        }

        [Fact]
        [Category("Unit")]
        public void DisposeTest()
        {
            int capacity = 4;

            ByteBuffer testByteBuffer = new ByteBuffer(capacity)
                .Put(BitConverter.GetBytes((short)0))
                .Put(BitConverter.GetBytes((short)0));

            IDisposable idisposable = (IDisposable)testByteBuffer;

            idisposable.Dispose();
            idisposable.Dispose();
        }

        #region Reserve

        private static long AllocateSource(out ByteBuffer buffer, int capacity, int fill = 8)
        {
            if (fill > capacity)
                fill = capacity;

            buffer = new ByteBuffer(capacity);
            byte[] source = new byte[fill];
            for (int i = 0; i < fill; i++)
                source[i] = (byte)(i + 0x80);
            buffer.Put(source);
            return buffer.Position;
        }

        [Fact]
        [Category("Unit")]
        public void ReserveDoesNotMovePosition()
        {
            long sourcePosition = AllocateSource(out ByteBuffer buffer, 16);

            long count = 8;
            buffer.Reserve(count);
            Assert.Equal(sourcePosition, buffer.Position);         // Position has not moved
        }

        [Fact]
        [Category("Unit")]
        public void ReserveIncreasesLengthByCount()
        {
            long sourcePosition = AllocateSource(out ByteBuffer buffer, 8, 6);
            long count = 4;
            long length = buffer.Reserve(count);

            Assert.Equal(sourcePosition + count, length);
        }

        [Fact]
        [Category("Unit")]
        public void ReserveIncreasesesLengthEnoughToReserveCountFromPosition()
        {
            const int reserve = 5;
            AllocateSource(out ByteBuffer buffer, sizeof(short) * 3, sizeof(short) * 3);
            buffer.WritePosition = sizeof(short);
            buffer.Write(long.MaxValue);
            int position = buffer.WritePosition;
            long length = buffer.Reserve(reserve);
            Assert.Equal(position + reserve, length);
        }

        [Fact]
        [Category("Unit")]
        public void ReserveDoesNotLengthIfEnoughRoomFromPosition()
        {
            const int allocate = 8;
            const int reserve = 5;
            AllocateSource(out ByteBuffer buffer, allocate);
            buffer.WritePosition = 0;
            long length = buffer.Reserve(reserve);

            Assert.Equal(allocate, length);
        }
        #endregion
    }
}
