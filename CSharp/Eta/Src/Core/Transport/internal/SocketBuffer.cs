/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Internal
{
    internal class SocketBuffer : EtaNode
    {
        public class SlicesPool : Pool
        {
            int m_Available = 0;
            int m_TotalSliceCount = 0;
            SocketBuffer m_SocketBuffer;

            public SlicesPool(Object owner) : base(owner)
            {
                m_SocketBuffer = owner as SocketBuffer;
            }

            public override EtaNode Poll()
            {
                TransportBuffer slice;
                if ((slice = (TransportBuffer)base.Poll()) != null)
                {
                    --m_Available;
                    slice.StartPosition = 0;
                    slice.HeaderLength = RIPC_WRITE_POSITION;
                }
                else
                {
                    ByteBuffer byteBuffer = new ByteBuffer(m_SocketBuffer.ByteBuffer, true);

                    // create a new slice
                    slice = new TransportBuffer(this, byteBuffer);
                    ++m_TotalSliceCount;
                }
                return slice;
            }

            public override void Add(EtaNode node)
            {
                base.Add(node);
                ++m_Available;

                if (m_Available == m_TotalSliceCount)
                {
                    // the SocketBuffer may being recycled
                    Pool socketBufferPool = m_SocketBuffer.Pool;
                    if (!socketBufferPool.IsSharedPoolBuffer)
                    {
                        if (!socketBufferPool.IsProtocolBuffer)
                        {
                            ((ChannelBase)socketBufferPool.PoolOwner).SocketBufferToRecycle(m_SocketBuffer);
                        }
                        else
                        {
                            m_SocketBuffer.ReturnToPool();
                        }

                    }
                    else
                    {
                        ((ServerImpl)socketBufferPool.PoolOwner).SocketBufferToRecycle(m_SocketBuffer);
                    }
                }
            }

            public bool AreAllSlicesBack()
            {
                return (m_Available == m_TotalSliceCount);
            }
        }

        internal const int PACKED_HDR = 2;
        internal const int RIPC_WRITE_POSITION = 3;
        internal const byte FRAGMENT_RIPC_FLAGS = 0x04;
        internal const byte FRAGMENT_HEADER_RIPC_FLAGS = 0x08;
        internal const int COMPRESSION_EXTRA_LEN = 16;

        internal byte[] ByteBuffer { get; private set; }
        public SlicesPool m_SlicesPool;
        int m_BytesUsed = 0;

        public SocketBuffer(Pool pool, int size)
        {
            ByteBuffer = new byte[size];
            Pool = pool;
            m_SlicesPool = new SlicesPool(this);
        }

        public TransportBuffer GetBufferSlice(int size, bool packedBuffer, int headerLength)
        {
            // locked by calling method
            TransportBuffer slice = null;
            bool isPacked;

            int _headerLength = headerLength;
            if (!packedBuffer)
            {
                isPacked = false;
            }
            else
            {
                _headerLength += PACKED_HDR;
                isPacked = true;
            }

            if ((ByteBuffer.Length - m_BytesUsed) >= (_headerLength + size))
            {
                slice = (TransportBuffer)m_SlicesPool.Poll(); // it returns non null
                slice.Data.Clear();

                slice.StartPosition = m_BytesUsed;
                slice.Data.Limit = m_BytesUsed + _headerLength + size;
                slice.Data.WritePosition = m_BytesUsed + _headerLength;
                m_BytesUsed = m_BytesUsed + _headerLength + size;

                // has to mark the position for the packed message length
                if (isPacked)
                {
                    slice.IsPacked = true;
                    slice.PackedMsgOffSetPosition = slice.StartPosition + headerLength;
                }
                else
                {
                    slice.IsPacked = false;
                }
            }

            return slice;
        }

        public TransportBuffer GetBufferSliceForFragment(int size)
        {
            // locked by calling method
            TransportBuffer slice;

            slice = (TransportBuffer)m_SlicesPool.Poll();
            slice.Data.Clear();
            slice.StartPosition = 0;
            slice.Data.Limit = size;
            m_BytesUsed = size;

            return slice;
        }

        public void Clear()
        {
            m_BytesUsed = 0;
        }
    }
}
