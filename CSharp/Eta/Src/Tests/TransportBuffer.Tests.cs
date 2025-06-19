/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Common;
using Xunit;
using Xunit.Categories;

namespace LSEG.Eta.Transports.Tests
{
    [Category("TransportBuffer")]
    public class TransportBufferTests
    {
        const int defaultBufferSize = 8192;

        static byte[] initializedBuffer = new byte[defaultBufferSize];

        static TransportBufferTests()
        {
            for (int i = 0; i < initializedBuffer.Length; i++)
            {
                initializedBuffer[i] = (byte)(i % 255);
            }
        }

        private static ITransportBuffer CreateTransportBuffer(int bufferSize = defaultBufferSize)
        {
            ByteBuffer buffer = new ByteBuffer(bufferSize + 3);
            ITransportBuffer transportBuffer = new TransportBuffer(buffer, RipcDataMessage.HeaderSize);
            return transportBuffer;
        }

        [Fact, Category("Unit")]
        public void InitializedOk()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();

            Assert.IsType<TransportBuffer>(transportBuffer);
            Assert.Equal(defaultBufferSize, transportBuffer.Capacity());
        }

        [Fact, Category("Unit")]
        public void InitializedBufferHasInitializedData()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();
            Assert.NotEmpty(transportBuffer.Data.Contents);
        }

        [Fact, Category("Unit")]
        public void LengthEqualsInitialSize()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();
            Assert.Equal(defaultBufferSize, transportBuffer.Length());
            Assert.Equal(transportBuffer.Capacity(), transportBuffer.Length());
        }


        [Fact, Category("Unit")]
        public void CapacityEqualsInitialSize()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();
            Assert.Equal(defaultBufferSize, transportBuffer.Capacity());
        }

        [Fact, Category("Unit")]
        public void CopyThrowsOnDestinationNull()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();

            Action action = new Action(() =>
            {
                transportBuffer.Copy(null);
            });

            Assert.Throws<ArgumentNullException>(action);
        }

        [Fact, Category("Unit")]
        public void CopyThrowsOnDestinationTooSmall()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer(defaultBufferSize + 1);
            var destination = new ByteBuffer(defaultBufferSize);

            transportBuffer.Data.WritePosition += transportBuffer.Capacity();

            Action action = new Action(() =>
            {
                transportBuffer.Copy(destination);
            });

            Assert.Throws<InsufficientMemoryException>(action);
        }

        [Fact, Category("Unit")]
        public void CopyToDataBufferOk()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();

            initializedBuffer.CopyTo(transportBuffer.Data.Contents, 0);

            transportBuffer.Data.WritePosition += initializedBuffer.Length;

            var destination = new ByteBuffer(defaultBufferSize);

            var result = transportBuffer.Copy(destination);

            Assert.Equal((int)TransportReturnCode.SUCCESS, result);
            Assert.Equal(initializedBuffer, destination.Contents);
        }

        ///
        /// This tests the getLength() method. It contains three test cases.
        ///
        /// 1. Positive test for read buffer length
        /// 2. Positive test for non-fragmented write buffer length
        /// 3. Positive test for fragmented write buffer length
        ///
        [Fact, Category("Unit")]
        public void GetLengthTest()
        {
            // 1. Positive test for read buffer length

            TransportBuffer readBuf = new TransportBuffer(103); // Included the RIPC header length(3)

            Assert.Equal(100, readBuf.Length());

            // 2. Positive test for non-fragmented write buffer length

            TransportBuffer writeBufNonFrag = new TransportBuffer(103); // Included the RIPC header length(3)
            writeBufNonFrag.IsReadMode = false;
           
            Assert.Equal(100, writeBufNonFrag.Length());

            for (int i = 0; i < 100; i++)
            {
                if (i == 50)
                {
                    Assert.Equal(47, writeBufNonFrag.Length());
                }
                writeBufNonFrag.Data.Put(new byte[] { (byte)i });
            }
            Assert.Equal(97, writeBufNonFrag.Length());

            // 3. Positive test for fragmented write buffer length

            TransportBuffer writeBufFrag = new TransportBuffer(6145, isBigBuffer: true);
            writeBufFrag.IsReadMode = false;
            Assert.Equal(6145, writeBufFrag.Length());
            for (int i = 0; i < 6145; i++)
            {
                if (i == 50)
                {
                    Assert.Equal(50, writeBufFrag.Length());
                }
                writeBufFrag.Data.Put(new byte[] { (byte)i });
            }
            Assert.Equal(6145, writeBufFrag.Length());
        }

        ///
        /// This tests the copy() method. It contains four test cases.
        ///
        /// 1. Positive test for read buffer copy
        /// 2. Positive test for non-fragmented write buffer copy
        /// 3. Positive test for fragmented write buffer copy
        /// 4. Negative test case for insufficient memory for copy
        ///
        [Fact, Category("Unit")]
        public void CopyTest()
        {
            // 1. Positive test for read buffer copy

            TransportBuffer readBuf = new TransportBuffer(100);
            for (int i = 0; i < 100; i++)
            {
                readBuf.Data.Put(new byte[] { (byte)i });
            }
            readBuf.Data.Rewind();
            // readBuf.Data.Position = 0;
            ByteBuffer readBufCopy = new ByteBuffer(100);
            Assert.Equal((int)TransportReturnCode.SUCCESS, readBuf.Copy(readBufCopy));
            for (int i = 0; i < readBuf.Length(); i++)
            {
                Assert.Equal(i, readBufCopy.Contents[i]);
            }

            // 2. Positive test for non-fragmented write buffer copy

            TransportBuffer writeBufNonFrag = new TransportBuffer(100);
            writeBufNonFrag.IsReadMode= false;
            for (int i = 0; i < 100; i++)
            {
                writeBufNonFrag.Data.Put(new byte[] { (byte)i });
            }
            writeBufNonFrag.Data.Rewind();
            ByteBuffer writeBufNonFragCopy = new ByteBuffer(100);
            Assert.Equal((int)TransportReturnCode.SUCCESS, writeBufNonFrag.Copy(writeBufNonFragCopy));
            for (int i = 0; i < writeBufNonFrag.Length(); i++)
            {
                // note: original was i+3
                Assert.Equal(i, writeBufNonFragCopy.Contents[i]);
            }

            // 3. Positive test for fragmented write buffer copy

            TransportBuffer writeBufFrag = new TransportBuffer(6145, isBigBuffer: true);
            writeBufFrag.IsReadMode = false;
            for (int i = 0; i < 6145; i++)
            {
                writeBufFrag.Data.Put(new byte[] { (byte)i });
            }
            ByteBuffer writeBufFragCopy = new ByteBuffer(6145);
            Assert.Equal((int)TransportReturnCode.SUCCESS, writeBufFrag.Copy(writeBufFragCopy));
            for (int i = 0; i < 6145; i++)
            {
                Assert.Equal((byte)i, writeBufFragCopy.Contents[i]);
            }

            // 4. Negative test case for insufficient memory for copy

            Assert.Throws<InsufficientMemoryException>(() => writeBufFrag.Copy(writeBufNonFragCopy));
        }

    }
}
