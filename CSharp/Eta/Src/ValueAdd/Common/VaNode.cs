/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Common
{
    /// <summary>
    /// Represents a Node in a Queue. Used with VaConcurrentQueue or VaQueue. Note that a node
    /// can only be in one queue at a time. It cannot be in more than one queue at the same time.
    /// Remove the node from one queue before adding it to another queue.
    /// </summary>
    public class VaNode
    {
        /// <summary>
        /// Gets or sets the next node
        /// </summary>
        public VaNode? Next { get; set; }

        /// <summary>
        /// Checks whether this node in a pool
        /// </summary>
        public bool InPool { get; internal set; }

        /// <summary>
        /// Returns this node to the pool.
        /// </summary>
        public virtual void ReturnToPool()
        {
            if (Pool != null)
            {
                Pool.Add(this);
            }
        }

        internal VaPool? Pool { get; set; }
    }
}
