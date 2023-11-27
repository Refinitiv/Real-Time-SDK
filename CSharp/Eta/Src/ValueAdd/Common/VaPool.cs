/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// Value Add Pool class.
    /// </summary>
    public class VaPool
    {
        private VaQueue m_Queue;

        private bool m_Debug;


        /// <summary>
        /// Creates a pool. This pool is not thread safe.
        /// </summary>
        public VaPool()
        {
            m_Queue = new VaQueue();
        }

        /// <summary>
        /// Creates a pool.
        /// </summary>
        /// <param name="useConcurrent">if true, the pool is backed by a <see cref="VaConcurrentQueue"/></param>
        public VaPool(bool useConcurrent)
        {
            m_Queue = useConcurrent ? new VaConcurrentQueue() : new VaQueue();
        }

        /// <summary>
        /// Creates a pool.
        /// </summary>
        /// <param name="useConcurrent">if true, the pool is backed by a <see cref="VaConcurrentQueue"/></param>
        /// <param name="debug">if true, the debugging mode is enabled</param>
        public VaPool(bool useConcurrent, bool debug)
        {
            m_Queue = useConcurrent ? new VaConcurrentQueue() : new VaQueue();
            m_Debug = debug;
        }

        /// <summary>
        /// Adds a node to the pool.
        /// </summary>
        /// <param name="node">the node to add</param>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual void Add(VaNode node)
        {
            if (m_Debug)
                m_Queue.VerifyQueue();

            if (node.InPool)
                return; // already in pool.

            if (node.Pool != this)
                node.Pool = this;

            node.InPool = true;
            m_Queue.Add(node);

            if (m_Debug)
                m_Queue.VerifyQueue();
        }

        /// <summary>
        /// Updates the specified node to point to this pool. This would be called
        /// when the node is created and assigns a pool to the node.
        /// </summary>
        /// <param name="node">the node to update</param>
        public void UpdatePool(VaNode node)
        {
            node.Pool = this;
        }

        /// <summary>
        /// Removes and returns the first item pooled.
        /// </summary>
        /// <returns>the first item pooled</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public VaNode? Poll()
        {
            if (m_Debug)
                m_Queue.VerifyQueue();

            VaNode? node = m_Queue.Poll();

            if (node != null)
                node.InPool = false;

            if (m_Debug)
                m_Queue.VerifyQueue();

            return node;
        }

        /// <summary>
        /// Returns the size of the pool.
        /// </summary>
        /// <returns>the size of the pool</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Size()
        {
            return m_Queue.Size();
        }
    }
}
