/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.ValueAdd.Common
{
    public class VaLimitedPool : VaPool
    {
        private int m_Limit = -1;

        public VaLimitedPool()
        {
        }

        public VaLimitedPool(bool useConcurrent) : base(useConcurrent)
        {
        }

        public VaLimitedPool(bool useConcurrent, bool debug) : base(useConcurrent,debug)
        {
        }

        public override void Add(VaNode node)
        {
            if (CheckLimit())
            {
                base.Add(node);
            }
        }

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
