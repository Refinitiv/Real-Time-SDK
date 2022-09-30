/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

using Refinitiv.Eta.Common;
using Xunit;
using Xunit.Categories;
using Refinitiv.Common.Interfaces;

namespace Refinitiv.Eta.Transports.Tests
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
            Assert.Equal(defaultBufferSize, transportBuffer.Capacity);
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
            Assert.Equal(defaultBufferSize, transportBuffer.Length);
            Assert.Equal(transportBuffer.Capacity, transportBuffer.Length);
        }


        [Fact, Category("Unit")]
        public void CapacityEqualsInitialSize()
        {
            ITransportBuffer transportBuffer = CreateTransportBuffer();
            Assert.Equal(defaultBufferSize, transportBuffer.Capacity);
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

            transportBuffer.Data.WritePosition += transportBuffer.Capacity;

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
    }
}
