/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Common
{
    /// <summary>
    ///  Limted Value Add Pool class. This class does not accept an element when reaching its limit.
    /// </summary>
    public class VaLimitedPool : VaPool
    {
        private int m_Limit = -1;

        /// <summary>
        /// Creates a pool.
        /// </summary>
        public VaLimitedPool()
        {
        }

        /// <summary>
        /// Creates a pool with concurrent option.
        /// </summary>
        /// <param name="useConcurrent"><c>true</c> to enable concurrent mode.</param>
        public VaLimitedPool(bool useConcurrent) : base(useConcurrent)
        {
        }

        /// <summary>
        /// Creates a pool with concurrent and debug options.
        /// </summary>
        /// <param name="useConcurrent"><c>true</c> to enable concurrent mode.</param>
        /// <param name="debug"><c>true</c> to enable debug mode.</param>
        public VaLimitedPool(bool useConcurrent, bool debug) : base(useConcurrent,debug)
        {
        }

        /// <summary>
        /// Adds a node to the pool.
        /// </summary>
        /// <remarks>This method ignores the node when reaching its limit.</remarks>
        /// <param name="node">The node to add</param>
        public override void Add(VaNode node)
        {
            if (CheckLimit())
            {
                base.Add(node);
            }
        }

        /// <summary>
        /// Sets the maximum limit of this pool.
        /// </summary>
        /// <param name="limit">The maximum number of nodes</param>
        public void SetLimit(int limit)
        {
            m_Limit = limit;
        }

        private bool CheckLimit()
        {
            return m_Limit < 0 || Size() < m_Limit;
        }
    }
}
