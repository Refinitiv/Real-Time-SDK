/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Keeps an ongoing count that can be used to get both total and periodic
    /// counts. The count can also be collected by a different thread than the one
    /// counting.
    /// </summary>
    public class CountStat
    {
        private long m_CurrentValue; // The current value. 
        private long m_PrevValue;	// Value returned from the previous call to the GetChange() method.

        /// <summary>
        /// Initializes values
        /// </summary>
        public void Init()
        {
            m_CurrentValue = 0;
            m_PrevValue = 0;
        }

        /// <summary>
        /// Get the difference between the current count and that of the previous calls.
        /// </summary>
        /// <returns>The change</returns>
        public long GetChange()
        {
            long currentValue = m_CurrentValue;

            currentValue -= m_PrevValue;
            m_PrevValue = m_CurrentValue;

            return currentValue;
        }

        /// <summary>
        /// Adds to the count.
        /// </summary>
        /// <param name="addend">The value to add.</param>
        public void Add(long addend)
        {
            m_CurrentValue += addend;
        }

        /// <summary>
        /// Gets the current overall count.
        /// </summary>
        /// <returns>the total</returns>
        public long GetTotal()
        {
            return m_CurrentValue;
        }

        /// <summary>
        /// Increment the count.
        /// </summary>
        public void Increment()
        {
            ++m_CurrentValue;
        }
    }
}
