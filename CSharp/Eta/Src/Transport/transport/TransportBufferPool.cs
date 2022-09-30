/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.ServiceModel.Channels;
using Refinitiv.Eta.Common;
using Refinitiv.Common.Interfaces;

namespace Refinitiv.Eta.Transports
{
    internal class TransportBufferPool : Pool , IDisposable
    {
        private BufferManager _bufferManager;

        public TransportBufferPool(object owner):
           base(owner)
        {
        }

        public TransportReturnCode InitPool(int numTransportBuffer,long maxBufferPoolSize, int maxBufferSize, out Error error)
        {
            error = null;

            try
            {
                _bufferManager = BufferManager.CreateBufferManager(maxBufferPoolSize, maxBufferSize);

                for(int i = 0; i < numTransportBuffer; i++)
                {
                    Add(new TransportBuffer());
                }
            }
            catch(Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = exp.Message,
                    SysError = 0,
                };

                return TransportReturnCode.FAILURE;
            }

            return TransportReturnCode.SUCCESS;
        }

        public TransportBuffer GetTransportBuffer(int bufferSize, bool packedBuffer, int headerLength, out Error error)
        {
            error = null;
            TransportBuffer transportBuffer = null;
            try
            {
                transportBuffer = (TransportBuffer)Poll();

                if (transportBuffer == null)
                {
                    Byte[] data = _bufferManager.TakeBuffer(bufferSize);
                    transportBuffer = new TransportBuffer(new ByteBuffer(data, true), headerLength)
                    {
                        Pool = this
                    };

                    transportBuffer.Data.Limit = bufferSize;
                }
                else
                {
                    Byte[] data = _bufferManager.TakeBuffer(bufferSize);
                    transportBuffer.SetData(new ByteBuffer(data, true), headerLength);
                    transportBuffer.Data.Limit = bufferSize;
                }

                transportBuffer.Data.WritePosition += packedBuffer ? RipcDataMessage.PackedHeaderSize : 0;
            }
            catch(Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.NO_BUFFERS,
                    Text = exp.Message,
                };

                return null;
            }

            return transportBuffer;
        }

        internal void ReturnTransportBuffer(ITransportBuffer buffer, out Error error)
        {
            error = null;
            TransportBuffer transportBuffer = (TransportBuffer)buffer;

            try
            {
                // Ensoure that the Data property is not null
                if (transportBuffer.Data != null && transportBuffer.IsBigBuffer == false)
                {
                    _bufferManager.ReturnBuffer(transportBuffer.Data.Contents);
                }

                transportBuffer.Clear();

                transportBuffer.ReturnToPool();
            }
            catch(Exception exp)
            {
                error = new Error
                {
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = exp.Message,
                };
            }
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    Clear();
                    _bufferManager.Clear();
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
        }
        #endregion
    }
}
