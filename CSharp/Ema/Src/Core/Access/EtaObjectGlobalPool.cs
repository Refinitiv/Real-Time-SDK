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
using System.Collections.Concurrent;
using System.Globalization;
using System.Threading;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal sealed class EtaObjectGlobalPool
    {
        private static readonly EtaObjectGlobalPool m_instance = new EtaObjectGlobalPool();

        public const int INITIAL_POOL_SIZE = 100;
        public const int MIN_BYTEBUFFER_SIZE = 8;

        internal ConcurrentBag<EncodeIterator> m_etaEncodeIteratorPool;
        internal ConcurrentBag<Buffer> m_etaBufferPool;
        internal ConcurrentDictionary<int, ConcurrentBag<ByteBuffer>> m_etaByteBufferBySizePool;  

        private EtaObjectGlobalPool()
        {
            m_etaEncodeIteratorPool = new ConcurrentBag<EncodeIterator>();
            m_etaBufferPool = new ConcurrentBag<Buffer>();
            m_etaByteBufferBySizePool = new ConcurrentDictionary<int, ConcurrentBag<ByteBuffer>>();

            InitEtaObjectPool(m_etaEncodeIteratorPool, INITIAL_POOL_SIZE);
            InitEtaObjectPool(m_etaBufferPool, INITIAL_POOL_SIZE);
            InitByteBufferPool(INITIAL_POOL_SIZE);
        }

        public static EtaObjectGlobalPool Instance { get => m_instance; }

        #region Methods for getting objects

        public EncodeIterator GetEncodeIterator()
        {
            EncodeIterator? result;
            if (!m_etaEncodeIteratorPool.TryTake(out result))
            {
                result = new EncodeIterator();
            }
            return result;
        }

        public Buffer GetBuffer()
        {
            Buffer? result;
            if (!m_etaBufferPool.TryTake(out result))
            {
                result = new Buffer();
            }
            return result;
        }

        public ByteBuffer GetByteBuffer2(int powOfTwoSize)
        {
            ByteBuffer? result;
            if (m_etaByteBufferBySizePool.ContainsKey(powOfTwoSize))
            {
                if (!m_etaByteBufferBySizePool[powOfTwoSize].TryTake(out result))
                {
                    result = new ByteBuffer(powOfTwoSize);
                    GC.SuppressFinalize(result);
                }               
            }
            else
            {
                var bag = new ConcurrentBag<ByteBuffer>();
                if (m_etaByteBufferBySizePool.TryAdd(powOfTwoSize, bag)) // if false, bag for this size already added by another thread
                {
                    InitByteBufferBag(bag, INITIAL_POOL_SIZE, powOfTwoSize);
                }
                if (!m_etaByteBufferBySizePool[powOfTwoSize].TryTake(out result))
                {
                    result = new ByteBuffer(powOfTwoSize);
                    GC.SuppressFinalize(result);
                }
            }
            return result;
        }

        public ByteBuffer GetByteBuffer(int minSize)
        {
            int val = (int)Math.Log2(minSize) + 1;
            return GetByteBuffer2((int)Math.Pow(2, val));
        }

        #endregion

        #region Methods for returning objects

        public void ReturnEncodeIterator(EncodeIterator? encodeIterator)
        {
            if (encodeIterator != null)
            {
                var buffer = encodeIterator.Buffer();
                encodeIterator.Clear();
                m_etaEncodeIteratorPool.Add(encodeIterator);
                ReturnBuffer(buffer);
            }
        }

        public void ReturnBuffer(Buffer buffer)
        {
            buffer.Clear();
            m_etaBufferPool.Add(buffer);
        }

        public void ReturnByteBuffer(ByteBuffer? byteBuffer)
        {
            if (byteBuffer != null
                && byteBuffer.Contents != null
                && m_etaByteBufferBySizePool.ContainsKey(byteBuffer.Contents.Length))
            {
                byteBuffer.Clear();
                m_etaByteBufferBySizePool[byteBuffer.Contents.Length].Add(byteBuffer);
            }         
        }

        #endregion

        private void InitEtaObjectPool<T>(ConcurrentBag<T> pool, int poolSize) where T : new()
        {
            for (int i = 0; i < poolSize; i++)
            {
                var val = new T();
                pool.Add(val);
            }
        }

        private void InitByteBufferPool(int byteBufferBagSize)
        {
            for (int i = 1; i < 4096; i *= 2)
            {
                var bag = new ConcurrentBag<ByteBuffer>();
                int currentSize = MIN_BYTEBUFFER_SIZE * i;
                InitByteBufferBag(bag, byteBufferBagSize, currentSize);
                m_etaByteBufferBySizePool!.TryAdd(currentSize, bag);
            }
        }

        private void InitByteBufferBag(ConcurrentBag<ByteBuffer> bag, int byteBufferBagSize, int byteBufferSize)
        {
            for (int j = 0; j < byteBufferBagSize; j++)
            {
                ByteBuffer byteBuffer = new (byteBufferSize);
                GC.SuppressFinalize(byteBuffer);
                bag.Add(byteBuffer);
            }
        }

        internal void Free()
        {
            foreach (var poolSize in m_etaByteBufferBySizePool)
                foreach (var buf in poolSize.Value)
                {
                    buf.Dispose();
                }
        }

        ~EtaObjectGlobalPool()
        {
            Free();
        }
    }
}
