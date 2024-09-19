/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    /// An iteratable version of the Value Add FIFO queue.
    /// </summary>
    public class VaIteratableQueue : VaQueue
    {
        VaNode? m_CurrentNode = null;
        VaNode? m_LastNode = null;

        /// <summary>
        /// Adds to the tail of the queue.
        /// </summary>
        /// <param name="node">the node to add</param>
        public override void Add(VaNode node)
        {
            base.Add(node);
            if(m_CurrentNode is null)
            {
                m_CurrentNode = node;
            }
        }

        /// <summary>
        /// Rewinds the iterator to the head of the queue.
        /// </summary>
        public void Rewind()
        {
            m_CurrentNode = _head;
            m_LastNode = null;
        }

        /// <summary>
        /// Returns whether or not the iterator has more nodes.
        /// </summary>
        /// <returns><c>true</c> if the iterator has more nodes, or <c>false</c> if the iterator doesn't</returns>
        public bool HasNext()
        {
            return m_CurrentNode != null;
        }

        /// <summary>
        /// Returns the next node in the queue.
        /// </summary>
        /// <returns>The next node</returns>
        public VaNode? Next()
        {
            m_LastNode = m_CurrentNode;
            if(m_LastNode != null)
                m_CurrentNode = m_LastNode.Next;
            return m_LastNode;
        }

        /// <summary>
        /// Removes the last node from the queue.
        /// </summary>
        public void Remove()
        {
            base.Remove(m_LastNode!);
        }
    }
}
