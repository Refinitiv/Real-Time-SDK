/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Runtime.CompilerServices;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class Encoder
    {
        internal bool m_containerComplete = false;

        internal EncodeIterator? m_encodeIterator;
        internal ByteBuffer? m_iteratorByteBuffer;
        internal Encoder? m_iteratorByteBufferOwner;
        internal bool m_releaseIteratorByteBuffer = true;
        internal Encoder? m_iteratorOwner;
        internal int m_allocatedSize;

        internal Action? EndEncodingEntry;
        internal Data? m_encoderOwner;
        internal EmaObjectManager? m_objectManager;
        internal EtaObjectGlobalPool m_etaPool = EtaObjectGlobalPool.Instance;

        public Encoder()
        {
        }

        public bool IsComplete { get => m_containerComplete; }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AcquireEncodeIterator(int byteBufferSize = 4096)
        {
            if (m_encodeIterator == null)
            {
                m_allocatedSize = byteBufferSize;
                m_encodeIterator = m_etaPool.GetEncodeIterator();
                Buffer buffer = m_etaPool.GetBuffer();
                m_iteratorByteBuffer = m_etaPool.GetByteBuffer(byteBufferSize);
                buffer.Data(m_iteratorByteBuffer);
                m_encodeIterator.SetBufferAndRWFVersion(buffer, Codec.MajorVersion(), Codec.MinorVersion());
                m_iteratorOwner = this;
                m_iteratorByteBufferOwner = this;

                m_releaseIteratorByteBuffer = true;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReallocateEncodeIteratorBuffer()
        {
            int newSize = m_encodeIterator!.Buffer().Data().Contents.Length << 1;
            ByteBuffer newByteBuffer = m_etaPool.GetByteBuffer(newSize);
            Buffer newBuffer = m_etaPool.GetBuffer();
            newBuffer.Data(newByteBuffer);
            var ret = m_encodeIterator.RealignBuffer(newBuffer);
            m_etaPool.ReturnBuffer(newBuffer);
            m_etaPool.ReturnByteBuffer(m_iteratorByteBufferOwner!.m_iteratorByteBuffer);
            m_iteratorByteBufferOwner!.m_iteratorByteBuffer = newByteBuffer;

            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to realign EncodeIterator buffer in EncodeIterator, return code: {ret.GetAsString()}");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ReleaseEncodeIterator()
        {
            if (m_encodeIterator != null)
            {
                if (m_iteratorOwner == this)
                {
                    m_etaPool.ReturnEncodeIterator(m_encodeIterator);
                    if (m_releaseIteratorByteBuffer)
                    {
                        m_etaPool.ReturnByteBuffer(m_iteratorByteBuffer);
                    }                
                }
                m_encodeIterator = null;
                m_iteratorByteBuffer = null;
                m_iteratorByteBufferOwner = null;
                m_releaseIteratorByteBuffer = false;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual void Clear()
        {
            ReleaseEncodeIterator();
            m_containerComplete = false;
            m_iteratorOwner = null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool OwnsIterator()
        {
            return m_iteratorOwner == this;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void PassEncIterator(Encoder? other)
        {
            if (other != null)
            {
                other.m_encodeIterator = m_encodeIterator;
                other.m_iteratorOwner = this;
                other.m_iteratorByteBufferOwner = m_iteratorByteBufferOwner;
            }          
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Buffer GetEncodedBuffer(bool dropByteBufferOwnership = true)
        {
            m_releaseIteratorByteBuffer = !dropByteBufferOwnership;
            return m_encodeIterator!.Buffer();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal int ConvertDataTypeToEta(int emaDataType)
        {
            return emaDataType >= DataType.DataTypes.REQ_MSG && emaDataType <= DataType.DataTypes.GENERIC_MSG
                ? DataType.DataTypes.MSG : emaDataType;
        }

        ~Encoder()
        {
            if (m_encodeIterator != null)
            {
                if (m_iteratorOwner == this)
                {
                    m_etaPool.ReturnEncodeIterator(m_encodeIterator);
                    if (m_releaseIteratorByteBuffer)
                        m_etaPool.ReturnByteBuffer(m_iteratorByteBuffer);
                }
                m_encodeIterator = null;
                m_iteratorByteBuffer = null;
            }
        }
    }
}
